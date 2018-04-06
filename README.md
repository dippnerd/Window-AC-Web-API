# Window AC Web API
ESP8266/NodeMCU-based Window Air Conditioner Web API for Infrared-Controlled ACs

This project uses a simple circuit and web server to allow you to control your "dumb" Window Air Conditioning units remotely using a Web API. This is based on the NodeMCU/ESP8266 ESP-12E and is pretty quick/easy to get setup. 

**NOTE: This does not currently include any kind of webpage or other UI for control, just the web API queries**

### Overview
Most houses I've lived have always had Window AC units. They get the job done, but one challenge with them is there is no central control like a thermostat that controls the whole house. I regularly change the temperature of my units manually depending on whether I'm home, what time of day it is, etc. While they each have an infrared (IR) remote, I'm not always lined up with the IR sensor, not to mention if I forget to change it and leave, it just remains on without me. There just had to be a better way to deal with this.

While there are devices you can buy that can send signals to these Window AC units to turn them on/off, etc, they lack any formal automation, and often rely on a proprietary app that may not play with other home automation software. What I am trying to accomplish with this is not only a way to send signals, but to add some "smarts" like the ability to track what temperature and mode the unit is set to, a web API for other software to talk to, as well as a way to retrieve this information in an easy to consume format (JSON). 

The end goal for me personally is to not only have a web-controlled AC, but to also integrate it with Apple HomeKit via the Homebridge software using a custom plugin. The idea here is I can present each room's AC as its own thermostat in HomeKit, then I can change the standard thermostat settings like Off/Auto/Cool (mapped to Off/Auto Eco/Cool on the AC) as well as change the temperature, all from one standard interface. Not only will this give me remote control of my AC even outside of my house, but it will also allow me to tie it into HomeKit automations for scenarios like when no one is home, turn off the AC, or as I approach home, turn it on Cool and set the temperature low to begin cooling the house, then after X minutes, set it to Auto at a higher temp. 

### Circuit
While I am working on some advanced circuit ideas, this is a good place to start. It requires no soldering or even a great understanding of electronics, you just need the NodeMCU, a compatible IR LED (I used 940nm wavelength) and some dupont wires. Just plug the positive leg of the LED to a GPIO pin and the other leg to ground. I recommend using a decent length of wire so you can just tape the LED to your AC and put the NodeMCU somewhere out of the way. That's all there is to the circuit! The next section will cover what changes are necessary in the code.

### Code
At the most basic level, you can simply enter your wifi SSID/Password to get running, but more than likely you're not using th exact same model of AC unit as me (Haier HWR05XCM-L) and need to at least update your IR codes, potentially other settings too. We'll cover it all in this section, including how to retrieve your IR codes.
