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

""" Composite widgets  
"""
import Tkinter
import Pmw
import math
import os
import string
import sys

import algorithms
import anag_utils
import self_control

class ColorWheel( Tkinter.Frame ) :
    """ Displays a colorwheel plus R/G/B Entry fields.

        Callback function (__init__ arg "command") is invoked any time there's
        a left-mouse-button-click in the colorwheel or a <return> in any of the
        Entry fields, and is passed the (r,g,b) tuple that describes the point
        clicked on the colorwheel (if it's been clicked) or the contents of the
        R/G/B Entry fields (if one of them has detected a <return> event).

        A click in the colorwheel always updates the contents of the R/G/B Entry
        fields.
    """

    def __init__(self, master,
                 command=lambda x: 0,
                 colorwheel_image="your gif or ppm here!",
                 label_text = None,
                 show_rgb_entries = 1,
                 rgb_initial_vals = {'r':0, 'g':0, 'b':0},
                 **kw ) :
        Tkinter.Frame.__init__(self,master,**kw)

        decls = anag_utils.Declarations( "decls",
            "self", "master", "command", "colorwheel_image", "label_text",
            "rgb_initial_vals",
            "show_rgb_entries", "kw",
            "self.command",
            "self.show_rgb_entries",
            "colorwheel_widget",
            "c",
            "rgb_frame",
            "rgb_subframe",
            "rgb_label",
            "self.rgb_entry"
            )
        self.decls = anag_utils.Declarations( "decls",
            "command", "show_rgb_entries", "the_image", "rgb_entry",
            instance=self
            )

        self.command = command
        self.show_rgb_entries = show_rgb_entries

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

        #
        # RGB coordinate Entry fields
        #
        if show_rgb_entries != 0:
            rgb_frame = Tkinter.Frame( self ) # Holds the three rgb_subframes.
            rgb_frame.pack()
    
            rgb_subframe = {} # Holds label ('R', 'G' or 'B') plus Entry field.
            rgb_label = {}
            self.rgb_entry = {}

            for c in 'r', 'g', 'b':
    
                rgb_subframe[c] = Tkinter.Frame( rgb_frame )
                rgb_subframe[c].pack(side='left')
    
                rgb_label[c] = Tkinter.Label( rgb_subframe[c],
                                              text = c.upper() )
                rgb_label[c].pack(side='top')
    
                self.rgb_entry[c] = Pmw.EntryField( 
                    rgb_subframe[c],
                    validate = {'validator':'real', 'min':0, 'max':1},
                    value = rgb_initial_vals[c])
                self.rgb_entry[c].component('entry').bind(
                    '<Return>', self._entryFieldHandler )
                self.rgb_entry[c].component('entry').configure( width=5 )
                self.rgb_entry[c].pack()


        decls.memberFunctionAudit( self )


    def setRgbEntries( self, rgb_tuple ):
        self.rgb_entry['r'].setentry( round(rgb_tuple[0],2) )
        self.rgb_entry['g'].setentry( round(rgb_tuple[1],2) )
        self.rgb_entry['b'].setentry( round(rgb_tuple[2],2) )


    def _colorWheelButtonHandler( self, event ) :
        """ Computes RGB coordinates corresponding to location of button click,
            and passes them to the user-definable callback ColorWheel.command.
        """
        decls = anag_utils.Declarations( "decls",
            "self", "event",
            "rgb_vect",
            "d",
            "coord",
            "rgb_list",
            "c",
            "x",
            "y"
            )

        rgb_vect = ''
        
        x = min(max(event.x-2,0),99)
        y = min(max(event.y-2,0),99)

        for d in self.the_image.get( x, y ) :
            rgb_vect = rgb_vect + d
        rgb_list = []
        for coord in rgb_vect.split( ' ' ) :
            rgb_list.append( string.atoi(coord)/255.0 )

        # Update the Entry fields.
        if self.show_rgb_entries != 0:
            self.rgb_entry['r'].setentry( round(rgb_list[0],2) )
            self.rgb_entry['g'].setentry( round(rgb_list[1],2) )
            self.rgb_entry['b'].setentry( round(rgb_list[2],2) )

        # Call the user-defined callback function.
        self.command( tuple(rgb_list) )

        decls.memberFunctionAudit( self )

    def _entryFieldHandler( self, event ):
        """ 
        Just like _colorWheelButtonHandler(), calls the user-defined  callback
        with an rgb tuple.  But this time the contents of the tuple reflect
        what was typed into the Entry field that registered this event.

        If contents of field isn't a valid number within [0.0,1.0] then
        pretend it was zero.
        """
        
        decls = anag_utils.Declarations( "decls", "event",
            "rgb_list", "c", "x" )

        rgb_list = []
        for c in 'r','g','b':
            try:
                x = float( self.rgb_entry[c].get() )
            except:
                x = 0.0
            x = min(max(x,0),1)
            self.rgb_entry[c].setentry( x )
            rgb_list.append( x )

        self.command( tuple(rgb_list) )
        decls.memberFunctionAudit( self )

# End of class ColorWheel


class RadioSelect( Pmw.RadioSelect ):
    """ Like Pmw.RadioSelect except it offers named constants equal to the
        button tags, for added safety.
    """

    class _ButtonNames:
        """ In add() we will insert attributes named for the tags.  Then
            client code will be able to say things like
            "if button_tag == constants.Foo:" (if one of the buttons has the
            label "Foo").
        """
        pass


    def __init__(self, parent, **kw ):
        Pmw.RadioSelect.__init__(self, parent, **kw)
        self.initialiseoptions(RadioSelect)
        # For justification of "initialiseoptions()", see
        #http://groups.google.com/groups?hl=en&safe=off&th=5d50ef2ddaf0cb8d,9&seekm=19971125000257.35549%40nms.otc.com.au#p

        self.decls = anag_utils.Declarations( "decls",
            "command",
            "butt_names",
            instance=self
        )
        self.butt_names = self._ButtonNames()

        if kw.has_key('command'): self._setCommand( kw['command'] )

    def configure( self, **kw ):
        if kw.has_key( 'state' ):
            for b in range(0,self.numbuttons()):
                self.button(b).configure( state = kw['state'] )
            return
        Pmw.RadioSelect.configure( self, **kw )
        if kw.has_key( 'command' ): self._setCommand( kw['command'] )

    def _setCommand( self, cmd ):
        self.command = cmd

    def add( self, button_label ):
        """ This is where we create the named constants.
            If the button labels contain characters that are not legal in
            a component name, we convert them to underbars in the attribute
            name (but we don't modify the text that will appear in the widget
            itself).  We don't pretend to handle all illegal characters or
            situations (e.g. leading character a numeral).  We just look for
            spaces, periods, commas, slashes and dashes.
         """
        decls = anag_utils.Declarations( "decls",
            "legal_button_label",
            "c",
            "button_label"
            )

        Pmw.RadioSelect.add( self, button_label )

        legal_button_label = []
        for c in button_label:
            if c in (' ', '.', ',', '/', '\\'):
                legal_button_label.append('_')
            else:
                legal_button_label.append(c)
        legal_button_label = string.join( legal_button_label, '' )
        if not legal_button_label[0].isalpha():
            legal_button_label = '_' + legal_button_label
        self.butt_names.__dict__[legal_button_label] = button_label
        decls.memberFunctionAudit( self )


    def setcurselection( self, button_label ):
        """ Like invoke(), except there's no callback. """
        temp = self.command # Need this temp, 'cuz self.command also becomes 0:
        self.configure( command = 0 )
        self.invoke( button_label )
        self.command = temp
        self.configure( command = self.command )

# End of class RadioSelect


class Checkbutton( Tkinter.Checkbutton ):
    """ Like Tkinter.Checkbutton but with extra functionality:
        get() and set() methods; no need to work through a Tkinter.*Var().

        State is 0 (off) or 1 (on).
    """

    def __init__(self, parent, **kw ):
        Tkinter.Checkbutton.__init__(self, parent, **kw)
        self.var = Tkinter.IntVar()
        self.decls = anag_utils.Declarations( "decls", "var", instance=self )
        Tkinter.Checkbutton.configure( self, variable = self.var, onvalue=1 )

    def set( self, x ):
        """ If x==1, then self's command callback is called. """
        decls = anag_utils.Declarations( "self", "x", "decls" )
        assert( x == 0  or  x == 1 )
        self.var.set( x )
        # Don't say "invoke" here; invoking from the 1 state turns it 0.
        #decls.memberFunctionAudit( self )

    def get( self ):
        return self.var.get()

# End of class Checkbutton


class EntryArray( Tkinter.Frame ):
    """ An n x 2 array of EntryFields that grows when you hit <Return>
        in the last one.

        In each pair, the left Entry must be an integer on [0,inf], the right
        Entry a real on [0,inf].

        This is only used once -- in control_vol.  It's thus fairly specialized
        to our needs there -- down to hardcoding the validate parameters of
        the EntryFields.
    """
    def __init__( self, master, label_text="", n_pairs=1, **kw ):
        Tkinter.Frame.__init__( self, master, **kw )

        if label_text != "":
            Tkinter.Label( self, text = label_text ).pack()

        self.n_pairs = 0 # Number of rows.  Gets incremented in newPair().
        self.pairs = [] # Array of pairs of EntryFields.
        self.row_frames = []
        for i in range(0,n_pairs):
            self.newPair()
        self.registeredEntryCallback = 0  # Function that can be set.


    def getNumPairs( self ):
        anag_utils.funcTrace()
        return self.n_pairs


    def isSaturated( self ):
        """
        Returns 1 if all the Entry widgets have something (other than '') in
        them.
        """
        for row in range(0, self.getNumPairs()):
            if(   (self.getElement(row,0).get() == '')
               or (self.getElement(row,1).get() == '') ):
                return 0
        return 1


    def getElement( self, i, j ):
        anag_utils.funcTrace()
        return self.pairs[i][j]


    def newPair( self ):
        """ Returns the new pair of EntryFields, as a list.
            Increments self.n_pairs.
        """
        pair = []  # Will hold a new pair of EntryFields.
        self.pairs.append( pair )
    
        row_frame = Tkinter.Frame( self )
        self.row_frames.append( row_frame )
        row_frame.pack()
    
        pair.append( Pmw.EntryField( 
                        row_frame,
                        validate = {'validator':'integer', 'min':0, 'max':255}))
        pair.append(
            Pmw.EntryField(
                row_frame,
                validate = {'validator':'real', 'min':0.0, 'max':1.0},
                command = lambda self=self, n=self.n_pairs:
                            self._entryHandler(n) ))
                # _entryHandler() will be called with the number of rows *now*.

        for i in (0,1):
            pair[i].pack( side='left' )
    
        self.n_pairs = self.n_pairs + 1
        return pair


    def deleteUnusedRows( self ):
        """
        If there are empty rows at the end, call forget on them.
        """
        for i in range(0, self.getNumPairs()):
            if(    (self.getElement( i, 0 ).get()=='')
               and (self.getElement( i, 1 ).get()=='') ):
                self.pairs[i][0].forget()
                self.pairs[i][1].forget()
                del self.pairs[i]
                self.row_frames[i].forget()
                del self.row_frames[i]
                self.n_pairs = self.n_pairs - 1
                


    def setText( self, str, i, j ): # Set the text in the ij-th Entry.
        anag_utils.funcTrace()
        self.getElement(i,j).setentry( str )


    def clear( self ):
        """ Eliminate all the entries. """
        anag_utils.funcTrace()

        for item in self.pairs:
            item[0].forget()
            item[1].forget()

        self.pairs = []
        self.n_pairs = 0

        for item in self.row_frames:
            item.forget()


    def setRegisteredEntryCallback( self, callback_function ):
        """
        An opportunity for clients to register a function that will get called
        anything the user expands the EntryArray by hitting "Return".
        The callback should be niladic.
        """
        self.registeredEntryCallback = callback_function


    #
    # Private interface below here.
    #

    def _entryHandler( self, row_index ):
        """ If this is in the last row, then create a new row.  Notice we only
            register this callback with the second Entry of each pair.
        """
        if (row_index == self.n_pairs - 1) and (self.isSaturated()==1):
            self.newPair()
            apply( self.registeredEntryCallback, () )


# End of class EntryArray


class Scale( Tkinter.Scale ):
    """
    Adds a setSterile() method.

    Button3 slews -- moves the slider without causing any callbacks until you
        release the mouse button..

    Users must not set the "command" option.  Instead, they should use
    setCallback().
    That's so we can disable the callback for the setSterile() method.
    """
    def __init__( self, master, callback=None, **kw ):
        """
        Arg **kw is passed to the Tkinter.Scale widget.
        """
        Tkinter.Scale.__init__( self, master, **kw )

        self.callback = callback
        self.configure( command = callback )

        self.discrete = 0

        self.bind('<B3-Motion>', self._slew)
        self.bind('<B3-ButtonRelease>', lambda e,self=self: self._b3handler(e))


    def setCallback( self, f ):
        self.configure( command = f )
        self.callback = f

    def discretize( self, on_off ):
        """ If set to 1, all the values are taken to be integers. """
        anag_utils.funcTrace()
        self.discrete = on_off

    def setSterile( self, pos ):
        self.configure( command=0 )
        if self.discrete == 1:
            self.set( int( pos + 0.5 ) )
        else:
            self.set( pos )
        self.configure( command = self.callback )


    def _b3handler( self, e ):
        """
        Button-3, the slew button
        """
        anag_utils.funcTrace()
        
        scale_min = float(self['from'])
        scale_max = float(self['to'])

        constrained_pos = max(scale_min, min(scale_max, self._scaledPos(e)))
        if float(self['from']) < constrained_pos:
            self.setSterile( constrained_pos - float(self['resolution']))
        else:
            self.setSterile( constrained_pos + float(self['resolution']))

        if self.discrete == 1:
            self.set( int(constrained_pos+0.5) )
        else:
            self.set( constrained_pos )

        # Gotta call this...
        apply( self.callback, (self.get(),) )
        # ...because for some strange reason, if you slew leftward but don't
        # move the mouse quite far enough, the scale looks updated but the
        # callback isn't called.  But of course now we'll get superfluous
        # callbacks.  To fix that, I've arranged for class EntryScale (which
        # is where this class, Entry, is almost always used) to keep track of
        # whether anything has changed, before invoking the callback.  See
        # the prev_scale_value variable in class EntryScale.


    def _scaledPos( self, event ):
        """
        For use by _b3handler().  Converts mouse's position in "world"
        coordinates to the bounds of the Scale widget.
        """
        anag_utils.funcTrace()
        
        if self['orient'] == 'horizontal':
            pos = event.x
        else:
            pos = event.y
        return float(self['from']) + float(pos) * \
            (float(self['to'])-float(self['from'])) / float(self['length'])


    def _slew( self, e ):
        """
        Move the slider, without invoking the command/callback at every point
        along the way.
        """
        anag_utils.funcTrace()

        # Disable the callback (which is what slewing is all about).  It would
        # be more efficient if we first checked to see if the callback has
        # already been disabled, as this function gets called many times as you
        # move the slider -- like ten times a second.
        self.configure( command=0 )

        # This part is to make the slider actually move:
        pos = self._scaledPos(e)
        if self.discrete == 1:
            self.set( int( pos + 0.5 ) )
        else:
            self.set( pos )

        # There's no need to reset the command to self.callback.  _b3handler()
        # will take care of that upon the button-up event.  In fact, if you do
        # reset to self.callback here, you'll be firing off the callback at
        # every point and you'll lose the whole slewing effect.


class Entry( Tkinter.Entry ):
    """
    Provides a set() method.
    """
    def __init__( self, master, **kw ):
        Tkinter.Entry.__init__( self, master, **kw )

    def set( self, text ):
        self.delete(0,Tkinter.END)
        self.insert(0, text)


class EntryScale( Tkinter.Frame, self_control.SelfControl ):
    """ 
    A Scale and a means to make it jump to an arbitrary point of arbitrary
    precision, without triggering the callback all along the way.
    That means is a button which, when pushed, swaps out the Scale and puts in
    its place an Entry.

    The state of the widget, as to whether the Scale or the Entry is visible,
    is persistent (i.e. 'save':1).
    """

    def __init__( self, master,
                  button_text=None,
                  **kw ):
        anag_utils.funcTrace()
        Tkinter.Frame.__init__( self, master )

        # Pass a dep_dict arg to the ctor if you want persistence:
        if kw.has_key( 'dep_dict' ):
            dep_dict = kw['dep_dict']
        else:
            dep_dict = {}

        self_control.SelfControl.__init__( self, dep_dict, [
            { 'name':'scale_is_visible', 'initval':1, 'set':2, 'get':1,
                'save':2 },
            { 'name':'slew_button' },
            { 'name':'scale' },
            { 'name':'scale_from'},
            { 'name':'scale_to'},
            { 'name':'scale_soft_min' },
            { 'name':'scale_soft_max' },
            { 'name':'scale_normal_resolution' },
            { 'name':'k_min_scale_resolution_order', 'initval':50}, # 1E-x
            { 'name':'scale_callback' },
            { 'name':'discrete' },
            { 'name':'legal_keys' },
            { 'name':'scale_frame'},
            { 'name':'flip_butt' },
            { 'name':'butt_image' },
            { 'name':'resolution_entry'},
            { 'name':'resolution_label'},
            { 'name':'resolution_frame'},
            { 'name':'scale_min_override_entry'},
            { 'name':'scale_min_override_label'},
            { 'name':'scale_min_override_frame'},
            { 'name':'scale_max_override_entry'},
            { 'name':'scale_max_override_label'},
            { 'name':'scale_max_override_frame'},
            { 'name':'value_entry' },
            { 'name':'dep_dict' },
            { 'name':'prev_scale_value'},
            { 'name':'no_validation'},
            { 'name':'orientation'}
        ] )

        self.decls = anag_utils.Declarations( 'decls', instance=self )

        self.legal_keys = (
            'from_', 'to',
            'scale_callback',
            'scale_normal_resolution',
            'scale_soft_min', 'scale_soft_max',
            'discrete', # If 1, then force resolution=1.
            'scale_is_visible', # 0 => default is entry mode
            'dep_dict',
            'orientation',
            'no_validation'  # 1 => allow any real number in Entry
            )

        self.scale_frame = Tkinter.Frame( self,
                               relief='groove', borderwidth=1 )
        self.scale_frame.columnconfigure(0,weight=10)

        self.resolution_frame = Tkinter.Frame( self.scale_frame)
        self.resolution_label = Tkinter.Label( self.resolution_frame,
            text='increment:' )
        self.resolution_label.pack(side='left')
        self.resolution_entry = Pmw.EntryField(
                                   self.resolution_frame,
                                   command=self._handleResolutionEntry )
        self.resolution_entry.component('entry').configure(width=15)
        self.resolution_entry.pack()

        self.scale_min_override_frame = Tkinter.Frame( self.scale_frame)
        self.scale_min_override_label = Tkinter.Label( self.scale_min_override_frame,
            text='min:' )
        self.scale_min_override_label.pack(side='left')
        self.scale_min_override_entry = Pmw.EntryField(
                                   self.scale_min_override_frame,
                                   command=self._handleScaleMinOverrideEntry )
        self.scale_min_override_entry.component('entry').configure(width=15)
        self.scale_min_override_entry.pack()

        self.scale_max_override_frame = Tkinter.Frame( self.scale_frame)
        self.scale_max_override_label = Tkinter.Label( self.scale_max_override_frame,
            text='max:' )
        self.scale_max_override_label.pack(side='left')
        self.scale_max_override_entry = Pmw.EntryField(
                                   self.scale_max_override_frame,
                                   command=self._handleScaleMaxOverrideEntry )
        self.scale_max_override_entry.component('entry').configure(width=15)
        self.scale_max_override_entry.pack()

        self.value_entry = Pmw.EntryField( self.scale_frame,
                                     command = self._valueEntryHandler )
        self.value_entry.component('entry').configure( width=14 )

        if 'orientation' in kw.keys():
            orientation = kw['orientation']
        else:
            orientation = 'horizontal'
        self.scale = Scale( 
                        self.scale_frame,
                        orient = orientation,
                        repeatdelay = 1000,
                        callback = lambda x, self=self: self._scaleHandler(x) )

        # Set defaults (go through self.legal_keys).
        self.scale_callback = None
        self.scale_normal_resolution = 1
        self.scale_soft_min = self.scale_soft_max = None
        self.discrete = 0
        self.scale_is_visible = 1
        self.configure( **kw )
        self.scale.discretize( self.discrete )

        if button_text:
            self.flip_butt = Tkinter.Button( self, text=button_text )
        else:
            self.butt_image = Tkinter.PhotoImage(
                file = os.getenv('CHOMBOVIS_HOME') + '/data/switch_arrows.gif' )
            # butt_image isn't used anywhere else, but if we don't make it an
            # instance variable ("self.") its reference gets lost.
            self.flip_butt = Tkinter.Button( self,
                                             image=self.butt_image )
        self.enableFlipButt()
        self.flip_butt.pack(side='left')

        self.scale_frame.pack(expand=1, fill=Tkinter.X, side='left')

        self.scale.pack()
        self.setScaleIsVisible(self.scale_is_visible)
        self.setScaleValue( self.getScaleValue() ) # initializes entry widget

        #print "multi_instance_qualifier=", self.multi_instance_qualifier
        self.multi_instance_qualifier =\
            self_control.generateMultiInstanceQualifier()


    def enableFlipButt( self ):
        self.flip_butt.bind('<Button-1>', lambda w, self=self: self.flip() )
        self.flip_butt.bind('<Shift-Button-1>', self._toggleFlipButt )
        self.flip_butt.configure( state='normal' )
    def disableFlipButt( self ):
        def no_op() : pass
        self.flip_butt.bind('<Button-1>', lambda w: no_op)
        self.flip_butt.bind('<Shift-Button-1>', no_op)
        self.flip_butt.configure( state='disabled' )


    def _toggleFlipButt( self, dummy ):
        anag_utils.funcTrace()
        if self.resolution_entry.winfo_ismapped():
            self.resolution_frame.grid_forget()
            self.scale_min_override_frame.grid_forget()
            self.scale_max_override_frame.grid_forget()
        else:
            self.resolution_frame.grid(row=1)
            self.scale_min_override_frame.grid(row=2)
            self.scale_max_override_frame.grid(row=3)


    def _handleResolutionEntry( self ):
        anag_utils.funcTrace()
        x = float(self.resolution_entry.get())
        self.scale_normal_resolution = x
        self.setScaleResolution( x )


    def _handleScaleMinOverrideEntry( self ):
        anag_utils.funcTrace()
        x = float(self.scale_min_override_entry.get())
        self.configure( from_ = x )

    def _handleScaleMaxOverrideEntry( self ):
        anag_utils.funcTrace()
        x = float(self.scale_max_override_entry.get())
        self.configure( to = x )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()
        self.setScaleIsVisible( self.scale_is_visible )

    def cleanup( self ):
        anag_utils.funcTrace()
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()        

    def _valueEntryHandler( self ):
        # Call the Scale's handler, then set the scale to the chosen value.
        # If the value's precision is beyond the Scale's current resolution,
        # the Scale will up its resolution accordingly.
        anag_utils.funcTrace()

        x = self.value_entry.get()
        if not self.no_validation:
            self.setScaleValue( x, override_scale_normal_resolution=1 )
        self._scaleHandler( x )


    def setScaleIsVisible( self, on_off ):
        """ If arg on_off==1, then show the scale.  If 0, show the entry. """
        if on_off == 1:
            self.value_entry.grid_forget()
            self.scale.grid(row=0,sticky=Tkinter.W+Tkinter.E)
            self.scale_is_visible = 1
        else:
            self.scale.grid_forget()
            self.value_entry.grid(row=0,sticky=Tkinter.W+Tkinter.E)
            self.scale_is_visible = 0

    def flip( self ):
        if self.scale_is_visible == 1:
            self.setScaleIsVisible( on_off = 0 )
        else:
            self.setScaleIsVisible( on_off = 1 )

    def configure( self, **kw ):
        anag_utils.funcTrace()
        if kw.has_key( 'state' ):
            self.scale.configure( state=kw['state'] )
            if( kw['state'] == 'normal' ):
                self.enableFlipButt()
            else:
                self.disableFlipButt()
            self.value_entry.component('entry').configure( state=kw['state'] )
        if kw.has_key( 'to' ):
            sign_to = int(kw['to'] > 0)*2 - 1
            if math.fabs(kw['to']) > pow(10,self.k_min_scale_resolution_order):
                kw['to'] = pow(10,self.k_min_scale_resolution_order) * sign_to
            self.scale_to = round(kw['to'], self.k_min_scale_resolution_order)
            # Gotta round; Tk scale widgets with more than 80 or so digits after
            # decimal point segfault.
            scale_resolution =\
                min( algorithms.findResolution( self.scale_to ),
                                                self.scale_normal_resolution )
            if self.discrete == 1:
                scale_resolution = 1
            self.setScaleResolution( scale_resolution )
        else:
            self.scale_to = float(self.scale['to'])
        if kw.has_key( 'from_' ):
            sign_from = int(kw['from_'] > 0)*2 - 1
            if math.fabs(kw['from_']) > pow(10,self.k_min_scale_resolution_order):
                kw['from_'] = pow(10,self.k_min_scale_resolution_order) * sign_from
            self.scale_from = round(kw['from_'], self.k_min_scale_resolution_order)

            scale_resolution =\
                min( algorithms.findResolution( self.scale_from),
                                                self.scale_normal_resolution )
            if self.discrete == 1:
                scale_resolution = 1
            self.setScaleResolution( scale_resolution )
        else:
            self.scale_from = float(self.scale['from'])

        self.scale_min_override_entry.setvalue( self.scale_from )
        self.scale_max_override_entry.setvalue( self.scale_to )

        if kw.has_key( 'scale_normal_resolution' ):
            self.setScaleResolution(
                max(
                    min( kw['scale_normal_resolution'],
                         self.scale['resolution'] ),
                    pow(10,-self.k_min_scale_resolution_order)))

        if kw.has_key( 'scale_soft_min' ):
            self.scale_soft_min = round( float(kw['scale_soft_min']),
                                         self.k_min_scale_resolution_order )
        if kw.has_key( 'scale_soft_max' ):
            self.scale_soft_max = round( float(kw['scale_soft_max']),
                                         self.k_min_scale_resolution_order )

        # Now deal with the keywords that we delegate to the superclass.
        for k in kw.keys():
            if k in self.legal_keys:
                self.__dict__[k] = kw[k]
            else:
                cmd = "self.scale.configure( " + k + "=kw['" + k + "'])"
                exec( cmd )

        # Sometimes (e.g. colormap ranges) you want to let the user type any
        # number in the Entry, even numbers outside the range.
        if not self.no_validation:
            if self.discrete == 1:
                self.value_entry.configure( 
                    validate={'validator':'integer',
                              'min':int(self.scale_from+0.5),
                              'max':int(self.scale_to+0.5),
                              'minstrict':0, 'maxstrict':0})
            else:
                self.value_entry.configure( 
                    validate={'validator':'real',
                              'min':self.scale_from,
                              'max':self.scale_to,
                              'minstrict':0, 'maxstrict':0 })
#       Weird! If from=2 and to=20, the validator rejects any first
#       digit '0' or '1', as though it suspects you're going to not type any
#       more digits.
#       Setting minstrict and maxstrict to 0 isn't perfect but together
#       with the "val = max(..." stuff above, it does the job.


    def getScaleValue( self ):
        anag_utils.funcTrace()
        return float(self.scale.get())


    def setScaleSterile( self, x ):
        """
        Sets scale without causing callback.  Useful in _refresh() methods.
        """
        anag_utils.funcTrace()
        scale_resolution = max(pow(10,-self.k_min_scale_resolution_order),
                               min( algorithms.findResolution( x ),
                                    self.scale_normal_resolution ))
        if self.discrete:
            scale_resolution = 1
        self.setScaleResolution( scale_resolution )
        self.scale.setSterile( round(x,self.k_min_scale_resolution_order) )
        if self.discrete == 1:
            self.value_entry.setentry( '%g' % int(x+0.5) )
        else:
            self.value_entry.setentry(
                '%g' % float(round(x,self.k_min_scale_resolution_order)))


    def setScaleValue( self, val, override_scale_normal_resolution=None ):
        """
        This one is trickier.  Arg val may be specified to a precision beyond
        the Scale's current resolution.  If so, we up the resolution.

        Calling function is responsible for checking min and max constraints.

        As long as arg override_scale_normal_resolution==None, we set the scale
        value to a multiple of self.scale_normal_resolution.
        """
        anag_utils.funcTrace()

        # Check for valid input.
        try:
            fval = float(val)
        except ValueError:
            anag_utils.error( val, ' is not a valid float value!' )
            return

        # Reject extremely small or large values; Tk scales segfault at about
        # 1E-90 or 1E90.
        if fval != 0:
            if abs(fval) < math.pow(10,-self.k_min_scale_resolution_order):
                fval = pow(10,-self.k_min_scale_resolution_order) \
                     * fval/abs(fval)
            if abs(fval) > math.pow(10,+self.k_min_scale_resolution_order):
                fval = pow(10,+self.k_min_scale_resolution_order) \
                     * fval/abs(fval)

        # Apply min/max constraints, but only if we're in scale mode.  When in
        # entry mode, we allow anything.
        if self.scale_is_visible == 1:
            if self.scale_soft_min == None:
                constrained_min = round( self.scale_from,
                                         self.k_min_scale_resolution_order )
            else:
                constrained_min = self.scale_soft_min
            if self.scale_soft_max == None:
                constrained_max = round( self.scale_to,
                                         self.k_min_scale_resolution_order )
            else:
                constrained_max = self.scale_soft_max
            constrained_fval = min( constrained_max,
                                   max( fval,
                                        max( self.scale_from,
                                             constrained_min)))
        else:
            constrained_fval = fval

        if override_scale_normal_resolution:
            scale_resolution = algorithms.findResolution( constrained_fval )
        else:
            scale_resolution =\
                min( algorithms.findResolution( constrained_fval ),
                     self.scale_normal_resolution )

        if self.discrete:
            scale_resolution = 1
        self.setScaleResolution( scale_resolution )

        self.scale.set( constrained_fval )
        if self.scale.get() != constrained_fval:
            self.setScaleResolution( min(
                algorithms.findResolution( constrained_fval ),
                self.scale_normal_resolution ) )

        self.value_entry.configure( validate={'validator':'real'} ) 
            # That was just to avoid a beep.  In configure(), we apply suitable
            # further restrictions.
        if self.discrete == 1:
            self.value_entry.setentry( '%g' % int(constrained_fval+0.5) )
        else:
            format = '%.' + str(len(str(constrained_fval))) + 'g'
            self.value_entry.setentry( format % constrained_fval )


    def setScaleResolution( self, r ):
        """
        Don't call self.scale.configure() directly, as we need to also set the
        contents of the resolution entry.
        """
        anag_utils.funcTrace()
        self.scale.configure( resolution = r )
        self.resolution_entry.setvalue(str(r))

        # Reset the scale's ends to what they were configured as.  This is
        # necessary because Tk rounds them to whatever the current resolution
        # is.
        self.scale.configure( from_ = self.scale_from )
        self.scale.configure( to = self.scale_to )


    def _scaleHandler( self, x ):
        """ After some preliminaries, invokes the callback assigned to the
            EntryScale constructor's "scale_callback" attribute.
        """
        anag_utils.funcTrace()

        # Check if x is different from the last value at which this handler was
        # called.  But only for discrete scales.  For float scales it's been
        # too tricky, especially at zero, and there doesn't seem to be any win.
        if self.discrete:
            if( (self.prev_scale_value != None )
            and (int(x) == self.prev_scale_value) ):
                return
            else:
                self.prev_scale_value = int(x)
        else:
            self.prev_scale_value = float(x)


        # Deregister this handler, so the user doesn't confuse himself when
        # it takes a long time to drag the slider around.  Change the color too,
        # to make it really obvious.
        self.scale.setCallback( 0 )
        bgsave = self.scale['bg']
        self.scale.configure( bg='#FF0000' )
        # This color-setting doesn't work.  It's something about thread priority
        # maybe.  Anyway, even when it takes 5 seconds to execute the callback,
        # we still don't see any change in the color.  Doesn't work if you try
        # fg instead of bg color either.

        if( self.no_validation
        and (  (float(x) > float(self.scale_to))
            or (float(x) < float(self.scale_from)))):
            constrained_x = float(x)
        else:   # Bound x within scale min/max.
            self.setScaleValue( x, override_scale_normal_resolution=None )
            constrained_x = float(self.getScaleValue())
        if self.scale_callback != None:
            apply( self.scale_callback, (constrained_x,) )

        # Reregister:
        self.scale.setCallback( lambda x, self=self: self._scaleHandler(x) )
        self.scale.configure( bg=bgsave )


class HorizRule( Tkinter.Frame ):
    """ A horizontal rule """
    def __init__(self, master, width):
        Tkinter.Frame.__init__( self, master )
        Tkinter.Canvas( self, height=1, width=width,
                        relief='groove', borderwidth=1 ).pack()


class SelfDescribingDialog( Pmw.Dialog ):
    """
    All the vis control panels derive from this -- ControlGrid, ControlIso, etc.

    It's self-describing through the two get*Description() methods.  These are
    for use by the MenuBar object -- for menu options and for status-bar help.

    This is an "abstract base class"; its get*Description() methods call
    anag_utils.fatal().
    """
    def __init__( self, master=None, **kw ):
        anag_utils.funcTrace()
        Pmw.Dialog.__init__( self, master, **kw )
        self.initialiseoptions(Pmw.Dialog)
        self.configure( buttons=('close',), command=self._buttonHandler )

        #self.bind('<Key>', self._handleKeyPress )
        self.prev_key = None

        self.withdrawGUI()

        
    def getShortDescription( self ):
        """ Suitable in length for a menu option -- about 10 characters max """
        anag_utils.fatal(
            "You must redefine SelfDescribingDialog.getShortDescription()" )
    def getLongDescription( self ):
        """
        Suitable in length for a status-bar description -- up to about 80 chars.
        """
        anag_utils.fatal(
            "You must redefine SelfDescribingDialog.getLongDescription()" )


    """
    Functions showGUI() and withdrawGUI() set self.show_dialog, which is a
    variable
    defined in the subclasses.  We define it in the subclasses because we want
    it to be saved (i.e. 'save':1).  I don't want to do that here because then
    this class would need to derive from SelfControl, and yet all its subclasses
    (control_cmap, control_iso, ...) themselves derive from SelfControl, and I
    don't want to get involved in "virtual base class" issues.
    """
    def showGUI( self ):
        anag_utils.funcTrace()
        self.show_dialog = 1
        Pmw.Dialog.show( self )


    def withdrawGUI( self ):
        anag_utils.funcTrace()
        self.show_dialog = 0
        Pmw.Dialog.withdraw( self )

    # There's a danger that these will be called; they're already defined on
    # the Dialog superclass.  When they're called, they don't set
    # self.show_dialog.
    def show( self ):
        assert(0)
    def withdraw( self ):
        assert(0)


    def restoreDialogVisibility( self ):
        """
        Meant to be called from all the subclasses' _restore() methods --
        calls showGUI() or withdrawGUI() on this dialog, according to the state
        that was saved.
        """
        anag_utils.funcTrace()
        if self.show_dialog == 0:
            self.withdrawGUI()
        else:
            self.showGUI()


    def _handleKeyPress( self, event ):
        """
        Exit Chombovis, on 'q'.
        """
    #    print "***", event.keysym
    #    if self.prev_key == None and event.keysym == 'Alt_L':
    #        self.prev_key = 'Alt_L'
    #    if self.prev_key == 'Alt_L' and event.keysym == 'x':
    #        sys.exit(0)
    # FIXME: This approach to identifying alt-q doesn't work consistently.
    #        Oh well, people asked for a single keystroke anyway.
        if event.keysym == 'q':
            sys.exit(0)


    def _buttonHandler( self, button_name ):
        """
        There's only one button -- 'close'.  But if we don't provide a handler
        for it, it'll just call Tkinter.Dialog.withdraw(), and that doesn't
        update our self.show_dialog.
        """
        anag_utils.funcTrace()
        self.withdrawGUI()


    def unitTest( self ):
        """
        OK to override this, though as a basic test of a GUI control panel,
        self.show() isn't a bad start.
        If you get the warning about
        "unitTest():You must redefine this function in every class", then make
        sure you have SelfDescribingDialog ahead of SelfControl in the list of
        base classes.
        """
        self.showGUI()


class OptionMenu( Pmw.OptionMenu ):
    """
    To Pmw.OptionMenu, adds a setcurselection() method that, unlike invoke(),
    doesn't trigger the callback.
    """
    def __init__( self, master, **kw ):
        Pmw.OptionMenu.__init__( self, master, **kw )
        self.initialiseoptions( Pmw.OptionMenu )
        self.var = Tkinter.StringVar()
        self.var.set( self.getcurselection() )
        self.configure( menubutton_textvariable = self.var )

    def setcurselection( self, tag ):
        self.var.set( tag )


class ComponentSelectionLists( Tkinter.Frame ):
    """
    A collection of Pmw.ScrolledListBox's.  Useful for selection data components
    for various vector visualizations.
    """
    def __init__( self,
                  master_frame, # Within which we pack this thing.
                  orientation, # 'horizontal' or 'vertical'.
                  main_label_text,  # Appears at top of entire thing.
                  num_lists,
                  list_items,  # E.g. data component names.
                  list_label_texts=None, # A list, of size arg num_lists.
                  max_visible_list_items=3, # If more, we get a scrollbar.
                  initial_selections=None, #Item to be highlighted in each list.
                  callback=None,  # See def _callbackDriver() below.
                  button_text='open',
                  button_callback=None, # See def packhide() below.
                  is_expanded=0  # If 1, list shows.  If 0, not.
                ):
        Tkinter.Frame.__init__( self, master_frame )
        anag_utils.funcTrace()

        Tkinter.Label( self, text=main_label_text ).pack()

        self.max_visible_list_items = max_visible_list_items

        # Press button to, alternately, pack or hide the rest of this widget.
        label_checkbutton_frame = Tkinter.Frame(self)
        label_checkbutton_frame.pack(anchor='nw')
        self.button_callback = button_callback
        self.packhide_button = Checkbutton( label_checkbutton_frame,
                                            text=button_text )
        self.packhide_button.configure(
            command = lambda butt = self.packhide_button, self=self:
                self._packhide( butt.get() ))
        self.packhide_button.pack(side='left', anchor='nw')
        self.moststuff_frame = Tkinter.Frame(self)
        self.moststuff_frame.pack(side='top')

        self.list_items = list_items
        list_list_frame = Tkinter.Frame( self.moststuff_frame )

        self.list_list = []
        for i in range(0,num_lists):
            if list_label_texts == None:
                item = Pmw.ScrolledListBox( self.moststuff_frame )
            else:
                item = Pmw.ScrolledListBox( self.moststuff_frame,
                                            labelpos = 'n',
                                            label_text = list_label_texts[i] )
            self.list_list.insert( i, item )
            item.component('listbox').configure(
                height = self.max_visible_list_items, exportselection = 0 )
            item.setlist( list_items )
            if initial_selections:
                self.setSelections( initial_selections )

            if callback:
                item.configure( selectioncommand =
                    lambda i=i, f=callback, widget=item, self=self:
                        self._callbackDriver( f, widget.getcurselection()[0],i))

            if orientation == 'horizontal':
                item.pack( side='left' )
            else:
                item.pack( side='top' )

        if is_expanded == 1:
            self.packhide_button.invoke()
        else:
            self._packhide( 0 )


    def disable( self ):
        anag_utils.funcTrace()
        # Until Tk8.4, there's no configure(state='disabled') for Tk.ListBox.
        try:
            for item in self.list_list:
                item.component('listbox').configure(state='disabled')
        except:
            self.packhide_button.set(0)
            self._packhide( 0 )
        self.packhide_button.configure( state='disabled' )


    def enable( self ):
        anag_utils.funcTrace()
        self.packhide_button.configure( state='normal' )
        # Until Tk8.4, there's no configure(state='normal') for Tk.ListBox.
        try:
            for item in self.list_list:
                item.component('listbox').configure(state='normal')
        except:
            if self.packhide_button.get() == 1:
                self._packhide( 1 )


    def configure( self, **kw ):
        anag_utils.funcTrace()
        if kw.has_key( 'button_callback' ):
            self.button_callback = kw['button_callback']


    def _packhide( self, yes_no ):
        """
        Toggle the state -- packed or hidden -- of this widget.
        Execute self.button_callback() (which would have been set in __init__()
        or in configure().
        """
        anag_utils.funcTrace()
        assert( yes_no==0  or  yes_no==1 )
        if yes_no == 0:
            self.moststuff_frame.forget()
        else:
            self.moststuff_frame.pack()
        if self.button_callback:
            self.button_callback( yes_no )


    def setButton( self, on_off ):
        """
        Without triggering any callbacks.
        """
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        self.packhide_button.set(on_off)
        self._packhide(on_off) # Don't know why this is necessary, but it is.


    def _callbackDriver( self, callback, selected_item, list_index ):
        """
        Registered as the "selectioncommand" of each Pmw.ScrolledListBox.
        Arg callback is a user-supplied function to which we pass the other
        two args.  Arg selected item is a string -- the highlighted item of
        the ScrolledListBox we're calling this function on.  Arg list_index is
        the index (0,...) that identifies that ScrolledListBox, i.e. the element
        of self.list_list.
        """
        anag_utils.funcTrace()
        apply( callback, (selected_item, list_index) )


    def setSelections( self, selections_list ):
        """
        Highlight, in each ScrolledListBox, the corresponding element of arg
        selections_list.
        """
        anag_utils.funcTrace()
        i=0
        for item in self.list_list:
            cur_selection = item.component('listbox').curselection()
            if cur_selection != ():
                item.component('listbox').select_clear( cur_selection[0] )
            selection_number= (list(self.list_items)).index(selections_list[i])
            item.select_set( selection_number ) 
            i = i+1

    def getSelections( self ):
        """
        Returns a tuple -- one item for each element of self.list_list -- of
        tuples -- the selected elements of that list.
        """
        anag_utils.funcTrace()
        result = []
        for item in self.list_list:
            str_selection = item.component('listbox').curselection()
            selection = []
            for s in str_selection:
                selection.append(int(s)) 
            result.append( selection )
        return tuple(result)


    def appendNewComponentName( self, compname ):
        for item in self.list_list:
            appendItemToPmwScrolledListBox( compname, item,
                                            self.max_visible_list_items )


class LevelWidget( Tkinter.Frame, self_control.SelfControl ):
    """ Control over the number of visible levels of refinement.
        Contains two Scales -- min and max.

        The public interface consists of getters for the state of the two
        Scales.

        The "min" Scale starts at 0, "max" starts at 
            self.vtk_data.getReader().GetNumLevels() - 1.
    """

    def __init__( self, master, dep_dict, **kw ):
        Tkinter.Frame.__init__(self,master,**kw)
        self_control.SelfControl.__init__( self, dep_dict,
          [
            {'name':'min_scale'},
            {'name':'max_scale'}
          ])

        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "self", "master", "dep_dict",
            "kw", "w" )


        Tkinter.Label( self, text="Visible levels:" ).pack(anchor=Tkinter.W)

        self.min_scale = EntryScale( self, length=120,
            discrete=1,
            button_text = "min",
            dep_dict = {'saved_states':self.saved_states},
            scale_callback = lambda x,
                               self=self:self._slideHandler(int(x), 'min') )
        self.min_scale.configure(
            from_ = self.vtk_data.getMinAvailableLevel(),
            to = self.vtk_data.getMaxAvailableLevel(),
            resolution=1 )
        self.min_scale.setScaleValue( self.vtk_data.getMinVisibleLevel() )

        self.max_scale = EntryScale( self, length=120,
            discrete=1,
            button_text = "max",
            dep_dict = {'saved_states':self.saved_states},
            scale_callback = lambda x,
                               self=self:self._slideHandler(int(x), 'max') )
        self.max_scale.configure(
            from_ = self.vtk_data.getMinAvailableLevel(),
            to = self.vtk_data.getMaxAvailableLevel(),
            resolution=1 )
        self.max_scale.setScaleValue( self.vtk_data.getMaxVisibleLevel() )


        for w in (self.min_scale, self.max_scale):
            w.pack( padx=20, anchor=Tkinter.W )

        decls.memberFunctionAudit( self )
      
    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()
        self.min_scale.setScaleValue( self.min_scale.getScaleValue() )
        self.max_scale.setScaleValue( self.max_scale.getScaleValue() )

    def setMinNoCallback( self, x ):
        """ Move the min scale to x, but without triggering the callback. """
        anag_utils.funcTrace()
        self.min_scale.setScaleSterile( x )
    def setMaxNoCallback( self, x ):
        """ Move the max scale to x, but without triggering the callback. """
        anag_utils.funcTrace()
        self.max_scale.setScaleSterile( x )

    #
    # No public interfaces below here...
    #


    #
    # Handler for the Scales.
    #
    def _slideHandler( self, lev, id ):
        """ 
        Callback handler for the min and max Scales.
        Arg lev is the level.
        Arg id is "min" or "max", i.e. it identifies the Slide.

        We don't worry about imposing the min<=max constraint; it's taken care
        of in vtk_self.  vtk_self is a class instance passed in through the
        dep_data arg to the constructor.  Whereas vtk_data refers to vtk_data
        itself, vtk_self refers to the vtk_ counterpart of the class that
        constructs for itself this LevelWidget -- control_vol, control_cmap,
        as well as control_data.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self", "lev", "id" )
        assert( id == "min"  or  id == "max" )

        if id == "min":
            if self.vtk_self.getMinVisibleLevel() == lev:
                return
            else:
                self.vtk_self.setMinVisibleLevel( lev ) # imposes constraints
                self.min_scale.setScaleValue(self.vtk_self.getMinVisibleLevel() )
                self.max_scale.setScaleValue(self.vtk_self.getMaxVisibleLevel() )
        else:
            if self.vtk_self.getMaxVisibleLevel() == lev:
                return
            else:
                self.vtk_self.setMaxVisibleLevel( lev ) # imposes constraints
                self.max_scale.setScaleValue(self.vtk_self.getMaxVisibleLevel() )
                self.min_scale.setScaleValue(self.vtk_self.getMinVisibleLevel() )

        decls.memberFunctionAudit( self )

#End of class LevelWidget


def appendItemToPmwScrolledListBox( new_item, scrolled_list_box, max_size ):
    """
    Add an item to the widget, while retaining the current selection.
    """
    slb = scrolled_list_box

    cur_selection = slb.component('listbox').curselection()
    if cur_selection != ():
        slb.component('listbox').select_clear( cur_selection[0] )
    all_items = slb.get()
    slb.clear()
    slb.setlist( all_items + (new_item,) )
    if cur_selection != ():
        slb.select_set( cur_selection ) 
    slb.component('listbox').configure(
        height = min(max_size, len(all_items)+1))


def showChildren( widget, restrict_to_class = None, class_hooks = None ):
    """
    Display the tree of child widgets.  
    Arg recursion_depth should be 0 
    Set optional arg to a list of class types, for example, (Tkinter.Scale,),
        and function only prints those types of widgets.
    Optional argument class_hooks, if used, should be a list of functions
        corresponding to each class mentioned in restrict_to_class.  When we
        come to a class in restrict_to_class, we pass the instance to the
        corresponding function in class_hooks.
    """
    _showChildren( widget, 0, restrict_to_class, class_hooks )

def _showChildren( widget, recursion_depth, restrict_to_class, class_hooks ):
    """
    "Private" implementation of showChildren().
    """
    for c in widget.winfo_children():
        if (not restrict_to_class) or (c.__class__ in restrict_to_class):
            anag_utils.warning( recursion_depth*'  ', c.__class__ )
            i = (list(restrict_to_class)).index(c.__class__)
            if class_hooks:
                class_hooks[i](c)
        else:
            anag_utils.warning( recursion_depth*'xx', c.__class__ )
        _showChildren(c, recursion_depth+1, restrict_to_class, class_hooks)

    

g_decls = anag_utils.Declarations( "g_decls", "root", "ss" )


######################################################
if( __name__ == "__main__" ) :


    g_decls = anag_utils.Declarations( 'g_decls',
        'ea', 'en', 'rs', 'root', 'ss', 'ss2', 'chkbutt', 'slb', 'es')
    anag_utils.setDebugLevel(4)

    """ Unit tests """

    root = Tkinter.Tk()

    """ Unit test for Entry """
    en = Entry( root )
    en.pack()
    en.set( 'foo' )
    en.set( 'bar' )
    assert( en.get() == 'bar' )

    """ Unit test for ScrolledListBox """
#    slb = ScrolledListBox( root,
#                           items = ('foo','bar','baz') )
#    slb.pack()
#    slb.component('listbox').activate(1)
#    anag_utils.info( 'getcurselection=', slb.getcurselection() )


    """ Unit test for EntryScale """
    anag_utils.info( 'Unit test for EntryScale class...' )

    def slewscaleFunc(x):
        anag_utils.funcTrace()
        print x

    ssTTPro207 = EntryScale( root,
                    from_ = 0.0,
                    to = 8.0,
                    scale_normal_resolution = 0.1,
                    length = 100,
                    scale_callback = lambda x: slewscaleFunc(x),
                    button_text='TTPro207',
                    relief=Tkinter.GROOVE, borderwidth=2
                  )
    ssTTPro207.setScaleValue( 0.001 )
    ssTTPro207.pack()
                        
    ssTTPro338 = EntryScale( root,
                    from_  = 0.0,
                    to     = 30.0,
                    scale_normal_resolution = 0.1,
                    length = 100,
                    scale_callback = lambda x: slewscaleFunc(x),
                    button_text='TTPro338',
                    relief=Tkinter.GROOVE, borderwidth=2
                  )
    ssTTPro338.setScaleValue( 15.0 )
    ssTTPro338.pack()


    ss2 = EntryScale( root, discrete = 1, button_text='integers',
                      from_=1, to=7 )
    ss2.pack()


    """ Unit test for EntryArray class """
    anag_utils.info( 'Unit test for EntryArray class...' )
    ea = EntryArray( root, label_text='An EntryArray', n_pairs=5 )
    ea.pack()
    print 'ea.getNumPairs()=',ea.getNumPairs()
    assert( ea.getNumPairs() == 5 )
    assert( type(ea.getElement(4,0)) == type(Pmw.EntryField(root)) )
    ea.setText( '2', 4,0 )
    ea.setText( '0.718', 4,1 )


    """ Unit test for RadioSelect class.
        We can test it automatically; we just check if the named constants
        work.
    """
    anag_utils.info( 'Unit test for RadioSelect class...' )
    rs = RadioSelect( parent=root, 
                      orient='vertical',
                      buttontype='radiobutton' )
    rs.pack()
    rs.add( 'Space Button' )
    rs.add( 'BarButton' )
    rs.add( 'Dot.Button' )
    rs.add( 'Comma, Button' )
    rs.add( 'Slash/Button' )
    rs.add( 'BSlash\\Button' )
    try:
        rs.invoke( rs.butt_names.Space_Button )
        rs.invoke( rs.butt_names.BarButton )
        rs.invoke( rs.butt_names.Dot_Button )
        rs.invoke( rs.butt_names.Comma__Button )
        rs.invoke( rs.butt_names.Slash_Button )
        rs.invoke( rs.butt_names.BSlash_Button )
    except AttributeError:
        anag_utils.error( 'RadioSelect test failed: probably bad add() method' )
    except:
        anag_utils.excepthook()
    if rs.getcurselection() != rs.butt_names.BSlash_Button:
        anag_utils.error( 'RadioSelect.getcurselection() failed.' )



    """ Unit test for Checkbutton class """
    chkbutt = Checkbutton( parent=root, text="anag_megawidgets Checkbutton" )
    chkbutt.set( 1 )
    chkbutt.pack()


    """ Unit test for ColorWheel class.

        We can't detect correct behavior automatically; you have to look at
        it.

        What you should see is four ColorWheel objects.  
        The first has no label, no R/G/B Entry fields, and does not respond
        to mouse clicks.
        The second has a label "a colorwheel", no R/G/B Entry fields, but does
        respond to mouse clicks (by printing the R/G/B coordinates).
        The third has no label but does have R/G/B Entry fields and does respond
        to mouse clicks and <Return> events in the the Entry fields.
        The fourth is like the third, except its only response to events is
        to update the Entry fields upon a click in the colorwheel image.
    """
    anag_utils.info( "Unit test for ColorWheel class..." )
    data_dir = '../data'
    """
    ColorWheel( 
        master=root,
        colorwheel_image=data_dir + '/ColorWheel.ppm',
        show_rgb_entries=0,
        relief=Tkinter.GROOVE, borderwidth=2
         ).pack( pady=4 )

    ColorWheel( 
        master=root, 
        command=lambda rgb_tuple: anag_utils.info( "rgb_tuple=", 
                                                   str(rgb_tuple)),
        colorwheel_image=data_dir + '/ColorWheel.ppm',
        show_rgb_entries=0,
        label_text="a colorwheel",
        relief=Tkinter.GROOVE, borderwidth=2
         ).pack( pady=4 )

    ColorWheel( 
        master=root, 
        command=lambda rgb_tuple: anag_utils.info( "rgb_tuple=", 
                                                   str(rgb_tuple)),
        colorwheel_image=data_dir + '/ColorWheel.ppm',
        relief=Tkinter.GROOVE, borderwidth=2
         ).pack( pady=4 )
    """

    ColorWheel( 
        master=root, 
        colorwheel_image=data_dir + '/ColorWheel.ppm',
        relief=Tkinter.GROOVE, borderwidth=2
         ).pack( pady=4 )

    root.mainloop()
