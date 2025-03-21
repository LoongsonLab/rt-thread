QEMU=/home/airxs/user/oscomp/rtthread/loong64/qemu-9.2.2/build/

${QEMU}qemu-system-loongarch64 -nographic -machine virt,dumpdtb=loongarch64_virt.dtb -m 256M -kernel rtthread.bin

dtc loongarch64_virt.dtb -o loongarch64_virt.dts -O dts
