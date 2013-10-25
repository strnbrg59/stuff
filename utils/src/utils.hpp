#ifndef INCLUDED_UTILS_HPP
#define INCLUDED_UTILS_HPP

#include <string>
#include <fstream>
#include <vector>
using std::vector;
using std::string;

namespace utils {
string& trim(string& str);
string toupper( string );
string tolower( string );
//char * strtok_r(char *s1, const char *s2, char **lasts);

double round(double x, double tol);
double factorial(int n);
double n_combinations(int n, int k);
double bincoef(int n, int m, double p);
double negbincoef(int n, int m, double p);
double bincoef_normapprox(int n, int m, double p);

template<typename T> T bounded(T const& t, T const& lo, T const& hi)
{
#ifndef max
#define defined_max
#define max(a,b) (a) < (b) ? (b) : (a)
#define min(a,b) (a) < (b) ? (a) : (b)
#endif
    return max(lo, min(hi, t));
#ifdef defined_max
#undef max
#undef min
#endif
}

template<typename T> T in_order(T const& a, T const& b, T const& c)
{
    return ((a<=b) && (b<=c)) || ((c<=b) && (b<=a));
}
}

template<class T> std::ostream &
operator<<( std::ostream& out, vector<T> const & v )
{
    for( typename vector<T>::const_iterator i = v.begin(); i != v.end(); ++i )
    {
        out << (*i) << " ";
    }
    return out;
}


#endif //  INCLUDED_UTILS_HPP
