from libVTKRubicPython import vtkRubicPythonIface
c = vtkRubicPythonIface()
c.init(3)

def turn(str):
    for ch in str:
        c.turn(ch)

def isUnscrambled():
    return c.isUnscrambled()

from random import randint
def randTurns(tl):
    turn(tl[randint(0,len(tl)-1)])    

import sys
# Use C++ random_moves program to do it faster.
def randTurnsUntilUnscrambled(tl):
    randTurns(tl)
    i=1
    while not isUnscrambled():
        sys.stderr.write("%d " % i)
        randTurns(tl)
        i += 1
    print ''
