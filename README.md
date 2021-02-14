# Puppy Alarm Clock
Do you hate waking up to your alarm clock? Well, now you can wake up to your puppy licking your face instead. 

This gadget allows you to set a wake up time, and then it will open your dogs crate at the specified time, allowing your dog to jump on your bed and wake you up. 

### Firmware
The firmware is based on the [restful server example](https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/restful_server) from the ESP-IDF. The firmware connects to the WiFi network and hosts a local website (so that any device connected to the same network can access it).

Users can connect to this local web page to set the alarm time (using post requests). The ESP gets the current time using NTP, and if the times match the ESP will turn on the motor, opening the dog create. 

### Webpage
The webpage has a live clock and selectable options to set the wake up time, which is then sent to the ESP 32 using a post request. The source code is in the `/front/web-demo/dist` folder.

### Hardware
I generated a simple 2-layer PCB using EAGLE. The files are in the `/board_files` folder. This PCB connects the [ESP-32 huzzah Dev board](https://www.adafruit.com/product/3405) to an H-Bridge that can drive the motor in either direction, and to a button that is used to reset the motor position (or any other desired function. It also has LED's to indicate motor spin direction. The board is powered using 5V from the micro-usb port on the dev board.

#### Pin Assignment:
The following GPIO's of the ESP32 Dev Board are connected

| ESP32  | Function       |
| ------ | -------        |
| GPIO27 | Motor Forwards |
| GPIO33 | Motor Backwards|
| GPIO16 | Button Input   |


### 3D Printed Housing
The STL files for the 3D printed housing are in the `/housing` folder. 
