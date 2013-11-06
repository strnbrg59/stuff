import sys
import Tkinter
import plots
import adc_control

try:
    import anag_megawidgets
except:
    cerr("Failed to import anag_megawidgets.  Set PYTHONPATH accordingly.")
    sys.exit(1)


class MinMaxScales(Tkinter.Frame):
    def __init__(self, master, minval, maxval, discrete,
                 extra_callback, **kw):
        Tkinter.Frame.__init__( self, master, **kw)
        self.legal_range = (minval, maxval)
        self.range_val = 0.0
        self.range_lock = False
        self.extra_callback = extra_callback
        self.oldMin = self.oldMax = -9E100;

        if discrete: scale_normal_resolution=1
        else:        scale_normal_resolution=4

        self.min_scale = anag_megawidgets.EntryScale(
            self,
            length=102,
            button_text = 'Min',
            scale_callback = self._minScaleHandler,
            dep_dict = {},
            no_validation=1,
            scale_normal_resolution=scale_normal_resolution,
            discrete=discrete
            )
        self.min_scale.configure(from_=minval, to=maxval)
        self.min_scale.setScaleValue(minval)

        self.max_scale = anag_megawidgets.EntryScale(
            self,
            length=102,
            button_text='Max',
            scale_callback = self._maxScaleHandler,
            dep_dict = {},
            no_validation=1,
            scale_normal_resolution=scale_normal_resolution,
            discrete=discrete
            )
        self.max_scale.configure(from_=minval, to=maxval)
        self.max_scale.setScaleValue(maxval)

        self.range_lock_button = anag_megawidgets.Checkbutton(
            self,
            command = self._rangeLockHandler,
            text = 'range lock' )

        self.min_scale.pack(expand=1,fill=Tkinter.X, padx=20, anchor=Tkinter.W);
        self.max_scale.pack(expand=1,fill=Tkinter.X, anchor=Tkinter.W, padx=20);
        self.range_lock_button.pack( anchor=Tkinter.W, padx=20)


    def getScaleValues(self):
        return (self.min_scale.getScaleValue(), self.max_scale.getScaleValue())


    def _minScaleHandler(self, val):        
        if self.min_scale.getScaleValue() == self.oldMin:
            return

        self.oldMin = self.min_scale.getScaleValue()
        if self.range_lock == False:
            self.range_val =\
                self.max_scale.getScaleValue() - self.min_scale.getScaleValue()
        else:
            self.max_scale.setScaleValue(
                self.min_scale.getScaleValue() + self.range_val)
        self.extra_callback(self.getScaleValues())

    def _maxScaleHandler(self, val):        
        if self.max_scale.getScaleValue() == self.oldMax:
            return

        self.oldMax = self.max_scale.getScaleValue()
        if self.range_lock == False:
            self.range_val =\
                self.max_scale.getScaleValue() - self.min_scale.getScaleValue()
        else:
            self.min_scale.setScaleValue(
                self.max_scale.getScaleValue() - self.range_val)
        self.extra_callback(self.getScaleValues())


    def _rangeLockHandler(self):
        self.range_lock = self.range_lock_button.get()


class Gui:
    def __init__(self, infilename=None):
        root = Tkinter.Tk()
        root.title("static oscilloscope")
        Tkinter.Label(root,text='X').pack()
        self.mmx_frame = Tkinter.Frame(root)
        self.mmx_frame.pack()
        anag_megawidgets.HorizRule(root, 200).pack()
        Tkinter.Label(root,text='Y').pack()
        self.mmy_frame = Tkinter.Frame(root)
        self.mmy_frame.pack()

        if infilename:
            root.title(infilename)
            self.plotFile(infilename)
        else:
            anag_megawidgets.HorizRule(root, 200).pack()
            self.n_channels=1
            self.n_channels_scale = Tkinter.Scale(root, label='n_channels',
                command = self.setNChannels, orient='horizontal',
                from_=1, to=4)
            self.n_channels_scale.pack()

            anag_megawidgets.HorizRule(root, 200).pack()
            self.delay_scale = anag_megawidgets.EntryScale(
                root,
                length=102,
                button_text='log_2(delay_cycles)',
                dep_dict = {},
                no_validation=1,
                discrete=1
                )
            self.delay_scale.configure(from_=0, to=16)
            self.delay_scale.setScaleValue(0)
            self.delay_scale.pack()
            anag_megawidgets.HorizRule(root, 200).pack()

            start_stop_butt_frame = Tkinter.Frame(root)
            self.start_butt = Tkinter.Button(start_stop_butt_frame,
                text='start adc', command = self.start)
            self.start_butt.pack(side='left')
            self.stop_butt = Tkinter.Button(start_stop_butt_frame,
                text='stop adc', command=self.stopAndPlot)
            self.stop_butt.pack()
            start_stop_butt_frame.pack()
        
        anag_megawidgets.HorizRule(root, 200).pack()
        self.extra_plot_butt = Tkinter.Button(root, text='extra plot',
            command = self.plotFile, state='disabled')
        self.extra_plot_butt.pack()

        root.mainloop()


    def start(self):
        self.start_butt.configure(state = 'disabled')
        self.stop_butt.configure(state = 'normal')
        self.n_channels_scale.configure(state = 'disabled')
        self.delay_scale.configure(state = 'disabled')
        adc_control.start(self.n_channels, int(self.delay_scale.getScaleValue()))


    def stopAndPlot(self):
        #self.start_butt.configure(state = 'normal')
        self.stop_butt.configure(state = 'disabled')
        adc_control.stop()
        plotfilename = adc_control.makeGnuplotFile()
        self.plotFile(plotfilename)
        self.extra_plot_butt.configure(state='normal')


    def plotFile(self, infilename=None):
        if infilename:
            self.infilename = infilename
        else:
            infilename = self.infilename

        (gp, xrange, yrange, extra_xscales_callback, extra_yscales_callback) =\
            plots.display(infilename)
        mmx = MinMaxScales(self.mmx_frame, xrange[0], xrange[1], False,
                           extra_xscales_callback)
        mmx.pack()
        mmy = MinMaxScales(self.mmy_frame, yrange[0], yrange[1], False,
                           extra_yscales_callback)
        mmy.pack()


    def setNChannels(self, n):
        self.n_channels = int(n)


if __name__ == "__main__":
    if len(sys.argv) == 2:
        Gui(sys.argv[1])
    else:
        Gui()
