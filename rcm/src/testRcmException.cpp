#include <string>
#include <iostream>

class RcmException
{
  public:
    RcmException( string message_ );
    void print();
  private:
    string message;
};

RcmException::RcmException( string message_ ) : message(message_)
{ }

void RcmException::print()
{
    cerr << message << '\n';
}

void foo( int a )
{
        if( a==1 )
                throw RcmException( "a==1" );
}

main( int argc, char* argv[] )
{
        try
        {
                foo( atoi(argv[1]));
        }
        catch( RcmException re )
        {
                re.print();
        }
}
