# Raspberry Pico Pi Atarti 2600 emulator
Atari 2600 emulator for Raspberry Pi Pico and Murmulator (development) board with VGA output.

Based on Atari 2600 emulator by HiFive https://github.com/dgrubb/HiFive1-2600/tree/master/atari

# Building
To get it working you should have an Murmulator (development) board with VGA output. 

Or connect your Raspberry Pi Pico to VGA using 8 resistors:
```
GP6 --> R1K  --> VGA #3 (Blue)
GP7 --> R330 --> VGA #3 (Blue)

GP8 --> R1K  --> VGA #2 (Green)
GP9 --> R330 --> VGA #2 (Green)

GP10 --> R1K  --> VGA #1 (Red)
GP11 --> R330 --> VGA #1 (Red)

GP12 --> R100  --> VGA #13 (Horizontal Sync)
GP13 --> R100 --> VGA #14 (Vertical Sync)
GND --> VGA #5,6,7,8,10
```

Make sure you have pico sdk installed. Then clone project and necessary libraries:

```bash
mkdir ~/pico
cd ~/pico
git clone https://github.com/xrip/pico-atarti2600-murmulator.git
git clone git@github.com:fruit-bat/pico-vga-332.git
```

You have two targets to build project:
1. Local linux machine with SDL2 to test it without flashing to Pi Pico
```bash
# make sure you have libsdl2-dev installed
sudo apt install libsdl2-dev
mkdir host-build
cd host-build
cmake -DPICO_PLATFORM=host ..
make -j4
# now you're ready to run it and enjoy
./atari2600
```

2. Raspberry Pi Pico
```bash
mkdir build
cd build
cmake ..
make -j4
# upload .uf2 file to your pico and enjoy
```
