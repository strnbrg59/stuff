#include "utils.hpp"
#include <cmath>
#include <cstring>
#include <cassert>
#include <iostream>
using namespace std;

namespace utils {
string toupper( string str )
{
    string result( str );
    for( unsigned int i=0;i<str.size();i++ )
    {
        result[i] = ::toupper( str[i] );
    }
    return result;
}

string tolower( string str )
{
    string result( str );
    for( unsigned int i=0;i<str.size();i++ )
    {
        result[i] = ::tolower( str[i] );
    }
    return result;
}

double factorial(int n)
{
    if(n<2) return 1;

    double result;
    result = n;
    for(int i=n-1; i>1; --i) result*=i;
    return result;
}

double n_combinations(int n, int k)
{
    assert(k>=0);
    assert(k<=n);
    if((k==0) || (k==n)) return 1;
    double result = n;
    result /= k;  // Beware division of int by int!
    for(int i=n-1; i>n-k; --i) {
        result *= i;
        result /= i-n+k;
    }
    return result;
}

char *
strtok_r(char *s1, const char *s2, char **lasts)
#if __GNUC__ >= 4
//throw()
#endif
{
    char *c = s1 ? s1 : *lasts;
    c += strspn(c, s2);
    if(*c == '\0') return 0;

    char *d  = c + strcspn(c, s2);
    if(*d != '\0') *d++ = '\0';
    *lasts = d;
    return c;
}

string& trim(string& str)
{
    char whitespace[] = " \t\r\n";
    unsigned pos = str.find_first_not_of(whitespace);
    str.erase(0, pos);
    pos = str.find_last_not_of(whitespace);
    str.erase(pos+1);
    return str;
}

double round(double x, double tol)
{
    return int(x/tol + 0.5)*tol;
}

double bincoef(int n, int k, double p)
{
    assert(k>=0 && k<=n);
    if(p!=p) return 0; // Have observed nan -- optimization gone awry
    assert(p>=0 && p<=1);
    if((k==0) || (k==n)) return pow(p,k)*pow(1-p,n-k);
    if(p==0)
    {
        if(k>0) return 0;
        else return n_combinations(n,k);
    }
    if(p==1)
    {
        if(k<n) return 0;
        else return n_combinations(n,k);
    }

    if(n<40)
    {
        return n_combinations(n,k)*pow(p,k)*pow((1-p),n-k);
    } else // Avoid over- and underflows.
    {
        double result = pow(1-p, n-k);
        for(int i=k;i>0;--i)
        {
            double top_term = (n-i+1) * p;
            double bot_term = i;
            result *= top_term/bot_term;
        }
        return result;
    }
}


double negbincoef(int n, int k, double p)
{
    return p * bincoef(n+k-1, k, 1-p);
}    


double bincoef_normapprox(int n, int k, double p)
{
    double npq = n*p*(1-p);
    return exp(-pow((k - p*n),2)/(2*npq)) / sqrt(2*M_PI*npq);
}


bool double_is_close(double x, double y, double tol)
{
    return tol > fabs(x-y);
}

}

#ifdef DO_DEMO
using namespace utils;

double compare(int n, int m, double p)
{
    cout << "exact=" << bincoef(n, m, p) << ", approx="
         << bincoef_normapprox(n, m, p) << '\n';
}

double sums(int n, double p)
{
    double sum=0, sumnormal=0;
    for(int i=0;i<=n;++i)
    {
        double exact = bincoef(n, i, p);
        double approx = bincoef_normapprox(n, i, p);
        sum += exact;
        sumnormal += approx;
    }
    cout << "sum(n=" << n << ", p=" << p << ")=" << sum << "; sumnormal="
         << sumnormal << '\n';
}

int main()
{
    cout << bincoef(5,3,.1) << '\n';
    cout << bincoef(144, 21, .2) << '\n';

    compare(5, 3, .1);
    compare(12, 6, .5);
    compare(44, 22, .2);
    compare(144, 123, .8);
    compare(144, 21, .2);

    sums(10,.5);
    sums(21,.2);
    sums(35,.1);
    sums(144,.1);

    cout << negbincoef(12, 3, .2) << " " << bincoef(12+3-1, 3, 1-.2)*.2 << '\n';
}
#endif
