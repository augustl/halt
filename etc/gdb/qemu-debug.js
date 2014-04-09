"use strict";
var child_process = require("child_process");
var BOOT_LOADER_ADDR = 0x3EB97000;

/**
 * Utility script for connecting a GDB to our uefi boot loader running in
 * qemu.
 *
 * GDB is not fond of the CPU architecture changing. But qemu starts in 16
 * bit and by the time the uefi bootloader runs, we're in 64 bit. So we'll
 * get ugly errors instead of nice data output on break points and what not.
 *
 * This script first sets up a break point at efi_main, the boot loader
 * entry point. When this break point hits, we get bogus data, but it did
 * break properly. Then we connect a second gdb and set it to 64 bit,
 * disconnect the first gdb, and voila. The second gdb works fine and we're
 * still at the efi_main break point. We can now set additional break points
 * and type 'continue' when we're ready, and get nice proper GDB in our
 * boot loader.
 */

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
