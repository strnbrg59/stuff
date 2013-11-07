#
#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
#
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

# File: algorithms.py
# Author: TDSternberg
# Created: 2/05/02

import math
import string
import anag_utils

# ordinate, abscissa, e0,e1, pos ):
def linePlaneIntersection( e0,e1, abscissa, pos, ordinate ):
    """
    Finds where a line intersects a plane (that's perpendicular to one of the
    coordinate axes).
    Returns the coordinate, on the ordinate axis, of where the line intersects
    the plane that's perpendicular to the abscissa axis and intersects it at
    pos.

    Arg ordinate is one of 'x','y' or 'z'.  Arg abscissa is either one of the
      other two axes.
    Args e0 and e1 are the two points on the line.
    Arg pos is a position on the abscissa axis.
    """
    anag_utils.funcTrace()

    axisnums = {'x':0, 'y':1, 'z':2}
    n_ord = axisnums[ ordinate ]
    n_abs = axisnums[ abscissa ]

    # Avoid division by zero: perturb numbers a little, if necessary.
    delta = max( 1E-14 * max(abs(e1[n_abs]),abs(e0[n_abs])), 1E-14 )
    if abs( e1[n_abs]-e0[n_abs] ) < delta:
        e1 = list(e1)
        e1[n_abs] = e1[n_abs] + delta

    # We figure out the point of intersection by the method of proportions.
    proportion = (pos - e0[n_abs]) / (e1[n_abs] - e0[n_abs])
    result = e0[n_ord] + proportion * (e1[n_ord] - e0[n_ord])

    return result


def findFaceIntersections( e0, e1, b0, b1 ):
    """
    Args e0 and e1 are two points on a line.
    Args b0 and b1 are two opposite corners of a box.
    Find the two places where the line intersects a face of the box, and return
    them as a dictionary (keys 'x','y','z', values float pairs).
    """
    anag_utils.funcTrace()

    intersections = []
    axisnums = {'x':0, 'y':1, 'z':2}
    face_data = {'x':('y','z'), 'y':('x','z'), 'z':('x','y')}

    for b in b0, b1:
        for absc in 'x','y','z':
            ord = face_data[absc] # ordinate
            ord_num = (axisnums[ord[0]], axisnums[ord[1]])

            intsect0 = linePlaneIntersection(  e0,e1, absc,
                                              b[axisnums[absc]], ord[0] )
            intsect1 = linePlaneIntersection( e0,e1, absc, 
                                              b[axisnums[absc]], ord[1] )

            if(  ( b0[ord_num[0]] <= intsect0 <= b1[ord_num[0]]
              or   b0[ord_num[0]] >= intsect0 >= b1[ord_num[0]])
            and ( b0[ord_num[1]] <= intsect1 <= b1[ord_num[1]]
              or  b0[ord_num[1]] >= intsect1 >= b1[ord_num[1]])
            ):
                intersection = [0,0,0]
                intersection[axisnums[absc]] = b[axisnums[absc]]
                intersection[ord_num[0]] = intsect0
                intersection[ord_num[1]] = intsect1
                intersections.append( intersection )
                #anag_utils.info( "appending intersection ", intersection )

    # FIXME: Can you compute result directly, i.e. not go through intersections?
    result = {}
    if len(intersections) > 1:
        result['x'] = (intersections[0][0], intersections[1][0])
        result['y'] = (intersections[0][1], intersections[1][1])
        result['z'] = (intersections[0][2], intersections[1][2])

    return result


def distanceToBox( e0, e1, b0, b1 ):
    """
    Args e0 and e1 are two points on a line.
    Args b0 and b1 are two opposite corners of a box.

    Returns the distance from e0 to the box, or None if the line doesn't
    intersect the box.
    """
    anag_utils.funcTrace()

    intersections = findFaceIntersections( e0, e1, b0, b1 )
    if intersections == {}:
        result = None
    else:
        d0 = pow( pow( e0[0] - intersections['x'][0], 2 ) +
                  pow( e0[1] - intersections['y'][0], 2 ) +
                  pow( e0[2] - intersections['z'][0], 2 ),
                  2.0 )
        d1 = pow( pow( e1[0] - intersections['x'][1], 2 ) +
                  pow( e1[1] - intersections['y'][1], 2 ) +
                  pow( e1[2] - intersections['z'][1], 2 ),
                  2.0 )
        result = min( d0, d1 )

    return result


def box1ContainsBox2( box1_extents, box2_extents ):
    """
    Args are extents of any rectangular prism (thus they have nothing else to do
    with the notion of a "box" in Chombo.

    Returns 1 if box1 contains box2 (using <=, not < comparisons), 0 otherwise.
    """
    if( (box1_extents[0] <= box2_extents[0] )
    and (box1_extents[1] <= box2_extents[1] )
    and (box1_extents[2] <= box2_extents[2] )
    and (box1_extents[3] >= box2_extents[3] )
    and (box1_extents[4] >= box2_extents[4] )
    and (box1_extents[5] >= box2_extents[5] ) ):
        return 1
    else:
        return 0


#
# Other functions.
#

def findResolution( x ):
    """
    Finds how "precise" the floating point number x is; for example, for x=3.14
    returns 0.01.
    """
    anag_utils.funcTrace()

    try: # We're not sure if x is a number or a string-rep of a number.
        x = float(x)
    except:
        pass
    if x == 0:
        return 1
    if str(x) == 'inf'  or  str(x) == '-inf'  or  str(x) == 'NaN':
        anag_utils.error( "Found 'inf' or at the very least a number\n"
            "too large or too small for Python to represent.  ChomboVis\n"
            "is now unstable and liable to crash at any moment.  Suggestion:\n"
            "load your file into \"chombobrowser\" instead of \"chombovis\"." )
        return 1

    normalized_x = x/pow(10,math.floor(math.log10(abs(x))))
    tol_exp = int(math.floor(math.log10(abs(normalized_x))) - 14)

    tol = 10.0 ** tol_exp
    for i in range( 0, -tol_exp ):
        if abs(normalized_x - round(normalized_x,i)) < tol:
            normalized_result = pow(10.0,-i)
            break
    normalized_result = pow(10.0,-i)

    result = normalized_result*(x/normalized_x)
    return result


def floatEquals( x, y, tol ):
    """ Returns 1 if abs(x-y) < tol.  Returns 0 otherwise. """
    anag_utils.funcTrace()
    #anag_utils.info( "x=",x,", y=",y,", tol=",tol )
    if abs(x-y) < abs(tol):
        return 1
    else:
        return 0


def roundDown10( x ):
    """
    Round down to the nearest power of 10, e.g. 10, 1, 0.1, 0.01, etc.
    Examples: roundDown10(0.123) = 0.1
              roundDown10(-0.0123) = -0.01
    """
    anag_utils.funcTrace()
    if   x == 0:
        return 0
    else:
        if str(x) == 'inf'  or  str(x) == '-inf'  or  str(x) == 'NaN':
            anag_utils.warning( "Abnormal number:", x )
            return x
        else:
            a = pow(10,math.floor(math.log10(abs(x))))
            if x < 0:
                return -a
            else:
                return a


def roundDown( x, p ):
    """
    Round to the nearest number less than or equal to x that can be expressed
    with p or fewer numerals to the right of the decimal point.
    """
    anag_utils.funcTrace()

    p = int(p)
    result = round( x,p )
    if result > x :
        result = result - pow(10.0,-p)
    return result

def roundUp( x, p ):
    """
    Round to the nearest number greater than or equal to x that can be expressed
    with p or fewer numerals to the right of the decimal point.
    """
    anag_utils.funcTrace()

    p = int(p)
    result = round( x,p )
    if result < x :
        result = result + pow(10.0,-p)
    return result


def prettyRound( x, p ):
    """
    Round to the nearest 10E-p, but truncate result at the p-th place after
    the decimal point (thus avoiding things like 3.139999999999).
    """
    format = '%' + str(p+3) + '.' + str(p) + 'f'
    return format%(int( x * pow(10,p) + 0.5 )/pow(10.0,p))


def tupleround( t, n ):
    """
    Return a string representation of t, a tuple of numbers, obtained by
    rounding each numerical element of the argument to n digits right of the
    decimal point, maintaining the argument's structure.

    Works on lists too.
    """
    result = []

    assert( type(t) == type(())  or  type(t) == type([]) )
    if type(t[0]) == type(()) or type(t[0]) == type([]):
        for e in t:
            result.append( tupleround(e, n) )
    else:
        for e in t:
            fmt =  '%20.' + str(n) + 'f'
            formatted = fmt % e
            result.append( string.lstrip(formatted) )
            
    if type(t) == type(()):
        return tuple(result)
    else:
        return result

def dictround( d, n ):
    """
    Return a string representation of d, a dictionary with numerical values,
    obtained by rounding each value to n digits right of the decimal point,
    maintaining the argument's structure.
    """
    result = {}

    assert( type(d) == type({}) )
    for k in d.keys():
        if type(d[k]) == type({}):
            result[k] = dictround(d[k],n)
        else:
           fmt =  '%20.' + str(n) + 'f'
           formatted = fmt % d[k]
           result[k] = string.lstrip(formatted)

    return result
