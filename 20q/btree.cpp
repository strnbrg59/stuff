#include <map>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
#include "btree.hpp"

ostream& operator<<(ostream& out, Printable const& p)
{
    p.print(out);
    return out;
}

Node::~Node()
{
    delete l_;
    delete r_;
}

ostream&
Node::print(ostream& out) const
{
    out << v_;
    if (l_ && r_) {
        out << "(" << *l_ << ", " << *r_ << ")";
    }
    return out;
}

ostream&
Node::print(ostream& out, int depth) const
{
    out << depth << " " << v_ << '\n';
    if (l_ && r_) {
        l_->print(out, depth+1);
        r_->print(out, depth+1);
    }
    return out;
}

ostream&
Tree::print(ostream& out) const
{
    out << *root_;
    return out;
}

Tree::Tree(string infilename)
{
    ifstream infile(infilename.c_str());
    pair<int, string> nodedat = infile_line_parser(infile);
    root_ = new Node(nodedat.second);

    pair<int, string> empty_nodedat = make_pair(-1,"");
    build_tree(infile, empty_nodedat, root_, nodedat.first);
}
    
Tree::~Tree()
{
    delete root_;
}

pair<int, string>
Tree::infile_line_parser(ifstream& infile) {
    char oneline[maxlinelen];

    infile.getline(oneline, maxlinelen);
    if (strspn(oneline, " \t\n\r") == strlen(oneline)) {
        return make_pair(-1, "");
    }
    //cout << "oneline=|" << oneline << "|\n";

    char *tok, *saveptr;
    tok = strtok_r(oneline, " ", &saveptr);
    int depth = atoi(tok);
    return make_pair<int,string>(depth, saveptr);
}

void
Tree::build_tree(ifstream& infile,
                pair<int,string>& next_nodedat,
                Node* curr_parent,
                int curr_depth)
{
    if (next_nodedat.second == "") {        // convention: empty
        next_nodedat = infile_line_parser(infile);
    }
    int new_depth = next_nodedat.first;
    if (new_depth <= curr_depth) {
        return;  // curr_parent is a leaf
    } else {
        curr_parent->l_ = new Node(next_nodedat.second);
        next_nodedat.second = "";
        build_tree(infile, next_nodedat, curr_parent->l_, new_depth);
        if (next_nodedat.second == "") {
            next_nodedat = infile_line_parser(infile);
        }

        curr_parent->r_ = new Node(next_nodedat.second);
        next_nodedat.second = "";
        build_tree(infile, next_nodedat, curr_parent->r_,
                   next_nodedat.first);
    }
}

#ifdef UNIT_TEST
int main()
{
    Tree tree("20q.dat");
    cout << tree << '\n';
}
#endif
