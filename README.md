# attiny85logbook
Geocaching digital logbook with tiny OLED screen.
Operated with two push buttons

## Hardware

### parts

1. Attiny85 microcontroller
2. SSD1306 I2C 128x32 OLED screen
3. 24C512 I2C EEPROM
4. Two pushbuttons (three if you add hard reset button)
5. programmer - for example USBAsp

### wiring

![Wiring diagram](https://github.com/panjanek/attiny85logbook/blob/1ffeb83ed07bc96e832b7d160870b4c904d53575/img/schematic.png "wiring diagram")

PCB pending...

## Software

### dependencies
1. Arduino IDE 2.3+
2. Tiny4kOLED library: https://github.com/datacute/Tiny4kOLED
3. TinyWireM library https://github.com/adafruit/TinyWireM
4. ATTinyCore board https://github.com/SpenceKonde/ATTinyCore

### customize to your geocache
1. Select language in `#define LANGUAGE LANG_EN` supported `LANG_EN` and `LANG_PL`. Feel free to make pull request with more languages.
2. Set cache name and author in `WELCOME_LINE1` and `WELCOME_LINE2`
3. If using smaller EEPROM set limit for logbook size in `LOGBOOK_MAX` and `LOGBOOK_MAX_STR` 

### steps to upload code to attiny
In Arduino IDE:
1. Install Spence Konde ATTinyCore board - may require adding  Board Manager URLs in File->PReferences. Set Additional Board Managers URLs to "http://drazzy.com/package_drazzy.com_index.json,https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json"
2. Set Tools -> Board -> ATTinyCore -> ATTiny85/45/25 (no bootloader)
3. Set Tools -> Chip -> ATtiny85
4. Set Tools -> Clocksource + Speed -> 1MHz internal
5. Set Tools -> millis()/micros<> -> Disabled
6. Set Tools -> Builtin Serial -> No receiving, transmit only
7. Compile the code
8. Run Tools -> Burn bootloader
9. Tun Sketch -> Upload using programmer

   
