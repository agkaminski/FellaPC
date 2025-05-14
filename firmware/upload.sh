#!/bin/bash

# Works with https://github.com/agkaminski/emuprom EPROM emulator
# It is emulating 27C256 EPROM, FellaPC uses 27C128, so the payload
# has to be put at address 0x4000 (A14 is stuck high).
# Connect /RST output of emuprom to the FellaPC reset button.

# $1 - path to the USB tty of connected emuprom.

set -e

FW="firmware.bin"

echo "Uploading $FW"
stty -F $1 0:4:cbe:a30:3:1c:7f:15:4:0:1:0:11:13:1a:0:12:f:17:16:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0
echo '!@0000#-' > $1 && xxd -ps -g1 $FW | cat > $1 && echo '+.' > $1
echo "Done"
