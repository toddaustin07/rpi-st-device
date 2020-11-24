#!/bin/bash

cd ~/st-device-sdk-c
mv src/port/bsp/posix/iot_bsp_wifi_posix.c /src/port/bsp/posix/iot_bsp_wifi_posix.ORIGc
cp ~/rpi-st-device/iot_bsp_wifi_rpi.c src/port/bsp/posix/iot_bsp_wifi_posix.c
mv Makefile MakefileORIG
cp ~/rpi-st-device/SDKMakefile Makefile
mv stdkconfig stdkconfigORIG
cp ~/rpi-st-device/stdkconfig stdkconfig


