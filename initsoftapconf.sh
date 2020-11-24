#!/bin/bash

sudo mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.ORIGconf
sudo cp ~/rpi-st-device/hostapd.conf /etc/hostapd/hostapd.conf
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.ORIGconf
sudo cp ~/rpi-st-device/dnsmasq.conf /etc/dnsmasq.conf
