#!/bin/bash

oclint ../src/electricui.c ../src/transports/eui_serial_transport.c ../src/utilities/eui_crc.c ../src/utilities/eui_offset_validation.c -- -c
