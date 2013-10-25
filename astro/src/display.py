"""
***WARNING: This doesn't work anymore, since I changed Python versions.***

Display greyscale pixel data found in a file.
Usage: "python display.py infile=<something>"

File format is, one line per image row, unsigned char base 10, e.g...
      25 0 13
      255 254 8
      0 87 22
...would be the data for a 3x3 array of pixels.

If any of the numbers exceed 255, then you need to use rescale() and define
some sort of normalization algorithm.  For an example, see below under
"if __name__ == '__main__'".
"""

import sys
import array
# See ~/software/Imaging-1.1.2/PIL for Image.py.  The whole PIL thing should be installed
# under /usr/local/lib/python2.2/site-packages.
import Image

class CmdLine:

    def __init__(self):

        #
        #Initialize all legal cmd-line variables.
        #
        self.infile = None
        self.sparse = 1   # 1 if infile format is "sparse", 0 if "raw".
                          # See def loadSparse() for definitions.
        #
        # Other stuff.
        #
        self.usage_message =\
            'Usage: ' + sys.argv[0] + ' infile=<something> [sparse=<0|1>]\n'

        self._gatherCmdLineOptions( self._crunch( sys.argv[1:] ))
        if self.infile == None:
            sys.stderr.write( self.usage_message )
            sys.exit(1)


    def _gatherCmdLineOptions( self, argvee ):
        """ Arg argvee is argv[1:] after _crunch(). """
        for item in argvee:
            if len(item.strip()) == 0:
                continue
            try:
                key, value = item.split( '=' )
                exec( 'x = self.' + key )
                # That will have thrown an exception, if self.key doesn't
                # exist, i.e. illegal command-line option.
                # So if we're here, things are looking good.  (But we still
                # haven't checked the reasonableness of the value.)
                exec( 'self.' + key + '="' + value + '"' )
            except:
                sys.stderr.write( "Invalid cmd-line arg: " + key + "\n" )
                sys.stderr.write( self.usage_message )
                sys.exit(1)


    def _crunch( self, argvee ):
        """
        Arg argvee is argv[1:]

        Normalize cmd-line, if possible, by removing blanks from around
        '=' signs.
        """

        str = ''
        for i in argvee:
            str = str + i + ' '
        str = str[:-1]

        pos = str.find(' =')
        while not pos == -1:
            str = str[:pos] + str[pos+1:]
            pos = str.find(' =')

        pos = str.find('= ')
        while not pos == -1:
            str = str[:pos+1] + str[pos+2:]
            pos = str.find('= ')

        return str.split(' ')

def signedChar( i ):
    """
    Convert an unsigned char (integer on 0,...,255) to signed char
    """
    assert( 0 <= i < 256 )
    if i < 128:
        return i
    else:
        return i - 256


def loadSparse( infile_name ):
    """
    Unlike the raw format, where the infile lists the brightness of every pixel,
    here infile lists only non-black pixel.  The format is:
    1. A header that gives the dimensions of the file (height, width).
    2. Subsequently, one pixel per line, with its coordinates (row, column)
       first.
    E.g.
    --------------
    768 1024
    24 129 255
    108 880 242
    --------------
    is a 768x1024 image with two non-black pixels -- at (24,129) and (108,880).
    """

    infile = open( infile_name )
    
    # Read header.
    header = infile.readline().split()
    assert( len(header) == 2 )
    n_rows = int(header[0])
    n_cols = int(header[1])
    assert( 0 < n_rows < 3000 and 0 < n_cols < 3000 )
    
    # Allocate the buffer and fill it with zeros (black pixels).
    nums = [0]*(n_rows*n_cols)

    # Read pixel data from infile.
    for line in infile.readlines():
        items = line.split()
        row = int( items[0] )
        col = int( items[1] )
        brightness = int( items[2] )
        nums[ row*n_cols + col ] = brightness

    return nums, n_cols


def rescale( rawdata_array, stride, algorithm ):
    """
    Arg algorithm is a function of two arguments: arg0 = brightness (each
    element of rawdata_array), arg1 = max brightness.  It should map to [0,255].

    Returns an Image object (whose method show() invokes xv).
    """

    max_val = max( rawdata_array )
    scaled_array = [0]*len(rawdata_array)

    i = 0L
    print "Looping over", len(rawdata_array), " elements of rawdata_array."
    for item in rawdata_array:
        if algorithm != 0:
            scaled_array[i] = signedChar(apply( algorithm, (item, max_val)))
        else:
            scaled_array[i] = signedChar(item)
        i = i+1
        if i%10000 == 0 : sys.stderr.write( str(i) + ' ' )
    sys.stderr.write( "Done scaling, calling array.array()...\n" )

    image_str = array.array('b',scaled_array).tostring()
    im = Image.fromstring( 'L', (stride,len(rawdata_array)/stride), image_str )
    return im


def loadRaw( infile_name ):
    """
    Load raw grayscale image data from a file.
    Format is, one line per image row, unsigned char base 10, e.g...
       25   0 13 101
      255 254  8   0
        0  87 22 209
    ...would be the data for a 4x4 array of pixels.

    returns Image object that can be displayed with Image.show().
    """

    image_str = ''
    infile = open( infile_name )
    num_lines = 0
    nums = []

    sys.stderr.write( "Reading raw data...\n" )

    for line in infile.readlines():
        num_lines = num_lines + 1
#       print "line=",num_lines
        if num_lines == 1: stride = len(line.split())
        for num in line.split():
            nums.append( int(num) )

    return nums, stride


if __name__ == '__main__':

    cmd_line = CmdLine()

    def rescaleAlgorithm( brightness, max_brightness ):
        return int( pow(brightness/float(max_brightness), 0.5) * 255 + 0.5 )

    if int(cmd_line.sparse) == 1:
        numbers, stride = loadSparse( cmd_line.infile )
        print "Done loading"
        im = rescale( numbers, stride, algorithm = 0 )
        print "Done rescaling"
    else:
        numbers, stride = loadRaw( cmd_line.infile )
        print "Done loading"
        im = rescale( numbers, stride, algorithm = rescaleAlgorithm )

    im.show()
