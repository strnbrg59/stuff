#include <cassert>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
using namespace std;

struct Node
{
    Node(string name): name_(name) { }
    string name_;
    set<Node*> children_;
};

int operator<(Node const& n1, Node const& n2)
{
    return strcmp(n1.name_.c_str(), n2.name_.c_str());
}

Node* dfs(Node& n, string s)
{
    if (n.name_ == s) {
        return &n;
    } else {
        if (n.children_.empty()) {
            return 0;
        } else {
            for (set<Node*>::iterator i = n.children_.begin();
                 i != n.children_.end(); ++i) {
                return dfs(**i, s);
            }
        }
    }
    return 0;
}

/* This won't compile until you provide a functor for find().
Node* bfs(Node& n, string s)
{
    if (n.name_ == s) {
        return &n;
    } else {
        if (n.children_.empty()) {
            return 0;
        } else {
            set<Node*>::iterator i = n.children_.find(s);
            if (i != n.children_.end()) {
                return **i;
            } else {
                for (set<Node*>::iterator i = n.children_.begin();
                     i != n.children_.end(); ++i) {
                    return dfs(**i, s);
                }
            }
        }
    }
    return 0;
}
*/

// Allocates root if null.
// Returns node that has arg parent as its name_.  Returns null if no such node
// is found.
Node* insert_edge(Node* root, string parent, string child)
{
    if (!root) {
        root = new Node(parent);
        root->children_.insert(new Node(child));
        return root;
    } else {
        Node* parents_node = dfs(*root, parent);
        if (parents_node) {
            parents_node->children_.insert(new Node(child));
        }
        return parents_node;
    }
}

template<typename T> ostream&
operator<(ostream& out, set<T> const& s)
{
    for (typename set<T>::const_iterator i=s.begin(); i!=s.end(); ++i) {
        out << *i << ' ';
    }
    return out;
}


void find_descendants_recursive(Node const& parent, set<string>& result)
{
    for (set<Node*>::iterator i=parent.children_.begin();
         i!=parent.children_.end();
         ++i) {
       result.insert((*i)->name_);
       find_descendants_recursive(**i, result);
    }
}

/* Result includes name of parent. */
set<string> find_descendants(string parent, Node& root)
{
    set<string> result;
    Node* parent_node = dfs(root, parent);
    if (parent_node) {
        result.insert(parent_node->name_);
        find_descendants_recursive(root, result);
    }
    return result;
}
