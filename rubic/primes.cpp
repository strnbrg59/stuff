#include "primes.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace std;


ostream& operator<<(ostream& out, vector<unsigned> const& vi)
{
    for (vector<unsigned>::const_iterator i = vi.begin(); i!=vi.end(); ++i)
    {
        out << *i << ' ';
    }
    return out;
}

ostream& operator<<(ostream& out, multiset<unsigned> const& vi)
{
    for (multiset<unsigned>::const_iterator i = vi.begin(); i!=vi.end(); ++i)
    {
        out << *i;
        multiset<unsigned>::const_iterator j = i;
        ++j;
        if (j != vi.end()) out << '*';
    }
    return out;
}

bool factor_of(unsigned k, unsigned n)
{
    return n == k*(n/k);
}


ostream& Primes::print(ostream& out) const
{
    out << primes_;
    return out;
}

ostream& operator<<(ostream& out, Primes const& p)
{
    return p.print(out);
}


void Primes::find_primes(unsigned maxx)
{
    unsigned i;
    if (primes_.empty()) i = 2;
    else                i = primes_.back();
    for (; i<maxx; ++i)
    {
        unsigned j=0;
        for (; j<primes_.size(); ++j)
        {
            if (factor_of(primes_[j], i)) {
                goto new_i;
            }
        }
        if (primes_.empty()) j=2;
        else                j = primes_.back();
        for (; j<=unsigned(sqrt(i)); ++j)
        {
            if (factor_of(j, i))
            {
                break;
            }
        }
        if (j>sqrt(i))
        {
            primes_.push_back(i);
        }
        new_i: j=j;

        if (i%100000 == 0) cerr << 'P';
    }
}

void Primes::extend_to(unsigned lim)
{
    this->find_primes(lim);
}


multiset<unsigned> Primes::find_factors(unsigned n,
                                        unsigned give_up_at)
{
    multiset<unsigned> result;
    this->extend_to(100);
    unsigned m = n;
    unsigned f;
    do
    {
        f = this->find_next_factor(m);
        if (f==0)
        {
            this->extend_to(primes_.back()*2);
            if (primes_.back() > give_up_at)
            {
                cerr << "giving up on factorization after " << primes_.back()
                     << ".";
                return result;
            }
            continue;
        }
        result.insert(f);
        m /= f;
    } while (m != 1);

    return result;
}

/** Returns 0 if no factor found.  That's your hint to call extend_to(). */
unsigned Primes::find_next_factor(unsigned n) const
{
    for (vector<unsigned>::const_iterator i=primes_.begin();
         i!=primes_.end(); ++i)
    {
        if (factor_of(*i, n))
        {
            return *i;
        }
    }
    return 0;
}

#ifdef UNIT_TEST
int main()
{
    Primes primes;

    multiset<unsigned> factors_100 = primes.find_factors(100);
    cout << factors_100 << '\n';
    multiset<unsigned> factors_1000 = primes.find_factors(1000);
    cout << factors_1000 << '\n';
    multiset<unsigned> factors_33205 = primes.find_factors(33205);
    cout << factors_33205 << '\n';
    multiset<unsigned> factors_52676 = primes.find_factors(52676);
    cout << factors_52676 << '\n';
    multiset<unsigned> factors_123456789 = primes.find_factors(123456789);
    cout << factors_123456789 << '\n';
    multiset<unsigned> factors_1234567895 = primes.find_factors(1234567895);
    cout << factors_1234567895 << '\n';
}
#endif
