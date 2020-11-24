#!/bin/bash

cd ~/st-device-sdk-c
rm /src/port/bsp/posix/iot_bsp_wifi_posix.c
mv src/port/bsp/posix/iot_bsp_wifi_posix.ORIGc /src/port/bsp/posix/iot_bsp_wifi_posix.c
rm Makefile
mv MakefileORIG Makefile
rm stdkconfig
mv stdkconfigORIG stdkconfig


