/*
  File:        NewtonFunctor.java
  Author:      Sternberg
  Date:
  Description: Interface for wrapping up a function we want to
               pass to newton's method (Newton.findZero()).
*/

/** This interface serves the Newton class, in its task of finding the
 *  zero of a function. <BR>
 *  The role of the functor is to provide a way to pass a function 
 *  argument.  In Java, you can't pass a pointer to a function.  So instead
 *  we pass an object -- a NewtonFunctor -- that carries the function in
 *  a well-known name, <EM>f</EM>.
*/
public interface NewtonFunctor
{
    public double f( double x );
}
