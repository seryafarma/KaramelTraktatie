# KaramelTraktatie
Small low cost desk clock with Arduino to display time and temperature.

## Introduction
This digital clock project is developed using TinyRTC module with added one wire temperature sensor.
It is kind of complex however by using ready made modules bought in Ebay and also software libraries from other people, things get easier.

*Note: This hobby project was developed a while ago in 2015 when the summer was very hot, while biking home I really wanted to know what the temperature in my room is. The code has been quite stable since then so not much updates.*

## Libraries needed:
### LedControl by Wayoda:
* http://playground.arduino.cc/Main/LedControl
* https://github.com/wayoda/LedControl

### OneWire
OneWire, maintained by Paul Stoffregen (website)
* http://playground.arduino.cc/Learning/OneWire

### DS1307
Updated by bricofoy from Arduino Forums
* http://forum.arduino.cc/index.php/topic,93077.0.html

## How to setup the time (via Serial Port)
### Easy way
Use the `time_updater.py` script (don't forget to install pyserial first in your laptop), 
### Manual way
You can send manually serial bytes like what is shown in the code. Make sure it is 19 bytes.
```cpp
// SET_YYMMDDdd_hhmmss
// SET_16030201_123001 (19 bytes)
```
