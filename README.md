# Beebotte Arduino examples
This repository includes some Arduino Sketches to interact with Beebotte IoT
Cloud platform.

These sketches have been tested on Arduino Uno with Ethernet Shield. If you
encounter any problem using the code on different hardware, please let us
know by opening an issue.

## LED Control
This sketch shows how Beebotte can be used to control a led connected to an
Arduino board. It uses MQTT protocol to subscribe to a topic where control
messages will be sent to turn ON or OFF the LED.

This example depends on:

* [PubSubClient](http://pubsubclient.knolleary.net/): MQTT Arduino library
* [ArduinoJson](https://bblanchon.github.io/ArduinoJson/):
  An elegant and efficient JSON library for embedded systems.

## Humidity and Temperature Reporting  
This sketch shows how to connect a DHT11 humidity and temperature sensor
(or equivalent) to an Arduino board to periodically collect data. Senses data
is then sent using MQTT to Beebotte.

This example depends on:

* [PubSubClient](http://pubsubclient.knolleary.net/): MQTT Arduino library
* [ArduinoJson](https://bblanchon.github.io/ArduinoJson/):
  An elegant and efficient JSON library for embedded systems.
* [DHT](https://github.com/adafruit/DHT-sensor-library): Arduino library
  for DHT11DHT22, etc Temp & Humidity Sensors

## LICENSE
MIT
