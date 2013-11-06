#include <iosfwd>
#include <vector>
#include <set>

class Primes
{
    std::vector<unsigned> primes_;
    void find_primes(unsigned maxx);
    std::ostream& print(std::ostream& out) const;
    unsigned find_next_factor(unsigned n) const;
public:
    void extend_to(unsigned limit);
    std::multiset<unsigned> find_factors(unsigned n,
                                         unsigned give_up_at=1000000);
    friend std::ostream& operator<<(std::ostream&, Primes const&);
};

std::ostream& operator<<(std::ostream& out, std::multiset<unsigned> const& vi);
