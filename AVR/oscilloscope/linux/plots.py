"""
Using gnuplot to display captured adc data.
"""
import sys
import math
import Gnuplot

def cerr(msg):
    sys.stderr.write(msg+'\n')


def simulateData(outfilename):
    outfile = open(outfilename, "w")
    def myfunc(x):
        return int(((math.sin(x/10.0) + math.cos(1+x/10.0*2)) + 2) * 63)
    for t in range(0,360):
        outfile.write(str(t) + " " + str(myfunc(t)) + '\n')
    outfile.close()


def loadData(infilename):
    """
    First line: "channels = <n>".

    Subsequent lines: two columns of numbers, with blank lines separating one
    batch of readings from one channel, and the vertical batch-end indicator
    lines also separated from the rest by a blank line.  Load each
    blank-line-delimited dataset into a list of pairs, and all those lists as
    elements in a grand list.

    Looks like gnuplot can't handle more than 40 or so separate datasets.  So
    we'll consolidate batches, producing only as many datasets as the AVR
    recorded channels.
    """

    infile = open(infilename)
    metadata = infile.readline()
    metadata = metadata.split()
    assert len(metadata) == 3
    assert metadata[0] == 'channels'
    assert metadata[1] == '='
    nChannels = int(metadata[2]) + 1  # Extra, for vertical batch-end lines

    result = []
    for c in range(0,nChannels): result.append([])
    lineNum = 1
    channel = 0

    for line in infile:
        lineNum += 1
        if line.strip() == '':
            channel = (channel + 1)%nChannels
            continue

        s = line.split()
        if len(s) != 2:
            cerr("Error: line " + str(lineNum) + " has " + str(len(s)) +
                 " tokens.")
            sys.exit(1)
        result[channel].append([float(s[0]), int(s[1])])

    return result


def dataRanges(d):
    """
    The data are a tuple of (x,y) pairs.
    Returns the tuple ((xmin,xmax),(ymin,ymax))
    """
    x = []
    y = []
    for dset in d[:-1]: # Exclude dummy "dataset" that's the vertical
        for p in dset:  # lines that separate batches.
            x.append(p[0])
            y.append(p[1])
    return ((min(x), max(x)), (min(y)-1, max(y)+1))


def guiScalesCallback(bound_gp, axis, r):
    """ Arg axis should be "xrange" or "yrange". """
    bound_gp.set_range(axis, r)
    bound_gp.replot()


def display(dataname, hang=False):
    """
    Start gnuplot and my gui to control it.  With the gui, register callback
    functions to control characteristics of the gnuplot display.  The gui
    shouldn't know anything about gnuplot, though.
    """
    data = loadData(dataname)

    gp = Gnuplot.Gnuplot(debug=0)
    gp.title(dataname)
    gp('set data style lines')
#   gp('set data style linespoints')

    xrange, yrange = dataRanges(data)
    gp.set_range('xrange', xrange)
    apply(gp.plot, data)

    # Odd that this "while 1" thing has to go in here (if I put it at module
    # scope the gnuplot plot doesn't come up).
    if hang == True:
        while 1: pass

    return (gp, xrange, yrange,
            lambda xrange: guiScalesCallback(gp,'xrange',xrange),
            lambda yrange: guiScalesCallback(gp,'yrange',yrange))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        cerr("Usage: plots.py <infile>");
        sys.exit(1)
    display(sys.argv[1], hang=True)
