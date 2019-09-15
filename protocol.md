# HP 34401a Front Panel Protocol Details

_Note: there are 2 versions of front panels in 34401a meters, original one
(MPR 34401-66502, fw version up to 06-04-01) and an updated one with slight modifications.
Protocols for both are very similar except for headers of some frames. Initial investigation
into this matter was performed using a meter with original front panel. Differences between
old and new panels are only the headers of text and annunciator frames, see details below._

### SPI but with a twist

34401a CPU uses a 3 wire serial protocol to facilitate bidirectional
communication with front panel microcontroller (referred to as FP from here on).
Schematics available in service manual revealed these lines to be labeled as
SCK, SO, SI, which immediately suggests SPI. Quick look at the data with a logic analyzer confirms this idea:

![sigrok capture](https://i.imgur.com/6gG5ygZ.png)

```shell
$ sigrok-cli -i capture.sr -P "spi:mosi=SI:clk=SCK:miso=SO" -A spi=mosi-data | xxd -r -p
  000027 mVDC       ☺    000027 mVDC       ☺    000026 mVDC       ☺
```

Yep, that is definitely SPI with clock idle high and trailing edge data capture,
bytes sent MSB first.

However a curious observation can be made: the slave pulls it's output low
before the master starts the clock, which is not a part of standard SPI
protocol. This can be explained by the fact that FP also uses the same output
line to do the scanning of the input buttons state. Whenever CPU has data to
send it first has to notify FP by pulling INT line (not pictured above) low,
at which point FP stops polling buttons and pulls SO low to signify that it's
ready to receive next byte. Similarly if there is a button press, FP notifies
CPU to start the clock by pulling SO low.

## Digging into data

All communication is wrapped in frames that have a header, body and a footer
(ack byte).
There are 3 types of frames sent from CPU to FP:
* Text data
* Annunciator state
* Control frame

And 1 type that is initiated by FP:
* Button press

### Text data

Text data frame is just a zero terminated string that will be displayed on the
text portion of the display. It's format is as follows:

| Line | Header    | Data             | Ack  |
|------|-----------|------------------|------|
| SI   | 0x00 0x7f | S0...SN 0x00     | 0x00 |
| SO   | 0x## 0x## | 0x##...0x## 0x## | 0xbb |

For meters with newer front panels the header is slightly different: `0x00 0xff`.

_Note: here and below `0x##` is used to indicate that byte doesn't matter.
However front panel seems to always respond with `0xdd` in such cases._

The `S0..SN` bytes are ASCII data that will be displayed on main
17 segment part of the display. If the string is shorter than 12 bytes, the
remaining characters on the display are cleared. Punctuation marks don't count
against string length and have non-standard codes:

- `0x84` for dot (`.`)
- `0x86` for comma (`,`)
- `0x8c` for semicolon (`:`)
- `0x8d` for special semicolon that is used in menus and it's only purpose
  seems to be triggering first character of the string to blink.
- `0x81` unknown control character (has no apparent effect)

Data portion is always terminated by `0x00` byte, after which CPU expects an
ACK from FP in the form of `0xbb` byte.

### Annunciator state

All annunciators on the display are updated any time any of them toggle. The
whole state is packed into a 2 byte bitmask.

Frame format:

| Line | Header    | Data      | Ack  |
|------|-----------|-----------|------|
| SI   | 0x7f 0x00 | B0   B1   | 0x00 |
| SO   | 0x## 0x## | 0x## 0x## | 0xbb |

For meters with newer front panels the header is slightly different: `0xff 0x00`.

Bitmask:

| Bits     | 7     | 6   | 5    | 4     | 3     | 2    | 1     | 0    |
|----------|-------|-----|------|-------|-------|------|-------|------|
| 1st byte | Ratio | Mem | Hold | Trig  | Man   | Rmt  | Adrs  | *    |
| 2nd byte |       | 4W  | Buz  | Diode | Shift | Rear | ERROR | Math |

The `Shift` annunciator is an oddball. It is not sent from CPU to FP, instead FP stores it's state internally and toggles it when user presses shift button or
cancels it with another button. The shift button press event is still sent
to CPU though (see button codes table below).

### Control frame

Control frames are used mainly for menus. They all have the same simple format
with no data:

| Line | Header    | Ack  |
|------|-----------|------|
| SI   |   B0 B1   | 0x00 |
| SO   | 0x## 0x## | 0xbb |

Here is a list of control frames with their deduced purpose:

| B0   | B1   | Purpose                                                   |
|------|------|-----------------------------------------------------------|
| 0x20 | 0x00 | Power on, or clear display                                |
| 0x71 | 0x2b | Enter menus (no visible change on display)                |
| 0x00 | 0x54 | Low brightness (toggles for a moment on menu transition)  |
| 0x62 | 0x54 | Normal brightness                                         |
| 0x00 | 0x2b | Exit menus (no visible change on display)                 |
| 0x00 | 0x49 | Blink 1st character                                       |
| 0x71 | 0x49 | Blink 2nd character                                       |
| 0x62 | 0x49 | Blink 3rd character                                       |
| 0x13 | 0x49 | Blink 4th character                                       |
| 0x54 | 0x49 | Blink 5th character                                       |
| 0x25 | 0x49 | Blink 6th character                                       |
| 0x36 | 0x49 | Blink 7th character                                       |
| 0x47 | 0x49 | Blink 8th character                                       |
| 0x38 | 0x49 | Blink 9th character                                       |
| 0x49 | 0x49 | Blink 10th character                                      |
| 0x5a | 0x49 | Blink 11th character                                      |
| 0x2b | 0x49 | Blink 12th character                                      |
| 0x1d | 0x00 | Unknown (sometimes spotted after menu exit)               |
| 0x80 | 0x00 | Unknown (possibly artifact of desynchronization)          |

Blink control frame is sent once for each character. FP then continuously
toggles the brightness of corresponding character until a new text data
frame is received.

### Button press

Unlike other frames header is only one byte:

| Line | Header | Data      | Ack  |
|------|--------|-----------|------|
| SI   | 0x00   | 0x## 0x## | 0x66 |
| SO   | 0x77   | B0 B1     | 0x## |

_Note: CPU seems to send 0x00 bytes during data transfer from FP, although it
likely does not matter._

Button codes are different depending on shift state. Here is full code table:

| Button  |   Alone   | With shift |
|---------|-----------|------------|
| DC V    | 0x5b 0x5b | 0x5b 0xe9  |
| AC V    | 0x9d 0x5b | 0x9d 0xe9  |
| Ohm     | 0xcf 0x5b | 0xcf 0xe9  |
| Freq    | 0xe9 0x5b | 0xe9 0xe9  |
| Cont    | 0x77 0x5b | 0x77 0xe9  |
| Null    | 0x7d 0x5b | 0x7d 0xe9  |
| MinMax  | 0x5b 0x9d | 0x5b 0xbb  |
| Left    | 0x9d 0x9d | 0x9d 0xbb  |
| Right   | 0xcf 0x9d | 0xcf 0xbb  |
| Down    | 0xe9 0x9d | 0xe9 0xbb  |
| Up      | 0xbb 0x9d | 0xbb 0xbb  |
| AutoMan | 0x7d 0x9d | 0x7d 0xbb  |
| Single  | 0x5b 0xcf | 0x5b 0x7d  |
| Shift   | 0x9d 0xcf | 0x9d 0xcf  |
