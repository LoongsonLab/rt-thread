# cp /home/tikifire/Temp/ahci_driver/target/loongarch64-unknown-linux-musl/debug/libls2k_driver.a ./drivers/
cp /home/tikifire/Temp/ahci_driver/target/loongarch64-unknown-linux-musl/release/libls2k_driver.a ./drivers/
scons -j20
sudo cp ./uImage /srv/tftp/
