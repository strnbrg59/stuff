#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H

class RcmException
{
  public:
    RcmException( std::string message_ );
    void print();
  private:
    std::string message;
};

void rcmError( std::string message );

Md countFrom0( const Md& x );
Md countFrom1( const Md& x );
bool isSymmetric( const Md& x );

// Function declarations for using amoeba
// See amoeba.cpp for instructions and an example for how to use amoeba.
void amoeba( Md& p, Vd& y, int ndim, double ftol, 
            double (*funk)( const Vd& params, const void* data), int &n_iter, 
            const void* data, int quiet=0 );
void init_simplex( Md& p, int dimension, double scale=-99 );
  // -99 means we calculate a reasonable scale.
void init_y( Vd& y, const Md& p, int dimension, const void* data, 
             double (*func)( const Vd&, const void* ) );

#endif

