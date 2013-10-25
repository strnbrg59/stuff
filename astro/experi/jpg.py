import sys
import array
import Image

class CmdLine:

    def __init__(self):

        #
        #Initialize all legal cmd-line variables.
        #
        self.jpg_name = ''
        self.do_create = 0
        self.do_load = 0
        self.do_load_raw = 0
        self.infile_name = 0
        #
        # Other stuff.
        #
        self.usage_message = "Usage: " + sys.argv[0] + \
            " [<key1>=<val1> [<key2>=<val2> [...]]]\n"


        self._gatherCmdLineOptions( self._crunch( sys.argv[1:] ))


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

def createImage( cmd_line ):
    pic_array = []
    for i in range(0,10):
        print ''
        for j in range(0,10):
            if j % 2 == 0:
                pic_array.append( signedChar( int(i*25.5)))
#                sys.stdout.write( str(int(i*25.5)) + ' ' )
            else:
                pic_array.append( 0 )
#                sys.stdout.write( '0  ' )
#    print ''        

    pic_string = array.array('b',pic_array).tostring()    
    print "pic_string=", pic_string
    im = Image.fromstring( 'L', (10,10), pic_string )

    im.show()


def loadRaw( infile_name ):
    """
    Load raw grayscale image data from a file.
    Format is, one line per image row, unsigned char base 10, e.g...
      25 0 13
      255 254 8
      0 87 22
    ...would be the data for a 3x3 array of pixels.

    returns Image object that can be displayed with Image.show().
    """

    image_str = ''
    infile = open( infile_name )
    num_lines = 0

    line = infile.readline()
    stride = len( line.split() )
    while line:
        num_lines = num_lines + 1
        nums = []
        for num in line.split():
            nums.append( signedChar(int(num)) )
        line_str = array.array('b',nums).tostring()
        image_str = image_str + line_str
        line = infile.readline()

    im = Image.fromstring( 'L', (num_lines, stride), image_str )
    return im


def loadImage( cmd_line ):

    im = Image.open( cmd_line.jpg_name )
    
    data = im.getdata()
    im.show()
    
    print "len(data)=", len(data)
    
    print "data[0]=", data[0]
    print "data[10]=", data[10]
    print "data[100]=", data[100]
    
    print "im.getbbox()=", im.getbbox()
    extents = im.getbbox()
    print "im.getpixel((0,0))=", im.getpixel((0,0))
    print "im.getpixel((10,10))=", im.getpixel((10,10))
    print "im.getpixel((100,100))=", im.getpixel((100,100))
    
    print "im.getextrema()=", im.getextrema()
    print "im.getbands()=", im.getbands()
    print "im.info=", im.info


if __name__ == '__main__':

    cmd_line = CmdLine()
    if   cmd_line.do_create != 0:
        createImage( cmd_line )
    elif cmd_line.do_load != 0:
        loadImage( cmd_line )
    elif cmd_line.do_load_raw != 0:
        im = loadRaw( cmd_line.infile_name )
        im.show()
