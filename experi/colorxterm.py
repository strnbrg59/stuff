#
#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#
# Author: Ted Sternberg
# Date: 2001
#
##
## Further hacks by post-LBL Ted.
##

import Tkinter
import os
import string
import math

class ColorWheel( Tkinter.Frame ) :
    """ Displays a colorwheel plus R/G/B Entry fields.

        Callback function (__init__ arg "command") is invoked any time there's
        a left-mouse-button-click in the colorwheel and is passed the (r,g,b) tuple
        that describes the point clicked on the colorwheel.
    """

    def __init__(self, master,
                 command=lambda x: 0,
                 colorwheel_image="your gif or ppm here!",
                 label_text = None,
                 **kw ) :
        Tkinter.Frame.__init__(self,master,**kw)

        self.command = command

        #
        # Color wheel
        #
        if label_text != None :
            Tkinter.Label( self, text=label_text
                         ).pack( side=Tkinter.TOP, anchor=Tkinter.W )

        self.the_image = Tkinter.PhotoImage( file=colorwheel_image )
        colorwheel_widget = Tkinter.Label(
                               self,
                               image=self.the_image,
                               height=100, width=100 )
        colorwheel_widget.pack()
        colorwheel_widget.bind('<Button-1>', self._colorWheelButtonHandler )

        colorwheel_widget.pack()


    def _colorWheelButtonHandler( self, event ) :
        """ Computes RGB coordinates corresponding to location of button click,
            and passes them to the user-definable callback ColorWheel.command.
        """
        rgb_vect = ''
        
        x = min(max(event.x-2,0),99)
        y = min(max(event.y-2,0),99)

        for d in self.the_image.get( x, y ) :
            rgb_vect = rgb_vect + d
        rgb_list = []
        for coord in rgb_vect.split( ' ' ) :
            rgb_list.append( string.atoi(coord)/255.0 )

        # Call the user-defined callback function.
        self.command( tuple(rgb_list) )


######################################################
g_colorbutton = None
def colorButton(frame, rgb_tuple):
    """
    Start an xterm with the indicated background color and a contrasting
    foreground color.
    """
    # Convert colors from [0.0,1.0] to [0,F].
    hex = ('0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
           'A', 'B', 'C', 'D', 'E', 'F')
    hex_bg = map(lambda x: hex[int(0.999*x*16)], rgb_tuple)
    # Foreground as "opposite color" doesn't look so nice.
    #hex_fg = map(lambda x: hex[int(0.999*(1-x)*16)], rgb_tuple)
    if reduce(lambda x,y: x+y, rgb_tuple) < 0.75:
        hex_fg = ('F','F','F')
    else:
        hex_fg = ('0','0','0')

    str_bg = '-bg rgb:' + hex_bg[0] + '/' + hex_bg[1] + '/' + hex_bg[2]
    str_fg = '-fg rgb:' + hex_fg[0] + '/' + hex_fg[1] + '/' + hex_fg[2]

    global g_colorbutton
    if g_colorbutton:
        g_colorbutton.forget()

    g_colorbutton = Tkinter.Button(
        frame, text="foreground color",
        bg=str_bg[4:], fg=str_fg[4:],
        command=lambda : os.system('xterm ' + str_bg + ' ' + str_fg + '&'))
    g_colorbutton.pack(expand=1, fill='both')


def hsv2rgb( h, s, v ):
    """ Convert h,s,v to r,g,b

        h,s,v in [0.0,1.0]
        r,g,b in [0.0,1.0]

        Returns r,g,b as a tuple.

        This algorithm taken from Foley & Van Dam
    """
    if v == 0.0 :
        r = g = b = 0.0
    else:
        if s == 0.0 :
            r = g = b = v
        else:
            h = h * 6.0
            if h >= 6.0 : h = 0.0
          
            i = math.floor( h )
            f = h - i
            p = v*(1.0-s)
            q = v*(1.0-s*f)
            t = v*(1.0-s*(1.0-f))
          
            if   i == 0:
                r = v; g = t; b = p
            elif i == 1:
                r = q; g = v; b = p
            elif i == 2:
                r = p; g = v; b = t
            elif i == 3:
                r = p; g = q; b = v
            elif i == 4:
                r = t; g = p; b = v
            elif i == 5:
                r = v; g = p; b = q

    return r,g,b


if( __name__ == "__main__" ) :
    root = Tkinter.Tk()
    root.title("xterm launcher")
    frame1 = Tkinter.Frame(root)
    frame1.pack()
    frame2 = Tkinter.Frame(root)
    frame2.pack(expand=1, fill='both')

    ColorWheel( 
        master=frame1, 
        command=lambda rgb_tuple: colorButton(frame2, rgb_tuple),
        colorwheel_image=os.path.dirname(__file__) + '/ColorWheel.ppm',
        relief=Tkinter.GROOVE, borderwidth=2
         ).pack( pady=4 )

    root.mainloop()
