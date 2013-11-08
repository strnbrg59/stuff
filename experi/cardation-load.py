#!/usr/bin/env python

#
# Simulate, or calculate, the load.
#

import getopt
import sys
from math import log

def analytic(ago, forgetfulness):
    noforget = log(ago+1.0)/log(2.0)
    load = noforget/(1 - forgetfulness*noforget)
    print "load =", load

# Ignores forgetfulness
def simulation(ago):
    load = {}
    for i in range(0, ago):
        if i in load.keys():
            load[i] += 1
        else:
            load[i] = 1
        j = 0
        elem = i + pow(2,j)
        while ago > elem:
            if elem in load.keys():
                load[elem] += 1
            else:
                load[elem] = 1
            j += 1
            elem = i + pow(2,j)

    for k in load:
        print k, load[k]

if __name__ == '__main__':
    ago = 16
    simulate = False
    forgetfulness = 0.0

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'sf:a:',
            ["simulate", "forgetfulness", "ago",])
        # "forgetfulness" is fraction missed on a typical day.
        # "ago" is how many days ago you started.
    except getopt.GetoptError:
        print "wrong cmdline args"
    for opt, arg in opts:
        if opt in ('-a', '--ago'):
            ago = int(arg)
        if opt in ('-s', '--simulate'):
            simulate = bool(arg)
        if opt in ('-f', '--forgetfulness'):
            forgetfulness = float(arg)
    if simulate:
        simulation(ago)
    else:
        analytic(ago, forgetfulness)
