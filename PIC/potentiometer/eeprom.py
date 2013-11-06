#!/usr/bin/env python

#
# Reads PICkit2 EEPROM dump (from stdin) and interprets it in various ways.
#

import string
import sys
import os


"""
Read EEPROM bytes from infile -- in the format dumped by the PICkit2
programming app.
Return the bytes as a list of decimal numbers on [0,1,...,255].
"""
def eeprom2dec( infile ):
        # Discard first line.
        infile.readline()        

        # Subsequent lines: First 9 chars are uninteresting.  Last four
        # chars are "00" followed by cr-lf, also uninteresting.
        # Of the rest, of each 4-char group the first two chars are a hex
        # byte and the second two are "00" (which is uninteresting).
        # The last line is also uninteresting
        result = []
        for line in infile:
                good = line[9:-4]
                hexbytes = []
                pos = 0
                while pos < len(good):
                        hexbytes.append(good[pos:pos+2])
                        pos += 4

                if len(hexbytes) == 0:
                        break

                print hexbytes
                
                # Convert hex to dec.
                decbytes = []
                for d in hexbytes:
                        decbytes.append( int(d,16) )
                result += decbytes
                print "  ", decbytes
        return result

if __name__ == '__main__':
        decdata = eeprom2dec( sys.stdin )
        print "len(eeprom data) =", len(decdata)
        print decdata

