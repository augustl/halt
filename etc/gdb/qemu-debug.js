"use strict";
var child_process = require("child_process");
var BOOT_LOADER_ADDR = 0x3EB97000;

function numToHex(num) {
    return "0x" + num.toString(16).toUpperCase();
}

function isBufferEquals(a, b) {
    if (!Buffer.isBuffer(a)) { return undefined; }
    if (!Buffer.isBuffer(b)) { return undefined; }
    if (a.length !== b.length) { return false; }

    for (var i = 0; i < a.length; i++) {
        if (a[i] !== b[i]) {
            return false;
        }
    }

    return true;
};

function createGdbQueue(gdbProcess) {
    var GDB_LINE = new Buffer([0x28, 0x67, 0x64, 0x62, 0x29, 0x20])
    var currentItem = null;
    var queue = [];

    function processQueue() {
        if (currentItem !== null) { return; }
        if (queue.length === 0) { return ; }

        currentItem = queue.shift();
        var command = currentItem.command + "\n";
        gdbProcess.stdin.write(command);
        process.stdout.write(command);
    }

    function dataListener(chunk) {
        if (isBufferEquals(chunk.slice(chunk.length - GDB_LINE.length), GDB_LINE)) {
            if (currentItem && currentItem.onFinished) {
                currentItem.onFinished();
            }

            currentItem = null;
            process.nextTick(processQueue);
        }
    }

    gdbProcess.stdout.on("data", dataListener);

    return {
        push: function (command, onFinished) {
            queue.push({command: command, onFinished: onFinished});
            processQueue();
        },

        kill: function () {
            queue.length = 0;
            currentItem = null;
            gdbProcess.stdout.removeListener("data", dataListener);
        }
    }
}

function getOffset(lines, re) {
    return parseInt(lines.filter(function (l) { return re.test(l) })[0].trim().split(" ")[0], 16);
}

child_process.exec("gdb -nx --batch -ex 'info files' uefi_bootloader/uefi_bootloader.efi", {cwd: process.cwd()}, function (err, stdout, stderr) {
    var lines = stdout.split("\n");
    var debugSymbolsCommand =  "add-symbol-file uefi_bootloader/uefi_bootloader-debug.efi "
        + numToHex(BOOT_LOADER_ADDR + getOffset(lines, /\.text/))
        + " -s .data "
        + numToHex(BOOT_LOADER_ADDR + getOffset(lines, /\.data/))

    var bootstrapGdb = child_process.spawn("gdb", [], {});
    var bootstrapGdbQueue = createGdbQueue(bootstrapGdb);

    bootstrapGdbQueue.push("file uefi_bootloader/uefi_bootloader.efi")
    bootstrapGdbQueue.push(debugSymbolsCommand);
    bootstrapGdbQueue.push("target remote :1234");
    bootstrapGdbQueue.push("set architecture i386:x86-64:intel");
    bootstrapGdbQueue.push("break efi_main");
    bootstrapGdbQueue.push("continue", function () {
        var gdb = child_process.spawn("gdb", [], {});
        gdb.stdout.pipe(process.stdout);
        gdb.stderr.pipe(process.stderr);

        var gdbQueue = createGdbQueue(gdb);
        gdbQueue.push("file uefi_bootloader/uefi_bootloader.efi")
        gdbQueue.push(debugSymbolsCommand);
        gdbQueue.push("set architecture i386:x86-64:intel", function () {
            gdbQueue.push("target remote :1234");
            bootstrapGdbQueue.kill();
            bootstrapGdb.kill();
            process.stdin.pipe(gdb.stdin);
        });
    });
})
