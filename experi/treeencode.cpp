#include <cassert>
#include <iostream>
#include "treeencode.hpp"
using std::cout;
using std::endl;
using std::vector;

Node::Node(char value)
  : m_value(value)
{
}


Node::~Node()
{
    for(vector<Node*>::const_iterator i=m_children.begin();
        i!=m_children.end();
        ++i)
    {
        delete *i;
    }
}    


char
Tree::RandNodeValue()
{
    return 65 + random()%(Node::maxValue);
}


Node*
Node::AddChild()
{
    Node* child = new Node(Tree::RandNodeValue());

    m_children.push_back(child);
    return child;
}


void
Node::Encode(vector<int>& encoding) const
{
    encoding.push_back(m_value);
    encoding.push_back((int)m_children.size());
    for(vector<Node*>::const_iterator i=m_children.begin();
        i!=m_children.end();
        ++i)
    {
        (*i)->Encode(encoding);
    }
}
    

std::ostream&
Node::Print(std::ostream& out) const
{
    static int indents;

    for(int i=0; i<indents; ++i) out << "  ";
    out << m_value << endl;

    ++indents;
    for(vector<Node*>::const_iterator i=m_children.begin();
        i!=m_children.end();
        ++i)
    {
        out << (**i);
    }
    --indents;

    return out;
}


std::ostream&
operator<<(std::ostream& out, Node const& node)
{
    return node.Print(out);
}


/** Create a tree of Node's.  Node m_value is a random counting number
 *      from 0-9.  Number of child nodes is random.
 *  Initializes the tree and then hands off the real work to the recursive
 *  function MakeChildren().
 *
 *  @var fertility: parameter governing (random) number of child nodes.
        Should be in (0.0, 1.0).
 *  @var randseed: argument to srandom().  If <=0, we don't call srandom()
 *      (as an aid to debugging).
*/
Node*
Tree::Populate(double fertility, unsigned int randseed)
{
    assert((fertility>0) && (fertility<1));

    if(randseed > 0)
    {
        srandom(randseed);
    }

    Node* root = new Node(Tree::RandNodeValue());
    // Maps to [A-Z].

    MakeChildren(root, fertility);

    return root;
}


static int s_decodingPosition;

/** Populate a tree of Node's, from the information in encoding. */
Node*
Tree::Decode(vector<int> const& encoding)
{
    Node* root = new Node(encoding[0]);
    root->Decode(encoding);

    return root;
}


void
Node::Decode(vector<int> const& encoding)
{
    int nChildren = encoding[s_decodingPosition*2 + 1];
    for(int i=0;i<nChildren;++i)
    {
        ++s_decodingPosition;
        Node* child = new Node(encoding[s_decodingPosition*2]);
        m_children.push_back(child);
        child->Decode(encoding);
    }
}
    

void
Tree::MakeChildren(Node* parent, double fertility)
{    
    bool makeChild = random() < fertility*RAND_MAX;
    while(makeChild
    && (parent->NumChildren() < Node::maxChildren))
    {
        Node* child = parent->AddChild();
        MakeChildren(child, fertility);
        makeChild = random() < fertility*RAND_MAX;
    }
}


void
Tree::Encode(std::vector<int>& encoding, Node const* root)
{
    root->Encode(encoding);
}


void
Tree::PrintEncoding(std::vector<int> const& encoding)
{
    for(vector<int>::const_iterator i=encoding.begin();
        i!=encoding.end();
        ++i)
    {
        cout << *i << ' ';
    }
    cout << endl;
}


/** Set foundIt to true if str is found along some path down through the
 *  tree.
 *  Precondition: foundIt should be initialized to false.
 *  Precondition: tree must be acyclic.
*/
void
FindString(Node const* head, char const* str, bool& foundIt)
{
    if(head->Value() == str[0])
    {
        if(str[1] == 0x0)
        {
            foundIt = true;
        } else
        {
            for(vector<Node*>::const_iterator i = head->m_children.begin();
            i != head->m_children.end();
            ++i)
            {
                FindString(*i, str+1, foundIt);
            }
        }
    }
}


/** Generates a tree, encodes it in a vector<int>, creates a new tree by
 *  decoding that vector<int>, and verifies the trees are equivalent.
 *
 *  Takes optional argument -- an int used to seed the random number generator.
*/
int main(int argc, char* argv[])
{
    int seed=0;
    if(argc>=2) seed = atoi(argv[1]);
    Node* root = Tree::Populate(0.5, seed);

    if(argc==3)
    {
        bool foundIt = false;
        FindString(root, argv[2], foundIt);
        cout << "FindString(" << argv[2] << ")=" << foundIt << endl;
    }

    cout << *root;

    //
    // Encode the tree.
    //
    vector<int> encoding;
    Tree::Encode(encoding, root);
    Tree::PrintEncoding(encoding);
    delete root;

    //
    // Create a new tree by decoding the first tree's encoding.
    //
    Node* newRoot = Tree::Decode(encoding);
    vector<int> newEncoding;
    Tree::Encode(newEncoding, newRoot);
    //Tree::PrintEncoding(newEncoding);
    delete newRoot;

    //
    // Check that new tree is same as old tree.
    //
    assert(encoding == newEncoding);
}
