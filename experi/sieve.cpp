#include <cmath>
#include <iostream>
#include <set>
using namespace std;

class Primes
{
    set<unsigned> primes_;
    void find_primes(unsigned maxx);
    ostream& print(ostream& out) const;
public:
    void extend_to(unsigned limit);
    multiset<unsigned> find_factors(unsigned n);
    friend ostream& operator<<(ostream&, Primes const&);
};


ostream& operator<<(ostream& out, set<unsigned> const& vi)
{
    for (set<unsigned>::const_iterator i = vi.begin(); i!=vi.end(); ++i)
    {
        out << *i << ' ';
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
    else                i = *(primes_.lower_bound(maxx));
    for (; i<maxx; ++i)
    {
        unsigned j=0;
        for (set<unsigned>::const_iterator iter=primes_.begin();
             iter!=primes_.end(); ++iter)
        {
            if (factor_of(*iter, i)) {
                goto new_i;
            }
        }
        if (primes_.empty()) j=2;
        else                j = *(primes_.lower_bound(maxx));
        for (; j<=unsigned(sqrt(i)); ++j)
        {
            if (factor_of(j, i))
            {
                break;
            }
        }
        if (j>sqrt(i))
        {
            primes_.insert(i);
        }
        new_i: j=j;
    }
}

void Primes::extend_to(unsigned lim)
{
    this->find_primes(lim);
}


multiset<unsigned> Primes::find_factors(unsigned n)
{
    multiset<unsigned> result;
    this->extend_to(n);
    return result;
}


int main()
{
    Primes primes;
    primes.extend_to(50);
    cout << primes << '\n';
    primes.extend_to(100);
    cout << primes << '\n';
}
