# HP 34401a 6½ digit multimeter OLED display mod

## Status [![Build Status](https://travis-ci.com/openscopeproject/HP34401a-OLED-FW.svg?branch=master)](https://travis-ci.com/openscopeproject/HP34401a-OLED-FW)

## About this project

Many of the old meters in 34401a series are still in great working condition,
however their aging VFD displays are failing in various ways. Low contrast,
leaking current in segment drivers, shattered glass are frequent cause of
these excellent devices being written off. While replacement VFDs are
still sold on some markets, they are not cheap and getting them intact in mail
is a lottery. Also replacing the VFD itself does not always resolve the issue
since in some cases it's the driver IC that is at fault. On older issue 34401a
meters (fw version 06-04-01 and prior) have a custom HP designed micro controller
with integrated high voltage VFD driver which is not obtainable, unless bought
as a whole front panel assembly, which again is not cheap.

After acquiring one of such devices off ebay and encountering the leaking driver
issue which lead to artifacts on the screen, a decision was made to rectify
this situation. This project is result of many evenings of carefully reverse
engineering the protocol between the CPU of the meter and it's front panel
controller IC and designing a sniffing device that would extract the data sent
to the front panel and display it on a more modern display instead.

After the protocol was reverse engineered a simple sniffing device based on
everyone's favorite stm32 f1 series micro was designed. First prototype used
a bluepill board and an Arduino compatible 3.5" TFT screen.

<img src="https://i.imgur.com/AId0xw1.jpg" alt="prototype" width="300px"
href="https://i.imgur.com/AId0xw1.jpg"/>

After the prototype showed promise it was quickly upgraded to a screen that
would work well inside the meter and a custom board was designed. In the
process a useful fast bargraph feature was added.

<img src="https://i.imgur.com/bZpQViy.jpg" alt="final design" width="300px"
href="https://i.imgur.com/bZpQViy.jpg"/>

And this is what final product looks like:

<img src="https://i.imgur.com/FP5pQ6R.jpg" alt="final product" width="300px"
href="https://i.imgur.com/FP5pQ6R.jpg"/>


### Links

For protocol details see `protocol.md`

For compilation and flashing instructions see `howto.md`

[KiCad files](https://github.com/openscopeproject/HP34401a-OLED-HW)

[Bill of materials](https://openscopeproject.org/InteractiveHtmlBomDemo/hp34401a_oled/ibom.html) (see my [plugin for KiCad](https://github.com/openscopeproject/InteractiveHtmlBom) if you want to generate a BOM like this for your project)

[More pictures and history of progress of this project on EEVBlog](https://www.eevblog.com/forum/repair/hp-34401a-dmm-with-leaking-segments/).

Similar (unfinished) project was done by user
[douarda on EEVBlog](https://www.eevblog.com/forum/repair/reverse-ingeeniring-hp-34xxx-display-panel-serial-protocol/) for
[HP34970A data acquisition unit](https://whatever.sdfa3.org/hp-34970a-data-acquisition-unit-communication-protocol.html)

# License and credits

Firmware source is distributed under MIT license. See LICENSE for more info.

Firmware is built with [platformio](https://platformio.org/) using
[stm32duino framework](https://github.com/rogerclarkmelbourne/Arduino_STM32)
and flashed via [maple bootloader modified for stm32duino](https://github.com/rogerclarkmelbourne/STM32duino-bootloader).

Display library (lib/SSD1322_Display) is based on
[MCUFRIEND_kbv library](https://github.com/prenticedavid/MCUFRIEND_kbv)
which builds on
[Adafruit_GFX library](https://github.com/adafruit/Adafruit-GFX-Library).
