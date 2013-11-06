import math
import sys
import optparse

def binning(datatuple, lobin, hibin, nbins, transformation=lambda x: x):
    """ Bins datatuple for presentation as histogram.
        Returns frequencies.

        Arg transformation is an optional function of one variable,
        e.g. lambda x: math.log(x).
    """
    result = []
    t_hibin = transformation(hibin)
    t_lobin = transformation(lobin)
    
    delta = (t_hibin-t_lobin)/(nbins+0.0)
    b0 = t_lobin
    b1 = t_lobin + delta
    sum = 0

    count = len(filter(lambda x: transformation(x) <= b0, datatuple))
    sum += count
    result.append((b0-delta/2.0, count))
    while b0 < t_hibin:
        count = len(filter(lambda x: b0 < transformation(x) <= b1, datatuple))
        sum += count
        result.append((b1-delta/2.0, count))
        b0 = b1
        b1 += delta
    count = len(filter(lambda x: t_hibin < transformation(x), datatuple))
    sum += count
    result.append((t_hibin+delta/2.0, count))
                   
    # Convert to frequencies
    result = map(lambda x: (x[0], x[1]/(sum+0.0)), result)
    return result

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option("-l", "--lobin", dest="lobin", type="float");
    parser.add_option("-t", "--topbin", dest="topbin", type="float");
    parser.add_option("-n", "--nbins", dest="nbins", type="int");
    parser.add_option("-f", "--infile", dest="infile", type="string");
    parser.add_option("-d", "--demo_mode", dest="demo_mode", action="store_true");
    options, args = parser.parse_args()

    if options.demo_mode:
        print binning((1, 4, 9, 16, 25, 36, 49, 64, 81), 0, 60, 2)

    if options.infile:
        infile = open(options.infile, 'r');
        data = []
        for line in infile:
            try:
                data.append(float(line))
            except:
                pass
        bins = binning(data, options.lobin, options.topbin, options.nbins)
        for r in bins:
            print r[0], r[1]

#
# For gnuplot, save the output of this in "histogram.out", and go:
# ~/$ gnuplot
# gnuplot> set style data histogram
# gnuplot> plot "histogram.out" using 2:xtic(1)
# Or use histograms.gp (in this directory):
# gnuplot 
#
