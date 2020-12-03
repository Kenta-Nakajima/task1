#!/bin/bash

sudo insmod myled.ko

sudo chmod 666 /dev/myled0

gcc Main.c -o Main
