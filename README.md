# BCounter

Simple counter with SSD1306 display

# Usage

## Buttons

- Button 1 wakes up, increments the counter and displays the count during 1 minute

- Button 2, if pressed while display is active, resets the counter

## Screen

Displays the count on 3 digits and a countdown bar to show remaining time until power off

Font size is 8x16 with doubling,
so cell size 20x32 allows 3 digits on the 72x40 display with
8 vertical pixels and 12 horizontal pixels empty.
Bottom bar takes 60 pixels horizontally,
one pixel per second of timer.

The entire 60x40 active area is centered.

# Circuit

