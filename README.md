# BCounter

Simple counter with SSD1306 display

## Usage

### Buttons

- Button 1 wakes up, increments the counter and displays the count during 30 seconds

- Button 2, if pressed for 10 seconds while display is active, resets the counter

### Screen

Displays the count on 3 digits and a countdown bar to show remaining time until power off (30 seconds).

## Circuit

### MCU

The ESP32-C3
([datasheet](https://documentation.espressif.com/esp32-c3_datasheet_en.pdf))
is very much overpowered for this application, but it's the cheapest 32-bit MCU available
on [AliExpress](https://www.aliexpress.com/item/1005009495310442.html),
though RP2040-based modules are also around the same price.

### Power

A button battery (1.5V) feeds a boost converter 
([AliExpress](https://www.aliexpress.com/item/1005005553352585.html))
that outputs 3.3V,
which feeds directly into the ESP32-C3 module.

The module has a red LED to show power input.
This LED consumes about 1mA, so it's best to remove it.

### Display

The OLED display
([AliExpress](https://www.aliexpress.com/item/1005006408257788.html))
is connected to GPIO0 to GPIO3 as follows

| OLED pin | GPIO | Note             |
|----------|------|------------------|
| GND      | 3    | 0V by software   |
| VCC      | 2    | 3.3V by software |
| SCL      | 1    |                  |
| SDA      | 0    |                  |

GPIO0 and GPIO1 are controlled by software. The MCU has enough output
to power the OLED module through the GPIO pins.

## Software

### Arduino IDE

Build was done using [Arduino IDE](https://docs.arduino.cc/software/ide/#ide-v2),
with the [Arduino ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) 
installed.

### Display

The datasheet for this particular display is available 
[here](https://www.icbanq.com/icdownload/data/ICBShop/Board/ZJY001_0.42_16P%20OLED.pdf?srsltid=AfmBOoqmlH9ZyInqXiyzBa5Ofm6whEkgQ9E289g3ANLAxYkE7rXdTKrI).
It's basically a SSD1306 driver, but care must be taken because
the starting column is at #28 (commands 0x0C and 0x11).

Font size is 8x16 with doubling,
so cell size 20x32 allows 3 digits on the 72x40 display with
8 vertical pixels and 12 horizontal pixels empty.
Bottom bar takes 60 pixels horizontally,
two pixels wide per second of timer.

The entire 60x40 active area is centered horizontally.

### Buttons

The reboot button is hard-wired to the reset pin.
The reset button is normally used for the strapping pin (GPIO9) to force
the MCU into download mode. When the MCU is already powered on, it can be read
on GPIO9 as a normal pull-up button.

### Deep sleep

The ESP32-C3's 
[deep sleep](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/low-power-mode/low-power-mode-soc.html#deep-sleep-mode)
is used to "turn off".
We don't need a wake-up trigger, since the user is expected to press the reboot button.
Note that the display is also turned off when going to sleep.

### Persistent storage

ESP-IDF has a simple API for mounting and using a SPIFFS partition, see
[documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/storage/spiffs.html).
We only use one file, containing the current count as text.
To avoid runaway counting, we reset the count to 1 when it reached 1000, which couldn't be displayed anyway.

## Box

A box has been designed to house the MCU module with the OLED module hard-soldered to it,
the boost converter and a LR44 button battery. The FreeCAD file is provided.

The two buttons end up deeply buried between the two modules,
so long columns help push them with a fingertip from outside the box.

The box must be put together using 5x M3x6 screws.
