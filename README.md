# Create RPI-based IOT devices that can directly connect to Samsung SmartThings cloud

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
- SmartThings Device SDK for Direct Connect Devices: https://github.com/SmartThingsCommunity/st-device-sdk-c
  - cloned to RPI
  - Note that only the 'core' SDK noted above is needed; do NOT clone the 'reference' SDK - it is not needed
  - *I am trying to remove the entire SDK cloning from the requirements here; instead of having to compile the SDK core code yourself, my thought is to provide a pre-compiled shared object file and header files that you only have to link to your device code.*
    
- Python 3.5 or later (required for SDK tools: keygen and qrgen)
	- additional packages:  pynacl, qrcode, pillow (via pip installer)
  
- RPI Sametime Device enabling package (this repository)

- hostapd and dnsmasq service modules for Debian (via apt install)	
	

  
Useful reading
---------------
- Official developer documentation for direct connected devices:  https://smartthings.developer.samsung.com/docs/devices/direct-connected-devices/overview.html
- How to build Direct Connect Devices on ST Community:  https://community.smartthings.com/t/how-to-build-direct-connected-devices/204055
  
  Note that any reference in the above documentation to toolchains and MCU boards can be ignored; instead we will be using a Raspberry Pi to be the device.
- How to Configure your Raspberry Pi for SmartThings Direct Connect (in this repository)

Work in progress
----------------
- Finalize wifi module; beta test volunteers
- Documentation and automation of configuration process
- Create an SDK API wrapper for devices written in Python (currently only C language is supported)
  
