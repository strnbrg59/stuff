#ifndef INCLUDED_PRINTABLE
#define INCLUDED_PRINTABLE

#include <ostream>

struct Printable
{
    virtual ~Printable() { }
    virtual void Print(std::ostream&) const = 0;
};
std::ostream& operator<<(std::ostream&, Printable const&);

struct NonconstPrintable
{
    virtual ~NonconstPrintable() { }
    virtual void Print(std::ostream&) = 0;
};
std::ostream& operator<<(std::ostream&, NonconstPrintable&);

#endif
