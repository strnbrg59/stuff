import urllib
import string
import getopt
import sys
import os

"""
Reads a FPD spreadsheet (that's been converted to ascii) and prepends
coordinates to each street address.
"""

if len(sys.argv) == 1 :
    sys.stderr.write( 'Usage: python gencoords.py <infile> -o <outfile>\n' )
    sys.exit(1)

options = getopt.getopt( sys.argv[2:], 'o:', 'outfile=' )

infilename = sys.argv[1]
assert( os.path.isfile( infilename ) )

assert( (options[0][0][0] == '-o') or (options[0][0][0] == '--outfile') )
outfilename = options[0][0][1]
outfile = open( outfilename, 'w' )

key = 'ABQIAAAA_UDgKGWyYuE0YHl4H0WTHhT4gZ6bPW9nPZnWiuOURMVI7YQk4xTl22ZUpiHksJnJe-8xrfEO1aXE9g'
google_url = 'http://maps.google.com/maps/geo'
city = 'Fremont CA'
for line in open( infilename ).readlines():
    address = string.join( (line[:-1] + ' ' + city).split(), '+' )
    f = urllib.urlopen( google_url + '?q=' + address + '&output=csv&key=' + key )
    coords = f.read().split(',')[2:]
    outfile.write( coords[0] + ', ' + coords[1] + ', ' + address + '\n' )

outfile.close()
