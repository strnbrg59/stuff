#include <iostream>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
struct DsBase;
typedef boost::shared_ptr<DsBase> Dsptr;

struct DsBase
{
    virtual void go() = 0;
};

struct DsDerived : DsBase
{
    virtual void go() { std::cerr << "Hello world\n"; }
};


struct ASCBase
{
    virtual Dsptr ptr() = 0;
    void call_go() { ptr()->go(); }
};

struct ASCDerived : ASCBase
{
    virtual Dsptr ptr() {
        if (!ptr_) {
            ptr_.reset(new DsDerived);
        }
        return ptr_;
    }

    Dsptr ptr_;
};

int main()
{
    ASCDerived ad;
    ad.call_go();    
}
