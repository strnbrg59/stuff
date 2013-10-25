#include <vector>
using namespace std;

struct Node
{
    typedef vector<Node>::iterator ChildrenIteratorT;

    explicit Node(int const& t) : val_(t) { }
    void AddChild(Node const& n) { children_.push_back(n); }
    ChildrenIteratorT FindDescendant(int const& t);
   
    int val_;
    vector<Node> children_;
};


Node::ChildrenIteratorT
Node::FindDescendant(int const& t)
{
    return children_.begin();
}

int main()
{
    Node n1(17);
    Node::ChildrenIteratorT p = n1.children_.begin();
    p->AddChild(Node(19));
}

