# Geocaching Gadget: attiny85logbook
If you are new to geocaching go to https://www.geocaching.com/ .
Each geocaching hide contains small paper logbook where people log their visit using pencil. 
Additionally, geocache can contain some fun objects.
This project is a geocaching gadget: Digital logbook with tiny OLED screen, operated with two push buttons and simple UI.
Geocachers can enter their nick to the digital logbook or browse the list of saved nicks.

## Features

* Allows user to input logbook entry (typically nick), max 21 characters using simple UI with two buttons: "select" and "enter"
* Logbook entries saved in permanent EEPROM memory
* Browsing list of saved entries
* Auto-sleep after ~30s of inacvtivity. Time to shutdown displayed as shrinking bar on the right
* Wakes on button press (hard reset button is optional in case of freeze)
* Ultra low current in sleep mode - only 0.2uA
* Around 10-20mA when active
* Can be powered from two AA or AAA batteries or CR123 battery (2.8V - 5V) for years of typical usage

## Hardware

### parts

1. Attiny85 microcontroller
2. SSD1306 I2C 128x32 OLED screen
3. 24C512 I2C EEPROM
4. Two pushbuttons (three if you add hard reset button) like this https://www.ebay.co.uk/itm/256536546595
5. programmer - for example USBAsp

### wiring

![Wiring diagram](https://github.com/panjanek/attiny85logbook/blob/1ffeb83ed07bc96e832b7d160870b4c904d53575/img/schematic.png "wiring diagram")

Gerber files pending...

![PCB layout](https://github.com/panjanek/attiny85logbook/blob/56c55d54027ca20edaae7995184f9218e9bcb9b6/img/logbook1-pcb.png "PCB layout")
![PCB layout](https://github.com/panjanek/attiny85logbook/blob/56c55d54027ca20edaae7995184f9218e9bcb9b6/img/logbook1-3d.png "PCB layout")

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
1. Install Spence Konde ATTinyCore board - may require adding  Board Manager URLs in File->Preferences. Add Additional Board Managers URL "http://drazzy.com/package_drazzy.com_index.json"
2. Set: Tools -> Board -> ATTinyCore -> ATTiny85/45/25 (no bootloader)
3. Set: Tools -> Chip -> ATtiny85
4. Set: Tools -> Clocksource + Speed -> 1MHz internal
5. Set: Tools -> millis()/micros<> -> Disabled
6. Set: Tools -> Builtin Serial -> No receiving, transmit only
7. Compile the code
8. Set: Tools -> Programmer -> your progrmmer, USBAsp recommended
9. Run: Tools -> Burn bootloader
10. Run: Sketch -> Upload using programmer

## prototyping

![UI](https://github.com/panjanek/attiny85logbook/blob/43c535eb7904cd81590bfdfc12cc7b34f31d3815/img/animation.gif "ui")

![Prototype](https://github.com/panjanek/attiny85logbook/blob/43c535eb7904cd81590bfdfc12cc7b34f31d3815/img/logbook1-proto-s.png "prototype")
![Prototype](https://github.com/panjanek/attiny85logbook/blob/43c535eb7904cd81590bfdfc12cc7b34f31d3815/img/logbook1-menu.png "prototype")
![Prototype](https://github.com/panjanek/attiny85logbook/blob/43c535eb7904cd81590bfdfc12cc7b34f31d3815/img/logbook1-input.png "prototype")
![Prototype](https://github.com/panjanek/attiny85logbook/blob/43c535eb7904cd81590bfdfc12cc7b34f31d3815/img/logbook1-browsing.png "prototype")
![case 1](https://github.com/panjanek/attiny85logbook/blob/ca09125d848bf7212f28f501e1ca2bf573f435aa/img/logbook1-case1-s.png "case")
![case 2](https://github.com/panjanek/attiny85logbook/blob/ca09125d848bf7212f28f501e1ca2bf573f435aa/img/logbook1-case2-s.png "case")




   
