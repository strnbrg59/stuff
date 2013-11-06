#ifndef INCLUDED_EXCEPTIONS_HPP
#define INCLUDED_EXCEPTIONS_HPP

#include <sstream>

//
// Exceptions
//

class Exception
{
  public:
    Exception( char const * format, ... );
    Exception();
  protected:
    enum { m_max_msg_length = 128 };
    char m_msg[ m_max_msg_length +1 ];
  friend std::ostream& operator<<( std::ostream&, Exception );
};
std::ostream& operator<<( std::ostream&, Exception );

class EofException : public Exception
{
  public:
    EofException( char const * format, ... );
};

class ReadErrorException : public Exception
{
  public:
    ReadErrorException( char const * format, ... );
};

class TimeoutException : public Exception
{
  public:
    TimeoutException( char const * format, ... );
};

class MiscException : public Exception
{
  public:
    MiscException( char const * format, ... );
};

class UnexpectedServerBehaviorException : public Exception
{
  public:
    UnexpectedServerBehaviorException( char const * format, ... );
};

#endif // INCLUDED_EXCEPTIONS_HPP
