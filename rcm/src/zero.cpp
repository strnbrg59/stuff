#include <string>
using std::string;

/** zero function, for initializing things.  The argument is a dummy
 *  and is ignored, but it's necessary since you can't overload on return
 * type alone.
*/
double zero(double)
{
    return 0.0;
}

/** zero function, for initializing things.  The argument is a dummy
 *  and is ignored, but it's necessary since you can't overload on return
 * type alone.
*/
int zero(int)
{
    return 0;
}

/** zero function, for initializing things.  The argument is a dummy
 *  and is ignored, but it's necessary since you can't overload on return
 * type alone.
*/
string zero(string)
{
    return string("");
}
