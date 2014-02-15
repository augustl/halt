TARGET_DISK=target/virtual_disk.img

is_virtual_disk_mounted() {
    [ -n "$(mount | grep $(readlink -f $TARGET_DISK))" ] && return 0 || return 1
}

ensure_mount() {
    # @if [ -n "$(shell mount | grep "$$(readlink -f $(TARGET_DISK) ) on $$(readlink -f $(MOUNT_DIR))")" ]; then \

    if ( is_virtual_disk_mounted )
    then
        continue
    else
        echo "Virtual disk not mounted! Run script/mount."
        exit
    fi
}
