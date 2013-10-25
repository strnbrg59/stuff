#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
using namespace std;

struct SSoftware {
    SSoftware(int v, int d) : version_(v), arch_(d) {}
    int version_;
    int arch_;
};

ostream& operator<<(ostream& out, SSoftware const& s)
{
    out << s.version_;
    return out;
}

ostream& operator<<(ostream& out, vector<SSoftware> const& s)
{
    out << "[ ";
    for (vector<SSoftware>::const_iterator i=s.begin(); i!=s.end(); ++i) {
        out << *i << ' ';
    }
    out << "]";
    return out;
}
ostream& operator<<(ostream& out, deque<SSoftware> const& s)
{
    out << "[ ";
    for (deque<SSoftware>::const_iterator i=s.begin(); i!=s.end(); ++i) {
        out << *i << ' ';
    }
    out << "]";
    return out;
}

struct FromTo {
    FromTo(SSoftware const& from, SSoftware const& to) : from_version_(from),
                                                       to_version_(to) {}
    SSoftware const& from_version_;
    SSoftware const& to_version_;
};


class Constraint;
typedef boost::shared_ptr<Constraint> ConstraintPtr;
typedef vector<ConstraintPtr> ConstraintVect;

struct Constraint {
public:
    Constraint(string model) : model_(model) {}
    bool operator()(FromTo const& fromto) const {
        if (!condition(fromto)) {
            complain();
            return false;
        } else {
            return true;
        }
    }
    virtual ~Constraint() {}
    virtual bool condition(FromTo const&) const = 0;
    virtual void complain() const = 0;
protected:
    string model_;
};

/*--------- User-customizable section -------------------------------*/

/** Subclasses of Constraint: each one expresses a feasibility condition
 *  that the upgrade (or downgrade) expressed in a FromTo must satisfy.
*/
struct SameParity : public Constraint {
    SameParity(string model) : Constraint(model) {}
    virtual ~SameParity() {}
    bool condition(FromTo const& from_to) const {
        return (int(from_to.from_version_.version_) % 2)
            == (int(from_to.to_version_.version_) % 2);
    }
    void complain() const { cerr << "(SameParity violated)"; }
};

struct NoBigJumps : public Constraint {
    NoBigJumps(string model) : Constraint(model) {}
    virtual ~NoBigJumps() {}
    bool condition(FromTo const& from_to) const {
        return fabs(from_to.to_version_.version_ -
                    from_to.from_version_.version_) < 3;
    }
    void complain() const { cerr << "(NoBigJumps violated)"; }
};

struct NoDowngrades : public Constraint {
    NoDowngrades(string model) : Constraint(model) {}
    virtual ~NoDowngrades() {}
    bool condition(FromTo const& from_to) const {
        return from_to.to_version_.version_ >= from_to.from_version_.version_;
    }
    void complain() const { cerr << "(NoDowngrades violated)"; }
};

struct ModelConstraints : public Constraint {
    ModelConstraints(string model) : Constraint(model) {}
    virtual ~ModelConstraints() {}
    bool condition(FromTo const& from_to) const {
        bool result = true;
        if ((from_to.to_version_.version_ > 5) && (model_ == "2050")) {
            result = false;
        }
        return result;
    }
    void complain() const { cerr << "(ModelConstraints violated)"; }
};

/** Expresses preference; typically we prefer the newer version of
 *  the software.
*/
bool operator<(SSoftware const& s1, SSoftware const& s2)
{
    return s1.version_ < s2.version_;
}
bool operator==(SSoftware const& s1, SSoftware const& s2)
{
    return s1.version_ == s2.version_;
}

/** List all subclasses of Constraint here. */
ConstraintVect make_constraint_vector(string model)
{
    ConstraintVect vect = boost::assign::list_of<ConstraintPtr>
        (ConstraintPtr(new SameParity(model)))
        (ConstraintPtr(new NoBigJumps(model)))
        (ConstraintPtr(new NoDowngrades(model)))
        (ConstraintPtr(new ModelConstraints(model)));
    return vect;
}
/*----------------------------------------------------------------*/

bool feasible_transition(FromTo const& fromto, ConstraintVect const& cv)
{
    bool result = true;
    for (ConstraintVect::const_iterator iter=cv.begin(); iter!=cv.end();
         ++iter) {
        result &= (**iter)(fromto);
    }
    return result;
}

/* Returns the path to the highest-value image by trying out all permutations
 * and choosing the best feasible one.
*/
vector<SSoftware>
best_path(SSoftware const& current, vector<SSoftware> const& proposed)
{
    ConstraintVect cv = make_constraint_vector("2050");

    deque<SSoftware> proposed_d(proposed.begin(), proposed.end());
    vector<SSoftware> result;
    SSoftware best_endpoint(current);
    sort(proposed_d.begin(), proposed_d.end());

    cerr << "=======\npermutations:\n";
    bool more_permutations = true;
    while (more_permutations) {
        cerr << current << ", " << proposed_d << " --> ";
        proposed_d.push_front(current);
        vector<SSoftware> path;
        deque<SSoftware>::const_iterator curr = proposed_d.begin();
        deque<SSoftware>::const_iterator next = curr+1;
        while (    (next != proposed_d.end())
                && (feasible_transition(FromTo(*curr, *next), cv))) {
            path.push_back(*next);
            ++curr;
            ++next;
        }
        proposed_d.pop_front();
        cerr << path << '\n';

        // Is this the best path so far?  Best means, ending with the highest-
        // rated SSoftware and taking the shortest path to there.
        if (   (!path.empty())
            && (   (best_endpoint < path.back())
                || (   (best_endpoint == path.back())
                    && (path.size() < result.size())))) {
            best_endpoint = path.back();
            result = path;
        }

        more_permutations =
            next_permutation(proposed_d.begin(), proposed_d.end());
    }
    cerr << "=======\n";

    return result;
}

int main()
{
    vector<SSoftware> vs = boost::assign::list_of(SSoftware(2,0))
                                                (SSoftware(6,0))
                                                (SSoftware(4,0))
                                                (SSoftware(9,0));
    vector<SSoftware> bp = best_path(SSoftware(0,0), vs);
    cerr << "Best path: " << bp << '\n';
}
