#!/usr/bin/env python
##   _______              __
##  / ___/ /  ___  __ _  / /  ___
## / /__/ _ \/ _ \/  ' \/ _ \/ _ \
## \___/_//_/\___/_/_/_/_.__/\___/ 
##
## This software is copyright (C) by the Lawrence Berkeley
## National Laboratory.  Permission is granted to reproduce
## this software for non-commercial purposes provided that
## this notice is left intact.
## 
## It is acknowledged that the U.S. Government has rights to
## this software under Contract DE-AC03-765F00098 between
## the U.S. Department of Energy and the University of
## California.
##
## This software is provided as a professional and academic
## contribution for joint exchange.  Thus it is experimental,
## is provided ``as is'', with no warranties of any kind
## whatsoever, no support, no promise of updates, or printed
## documentation.  By using this software, you acknowledge
## that the Lawrence Berkeley National Laboratory and
## Regents of the University of California shall have no
## liability with respect to the infringement of other
## copyrights by any part of this software.
##

## Author: TDSternberg

import glob
import getopt
import re
import os
import os.path
import sys
import time
import types

class CmdLine:
    """
    Detects cmd-line options and then can be queried for them.
    """
    def __init__( self ):
        self.usage_message =\
        """
        ******************************************************************
        Given a collection of hdf5 files (in Chombo format), produces a
        collection of corresponding ppm files and, optionally, goes on to
        turn them into an .mpg movie.
    
        Usage: movie.py [-s state_file | -u user_script] [-n]
               [-o output_mpg] [-w slowness] [-t temp_dir] [-b bit_rate]
               [-x x_size -y y_size]
               hdf5_files | ppm_files

        -o output_mpg: The name of the file to save the mpg movie to.  If
            this option is omitted, no movie is made; only ppm files are made.
            In the absence of the -o option, the -w, -b, and -p options will be
            ignored, and moreover the last argument can not be a collection
            of ppm files but has to be hdf5 files.

        -s state_file: A .state file in which ChomboVis has saved its state
            (and with which ChomboVis can be restarted into that state).  See
            File-->SaveState in the ChomboVis menus.

        -u user_script: A Python script typically utilizing the ChomboVis API.
            This is an alternative to -s as a way of bringing ChomboVis to a
            desired state before saving a ppm.  See the API section of the
            ChomboVis user's guide, at
            http://seesar.lbl.gov/anag/chombo/chomboVis/UsersGuide.html.
            If you go the user_script route, your script should end with the
            following boilerplate:

              import os
              import sys
              c.misc.hardCopy( outfile_name=os.getenv('IMAGE_FILE') )
              sys.exit(0)

            It is also your responsibility to call
            ChomboVis.misc.setRenderWidgetSize(); the -x and -y options only
            have an effect if used with -s.

            The "state_file" method is easier to use but has the disadvantage
            that ChomboVis is usually not backward-compatible with state files
            saved in previous versions.  The user script API, however, does
            maintain a high degree of backward compatibility.  A user script
            also allows you to change certain settings from frame to frame, if
            that is desireable.  If you do not use either -s or -u, the image
            you obtain will be the default you see when ChomboVis starts.

        -n : render off-screen (rather than flashing every frame on your
            monitor).

        -w slowness: Factor by which to slow down the action.  Default is 3.
            Reasonable values range from 1 to 20.  This does affect the mpg
            size, so don't try 1000.  Ignored, if -o option not given.

        -b bit rate: Default is 1152000.0 bits/second.  Use a higher number (if
            you dare) to obtain a higher-resolution movie (and a larger mpg
            file).  Ignored, if -o option not given.

        -p ppm prefix: This is first part of the name assigned to each generated
            ppm file.  By default it's "chombovis_movie_frame', but if you
            prefer something else, you can supply it here.  Whatever you choose,
            though, will be suffixed with a number; mpeg2encode (the program
            that produces the mpg movie) expects its input ppm files to be
            ordered like that.  Ignored, if -o option not given.

        -t temp_dir: Directory in which to store temporary files.  Default is
            /tmp.  Do not use current working directory for this.

        -x x_size, -y y_size: desired size of the images.  Default is whatever
            is in your state file, or 600x600 (300x300 in off_screen mode)
            if there is no state file *or* user script; thus use -x and -y to
            override the state file.
            These options have no effect in conjunction with the -u option.
            When using the -u option, call
            ChomboVis.misc.setRenderWidgetSize(x_size,y_size) in your script.
            

        hdf5_files: Where you specify the hdf5 files you want to run
            ChomboVis on.  hdf5_files can be a list of file names, spelled out
            explicitly or described with a wildcard.  If the files are gzipped,
            this script will gunzip them in /tmp (or the place you specify with
            the -t option).  It's a good idea to surround wildcard expressions
            in single quotes.

        ppm_files: This is if you have already run ChomboVis on your hdf5 files
            but you would like to produce a movie that is faster or slower than
            the one you got the first time.  Warning: do not mix hdf5 and ppm
            files; you must use one or the other.  Ignored, if -o option not
            given.

        Example1: movie.py -s eb_on.state -o turb.mpg -w10 -x900 -y900 '*.hdf5'
        Example1: movie.py -s eb_on.state                  -x600 -y700 '*.hdf5'
        Example2: movie.py -u eb_on.py -o turb.mpg '*.hdf5.gz'
        Example3: movie.py -w5 -t/tmp -o turb.mpg '*.ppm'
        Example4: movie.py -pplt -o turb.mpg 'plt*.hdf5'

        This program runs ChomboVis repeatedly, and produces a ppm file for
        every scene of your movie (1 scene corresponds to one hdf5 file).
        After producing your ppm files, if you gave the -o options, this program
        runs mpeg2encode to produce the mpg.  (If you don't have mpeg2encode,
        the source is on our download site as mpeg2encode.tar.gz.)
        ******************************************************************
        """

        # Defaults:
        self.state_file = self.user_script = None
        self.temp_dir = '/tmp'
        self.slowness = 3
        self.x_size = self.y_size = None
        self.bit_rate = 1152000.0
        self.hdf5_files = self.ppm_files = None
        self.mpg_out = None
        self.movie_frame_format = 'chombovis_movie_frame%.6d' 
        self.off_screen = 0

        # Parse
        try:
            cmdline_optlist, cmdline_args =\
                getopt.getopt( sys.argv[1:], 's:u:no:w:p:b:t:x:y:' )
        except:
            # Catches '-h' and '--help' along with other errors.
            sys.stderr.write( self.usage_message + '\n' )
            sys.exit(0)
        for key, value in cmdline_optlist:
            if key == '-s':
                self.state_file = value
            if key == '-u':
                self.user_script = value
            if key == '-n':
                self.off_screen = 1
            if key == '-o':
                if value[-4:] != '.mpg':
                    self.mpg_out = value + '.mpg' 
                    # mpeg2encode fails silently on other extensions
                else:
                    self.mpg_out = value
            if key == '-w':
                self.slowness = int(value)
            if key == '-t':
                self.temp_dir = os.path.expanduser(value)
            if key == '-x':
                self.x_size = value
            if key == '-y':
                self.y_size = value
            if key == '-b':
                self.bit_rate = value
            if key == '-p':
                self.movie_frame_format = value + '%.6d'

        if len(cmdline_args) == 1:  # Wildcard expression in single quotes.
            filenames = glob.glob(cmdline_args[0])
            filenames.sort()
        else:
            filenames = cmdline_args
        if (filenames!=[]) and  (filenames[0].find('.hdf5') != -1):
            self.hdf5_files = filenames
        elif (filenames!=[]) and (filenames[0].find('.ppm') != -1):
            self.ppm_files = filenames

        #
        # Error handling.
        #
        if self.hdf5_files == self.ppm_files == None:
            sys.stderr.write( 'Error: you must specify hdf5 or ppm files.\n' )
            sys.exit(1)
        if (self.state_file != None) and (self.user_script != None):
            sys.stderr.write(
                'Error: illegal to use both -u and -s options.\n' )
            sys.exit(2)
        if( ((self.x_size != None) and (self.y_size == None))
        or  ((self.x_size == None) and (self.y_size != None)) ):
            sys.stderr.write(
                'Error: you must use both -x and -y, or neither.\n')
            sys.exit(3)
        if self.temp_dir == '.':
            sys.stderr.write( 'Error: temp dir must not be cwd.\n' )
            sys.exit(5)
        if (not self.mpg_out) and self.ppm_files:
            sys.stderr.write( 'Error: when not using -o option, this program '
                + 'works only on hdf5 files, and not on ppm files.\n' )
            sys.exit(6)


    #
    # Accessors
    # 
    def getStateFile( self ): return self.state_file
    def getUserScript( self ): return self.user_script
    def getHDF5Files( self ): return self.hdf5_files
    def getPPMFiles( self ): return self.ppm_files
    def getMpgOut( self ) : return self.mpg_out
    def getSlowness( self ): return self.slowness
    def getTempDir( self ): return self.temp_dir
    def getXSize( self ): return self.x_size
    def getYSize( self ): return self.y_size
    def getBitRate( self ): return self.bit_rate
    def getMovieFrameFormat( self ): return self.movie_frame_format


def generateMpeg2EncodeParFile(
    tmp_dir, movie_frame_format,
    horiz_size, vert_size, bit_rate, n_pictures, outfile_name ):
    """
    Make a file, named by arg outfile_name, appropriate for running mpeg2encode
    to make a movie out of our ChomboVis-generated ppm files.

    We have a template of such a parameters file hard-coded right here.  We just
    replace the fields corresponding to the arguments of this function (but
    note that horiz and vert size appears in two places in the parameters file).

    The arguments to this function (other than outfile_name) are integers (i.e.
    numbers, not strings).
    """

    # Make sure x & y sizes are even numbers.  (VtkVtk.setRenWidgetSize() should
    # have taken care of that.)
    if( (horiz_size != 2 * (horiz_size/2))
    or  (vert_size  != 2 * (vert_size/2)) ):
        sys.stderr.write( "Error: ppm file horiz and vert sizes must be even "
                          "numbers!\n" )
        sys.exit(6)


    # The line-by-line comments below come from NTSC.par, distributed with the
    # ImageMagick source code.
    # The values I obtained by running ImageMagick's convert program on a few
    # of our ppm files.
    template =\
    [
        'mpeg2encode params generated by the ChomboVis movie.py example script /* comment */                 ',
        'edited_below            /* name of source files */                                                  ',
        '-                       /* name of reconstructed images ("-": do not store) */                      ',
        '-                       /* name of intra quant matrix file     ("-": default matrix) */             ',
        '-                       /* name of non intra quant matrix file ("-": default matrix) */             ',
        './mpeg2encode.log       /* name of statistics file ("-": stdout ) */                                ',
        '2                       /* input picture file format: 0=*.Y,*.U,*.V, 1=*.yuv, 2=*.ppm */            ',
        '55                      /* number of frames */                                                      ',
        '0                       /* number of first frame */                                                 ',
        '00:00:00:00             /* timecode of first frame */                                               ',
        '12                      /* N (# of frames in GOP) */                                                ',
        '3                       /* M (I/P frame distance) */                                                ',
        '1                       /* ISO/IEC 11172-2 stream */                                                ',
        '0                       /* 0:frame pictures, 1:field pictures */                                    ',
        '400                     /* horizontal_size */                                                       ',
        '300                     /* vertical_size */                                                         ',
        '8                       /* aspect_ratio_information 1=square pel, 2=4:3, 3=16:9, 4=2.11:1 */        ',
        '3                       /* frame_rate_code 1=23.976, 2=24, 3=25, 4=29.97, 5=30 frames/sec. */       ',
        '1152000.0               /* bit_rate (bits/s) */                                                     ',
        '20                      /* vbv_buffer_size (in multiples of 16 kbit) */                             ',
        '0                       /* low_delay  */                                                            ',
        '1                       /* constrained_parameters_flag */                                           ',
        '4                       /* Profile ID: Simple = 5, Main = 4, SNR = 3, Spatial = 2, High = 1 */      ',
        '8                       /* Level ID:   Low = 10, Main = 8, High 1440 = 6, High = 4          */      ',
        '1                       /* progressive_sequence */                                                  ',
        '1                       /* chroma_format: 1=4:2:0, 2=4:2:2, 3=4:4:4 */                              ',
        '1                       /* video_format: 0=comp., 1=PAL, 2=NTSC, 3=SECAM, 4=MAC, 5=unspec. */       ',
        '5                       /* color_primaries */                                                       ',
        '5                       /* transfer_characteristics */                                              ',
        '5                       /* matrix_coefficients */                                                   ',
        '400                     /* display_horizontal_size */                                               ',
        '300                     /* display_vertical_size */                                                 ',
        '0                       /* intra_dc_precision (0: 8 bit, 1: 9 bit, 2: 10 bit, 3: 11 bit) */         ',
        '0                       /* top_field_first */                                                       ',
        '1 1 1                   /* frame_pred_frame_dct (I P B) */                                          ',
        '0 0 0                   /* concealment_motion_vectors (I P B) */                                    ',
        '0 0 0                   /* q_scale_type  (I P B) */                                                 ',
        '0 0 0                   /* intra_vlc_format (I P B)*/                                               ',
        '0 0 0                   /* alternate_scan (I P B) */                                                ',
        '0                       /* repeat_first_field */                                                    ',
        '1                       /* progressive_frame */                                                     ',
        '0                       /* P distance between complete intra slice refresh */                       ',
        '0                       /* rate control: r (reaction parameter) */                                  ',
        '0                       /* rate control: avg_act (initial average activity) */                      ',
        '0                       /* rate control: Xi (initial I frame global complexity measure) */          ',
        '0                       /* rate control: Xp (initial P frame global complexity measure) */          ',
        '0                       /* rate control: Xb (initial B frame global complexity measure) */          ',
        '0                       /* rate control: d0i (initial I frame virtual buffer fullness) */           ',
        '0                       /* rate control: d0p (initial P frame virtual buffer fullness) */           ',
        '0                       /* rate control: d0b (initial B frame virtual buffer fullness) */           ',
        '2 2 11 11               /* P:  forw_hor_f_code forw_vert_f_code search_width/height */              ',
        '1 1 3 3                 /* B1: forw_hor_f_code forw_vert_f_code search_width/height */              ',
        '1 1 7 7                 /* B1: back_hor_f_code back_vert_f_code search_width/height */              ',
        '1 1 7 7                 /* B2: forw_hor_f_code forw_vert_f_code search_width/height */              ',
        '1 1 3 3                 /* B2: back_hor_f_code back_vert_f_code search_width/height */              ',
    ]

    result = template[:]
    result[1] = tmp_dir + '/' + movie_frame_format + ' /* name of source files */'
    result[7] = str(n_pictures) +  ' /* number of frames */'
    result[14] = str(horiz_size) + ' /* horizontal_size */'
    result[15] = str(vert_size)  + ' /* vertical_size */'
    result[18] = str(bit_rate)  +  ' /* bit rate */'
    result[30] = str(horiz_size) + ' /* display_horizontal_size */'
    result[31] = str(vert_size)  + ' /* display_vertical_size */' 

    outfile = open( outfile_name, 'w' )
    for line in result:
        outfile.write( line + '\n' )
    outfile.close()


def FindPPM_XY( ppmfile ):
    """
    Return (x_size,y_size) -- the dimensions of the ppm file, which appear on
    the PPM file's second non-comment line.
    """
    infile = open(ppmfile)
    p6=infile.readline()  # throw it away

    compiled_regex = re.compile('^\s*#')
    line = infile.readline()
    while( compiled_regex.search(line,0) != None ):
        line = infile.readline()
    if len(line.split()) != 2:
        print "Error: can't figure out the size of your ppm file.  You'll need"
        print "to hand-edit the encode.par and then run mpeg2encode by hand."
        return cmd_line.getXSize(), cmd_line.getYSize()
    else:
        items = line.split()
        return int(items[0]), int(items[1])


def workFromPPMs( cmd_line ):
    """
    No need to run ChomboVis; it's already run and the ppm's are available.
    If we're here, then the user just wants to redo the movie at a different
    slowness factor.
    """
    i_ppm = 0
    for ppmfile in cmd_line.getPPMFiles():
        # Make a bunch of symlinks.  Name them same as image_file_name but
        # with an extra index in there.
        for j in range(0, cmd_line.getSlowness()):
            if cmd_line.getTempDir()[-1:] == '/':
                extra_slash = ''
            else:
                extra_slash = '/'
            symlink_name = cmd_line.getTempDir() + extra_slash +\
              cmd_line.getMovieFrameFormat() % (i_ppm*cmd_line.getSlowness()+j)\
              + '.ppm'
            os.system( 'if test -f ' + symlink_name + ' ; then rm -f ' +
                       symlink_name + '; fi')
            cmd = 'ln -s ' + os.path.abspath(ppmfile) + ' ' + symlink_name
            os.system( cmd )


        i_ppm += 1

    # Figure out the size, from one of the ppm files.
    ppm_xy = FindPPM_XY( ppmfile )

    return len(cmd_line.getPPMFiles()), ppm_xy


def workFromHDF5s( cmd_line ):
    """
    Run ChomboVis on a collection of hdf5 files.
    Returns the number of ppm files produced (not including symlinks).
    """
    i_ppm = 0 # Counts ppm files produced.

    for hdf5file in cmd_line.getHDF5Files():
        # Assemble name of file to save image to.
        if cmd_line.getMpgOut():
            image_file_name = os.getenv('PWD') + '/' +\
                              cmd_line.getMovieFrameFormat() % i_ppm + '.ppm'
        else:
            image_file_name = os.getenv('PWD') + '/' + hdf5file[:-5] + '.ppm'

        # Gunzip (into /tmp) if file ends in .gz.
        if hdf5file[-8:] == '.hdf5.gz':
            print "gunzipping", hdf5file, "..."
            new_name = cmd_line.getTempDir() +'/movieframe.hdf5'
            os.system( 'gzip -dfc ' + hdf5file + ' > ' + new_name )
            hdf5file = new_name
            print "hdf5file now", hdf5file

        # Assemble and invoke command that will start a ChomboVis on hdf5file.
        if cmd_line.getUserScript():
            os.putenv('IMAGE_FILE', image_file_name)
            # Env variable created for user script's convenience.
            command = os.getenv('CHOMBOVIS_HOME') + "/bin/" +\
                      '/chombovis off_screen=' + str(cmd_line.off_screen) +' '+\
                      ' ignore_rc=1 ' +\
                      'infile=' + hdf5file +\
                      ' user_script=' + cmd_line.getUserScript()
        else:
            command = os.getenv('CHOMBOVIS_HOME') + '/bin/chombovis cmd=\''
            if cmd_line.getXSize():
                command = command +\
                    'c.misc.setRenderWidgetSize(' +\
                         str(cmd_line.getXSize()) + ',' +\
                         str(cmd_line.getYSize()) + ');'
            command = command +\
                'c.misc.hardCopy(\"' +\
                image_file_name + '\");sys.exit(0)\' ' +\
                'off_screen=' + str(cmd_line.off_screen) + ' ' +\
                ' ignore_rc=1 ' +\
                'infile=' + hdf5file
            if cmd_line.getStateFile():
                command = command + ' state_file=' + cmd_line.getStateFile()

        print command
        os.system(command)

        if cmd_line.getMpgOut():
            # Make a bunch of symlinks.  Name them same as image_file_name but
            # with an extra index in there.
            for j in range(0, cmd_line.getSlowness()):
                if cmd_line.getTempDir()[-1:] == '/':
                    extra_slash = ''
                else:
                    extra_slash = '/'
                symlink_name = cmd_line.getTempDir() + extra_slash +\
                  cmd_line.getMovieFrameFormat() % (i_ppm*cmd_line.getSlowness()+j)\
                  + '.ppm'
                os.system( 'if test -f ' + symlink_name + ' ; then rm -f ' +
                           symlink_name + '; fi')
                os.system( 'ln -s ' + image_file_name + ' ' + symlink_name )

        i_ppm += 1
    num_ppmfiles = i_ppm

    # Figure out the size, from one of the ppm files.
    ppm_xy = FindPPM_XY( image_file_name )

    return num_ppmfiles, ppm_xy


if __name__ == '__main__':
    if not os.getenv('CHOMBOVIS_HOME'):
        sys.stderr.write(
            'You must set your CHOMBOVIS_HOME environment variable\n' )
        sys.exit(4)
    cmd_line = CmdLine()

    if   cmd_line.getHDF5Files():
        num_ppmfiles, (xsize,ysize) = workFromHDF5s( cmd_line )
    elif cmd_line.getPPMFiles():
        num_ppmfiles, (xsize,ysize) = workFromPPMs( cmd_line )
    else:
        assert( None ) # Should have been caught in class CmdLine.

    if not cmd_line.getMpgOut():
        sys.exit(0)

    # Generate the mpeg2encode parameters file.
    # We assume the ppms generated are all the same size.
    mpeg2encode_params_filename = 'encode.par'
    generateMpeg2EncodeParFile(
        cmd_line.getTempDir(), cmd_line.getMovieFrameFormat(),
        xsize,ysize, cmd_line.getBitRate(),
        num_ppmfiles * cmd_line.getSlowness(), mpeg2encode_params_filename )

    # Give user a chance to kill this program, so he can run mpeg2encode
    # by hand.
    print "Your mpeg2encode params file, ", mpeg2encode_params_filename, \
          "is ready.  mpeg2encode will start in 3 seconds.  (And when it\n",\
          "finishes, we will erase all your temporary files.)"
    time.sleep(3)
    
    os.system( 'mpeg2encode ' + mpeg2encode_params_filename +
               ' ' + cmd_line.getMpgOut() )
    format = cmd_line.getMovieFrameFormat()
    os.system( 'rm -f ' + cmd_line.getTempDir() + '/' +
               format[:format.find('%')] + '*' )
