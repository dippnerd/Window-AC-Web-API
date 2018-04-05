# Window AC Web API
Arduino-Uno-based Window Air Conditioner Web API for Infrared-Controlled ACs

This project uses a simple Arduino circuit and web server to allow you to control your "dumb" Window Air Conditioning units remotely using a Web API. This is based on the Arduino Uno board and a compatible W5100 Ethernet module to access the web server. This could be made wireless, but won't be covered in here.

Below you will find details on the project including how to build the necessary circuitry for different uses, as well as how to retrieve your AC IR codes.

**NOTE: This does not currently include any kind of webpage or other UI for control, just the web API queries**

### Overview
Most houses I've lived have always had Window AC units. They get the job done, but one challenge with them is there is no central control like a thermostat that controls the whole house. I regularly change the temperature of my units manually depending on whether I'm home, what time of day it is, etc. While they each have an infrared (IR) remote, I'm not always lined up with the IR sensor, not to mention if I forget to change it and leave, it just remains on without me. There just had to be a better way to deal with this.

While there are devices you can buy that can send signals to these Window AC units to turn them on/off, etc, they lack any formal automation, and often rely on a proprietary app that may not play with other home automation software. What I am trying to accomplish with this is not only a way to send signals, but to add some "smarts" like the ability to track what temperature and mode the unit is set to, a web API for other software to talk to, as well as a way to retrieve this information in an easy to consume format (JSON). 

The end goal for me personally is to not only have a web-controlled AC, but to also integrate it with Apple HomeKit via the Homebridge software using a custom plugin. The idea here is I can present each room's AC as its own thermostat in HomeKit, then I can change the standard thermostat settings like Off/Auto/Cool (mapped to Off/Auto Eco/Cool on the AC) as well as change the temperature, all from one standard interface. Not only will this give me remote control of my AC even outside of my house, but it will also allow me to tie it into HomeKit automations for scenarios like when no one is home, turn off the AC, or as I approach home, turn it on Cool and set the temperature low to begin cooling the house, then after X minutes, set it to Auto at a higher temp. 

### Circuits
I built this project to offer both an easy route and a more advanced one for the circuitry. At the most basic level, you can simply attach an IR LED to your Arduino to get up and running with basic control. However, if you want more advanced control and visibility into your AC, there is a circuit and code that extends the API to monitor the LEDs on your AC unit to determine whether power is on and which mode is enabled. This is more experimental and may vary between brands and models, use at your own risk!

#### Simple
The simple circuit is a great place to start, it requires no soldering or even a great understanding of electronics. You simply need your Arduino Uno, the W5100 Ethernet board that stacks on top of it, a compatible IR LED (I used 940nm wavelength) and some wires. The compatible IR library being used requires the IR LED be plugged into digital pin 3 on the Arduino, as well as ground. I recommend using a decent length of wire so you can just tape the LED to your AC and put the Arduino somewhere out of the way. That's all there is to the circuit! I'll go into detail further down on what to modify in the code to get started.

#### Advanced
The advanced circuit isn't hard, but will require a little more understanding of electronics and require some additional components. Component-wise we're adding some photoresistors, regular resistors, and a DHT11 temperature/humidity sensor. I used 220k Ohm resistors, but your setup may vary depending on the LEDs on your AC. The idea is that we're using the photoresistors and resistors to build a circuit that will allow the digital pins to read a 0 or 1 to represent whether the LED is off(0) or on(1). You may need to tweak the resistors to get a proper value accordingly. The DHT11 simply allows us to monitor the ambient temperature and humidity of the room instead of just assuming it is the same as what the AC is set to. This isn't super important and can be removed, but may be nice to have. 
