#ifndef _INCLUDED_TREEENCODE_HPP_
#define _INCLUDED_TREEENCODE_HPP_

#include <vector>


/** Node of a (not necessarily binary) tree. */
class Node
{
public:
    typedef char ValueType;

    Node(ValueType value);
    ~Node();

    ValueType Value() const { return m_value; }
    Node* AddChild();
    int NumChildren() const { return m_children.size(); }
    void Encode(std::vector<int>&) const;
    void Decode(std::vector<int> const&);
    enum {maxChildren = 2, maxValue = 26};

private:
    ValueType m_value;
    std::vector<Node*> m_children;

    friend void FindString(Node const*, ValueType const*, bool&);
    friend std::ostream& operator<<(std::ostream&, Node const&);
    std::ostream& Print(std::ostream&) const;
};


std::ostream& operator<<(std::ostream&, Node const&);

void
FindString(Node const* head, char const* str, bool& foundIt);

class Tree
{
public:
    static Node* Populate(double fertility, unsigned randseed);
    static void Delete(Node*);
    static void Encode(std::vector<int>&, Node const*);
    static Node* Decode(std::vector<int> const&);
    static void PrintEncoding(std::vector<int> const&);
    static Node::ValueType RandNodeValue();

private:
    static void MakeChildren(Node* parent, double fertility);
};


#endif // include guard
