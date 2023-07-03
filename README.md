# Create Raspberry PI-based IOT device applications that can directly connect to Samsung SmartThings

This repository holds everything needed to set up a Raspberry Pi to act as a SmartThings direct connected device.

02/14/22 UPDATE:  Fix was implemented to solve hostapd service detection issue on Bullseye OS (problem was seen during device provisioning process)

09/16/22 UPDATE:  The new Raspberry Pi OS provides a new option to use Network Manager instead of dhcpcd.  You must stay with dhcpcd.

**02/15/23 NOTICE:  A new release of the SmartThings core SDK for Direct Connected Devices (v.1.8.0) is now available.  It appears that this release now supports a Raspberry Pi configuration natively, so my package here may no longer be required.  I have not tested it myself, but refer to the "raspberry" subdirectory within the SDK example directory.

What is a direct connected device?
----------------------------------
The SmartThings platform is evolving. In 2023 there will be three methods for IOT devices to integrate with SmartThings: 1) devices that talk directly to the SmartThings hub (Edge), 2) cloud-connected devices, and 3) direct-connected devices.  Former devices that were developed in the SmartThings IDE using device type handlers with Groovy code will need to be migrated to one of the those three new options.

* Hub-connected: This is a viable option for hobbiest/developers who are willing to learn how to develop Edge drivers that can talk to their LAN-based device applications.  All communication is local - no cloud.  Some existing Edge drivers can simplify this setup by using MQTT or simple HTTP commands.
* Cloud-connected: requires you to run your code on a cloud server - either your own, self-managed server or on AWS.  Most hobbiests will not want to open their home LAN to the internet, which would be required to host your own cloud server, due to complexity and security risks.  There are less risky and simpler options such as ngrok, which creates a secure tunnel to your local machine from the internet.  Using AWS may also be an option, but having your devices running from cloud to cloud may not give the best performance and then there are the AWS service fees.
* Direct-connected: this alternative allows you to connect directly to SmartThing's cloud, without an intermediate cloud server as in above.  In this approach, your device apps are running on your own computer (i.e. Raspberry Pi), on a local LAN with internet access, using an SDK API to talk to the SmartThings platform.

SmartThings's concept for "direct-connected" devices are wifi-enabled microcontroller-based IOT devices.  In fact the SDK that was developed in support of this very much revolves around MCUs like the popular ESP32 system-on-a-chip microcontroller with integrated wifi.  However with modifications, the SDK can be implemented on a Raspberry Pi as well.  The only downside to this approach is the way devices must be individually provisioned.  However, what direct connected devices enables is an environment where you can write and run applications totally under your control, with full integration with SmartThings.  No cloud server to manage, no learning Edge development - just a simple API to send and receive commands and attributes for whatever kind of IOT device application you can dream up and implement on a Raspberry Pi using either C or Python.

Pre-requisites
--------------
## Hardware
- Raspberry Pi Model 3 or 4 or Zero W
	- must include the standard capabilities including integrated wireless with AP (Access Point) capability
	

## Accounts
- Samsung SmartThings developer account for developer workspace: https://smartthings.developer.samsung.com/workspace/	
	
- Github account (if you're reading this I guess you already have one!)
  
  
## Software
    
- Raspberry Pi O/S (Version 10 Buster or Bullseye preferred, but as far back as Jessie can also work) Full or Lite
        - Python 3.5 or later (required for SDK tools: keygen and qrgen)
	- additional packages:  pynacl, qrcode, pillow (via pip installer)
	- only limited testing has been done on 64-bit Raspberry Pi OS; the Python wrapper works only on 32-bit OS
  
- RPI SmartThings device enabling package (this repository)

- SmartThings core SDK (will be installed by the setup script, including additional dependencies); this package tested with v.1.7.0
	
  
Useful reading
---------------
- Official developer documentation for direct connected devices:  https://smartthings.developer.samsung.com/docs/devices/direct-connected-devices/overview.html
- SmartThings API Reference: https://github.com/SmartThingsCommunity/st-device-sdk-c/blob/master/doc/APIs.md
- How to build Direct Connect Devices on ST Community:  https://community.smartthings.com/t/how-to-build-direct-connected-devices/204055
  - Note that the above community post is not for Raspberry Pi, so any reference to toolchains and MCU boards can be ignored
  - Also, those instructions should only be referenced as specified in the Configuration Guide in this repository
- Quickstart Guide for Raspberry Pi:  https://github.com/toddaustin07/rpi-st-device/blob/main/QuickstartGuide.pdf
- Configuration Guide for Raspberry Pi:  https://github.com/toddaustin07/rpi-st-device/blob/main/ConfigGuide.pdf

Two Ways to Proceed
-------------------
1) Use a fully-automated bash script to quickly get up and running ('mastersetup')
2) Follow step-by-step MANUAL configuration guide

For either option, start here ==> http://toddaustin07.github.io

