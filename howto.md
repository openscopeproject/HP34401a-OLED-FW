# Compiling

1. Install [platformio core](http://docs.platformio.org/en/latest/installation.html)
    or one of [platformio IDEs](http://docs.platformio.org/en/latest/ide/pioide.html#pioide).
2. Install Adafruit_GFX library. Run this in root directory of this repository:
   ```shell
   pio lib install 13
   find . -type f -name 'Adafruit_SPITFT*' -delete
   ```
   Need to delete SPITFT stuff because it pulls another dependency
   we don't need.
3. Compile:
   ```shell
   pio run
   ```

Artifacts including `firmware.bin` will be generated in `.pioenvs/genericSTM32F103C8`.

# Flashing

First you need to flash the
[stm32duino bootloader](https://github.com/rogerclarkmelbourne/STM32duino-bootloader)
using serial. Grab  [the binary](https://github.com/rogerclarkmelbourne/STM32duino-bootloader/raw/master/bootloader_only_binaries/generic_boot20_pc13.bin) and [flash it](https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/stm32duino-bootloader#installing-the-stm32duino-bootloader) (you only have to do this once).

After this you can flash the board using USB:
```shell
pio run -t upload
```

## But I don't want to install platformio or stm flash loader

In that case just install the bootloader using any tool you like to flash stm32
devices that don't support DFU protocol and then upload precompiled
`firmware.bin` directly using [maple upload tool](https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/master/tools/win):

```shell
maple_upload COM4 2 1EAF:0003 path/to/firmware.bin
```

Note, this tool needs to be able to find
[dfu_util](https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/master/tools/win/dfu-util-0.9-win64)
in $PATH.
