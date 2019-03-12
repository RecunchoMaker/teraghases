
# Maple-mini

## Links

- [Maple Mini at STM32duino](http://wiki.stm32duino.com/index.php?title=Maple_Mini)
- [STM32duino bootloader](https://github.com/rogerclarkmelbourne/STM32duino-bootloader)

## Install USB-DFU bootloader

    wget https://github.com/rogerclarkmelbourne/STM32duino-bootloader/raw/master/bootloader_only_binaries/maple_mini_boot20.bin

    openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg \
        -c "init ; program maple_mini_boot20.bin 0x8000000; reset ; exit"

    lsusb | grep 1eaf
    #  Bus 002 Device 074: ID 1eaf:0003  

## Firmware uploader

    sudo apt install dfu-util

    # /etc/udev/rules.d/45-maple.rules : 
    ATTRS{idVendor}=="1eaf", ATTRS{idProduct}=="0003", ENV{ID_MM_DEVICE_IGNORE}="1"
    ATTRS{idVendor}=="1eaf", ATTRS{idProduct}=="0003", ENV{MTP_NO_PROBE}="1"
    SUBSYSTEMS=="usb", ATTRS{idVendor}=="1eaf", ATTRS{idProduct}=="0003", MODE:="0666"
    KERNEL=="ttyACM*", ATTRS{idVendor}=="1eaf", ATTRS{idProduct}=="0003", MODE:="0666"

    dfu-util --reset --alt 2 -D bin/stm32/leaflabs-maple.hex

## Platformio environment

    [env:maple_mini_b20]
    platform = ststm32
    board = maple_mini_b20
    framework = arduino
    board_build.core = maple
    upload_protocol = dfu
    upload_port = "Maple DFU"


