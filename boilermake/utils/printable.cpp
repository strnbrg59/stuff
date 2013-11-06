#include "printable.hpp"

std::ostream&
operator<<(std::ostream& out, Printable const& object)
{
    object.Print(out);
    return out;
}

std::ostream&
operator<<(std::ostream& out, NonconstPrintable& object)
{
    object.Print(out);
    return out;
}
