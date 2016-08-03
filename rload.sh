#!/bin/sh
sudo rmmod cdc_acm #interferes and claims the device sometimes
sudo rmmod k8101
sudo insmod k8101.ko
sudo chmod 666 /dev/msgboard1
