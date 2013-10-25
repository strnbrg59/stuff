#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
using namespace std;
#include "btree.hpp"

int main()
{
    char reply[80];
    bool breply;
    string infilename = "20q.dat";
    Tree tree(infilename);
    Node* node = tree.root_;
    Node* prev_node = 0;
    while (node) {
        cout << node->v_ << " ?\n";
        cin.getline(reply, maxlinelen);
        breply = toupper(reply[0]) == 'Y';
        prev_node = node;
        if (breply) {
            node = node->r_;
        } else {
            node = node->l_;
        }
    }
    if (breply) {
        cout << "I win.\n";
    } else {
        char secret[maxlinelen], new_q[maxlinelen];
        cout << "I give up.  What is the answer? ";
        cin.getline(secret, maxlinelen);
        cout << "What is a good yes/no question to distinguish between "
             << prev_node->v_ << " and " << secret << "?\n";
        cin.getline(new_q, maxlinelen);
        cout << "Thank you.  And, regarding " << secret << ", what is the "
             << "correct answer?\n";
        cin.getline(reply, maxlinelen);
        assert( (toupper(reply[0])=='Y') || (toupper(reply[0])=='N'));
        breply = toupper(reply[0]) == 'Y';

        Node* new_node1 = new Node(prev_node->v_);
        Node* new_node2 = new Node(secret);
        if (breply) {
            prev_node->l_ = new_node1;
            prev_node->r_ = new_node2;
        } else {
            prev_node->l_ = new_node2;
            prev_node->r_ = new_node1;
        }
        prev_node->v_ = new_q;

        ofstream outfile(infilename.c_str());
        tree.root_->print(outfile, 0);
    }
}
