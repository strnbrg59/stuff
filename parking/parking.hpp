#include <map>
#include <set>

class Space
{
    double start_;
    double end_;
public:
    Space(double a, double b) : start_(a), end_(b) {}
    std::pair<Space,Space> divide(double& absolute_rear) const;
    Space& operator+=(Space const&);

    double size() const;
    double start() const { return start_; }
    double end() const { return end_; }

    static double car_length;

    friend std::ostream& operator<<(std::ostream&, Space const&);
    friend bool operator<(Space const& s1, Space const& s2);
};

bool operator<(Space const& s1, Space const& s2) {
    return s1.start_ < s2.start_;
}

class Curb
{
    double length_;
    std::set<Space> spaces_;
    std::set<double> rears_;

public:
    Curb(double length);
    void arrive();
    void depart();
    void debug_dump() const;
    bool full() const { return spaces_.empty(); }
    unsigned cars_parked() const { return rears_.size(); }
};


std::ostream& operator<<(std::ostream&, Space const&);

bool equal_enough(double x, double y);

void simulation();
