#include <set>
using namespace std;

struct Node
{
    bool operator<(Node const& that) const { return this < &that; }
    set<Node> children_;
};


int main()
{
    Node n1;
    set<Node>::iterator p = n1.children_.begin();
/*
    Node** pp = (Node**)&p;
    (*pp)->children_.insert(Node());
*/
    p->children_.insert(Node());
}

