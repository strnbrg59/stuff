from turtle import *
import math


#########################################################################
### Examples of Python functions that use functions from turtle.py
#########################################################################
def spin():
    setslowdown(0.1)
    for i in range(0,20):
        rt(18)
    for i in range(0,20):
        up(18)

def square():
    for i in range(0,4):
        fd(100)
        rt(90)

def cube():
    setslowdown(0.1)
    square()
    rl(90)
    square()
    rl(90)
    square()
    rl(90)
    square()


def square(r):
    for i in range(0,4):
        fd(r)
        rt(90)

import random
def randsquares(r):
    setslowdown(0.01)
    ht()
    for i in range(0,10000):
        square(r)
        r1 = random.randint(0,4)
        r2 = random.randint(0,2)
        if   r1 == 0:
            rl(90)
        elif r1 == 1:
            rl(-90)
        elif r1==2:
            up(90)
        elif r1==3:
            up(-90)
        if   r2 == 0:
            fd(r)
        else:
            fd(-r)
    st()


def srandwalk():
    randwalk2d( forward = sfd )

def randwalk2d( forward = fd ):
    for i in range(0,1000):
        setpencolor( random.randint(0,255)/255.0,
                            random.randint(0,255)/255.0,
                            random.randint(0,255)/255.0 )
        forward( random.randint(1,3) )
        rt( random.randint(-45,46) )


def randwalk3d():
    for i in range(0,1000):
        fd( random.randint(1,3) )
        if i%2 == 0:
            rt( random.randint(-45, 46) )
        else:
            up( random.randint(-45, 46) )


def spiro():
    setslowdown(0)
    for i in range(0,30):
        rt(33)
        for i in range(0,360):
                fd(1)
                rt(1.4)

def dna():
    #setslowdown(0.01)
    for i in range(0,36*3):
        pencolor = [0,0,0]
        pencolor[i%3] = 1
        apply( setpencolor, tuple(pencolor) )
        fd(20)
        rt(90)
        pu()
        fd(60)
        rt(90-10)
        pd()
#       up(10)
        rl(10)
    ht()


def rect(l,h):
    for i in 1,2:
        fd( l )
        lt( 90 )
        fd( h )
        lt( 90 )

def rectprism(l,h,w):
    rect(h,w)
    up(90)
    fd(l)
    dn(90)
    rect(h,w)
    dn(90)
    fd(l)
    up(90)

    rl(-90)
    rect(h,l)
    up(-90)
    fd(w)
    dn(-90)
    rect(h,l)
    rl(90)

    rt(90)
    fd(w)
    lt(90)

def centralRectprism(l,h,w):
    """
    Like rectprism() but initial (and ending) position is not the low corner
    but rather the centroid.
    """
    # Move from centroid to low corner.
    pu()
    dn(90)
    fd(l/2.0)
    up(90)
    rt(180)
    fd(h/2.0)
    rt(90)
    fd(w/2.0)
    rt(90)
    pd()

    rectprism(l,h,w)

    # Go back to centroid.
    pu()
    rt(-90)
    fd(-w/2.0)
    rt(-90)
    fd(-h/2.0)
    rt(-180)
    up(-90)
    fd(-l/2.0)
    dn(-90)
    pd()


def moebius( l,h,w ):
    """
    Not there yet.
    """
    n_iters=72
    up_angle = 360.0/n_iters
    rl_angle = 360.0/n_iters

    for i in range(0,n_iters):

        pencolor = [0,0,0]
        pencolor[0] = 1
        pencolor[1] = i/(n_iters+0.0)
        pencolor[2] = i/(n_iters+0.0)
#       pencolor[i%3] = 1
        apply( setpencolor, tuple(pencolor) )

        setslowdown(0.025)
        fd(0)
        setslowdown(0.0)

        up( up_angle*i )
        centralRectprism(l,h,w)
        up( -up_angle*i )

        rl( rl_angle )

        # Go to start position of next prism.
        pu()
        lt(90)
        fd(w)
        rt(90)
        pd()

        # Reorient centroid
#       up( up_angle )
