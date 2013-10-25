#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
using namespace std;

struct Software {
    Software(int v, int d) : version_(v), arch_(d) {}
    int version_;
    int arch_;
};

ostream& operator<<(ostream& out, Software const& s)
{
    out << s.version_;
    return out;
}

ostream& operator<<(ostream& out, vector<Software> const& s)
{
    out << "[ ";
    for (vector<Software>::const_iterator i=s.begin(); i!=s.end(); ++i) {
        out << *i << ' ';
    }
    out << "]";
    return out;
}
ostream& operator<<(ostream& out, deque<Software> const& s)
{
    out << "[ ";
    for (deque<Software>::const_iterator i=s.begin(); i!=s.end(); ++i) {
        out << *i << ' ';
    }
    out << "]";
    return out;
}

template<typename SoftwareT>
struct FromTo {
    FromTo(SoftwareT const& from, SoftwareT const& to) : from_version_(from),
                                                         to_version_(to) {}
    SoftwareT const& from_version_;
    SoftwareT const& to_version_;
};


template<typename SoftwareT> struct Constraint {
public:
    Constraint(string model) : model_(model) {}
    bool operator()(FromTo<SoftwareT> const& fromto) const {
        if (!condition(fromto)) {
            complain();
            return false;
        } else {
            return true;
        }
    }
    virtual ~Constraint() {}
    virtual bool condition(FromTo<SoftwareT> const&) const = 0;
    virtual void complain() const = 0;

    typedef Constraint<SoftwareT> PtrType;
    typedef vector< Constraint<SoftwareT> > VectorOf;
protected:
    string model_;
};

/*--------- User-customizable section -------------------------------*/

/** Subclasses of Constraint: each one expresses a feasibility condition
 *  that the upgrade (or downgrade) expressed in a FromTo must satisfy.
*/
struct SameParity : public Constraint<Software> {
    SameParity(string model) : Constraint<Software>(model) {}
    virtual ~SameParity() {}
    bool condition(FromTo<Software> const& from_to) const {
        return (from_to.from_version_.version_ % 2)
            == (from_to.to_version_.version_ % 2);
    }
    void complain() const { cerr << "(SameParity violated)"; }
};

struct NoBigJumps : public Constraint<Software> {
    NoBigJumps(string model) : Constraint<Software>(model) {}
    virtual ~NoBigJumps() {}
    bool condition(FromTo<Software> const& from_to) const {
        return fabs(from_to.to_version_.version_ -
                    from_to.from_version_.version_) < 3;
    }
    void complain() const { cerr << "(NoBigJumps violated)"; }
};

struct NoDowngrades : public Constraint<Software> {
    NoDowngrades(string model) : Constraint<Software>(model) {}
    virtual ~NoDowngrades() {}
    bool condition(FromTo<Software> const& from_to) const {
        return from_to.to_version_.version_ >= from_to.from_version_.version_;
    }
    void complain() const { cerr << "(NoDowngrades violated)"; }
};

struct ModelConstraints : public Constraint<Software> {
    ModelConstraints(string model) : Constraint<Software>(model) {}
    virtual ~ModelConstraints() {}
    bool condition(FromTo<Software> const& from_to) const {
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
template<typename SoftwareT>
bool operator<(SoftwareT const& s1, SoftwareT const& s2)
{
    return s1.version_ < s2.version_;
}
template<typename SoftwareT>
bool operator==(SoftwareT const& s1, SoftwareT const& s2)
{
    return s1.version_ == s2.version_;
}

/** List all subclasses of Constraint here. */
template<typename SoftwareT>
typename Constraint<SoftwareT>::VectorOf make_constraint_vector(string model)
{
    typedef typename Constraint<SoftwareT>::PtrType PtrType;
    typename Constraint<SoftwareT>::VectorOf vect;
/*
        boost::assign::list_of< typename Constraint<SoftwareT>::PtrType>
            (PtrType(new SameParity(model)))
            (PtrType(new NoBigJumps(model)))
            (PtrType(new NoDowngrades(model)))
            (PtrType(new ModelConstraints(model)));
*/
    return vect;
}
/*----------------------------------------------------------------*/

template<typename SoftwareT>
bool feasible_transition(FromTo<SoftwareT> const& fromto,
                         typename Constraint<SoftwareT>::VectorOf const& cv)
{
    bool result = true;
    for (typename
         Constraint<SoftwareT>::VectorOf::const_iterator iter=cv.begin();
         iter!=cv.end(); ++iter) {
        result &= (**iter)(fromto);
    }
    return result;
}

/* Returns the path to the highest-value image by trying out all permutations
 * and choosing the best feasible one.
*/
template<typename SoftwareT> vector<SoftwareT>
best_path(SoftwareT const& current, vector<SoftwareT> const& proposed)
{
    vector< Constraint<SoftwareT> > cv
        = make_constraint_vector<SoftwareT>("2050");

    deque<SoftwareT> proposed_d(proposed.begin(), proposed.end());
    vector<SoftwareT> result;
    SoftwareT best_endpoint(current);
    sort(proposed_d.begin(), proposed_d.end());

    cerr << "=======\npermutations:\n";
    bool more_permutations = true;
    while (more_permutations) {
        cerr << current << ", " << proposed_d << " --> ";
        proposed_d.push_front(current);
        vector<SoftwareT> path;
        typename deque<SoftwareT>::const_iterator curr = proposed_d.begin();
        typename deque<SoftwareT>::const_iterator next = curr+1;
        while (    (next != proposed_d.end())
                && (feasible_transition(FromTo<SoftwareT>(*curr, *next), cv))) {
            path.push_back(*next);
            ++curr;
            ++next;
        }
        proposed_d.pop_front();
        cerr << path << '\n';

        // Is this the best path so far?  Best means, ending with the highest-
        // rated SoftwareT and taking the shortest path to there.
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
    vector<Software> vs = boost::assign::list_of(Software(2,0))
                                                (Software(6,0))
                                                (Software(4,0))
                                                (Software(9,0));
    vector<Software> bp = best_path(Software(0,0), vs);
    cerr << "Best path: " << bp << '\n';
}
