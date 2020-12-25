# Adefesio 32 bits

## Setup

    install gcc-arm-none-eabi openocd
    curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/scripts/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
    sudo udevadm control --reload-rules && sudo udevadm trigger

## Get source code

    git submodule init
    git submodule update

## Compile libopencm3

    cd libopencm3 && make






