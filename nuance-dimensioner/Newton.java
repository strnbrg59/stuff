/*
  File:        NewtonFunctor.java
  Author:      Sternberg
  Date:
  Description: Newton's method for finding the zero of a function.
*/

public class Newton
{
    /** Apply Newton's method: find x such that functor.f(x)=0.
     *  @arg functor Object whose f() method is the function we care about.
     *  @arg guess User's preliminary guess for x that solves f(x)=0.
     *  @arg legal_min Minimum of legal domain for x.
     *  @arg legal_max Maximum of legal domain for x.
    */
    public static double findZero( NewtonFunctor functor, double guess,
        double legal_min, double legal_max ) throws BackEndException
    {
        int n_iter = 0;
        double delta;

        while( Math.abs(functor.f(guess)) > k_tolerance )
        {
            if( Math.abs(guess) < k_delta )
            {
                delta = k_delta;
            }
            else
            {
                delta = k_delta*guess;
            }

            double derivative = 
                (functor.f(guess+delta) - functor.f(guess))/delta;

            if( Math.abs(derivative) < k_tolerance )
            {
                System.out.println( "Newton: derivative=" + derivative );
                guess *= 2.0; // prevent blow-ups
            }
            else
            {                
                guess = guess - functor.f(guess)/derivative;
            }
            if( guess < legal_min || guess > legal_max )
            {
                guess = (legal_min + legal_max)/2.0;
            }


            if( ++n_iter > 100 )
            {
                throw new BackEndException("Newton: too many iterations");
            }
        }

        return guess;
    }

    /** Apply Newton's method: find x such that functor.f(x)=0.
     *  @arg functor Object whose f() method is the function we care about.
     *  @arg guess User's preliminary guess for x that solves f(x)=0.
    */
    public static double findZero( NewtonFunctor functor, double guess )
    throws BackEndException
    {
        return findZero(functor,guess, -10E50, 10E50);
    }

    /** For testing */
    public static void main( String[] args )
    {
        try
        {
            double result = Newton.findZero( 
              new NewtonFunctor()
              {
                  public double f( double x )
                  {
                      return (x+2.71)*(x-3.14);
                  }
              }, 
              10 
            );
            System.out.println( "result=" + result );
        }
        catch (BackEndException bee)
        {
            System.err.println( bee );
            System.exit(1);
        }
    }

    static final double k_tolerance = 10E-7;
    static final double k_delta = 10E-7;
}
