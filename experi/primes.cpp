#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <set>
using namespace std;

typedef unsigned ul;

class Primes
{
    vector<ul> primes_;
    void find_primes(ul maxx);
    ostream& print(ostream& out) const;
    ul find_next_factor(ul n) const;
public:
    void extend_to(ul limit);
    multiset<ul> find_factors(ul n);
    friend ostream& operator<<(ostream&, Primes const&);
};


ostream& operator<<(ostream& out, vector<ul> const& vi)
{
    for (vector<ul>::const_iterator i = vi.begin(); i!=vi.end(); ++i)
    {
        out << *i << ' ';
    }
    return out;
}

ostream& operator<<(ostream& out, multiset<ul> const& vi)
{
    for (multiset<ul>::const_iterator i = vi.begin(); i!=vi.end(); ++i)
    {
        out << *i << ' ';
    }
    return out;
}

bool factor_of(ul k, ul n)
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


void Primes::find_primes(ul maxx)
{
    ul i;
    if (primes_.empty()) i = 2;
    else                i = primes_.back();
    for (; i<maxx; ++i)
    {
        ul j=0;
        for (; j<primes_.size(); ++j)
        {
            if (factor_of(primes_[j], i)) {
                goto new_i;
            }
        }
        if (primes_.empty()) j=2;
        else                j = primes_.back();
        for (; j<=ul(sqrt(i)); ++j)
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

        if (i%100000 == 0) cerr << '.';
    }
}

void Primes::extend_to(ul lim)
{
    this->find_primes(lim);
}


multiset<ul> Primes::find_factors(ul n)
{
    multiset<ul> result;
    this->extend_to(100);
    ul m = n;
    ul f;
    do
    {
        f = this->find_next_factor(m);
        if (f==0)
        {
            this->extend_to(primes_.back()*primes_.back());
            continue;
        }
        result.insert(f);
        m /= f;
    } while (m != 1);

    return result;
}

/** Returns 0 if no factor found.  That's your hint to call extend_to(). */
ul Primes::find_next_factor(ul n) const
{
    for (vector<ul>::const_iterator i=primes_.begin();
         i!=primes_.end(); ++i)
    {
        if (factor_of(*i, n))
        {
            return *i;
        }
    }
    return 0;
}


int main()
{
    Primes primes;

    multiset<ul> factors_100 = primes.find_factors(100);
    cout << factors_100 << '\n';
    multiset<ul> factors_1000 = primes.find_factors(1000);
    cout << factors_1000 << '\n';
    multiset<ul> factors_33205 = primes.find_factors(33205);
    cout << factors_33205 << '\n';
    multiset<ul> factors_52676 = primes.find_factors(52676);
    cout << factors_52676 << '\n';
    multiset<ul> factors_123456789 = primes.find_factors(123456789);
    cout << factors_123456789 << '\n';
}
