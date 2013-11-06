#include <cmath>
#include <cassert>
#include <string>
#include <map>
#include <iostream> 
#include "fingers.hpp"
#include "cmdline.hpp"
using namespace std;

/** Signed distance! */
double
distance(Chorda const& s0, Chorda const& s1, Note const& n0, Note const& n1)
{
    double ns0 = s0.Freq();
    double ns1 = s1.Freq();
    double dn0 = n0.Freq();
    double dn1 = n1.Freq();
    return CmdlineFactory::TheCmdline().StringLength() *
           fabs(ns1/dn1 - ns0/dn0);
}

int
position(Finger f, Chorda const& s, Note const& n)
{
    if(f==0) return 0;
    int result = int(12*log(n.Freq()/s.Freq())/log(2.0) + 0.5)
               + 1 - f;
    return result;
}


bool
isFeasible(Finger f, Chorda const& s, Note const& n)
{
    double tol = 0.01; // tempering
    double nfreq = n.Freq();
    double sfreq = s.Freq();
    bool n_equals_string = (fabs(nfreq - sfreq) < tol);
    bool n_below_string = nfreq < sfreq - tol;
    bool n_too_far_above_string = nfreq > 3*sfreq;
    bool result = ((f==0) && n_equals_string)
       || ((f!=0)
           && (!n_equals_string && !n_below_string && !n_too_far_above_string));
    return result;
}
