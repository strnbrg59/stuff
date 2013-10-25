#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/shared_ptr.hpp>
using namespace std;

struct Printable
{
    virtual ostream& Print(ostream&) = 0;
    virtual ~Printable() {}
};

template<typename T> class Node : public Printable
{
public:
    typedef boost::shared_ptr<Node<T> > NodePtrT;

    typedef vector<NodePtrT> ChildrenT;
    typedef typename ChildrenT::const_iterator ChildConstIteratorT;
    typedef typename ChildrenT::iterator ChildIteratorT;
    static ChildIteratorT NotFound();

    Node(T const& t) : val_(t) { }

    void AddChild(T const& t) { children_.push_back(NodePtrT(new Node(t))); }

    ChildIteratorT FindChild(T const& t);
    ChildIteratorT FindDescendantDepthFirst(T const& t);
    ChildIteratorT FindDescendantBreadthFirst(T const& t);

    virtual ostream& Print(ostream&);
    T const& val() const { return val_; }

private:
    T val_;
    ChildrenT children_;
};


template<typename T> typename Node<T>::ChildIteratorT
Node<T>::NotFound()
{
    static ChildrenT dummy;
    return dummy.end();
}


template<typename T> class Tree : public Printable
{
public:
    Tree() : root_(Node<T>(T())) { } // Root is a dummy node
    typename Node<T>::ChildIteratorT AddEdge(T const& parent, T const& child);
    virtual ostream& Print(ostream&);
private:
    Node<T> root_;
};


template<typename T> typename Node<T>::ChildIteratorT
Node<T>::FindChild(T const& t)
{
    // This would be a lot faster if Node::children_ were a map keyed to T.
    for (ChildIteratorT i = children_.begin(); i!=children_.end(); ++i)
    {
        if ((*i)->val_ == t)
        {
            return i;
        }
    }
    return NotFound();
}

template<typename T> typename Node<T>::ChildIteratorT
Node<T>::FindDescendantDepthFirst(T const& t)
{
    for (ChildIteratorT i = children_.begin(); i!=children_.end(); ++i)
    {
        if ((*i)->val_ == t)
        {
            return i;
        } else
        {
            ChildIteratorT result = (*i)->FindDescendantDepthFirst(t);
            if (result != NotFound())
            {
                return result;
            }
        }
    }
    return NotFound();
}


/** Still mostly a depth-first algorithm. */
template<typename T> typename Node<T>::ChildIteratorT
Node<T>::FindDescendantBreadthFirst(T const& t)
{
    for (ChildIteratorT i = children_.begin(); i!=children_.end(); ++i)
    {
        if (i->val_ == t)
        {
            return i;
        }
    }

    for (ChildIteratorT i = children_.begin(); i!=children_.end(); ++i)
    {
        ChildIteratorT result = i->FindDescendantBreadthFirst(t);
        if (result != NotFound())
        {
            return result;
        }
    }
    return NotFound();
}


/** Returns iterator to parent Node.  If no node is found with parent value,
 *  edge is added under root_.
*/
template<typename T> typename Node<T>::ChildIteratorT
Tree<T>::AddEdge(T const& parent, T const& child)
{
    typename Node<T>::ChildIteratorT p = root_.FindDescendantDepthFirst(parent);
    if (p != Node<T>::NotFound())
    {
        (*p)->AddChild(child);
        return p;
    } else
    {
        root_.AddChild(parent);
        typename Node<T>::ChildIteratorT i = root_.FindChild(parent);
        (*i)->AddChild(child);
        return i;
    }
}
        
template<typename T> ostream&
Node<T>::Print(ostream& out)
{
    out << val_ << " ";
    if (!children_.empty()) out << "(";
    for (ChildIteratorT i=children_.begin(); i!=children_.end(); ++i)
    {
        out << **i;
        if (i+1 != children_.end()) out << " ";
    }
    if (!children_.empty()) out << ")";
    return out;
}


template<typename T> ostream&
Tree<T>::Print(ostream& out)
{
    out << root_;
    return out;
}


ostream& operator<<(ostream& out, Printable& p)
{
    p.Print(out);
    return out;
}


int main()
{
    Tree<int> i_tree;
    i_tree.AddEdge(1, 11);
    i_tree.AddEdge(1, 12);
    i_tree.AddEdge(1, 13);
    i_tree.AddEdge(2, 21);
    cout << i_tree << '\n';

    Tree<string> s_tree;
    s_tree.AddEdge("a", "aa");
    s_tree.AddEdge("a", "ab");
    s_tree.AddEdge("a", "ac");
    s_tree.AddEdge("b", "ba");
    s_tree.AddEdge("ab", "aba");
    s_tree.AddEdge("ba", "baa");
    s_tree.AddEdge("aba", "abaa");
    cout << s_tree << '\n';

    Tree<string> s_tree2(s_tree);
    cout << s_tree2 << '\n';
}

