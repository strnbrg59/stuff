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

# File: box_layout_data.py
# Author: TDSternberg
# Created: 8/01/03

"""
Python interface to the C++ BoxLayoutData class.
"""

import math
import anag_utils
import self_control
import box_layout
import box
import libbld

class BoxLayoutData( self_control.SelfControl ):
#Cut to here
    """
    A collection of boxes and, for each one, a single component of field data.
    A VisualizableDataset contains one BoxLayoutData for every combination of
    level and component.

    There are several ways to obtain a BoxLayoutData:
    1. VisualizableDataset.getBoxLayoutData()
    2. BoxLayoutData.clone()
    3. The return value from the +, -, *, / or ** operators (see below).

    All the familiar arithmetic and transcendental functions are here:

    +, -, *, /, **, +=, -=, *=, /=, **= (where the arguments can be a
    BoxLayoutData and a scalar, a scalar and a BoxLayoutData, or two
    BoxLayoutDatas);

    log, exp, abs;
    sin,  cos,  tan,  asin,  acos,  atan;
    sinh, cosh, tanh, asinh, acosh, atanh.

    Though the arithmetic operators can be used in the natural infix style, the
    other functions have to be called on a BoxLayoutData:
    >>> b = c.reader.getVisualizableDataset().getBoxLayoutData(2,15)
    >>> b.log()
    and not "log(b)".

    To avoid confusion, none of the arithmetic or transcendental
    functions (or abs()) are covered in the documentation below.  This was done
    because pydoc would have shown them in alphabetical order (hence log between
    cos and sin), and the arithmetic operators would have shown up under their
    official Python names like __iadd__, __rpow__, etc.  Also, the other
    functions (clamp, setDatum, etc) would have been hard to find.
    """

#Cut from here
    def __init__( self, bld_ptr, data_centering ):
        """
        Arg bld_ptr is a handle to a BoxLayoutData pointer, that we
        obtain from a VisualizableDataset.
        """
        anag_utils.funcTrace()
        self_control.SelfControl.__init__( self, dep_dict={}, metadata = [
           { 'name':'bld_ptr' }
        ])        
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'bld_ptr' )

        self.bld_ptr = bld_ptr

        # The C++ BoxLayoutData doesn't know anything about data centering
        # because, since it's away from the public interface, it can pretend
        # that everything is cell-centered.  But this class here is part of the
        # API, and we need to know the data centering for crop().
        self.data_centering = data_centering


#Cut to here
    def clone( self ):
        """
        Returns a new BoxLayoutData, identical to this one (except for the C++
        implementation detail of its reference count).
        """
        anag_utils.funcTrace()
        return BoxLayoutData( libbld.Clone( self.bld_ptr ), self.data_centering)


    def getNumBoxes( self ):
        """ Return the number of boxes. """
        anag_utils.funcTrace()
        return libbld.GetNumBoxes( self.bld_ptr )


    def getBoxLayout( self ):
        """
        Returns a copy of the box layout as a tuple of tuples of tuples.
        The box dimensions refer to cells, and not necessarily data values --
        this is of relevance when the data are not cell centered.

        The box dimensions reported do not include layers of ghost cells.
        """
        anag_utils.funcTrace()
        boxes = libbld.GetBoxLayout( self.bld_ptr )
        if len(boxes) == 0:
            result = box_layout.BoxLayout( (), 0, (0,0,0) )  # Early return
        else:
            dx = libbld.GetDx( self.bld_ptr )
            origin =  libbld.GetOrigin( self.bld_ptr )
            result =\
                 box_layout.BoxLayout( boxes = boxes, dx = dx, origin = origin )

        result.shrink( self.data_centering, 0 )
        return result


    def getDx( self ):
        """ Returns a 2- or 3-tuple """
        anag_utils.funcTrace()
        return libbld.GetDx( self.bld_ptr )

    def getFArrayAsTupleMatrix( self, box_num ):
        """
        Returns a copy of the FArray, as a tuple of tuples [of tuples] of
        reals.  (It's not in Fortran order anymore.)
        If you have Python's numarray module (formerly known as NumPy) installed,
        you can obtain a matrix by
        >>> import numarray
        >>> m = numarray.array( bld.getFArrayAsTupleMatrix(box_num) )
        And away you go.  The numarray manual is at www.numpy.org.
        """
        anag_utils.funcTrace()
        result = libbld.GetFArrayAsTupleMatrix( self.bld_ptr, box_num )
        # FIXME: What to do about 2D/3D?  Don't want 2D data to show up like
        # (((1.0,), (3.0,), (5.0,), (7.0,)), ((2.0,), (4.0,), (6.0,), (8.0,)))
        return result


    def getFArray( self, box_num ):
        """
        Returns a handle to a pointer to an FArray.  Used by user-supplied
        Python-wrapped C++ functions.
        """
        anag_utils.funcTrace()
        result = libbld.GetFArray( self.bld_ptr, box_num )
        return result
        # FIXME: What do we do about releasing the FArray?  Supply a release
        # function here?  Define an FArray object in Python, that has a dtor?


    def getNumCells( self ):
        """ Return the number of cells. """
        anag_utils.funcTrace()
        return libbld.GetNumCells( self.bld_ptr, -1 )

    def sum( self ):
        """ Sum of cell values """
        anag_utils.funcTrace()
        return libbld.Sum( self.bld_ptr, -1 )

    def sumOfSquares( self ):
        """ Sum of squares of cell values """
        anag_utils.funcTrace()
        return libbld.SumOfSquares( self.bld_ptr, -1 )

    def stdDev( self ):
        """ Standard deviation of cell values """
        anag_utils.funcTrace()
        s  = self.sum()
        ss = self.sumOfSquares()
        n  = self.getNumCells()
        dev = ss - s*s/(n+0.0)
        if dev <= 0.0: # Will be <0 only as result of floating pt trunc error.
            return 0
        else:
            result = pow( (ss - s*s/(n+0.0))/(n-1.0), 0.5 )
            return result

    def getDatum( self, box_num, i, j, k=0 ):
        """
        Return the value of the (i,j,k)-th cell of the boxNum-th box.
        """
        anag_utils.funcTrace()
        return libbld.GetDatum( self.bld_ptr, box_num, i, j, k )

    def setDatum( self, x, box_num, i, j, k=0 ):
        """
        Set to x the value of the (i,j,k)-th cell of the boxNum-th box.
        """
        anag_utils.funcTrace()
        libbld.SetDatum( self.bld_ptr, x, box_num, i, j, k )


    def clamp( self, clamping_descriptor ):
        """
        Arg clamping_descriptor describes the subset of the BoxLayout to which,
        until unclamp() is called, all pointwise operations and summary
        statistics will be restricted.  Arg clamping_descriptor can be:
            (a) a single integer indicating one of the BoxLayout's boxes,
            (b) a tuple of integers indicating several of the BoxLayout's boxes,
         or (c) an instance of class Box, indicating a general box by its
                corners in the BoxLayout's index space.  In this case, the
                box extents are interpreted in terms of cells and not
                necessarily values; this is of relevance when the data are not
                cell-centered.  So for example, if the box=((0,0),(1,1)) that's
                a 2x2 box but when the data are node-centered the corresponding
                FAB contains 9 data values.
        """
        anag_utils.funcTrace()
        if   isinstance( clamping_descriptor, type(5) ):        # one box num
            libbld.Clamp( self.bld_ptr, (clamping_descriptor,) )
        elif (  isinstance(clamping_descriptor, type((1,2,3)))
        and isinstance( clamping_descriptor[0], type(5)) ): # box nums
            libbld.Clamp( self.bld_ptr, clamping_descriptor )
        elif( isinstance(clamping_descriptor, box.Box ) ): # general box
            if( len(clamping_descriptor[0]) == len(clamping_descriptor[1])
                == 2 ):  # 2D mode
                  clamping_descriptor = box.Box((
                    clamping_descriptor[0] + (0,),
                    clamping_descriptor[1] + (0,) ))
            clamping_descriptor.grow( self.data_centering, 0 )
            libbld.ClampToGeneralBox( self.bld_ptr,
                                      clamping_descriptor.getCorners() )
        else:
            anag_utils.error(
                "Arg clamping_descriptor must be an int, a tuple of ints, "
                "or a box.Box." )
            return


    def crop( self, crop_box ):
        """
        In-place surgery: throw away cells outside arg crop_box, which is an
        instance of class Box.  The remaining boxes are the intersections of
        the original boxes and crop_box.  If there were ghost cells, we keep
        them and so in the result there may well be some (ghost) cells outside
        crop_box.

        Arg crop_box is interpreted in terms of cells, not (necessarily) values;
        this is of relevance when the data are not cell-centered.  So for
        example, if crop_box=((0,0),(1,1)) that's a 2x2 box but when the data
        are node-centered the correponsing FAB contains 9 data values.
        """
        anag_utils.funcTrace()

        if not isinstance( crop_box, box.Box ):
            anag_utils.error( "Arg crop_box must be of type box.Box." )
            return

        if( len(crop_box[0]) == len(crop_box[1]) == 2 ):  # 2D mode
              crop_box = box.Box(( crop_box[0] + (0,), crop_box[1] + (0,) ))
        else:
              crop_box = box.Box((crop_box[0], crop_box[1]))

        # Now adjust for data centering; away from the API, we pretend
        # is cell-centered.
        crop_box.grow( incr = self.data_centering, at_both_ends=0 )
        libbld.CropToGeneralBox( self.bld_ptr, crop_box.getCorners() )


    def unclamp( self ):
        """
        See comments under clamp().
        """
        anag_utils.funcTrace()
        libbld.UnClamp( self.bld_ptr )

    def min( self ):
        """
        Minimum element.
        """
        anag_utils.funcTrace()
        return libbld.Min( self.bld_ptr, -1 )
    def max( self ):
        """
        Maximum element.
        """
        anag_utils.funcTrace()
        return libbld.Max( self.bld_ptr, -1 )

#Cut from here

    def abs( self ):
        """
        Set all values (modulo clamping) to absolute value of themselves.
        """
        anag_utils.funcTrace()
        libbld.AbsoluteValue( self.bld_ptr, -1 )
        return self

    def sin( self ):
        """
        Set all values (modulo clamping) to sine of themselves.
        """
        anag_utils.funcTrace()
        libbld.Sin( self.bld_ptr, -1 )
        return self
    def cos( self ):
        """
        Set all values (modulo clamping) to cosine of themselves.
        """
        anag_utils.funcTrace()
        libbld.Cos( self.bld_ptr, -1 )
        return self
    def tan( self ):
        """
        Set all values (modulo clamping) to tangent of themselves.
        """
        anag_utils.funcTrace()
        libbld.Tan( self.bld_ptr, -1 )
        return self
    def asin( self ):
        """
        Set all values (modulo clamping) to arcsine of themselves.
        """
        anag_utils.funcTrace()
        libbld.Asin( self.bld_ptr, -1 )
        return self
    def acos( self ):
        """
        Set all values (modulo clamping) to arccosine of themselves.
        """
        anag_utils.funcTrace()
        libbld.Acos( self.bld_ptr, -1 )
        return self
    def atan( self ):
        """
        Set all values (modulo clamping) to arctangent of themselves.
        """
        anag_utils.funcTrace()
        libbld.Atan( self.bld_ptr, -1 )
        return self
    def sinh( self ):
        """
        Set all values (modulo clamping) to hyperbolic sine of themselves.
        """
        anag_utils.funcTrace()
        libbld.SinH( self.bld_ptr, -1 )
        return self
    def cosh( self ):
        """
        Set all values (modulo clamping) to hyperbolic cosine of themselves.
        """
        anag_utils.funcTrace()
        libbld.CosH( self.bld_ptr, -1 )
        return self
    def tanh( self ):
        """
        Set all values (modulo clamping) to hyperbolic tangent of themselves.
        """
        anag_utils.funcTrace()
        libbld.TanH( self.bld_ptr, -1 )
        return self
    def asinh( self ):
        """
        Set all values (modulo clamping) to inv hyperbolic sine of themselves.
        """
        anag_utils.funcTrace()
        libbld.AsinH( self.bld_ptr, -1 )
        return self
    def acosh( self ):
        """
        Set all values (modulo clamping) to inv hyperbolic cosine of themselves.
        """
        anag_utils.funcTrace()
        libbld.AcosH( self.bld_ptr, -1 )
        return self
    def atanh( self ):
        """
        Set all values (modulo clamping) to inv hyper. tangent of themselves.
        """
        anag_utils.funcTrace()
        libbld.AtanH( self.bld_ptr, -1 )
        return self


    def log( self ):
        """
        Set all values (modulo clamping) to natural log of themselves.
        """
        anag_utils.funcTrace()
        libbld.NaturalLog( self.bld_ptr, -1 )
        return self

    def exp( self ):
        """
        Antilogarithm.
        """
        anag_utils.funcTrace()
        libbld.Exp( self.bld_ptr, -1 )
        return self


    def __ipow__( self, x ):
        """
        In-place operator**().
        Raise all elements of this BoxLayoutData (or its clamped box) to the
        x-th power.  If x is itself a BoxLayoutData, then raise elements of this
        BoxLayoutData to the power of corresponding elements of x.
        """
        anag_utils.funcTrace()
        if isinstance( x, BoxLayoutData ):
            libbld.PowBLD( self.bld_ptr, x.bld_ptr, -1 )
        else:
            libbld.Pow( self.bld_ptr, x, -1 )
        return self

    def __pow__( self, x ):
        """
        Operator**()
        Return a new BoxLayoutData that is the result of raising all elements
        of this one (or its clamped box) to the x-th power.  x can be either a
        scalar (int or real), or a BoxLayoutData itself.
        """
        anag_utils.funcTrace()
        result = self.clone()
        return result.__ipow__( x )

    def __rpow__( self, x ):
        """
        Operator**() for BoxLayoutData the argument to the right of **.
        Return a new BoxLayoutData that is the result of raising all elements
        of this one (or its clamped box) to the x-th power.  x can be either a
        scalar (int or real), or a BoxLayoutData itself.
        """
        anag_utils.funcTrace()
        if isinstance( x, BoxLayoutData ):
            return ( self * x.log(x) ).exp()
        else:
            return (self * math.log(x)).exp()


    def inplaceArithmetic( self, arg, libbld_operatorBLD, libbld_operator ):
        """
        Does work for __iadd__, __isub__, __imul__ and __idiv__
        """
        anag_utils.funcTrace()
        if isinstance( arg, BoxLayoutData ):
            apply( libbld_operatorBLD, (self.bld_ptr, arg.bld_ptr, -1) )
        else:
            apply( libbld_operator,    ( self.bld_ptr, arg, -1 ) )
        return self


    def __iadd__( self, x ):
        """
        Operator+=().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self.inplaceArithmetic( x, libbld.PlusEqualsBLD,
                                          libbld.PlusEquals )

    def __add__( self, x ):
        """
        Operator+().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        result = self.clone()
        return result.__iadd__(x)

    def __radd__( self, x ):
        """
        Integer or float + BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self + x


    def __isub__( self, x ):
        """
        Operator-=().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self.inplaceArithmetic( x, libbld.MinusEqualsBLD,
                                          libbld.MinusEquals )

    def __sub__( self, x ):
        """
        Operator-().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        result = self.clone()
        return result.__isub__(x)

    def __rsub__( self, x ):
        """
        Integer or float - BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self*(-1) + x


    def __imul__( self, x ):
        """
        Operator*=(). Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self.inplaceArithmetic( x, libbld.TimesEqualsBLD,
                                          libbld.TimesEquals )
    def __mul__( self, x ):
        """
        Operator*().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        result = self.clone()
        return result.__imul__(x)

    def __rmul__( self, x ):
        """
        Integer or float * BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self * x


    def __idiv__( self, x ):
        """
        Operator/=().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self.inplaceArithmetic( x, libbld.DivideEqualsBLD,
                                          libbld.DivideEquals )
    def __div__( self, x ):
        """
        Operator/().  Arg is either a scalar, or another BoxLayoutData.
        """
        anag_utils.funcTrace()
        result = self.clone()
        return result.__idiv__(x)

    def __rdiv__( self, x ):
        """
        Integer or float / BoxLayoutData.
        """
        anag_utils.funcTrace()
        return self.__pow__(-1).__mul__( x )


    def unitTest( self ):
        anag_utils.funcTrace()
        anag_utils.info( "self.getDatum(0,1,2,0)=",self.getDatum(0,1,2,0) )

        self.setDatum( -3.14159, 0,1,2,0 );
        anag_utils.info( "self.getDatum(0,1,2,0)=",self.getDatum(0,1,2,0) )

        self.clamp(0)

        self.abs()
        anag_utils.info( "After abs(): self.getDatum(0,1,2,0)=",
            self.getDatum(0,1,2,0) )

        self.log()
        anag_utils.info( "After log(): self.getDatum(0,1,2,0)=",
            self.getDatum(0,1,2,0) )

        self += 10.0
        anag_utils.info( "After +=(10.0): self.getDatum(0,1,2,0)=",
            self.getDatum(0,1,2,0) )

        self -= 10.0
        anag_utils.info( "After -=(10.0): self.getDatum(0,1,2,0)=",
            self.getDatum(0,1,2,0) )
