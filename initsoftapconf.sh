#!/bin/bash

sudo mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.ORIGconf
sudo cp ~/rpi-st-device/RPIhostapd.conf /etc/hostapd/hostapd.conf
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.ORIGconf
sudo cp ~/rpi-st-device/RPIdnsmasq.conf /etc/dnsmasq.conf
