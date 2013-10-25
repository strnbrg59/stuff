#!/bin/env python

import sys
import getopt

print "sys.argv =", sys.argv

try:
    opts, args = getopt.getopt(sys.argv[1:], 'np:', ["new", "population"])
except getopt.GetoptError:
    print "wrong cmdline args"

print "opts, args =", opts, args

for opt, arg in opts:
    if opt in ('-n', '--new'):
        print "opt was new"
    elif opt in ('-p', '--population'):
        print "opt was population, value was", arg
    else:
        print "opt was other"
