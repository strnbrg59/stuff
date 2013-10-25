/** Given a sorted, circular list of ints, a new item and a pointer to a
    starting node, insert the item.
*/

#include <cstdlib>
#include <iostream>
using std::cout;
using std::ostream;

struct Node
{
    Node(int val) : _val(val), _next(0) { }
    int _val;
    Node* _next;
};


class CircularList
{
public:
    CircularList(int size, int maxval);
    void InsertAfter(Node* prev, Node* newone);
    void Insert(int newVal, Node* start);
    void Remove(Node* n);
    ostream& Print(ostream&) const;
private:
    Node* _root;
};


ostream& CircularList::Print(ostream& out) const
{
    Node* curr = _root;
    do {
        out << curr->_val << '\n';
        curr = curr->_next;
    } while(curr != _root);

    return out;
}

ostream& operator<<(ostream& out, CircularList const& cl)
{
    return cl.Print(out);
}



/** Insert newone right after prev. */
void CircularList::InsertAfter(Node* prev, Node* newone)
{
    Node* next = prev->_next;
    prev->_next = newone;
    newone->_next = next;
}


CircularList::CircularList(int size, int maxval)
{
    _root = new Node(-1);
    _root->_next = _root;

    for(int i=0;i<size;++i)
    {
        Insert(random()%(maxval+1), _root);
    }

    Node* least = _root->_next;
    Remove(_root);
    _root=least;
}


/** We ignore start -- just begin at _root.  Beginning from start would be an
    optimization.
 */
void CircularList::Insert(int newVal, Node* start)
{
    Node* newNode = new Node(newVal);
    Node* curr = _root;
    Node* prev = 0;
    while(curr->_val < newVal)
    {
        prev = curr;
        curr = curr->_next;
        if(curr == _root) break;
    }
    InsertAfter(prev, newNode);
}


void CircularList::Remove(Node* n)
{
    // Refuse, if n is the only node.
    if(n->_next == n) {
        cout << "Error: Remove() called on one-node list.\n";
        return;
    }

    // Find prior node. 
    Node* curr = n;
    while(curr->_next != n) {
        curr = curr->_next;
    }
    curr->_next = n->_next;
    delete n;
}


int main(int argc, char** argv)
{
    CircularList cl(atoi(argv[1]), atoi(argv[2]));
    cout << cl << '\n';
}
