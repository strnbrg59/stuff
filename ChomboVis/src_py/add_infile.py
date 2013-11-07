"""
Canonicalize the chombovis command line: add "infile=" if it's not already
there.
The input has already had superfluous spaces removed, i.e. it's been passed
through sed 's/ *= */=g'.
"""

import sys
import string

args = string.split(sys.stdin.readline())
if len(args) > 0:
    if (args[-1].find('=') == -1) and (args[-1][0] != '-'):
        args[-1] = 'infile=' + args[-1]
args = string.join(args)
print args
