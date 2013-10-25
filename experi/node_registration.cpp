#include <iostream>
#include <string>
using namespace std;

template<typename T> class Node
{
public:
    void setVal(string path, T value) {
        m_path = path;
        m_value = value;
    }
    ostream& print(ostream& out) const {
        out << m_path << " = " << m_value;
        return out;
    }

    typedef void (Node<T>::*SetterT)(string, T);

private:
    string m_path;
    T      m_value;
};

template<typename T> ostream&
operator<<(ostream& out, Node<T> const& n) {
    return n.print(out);
}

template<typename T> void
assignValToNode(string path, Node<T>& node, typename Node<T>::SetterT member,
                T value) {
    (node.*member)(path, value);
}


int main()
{
    Node<string> stringNode;
    assignValToNode<string>("/cmc/state/appliance/health",
                            stringNode,
                            &Node<string>::setVal,
                            "fit-as-a-fiddle");
    cout << "StringNode: " << stringNode << '\n';

    Node<int> intNode;
    assignValToNode<int>("/numbers/fingers",
                         intNode,
                         &Node<int>::setVal,
                         10);
    cout << "IntNode: " << intNode << '\n';

    Node<float> floatNode;
    assignValToNode<float>("/numbers/transcendental/pi",
                           floatNode,
                           &Node<float>::setVal,
                           3.14159);
    cout << "FloatNode: " << floatNode << '\n';
}
