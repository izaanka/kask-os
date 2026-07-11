# Kask OS — Raspberry Pi Boot Guide

## Supported Boards
- Raspberry Pi Zero 2W (BCM2710A1 / Cortex-A53)
- Raspberry Pi 4 Model B (BCM2711 / Cortex-A72)

## What You Need

### RPi Firmware Files (download once)
```
wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat
```
For Pi 4 also add:
```
wget https://github.com/raspberrypi/firmware/raw/master/boot/start4.elf
wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup4.dat
```

## Build

```bash
cd kask-os
make kernel8.img        # builds ARM64 kernel and creates kernel8.img
```

## SD Card Setup

1. Format a microSD card as **FAT32** (single partition)
2. Copy to the SD card root:
   ```
   bootcode.bin
   start.elf   (+ start4.elf for Pi 4)
   fixup.dat   (+ fixup4.dat for Pi 4)
   kernel8.img          ← from make kernel8.img
   rpi/config.txt       ← rename to config.txt on the card
   ```

## Serial Connection (required — no HDMI output)

Kask OS outputs to **UART serial** at **115200 baud** on:
- **GPIO 14** = TXD (connect to RX on USB-serial adapter)
- **GPIO 15** = RXD (connect to TX on USB-serial adapter)
- **GND** (any GND pin)

Connect with:
```bash
screen /dev/ttyUSB0 115200
# or
minicom -D /dev/ttyUSB0 -b 115200
```

## QEMU Test (no hardware needed)

```bash
make run-arm          # emulates Pi 3 / Zero 2W (BCM2837)
make run-arm-pi4      # emulates Pi 4 (BCM2711)
```
