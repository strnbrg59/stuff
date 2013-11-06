#ifndef INCLUDED_FINGERS
#define INCLUDED_FINGERS

#include "printable.hpp"
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

typedef unsigned int Finger;

class Chorda : public Printable {
public:
    Chorda(int n) : m_rep(n) {}
    virtual ~Chorda() {}
    double Freq() const;
    operator int() const { return m_rep; }
    virtual void Print(std::ostream&) const;
private:
    int m_rep;
};

class Note : public Printable
{
public:
    Note() {}
    Note(Note const&);
    Note(std::string);
    std::string Name() const { return m_rep; }
    double Freq() const;
    virtual void Print(std::ostream&) const;
private:
    std::string m_rep;
};


std::ostream& operator<<(std::ostream&, Printable const&);

typedef std::vector<Note> NoteVector;
typedef NoteVector::const_iterator NoteIterator;

class Infile
{
public:
    Infile(std::string filename);
    NoteIterator begin() const;
    NoteIterator end() const;
private:
    std::string m_filename;
    std::vector<Note> m_noteElements;
};


class Node;

int position(Finger f, Chorda const& s, Note const& n);

double note2freq(std::string note);

bool
isFeasible(Finger f, Chorda const& s, Note const& n);

double
distance(Chorda const& s0, Chorda const& s1, Note const& n0, Note const& n1);


class Node : public Printable
{
public:
    Node(NoteIterator);
    Node(Finger const&, Chorda const&, Node const*, NoteIterator);
    Node(Node const&);
    ~Node();
    double Cost() const { return m_cost; }
    void Print(std::ostream&) const;
    void MakeChildren();
    void ResetDepths();
    void DeparentChild(Node const* child);
private:
    Finger m_f;
    Chorda m_s;
    double m_cost; // Of transition from parent Node.
    double m_accumulatedCost;
    int m_position;
    unsigned m_depth;
    NoteIterator m_noteIter;

    Node const* m_parent;
    typedef std::vector<Node*> ChildVector;
    ChildVector m_children;
    bool m_isUrRoot;
    boost::shared_ptr<Node> m_lastFingeredNode;
    static NoteIterator s_noteIterEnd;
    static bool s_urRootConstructed;

    int ShiftSize(Node const* that) const;

    friend class Tree;
};


class Tree
{
public:
    Tree(Infile const&);
    ~Tree();
    void Grow();
    Node* FindBestChild() const; 
    void Shift(Node* bestChild);
    static std::pair<Node const*, double> s_minCost;
private:
    Node* m_root;
};

#endif
