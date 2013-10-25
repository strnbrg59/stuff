#include <string>
#include <iostream>
#include <iomanip>
#include <boost/shared_ptr.hpp>
using namespace std;

class Node
{
    string name_;
    Node* parent_;
    Node* left_;
    Node* right_;
    unsigned depth_;
public:
    Node(Node* parent, string name);
    ~Node();
    friend ostream& operator<<(ostream& out, Node const* node);
    void Add(string child_name);
    void print_in_order(ostream&) const;
    const Node* find_descendent(string name) const;
};

ostream& operator<<(ostream& out, Node const* node);

string random_name(unsigned len);

void print_in_order(ostream&, Node const*);

//==================================================

Node::Node(Node* parent, string name)
  : name_(name), parent_(parent),
    left_(0), right_(0)
{
    if (parent) {
        depth_ = parent->depth_ + 1;
    } else {
        depth_ = 0;
    }
}

Node::~Node()
{
    delete left_;
    delete right_;
}

ostream&
operator<<(ostream& out, Node const* node)
{
    out << setw(4*node->depth_+1) << ' ' << node->name_ << '\n';
    if (node->left_) {
        out << node->left_;
    } else {
        out << setw(4*(node->depth_+1)+1) << ' ' << "-\n";
    }
    if (node->right_) {
        out << node->right_;
    } else {
        out << setw(4*(node->depth_+1)+1) << ' ' << "-\n";
    }
    return out;
}

void
Node::Add(string child_name)
{
    if (child_name < name_) {
        if (left_) {
            left_->Add(child_name);
        } else {
            Node* child(new Node(this, child_name));
            left_ =  child;
        }
    } else if (child_name > name_) {
        if (right_) {
            right_->Add(child_name);
        } else {
            Node* child(new Node(this, child_name));
            right_ = child;
        }
    }
    // If ==, just reject it.
}

void
Node::print_in_order(ostream& out) const
{
    if (left_) {
        left_->print_in_order(out);
    }
    out << name_ << '\n';
    if (right_) {
        right_->print_in_order(out);
    }
}

unsigned g_find_descendent_count;

const Node*
Node::find_descendent(string needle) const
{
    ++g_find_descendent_count;

    if (name_ == needle) {
        return this;
    } else if ((needle < name_) && left_) {
        const Node* result = left_->find_descendent(needle);
        if (result) {
            return result;
        }
    } else if (right_) {
        const Node* result = right_->find_descendent(needle);
        if (result) {
            return result;
        }
    }

    return 0;
}

string random_name(unsigned len)
{
    string result;
    for (unsigned i=0;i<len;++i) {
        result += random()%26 + 'a';
    }
    return result;
}


int main(int argc, char* argv[])
{
    string usage(
        string("[NEEDLE=xxx]") + argv[0] + " -n N [-s seed] [-f find_name]");
    int optchar;
    unsigned N = 0;
    const char* getopt_options = "s:n:";
    optchar = getopt(argc, argv, getopt_options);
    while (optchar != -1) {
        switch (optchar) {
            case 's' : srand(atoi(optarg)); break;
            case 'n' : N = atoi(optarg); break;
            default : cerr << "Usage: " << usage << '\n';
                      exit(1);
        }
        optchar = getopt(argc, argv, getopt_options);
    }
    if (N==0) {
        cerr << usage << '\n';
        exit(1);
    }

    Node* root(new Node(0, "mmmm"));

    for (unsigned i=1;i<N;++i) {
        string new_name = random_name(4);
        root->Add(new_name);
    }

    cout << root << '\n';
    root->print_in_order(cout);

    const char* needle = getenv("NEEDLE");
    if (needle) {
        cout << "Searching for " << needle << " ...\n";
        const Node* foundit = root->find_descendent(needle);
        if (foundit) {
            cout << foundit << '\n';
        } else {
            cout << "NULL\n";
        }
        cout << "g_find_descendent_count=" << g_find_descendent_count << '\n';
    }

    delete root;
}
