QEMU=/home/airxs/user/oscomp/rtthread/loong64/qemu-9.2.2/build/

${QEMU}qemu-system-loongarch64 -nographic -machine virt -m 256M -kernel rtthread.elf \
-drive if=none,file=sd.bin,format=raw,id=blk0 -device virtio-blk-device,drive=blk0,bus=virtio-mmio-bus.0 \
-s -S 