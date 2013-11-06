#include "rubic.h"
#include <cassert>
#include <iostream>
using namespace std;

int main()
{
    Permutation<3> I;
    Turns<3> t;

    // Z commutator (terminology?)
    Permutation<3> zcom = t.F*t.R*t.f*t.r;
    assert(I == pow(zcom,6));

    Permutation<3> everything = t.F*t.R*t.U
                              * t.B*t.L*t.D;
    assert(I == pow(everything,60));

    Permutation<3> furball = t.F * t.R * t.B * t.L;
    assert(I == pow(furball,315));

    Turns<4> t4;
    Permutation<4> I4;
    Permutation<4> furball4 = t4.F * t4.R * t4.B * t4.L;
    assert(I4 == pow(furball4,1260));

    Permutation<3> edgeflip = t.B * t.L * t.U * t.l * t.u * t.b;
    assert(I == pow(edgeflip,6));

    //
    // To find the period of any transformation, use ./bin/orbitlength.
    //
}
