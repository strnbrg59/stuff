namespace ns {
struct Node
{
    template<typename T> T foo();
};

// Note even the specializations have to be declared, if you use
// a namespace.
template<> int Node::foo();

}

using namespace ns;

template<> int
Node::foo()
{
    return int();
}
