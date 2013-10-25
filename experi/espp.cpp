/**
 * Compute expected ESPP return.
 * Usage: "espp <sd>" where sd=std dev of ror over one period
 * Example:
    $ ./espp 0.0; ./espp 0.2; ./espp 0.5; ./espp 1.0
    1.17647
    1.27031
    1.41107
    1.64566
 *
 */
#include <iostream>
#include <cassert>
#include <cstdlib>
using namespace std;

double urand();
double nrand(double mu, double sigma);

int main(int argc, char* argv[])
{
    assert(argc == 2);
    double s = atof(argv[1]); // sd of single-period rate of return, e.g. 0.2
    int n_iter = 10000;
    double sum = 0;
    for (int i=0; i<n_iter; ++i) {
        double p =  1 + nrand(0,s);     // end-of-period price
        double r = p/(0.85*min(1.0,p)); // rate of return
        sum += r;
    }

    cout << sum/n_iter << '\n';
}

double urand() {
    return random()/double(RAND_MAX);
}
double nrand(double mu, double sigma) {
    double sum=0;
    for(int i=0;i<12;++i) {
        sum += urand();
    }
    return mu + (sum-6)*sigma;
}
