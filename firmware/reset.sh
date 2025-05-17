#!/bin/bash

# Works with https://github.com/agkaminski/emuprom EPROM emulator
# Takes control of the bus momentarly to just to reset the target
# Connect /RST output of emuprom to the FellaPC reset button.

# $1 - path to the USB tty of connected emuprom.

set -e

echo "Reseting the target"
stty -F $1 0:4:cbe:a30:3:1c:7f:15:4:0:1:0:11:13:1a:0:12:f:17:16:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0
echo '!.' > $1
echo "Done"
