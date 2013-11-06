"""
Reads in the output of gencoords.py and generates a web page that
shows the location of the incidents.
Typical usage:
$ python make_map.py ../data/home_burglary_july.coords -o ~/public_html/maps/home_burglary_july.html
"""

import getopt
import sys
import os

#
# Process command line.
#
if len(sys.argv) == 1 :
    sys.stderr.write( 'Usage: python make_map.py <infile> -o <outfile>\n' )
    sys.exit(1)

options = getopt.getopt( sys.argv[2:], 'o:', 'outfile=' )

# Infile format:
# 37.556725, -121.994337, 3329+HOWARD+CO+Fremont+CA
#
infilename = sys.argv[1]
if not os.path.isfile( infilename ):
	sys.stderr.write( 'File not found:|' + infilename + '|\n' )
	sys.exit(1)

assert( (options[0][0][0] == '-o') or (options[0][0][0] == '--outfile') )
outfilename = options[0][0][1]
outfile = open( outfilename, 'w' )

#
# Prepend javascript boilerplate
#
for line in open('./javascript.header').readlines():
    outfile.write( line )

#
# Center the map at near the center of Fremont (3939 Bidwell Drive)
#
outfile.write( 'map.setCenter(new GLatLng(37.543217, -121.976186), 12);\n' )

#
# Insert javascript commands to place crime-icons on map.
#
overlaycmd1 = 'map.addOverlay(new GMarker(new GLatLng('
overlaycmd2 = '),baseIcon));\n'
for line in open(infilename).readlines():
    (latitude,longitude) = line.split(',')[0:2]
    outfile.write( overlaycmd1 + latitude + ',' + longitude + overlaycmd2 )

#
# Append javascript boilerplate
#
for line in open('./javascript.footer').readlines():
    outfile.write( line )
outfile.close()
