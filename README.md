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
At the most basic level, you can simply enter your wifi SSID/Password to get running, but more than likely you're not using th exact same model of AC unit as me (Haier HWR05XCM-L) and need to at least update your IR codes, potentially other settings too. Below we'll cover what things (may) need changed:
```
const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_WIFI_PASSWORD_HERE";
```
First and foremost you'll need to change these to reflect the SSID (name) of your Wi-Fi network, as well as the password for it.


The IR codes look something like:
```
const unsigned long IR_POWER = 0x19F69867;
const unsigned long IR_MODE = 0x19F610EF;
const unsigned long IR_UP = 0x19F6A05F;
const unsigned long IR_DOWN = 0x19F6906F;
const unsigned long IR_SPEED = 0x19F620DF;
```
I'll update this later to pertain to this exact setup, but for now, the easiest way to get your IR codes is following the [steps outlined here](https://techcontostones.blogspot.com/2013/09/controlling-air-conditioner-with-your.html). Once you have your AC's IR codes, update them in the code. 

This may be all you need to get running, but there are some other variables to consider in case your AC is wildly different from mine:
```
const int minTemp = 61;
const int maxTemp = 86;
```
These define the minimum and maximum temperatures your AC can reach. Mine stops at 61 and 86. These are important, as it calculates these values to properly calibrate at start-up.

```
int acTemp = 72;
String currentMode = "auto";
```
This is the default temperature and mode to start the AC. Mode must be manually calibrated, meaning the value here must match what the AC is set to when first turned on, it has no way of tracking this, so it is important for it to know which setting it is starting on.

```
const int toCool = 3;
const int toAuto = 1;
const int fanSpeed = 1;
```
These are the number of "steps" to get to each mode. From Auto to Cool, mine requires 3 clicks, but only 1 from Cool to Auto. The Fan Speed on my unit gets changed from Auto Cool to Low when it changes to modes like Dehum (which it has to step over to get to Cool), so I have code in place to account for where this happens. This variable just accounts for how many clicks from Low to Auto Cool are required.

```
#define IR_LED 4 
```
As the comment on this line describes, this is needed to set the GPIO pin for the IR LED. Ideally you should just leave this alone and use pin 4 (D2) as described.

### Web API
So you've got your circuit setup, updated the code, but now what? This has no real UI or anything, it's just an API to be used for other projects. First you'll need to get the IP address. If you set everything up in the Arduino IDE, once this gets started you should see some output like this:
```
WiFi connected
Server started
10.10.10.10
```
The "10.10.10.10" should be different for yours, that is the IP address for the server. Simply typing that in won't result in anything useful, but you can use a browser or the "curl" command from a command line to access the API in the following ways:
```
/status
```
This takes no action, but returns JSON data about the current status of the unit, as such:
```
{
  "Status": {
    "Temperature": 72,
    "Power": "1",
    "Mode": "auto"
  }
}
```
This tells us that the AC is set to 72F, the power is On (1=On, 0=Off) and the mode is set to "auto". 

```/temp/68``` This tells it to set the temperature to 68F. 

```/power``` This toggles the power on/off (remember, it can only assume state based on what it already knows)

```/mode/cool``` This changes the mode. It supports "cool" and "auto" as modes, which toggles between "Cool" and "Energy Saver" on my unit

Each of these will take action and then return the JSON status to confirm the changes. 
