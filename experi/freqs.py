import histograms
import math
import sys

data = []
iline=0
for line in sys.stdin:
    data.append(int(line))
    iline += 1

if len(data) > 0:
    bins = histograms.binning(data, 0, 200, 20, lambda x: x)
    for i in bins:
        print "%10.4f    " % i[0], i[1]

