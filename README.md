#Create RPI-based IOT devices that can directly connect to Samsung SmartThings cloud

This repository holds everything needed to set up a Raspberry Pi to act as a SmartThings direct connected device.

Pre-requisites:
	- Raspberry Pi 3 or later running Raspian O/S (so far tested on Jessie)
  - assumed to include numerous capabilities including wifi, GNU C compiler (gcc), iw, wpa_cli, rfkill, GPicView, wlan0 device defined
  
  - Github account
  
  - SmartThings Device SDK for Direct Connect Devices: https://github.com/SmartThingsCommunity/st-device-sdk-c
    - cloned to RPI
    - Note that only the 'core' SDK noted above is needed; do NOT clone the 'reference' SDK - it is not needed
    - * I am trying to remove the SDK from the requirements here; instead of having to compile the SDK core code yourself, my aim is to provide a pre-compiled shared object file and header files that you only have to link to your device code.
    
    
  - Python 3.5 or later (required for SDK tools keygen and qrgen)
  
  - Wireless router / access point configured with WPA or WPA2 authentication (a bug in the SDK currently prevents OPEN authorization (no password) from working)

  - hostapd and dnsmasq modules for Debian

  - Samsung/SmartThings developer account for developer workspace: https://smartthings.developer.samsung.com/workspace/
  
  
Useful reading:
  Official developer documentation for direct connected devices:  https://smartthings.developer.samsung.com/docs/devices/direct-connected-devices/overview.html
  How to build Direct Connect Devices on ST Community:  https://community.smartthings.com/t/how-to-build-direct-connected-devices/204055
  
  Note that any reference in the above documentation to toolchains and MCU boards can be ignored; instead we will be using a Raspberry Pi to be the device.
  

Future Work
  Eliminate or make optional the ST SDK clone and compile step
  Provide a library of additional functions that can be used by the device app, such as RPI initialization and device status console window
  Provide a SDK API wrapper for devices written in Python (currently only C language is supported)
  