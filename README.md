# Create Raspberry PI-based IOT devices that can directly connect to Samsung SmartThings

This repository holds everything needed to set up a Raspberry Pi to act as a SmartThings direct connected device.

What is a direct connected device?
----------------------------------
The SmartThings platform is evolving. In the not-too-distant future there will be three methods for IOT devices to integrate with SmartThings: 1) zwave and zigbee devices that connect directly to the SmartThings hub, 2) cloud-connected devices, and 3) direct-connected devices.  Former devices that were developed in the SmartThings IDE using device handlers with Groovy code will need to be migrated to one of the those three options.

For an individual wanting to develop their IOT applications to run locally on their own LAN, the direct-connection method is really the only choice.  Most would not want the responsibility and security concerns of opening up their home LAN to the internet, which would come with implementing their own cloud server. Using AWS Lambda is an alternative, but again that's not a local solution.  And implementing your own zigbee or zwave device is probably outside the scope of most people as well.  So that leaves direct-connected devices as the best option for those wanting to keep things local, free, and under your control.

SmartThings's concept for "direct-connected" devices are wifi-enabled microcontroller-based IOT devices.  In fact the SDK that was developed in support of this very much revolves around MCUs like the popular ESP32 system-on-a-chip microcontroller with integrated wifi.  However with modifications, this can be implemented on a Raspberry Pi as well.  What this enables is an ideal configuration where the sky is the limit as far as application code that you can write and run and have fully integrated with SmartThings.  No cloud server to manage, no IDE, no Groovy code - just a simple API to send and receive commands and attributes for whatever kind of IOT device application you can dream up and implement on a Raspberry Pi.  


Pre-requisites
--------------
## Hardware
- Raspberry Pi 3 or later running Raspberry Pi O/S 
	- assumed to include the standard capabilities including working wireless device with AP capability
	

## Accounts
- Samsung SmartThings developer account for developer workspace: https://smartthings.developer.samsung.com/workspace/	
	
- Github account (if you're reading this I guess you already have one!)
  
  
## Software
    
- Rasbian O/S with Python 3.5 or later (required for SDK tools: keygen and qrgen)
	- additional packages:  pynacl, qrcode, pillow (via pip installer)
  
- RPI SmartThings device enabling package (this repository)

- SmartThings core SDK
	
  
Useful reading
---------------
- Official developer documentation for direct connected devices:  https://smartthings.developer.samsung.com/docs/devices/direct-connected-devices/overview.html
- SmartThings API Reference: https://github.com/SmartThingsCommunity/st-device-sdk-c/blob/master/doc/APIs.md
- How to build Direct Connect Devices on ST Community:  https://community.smartthings.com/t/how-to-build-direct-connected-devices/204055
  - Note that the above community post is not for Raspberry Pi, so any reference to toolchains and MCU boards can be ignored
  - Also, these instructions should only be followed as outlined in the Configuration Guide in this repository
- Quickstart Guide for Raspberry Pi:  https://github.com/toddaustin07/rpi-st-device/blob/main/QuickstartGuide.pdf
- Configuration Guide for Raspberry Pi:  https://github.com/toddaustin07/rpi-st-device/blob/main/ConfigGuide.pdf

Two Ways to Proceed
-------------------
1) Use a fully-automated bash script to quickly get up and running
2) Follow step-by-step MANUAL configuration guide

For either option, start here ==> http://toddaustin07.github.io

Work in progress
----------------
- Beta test volunteers
- GUI-based device application examples
- SmartThings API wrapper for devices written in Python (currently only C language is supported)
  
