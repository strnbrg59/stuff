#include <boost/shared_ptr.hpp>

template<typename T> struct PtrTrait
{
    typedef boost::shared_ptr<T> Ptr;
};

class C : public PtrTrait<C>
{
};

int main()
{
    C::Ptr cptr;
}
