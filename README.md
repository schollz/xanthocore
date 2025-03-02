# daisy-softcut

## dev 

### using web uploader

1. Press reset, then quickly press the `BOOT` button.
2. Go to https://electro-smith.github.io/Programmer/
3. Click "Connect" to "Daisy Bootloader"
4. Choose "xanthocore.bin"
5. Click "Program"

###  upload

Press the `RESET` button on the Daisy Seed and then run the following:

```bash
make upload
```

### debug

```bash
make debug
```

## prequisites

Install libusb

```
sudo apt install -y libusb-1.0-0-dev screen
```

Build openocd

```bash
git clone https://git.code.sf.net/p/openocd/code openocd-code
cd openocd-code/
./bootstrap 
./configure --enable-stlink
make -j8
sudo make install
```

Add udev rules ([info](https://forum.electro-smith.com/t/st-link-and-cortex-debugger-on-ubuntu-24-04/5260))

```bash
cd dev/udev-rules
sudo cp *.* /etc/udev/rules.d/
sudo systemctl reload udev
sudo udevadm trigger
````

Install libncurses5-dev and lib32ncurses5-dev

```bash
wget http://archive.ubuntu.com/ubuntu/pool/universe/n/ncurses/libtinfo5_6.4-2_amd64.deb && sudo dpkg -i libtinfo5_6.4-2_amd64.deb && rm -f libtinfo5_6.4-2_amd64.deb
wget http://archive.ubuntu.com/ubuntu/pool/universe/n/ncurses/libncurses5_6.4-2_amd64.deb && sudo dpkg -i libncurses5_6.4-2_amd64.deb && rm -f libncurses5_6.4-2_amd64.deb
sudo apt install lib32ncurses5-dev libncurses5 libncurses5-dev -y 
```

Install `arm-none-eabi-gcc` (correct version):

```bash
cd $HOME/bin
wget https://github.com/schollz/daisy1/releases/download/prereqs/gcc-arm-none-eabi-10-2020-q4-major.zip
unzip gcc-arm-none-eabi-10-2020-q4-major.zip
rm gcc-arm-none-eabi-10-2020-q4-major.zip
```


# License

GNU General Public License v3.0

softcut is a library created by [monome](https://github.com/monome/softcut-lib/blob/main/readme.md) which is licensed under GNU General Public License v3.0.

