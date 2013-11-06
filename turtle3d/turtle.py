# Turtle graphics using OpenGL.  Example:
# $ python -i turtle.py
# >>> rt(90)
# >>> fd(100)
#
# Author: Ted Sternberg, strnbrg@trhj.homeunix.net
#

import os
import sys
import time

global g_fildes, g_slowdown
g_slowdown = 0.00


def turtleInit():
    global g_fildes
    time.sleep(0.1) # Give child time to open stdin
    g_fildes = os.popen( './draw', 'w', 1 );


def cmd( keywd, arg=None ):
    if g_slowdown:
        time.sleep( g_slowdown )
    g_fildes.write( keywd )
    if arg != None:
        if type(arg) == type(()):
            for a in arg:
                g_fildes.write( ' ' + str(a) )
        else:
            g_fildes.write( ' ' + str(arg) )
    g_fildes.write( '\n' )


def fd( r ):
    cmd( 'fd', r )

def bk( r ):
    cmd( 'bk', r )

def sfd( r ):
    cmd( 'sfd', r )

def sbk( r ):
    cmd( 'sbk', r )

def rt( theta ):
    cmd( 'rt', theta )

def lt( theta ):
    cmd( 'lt', theta )

def up( theta ):
    cmd( 'up', theta )

def dn( theta ): # down
    cmd( 'dn', theta )

def rl( theta ): # roll
    cmd( 'rl', theta )


def ht():
    cmd( 'ht' )
def st():
    cmd( 'st' )

def pd():
    cmd( 'pd' )
def pu():
    cmd( 'pu' )


def setpencolor( r, g, b ):
    cmd( 'setpencolor', (r,g,b) )


def sphere( on_off ):
    if   on_off == 0:
        cmd( 'sphere 0' )
    elif on_off == 1:
        cmd( 'sphere 1' )
    else:
        print 'Error: usage: "sphere 0" or "sphere 1"'

def lighting( on_off ):
    if   on_off == 0:
        cmd( 'lighting 0' )
    elif on_off == 1:
        cmd( 'lighting 1' )
    else:
        print 'Error: usage: "lighting 0" or "lighting 1"'


def clean():
    cmd( 'clean' )


def setslowdown( seconds ):
    global g_slowdown
    g_slowdown = seconds


def bye():
    cmd( 'bye' )



if __name__ == '__main__':
    turtleInit()
