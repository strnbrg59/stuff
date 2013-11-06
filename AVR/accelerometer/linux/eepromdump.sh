#!/bin/sh

outfile=/tmp/eeprom.dump
avrdude -P/dev/ttyUSB0 -pt85 -v -cstk500v2 -U eeprom:r:$outfile:d >/dev/null 2>&1
cat $outfile
