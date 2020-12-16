# Create Raspberry PI-based IOT devices that can directly connect to Samsung SmartThings

This repository holds everything needed to set up a Raspberry Pi to act as a SmartThings direct connected device.

Pre-requisites
--------------
## Hardware
- Raspberry Pi 3 or later running Raspberry Pi O/S 
	- assumed to include the standard capabilities including working wireless device with AP capability
	

## Accounts
- Samsung SmartThings developer account for developer workspace: https://smartthings.developer.samsung.com/workspace/	
	
- Github account (if you're reading this I guess you already have one!)
  
  
## Software
    
- Python 3.5 or later (required for SDK tools: keygen and qrgen)
	- additional packages:  pynacl, qrcode, pillow (via pip installer)
  
- RPI SmartThings device enabling package (this repository)
	
  
Useful reading
---------------
- Official developer documentation for direct connected devices:  https://smartthings.developer.samsung.com/docs/devices/direct-connected-devices/overview.html
- SmartThings API Reference: https://github.com/SmartThingsCommunity/st-device-sdk-c/blob/master/doc/APIs.md
- How to build Direct Connect Devices on ST Community:  https://community.smartthings.com/t/how-to-build-direct-connected-devices/204055
  Note that the above community post is not for Raspberry Pi, so any reference to toolchains and MCU boards can be ignored; these instructions should only be followed as outlined in the Configuration Guide in this repository
- Quickstart Guide (in this repository)
- How to Configure your Raspberry Pi for SmartThings Direct Connect (in this repository)

Two Ways to Proceed
-------------------
1) Download mastersetup script to your Pi home directory, chmod +x to make it executable, and run it: http://toddaustin07.github.io
2) Follow step-by-step manual configuration guide http://toddaustin07.github.io/ConfigGuide.pdf

Work in progress
----------------
- Beta test volunteers
- GUI-based device application examples
- SmartThings API wrapper for devices written in Python (currently only C language is supported)
  
