#include <iostream>
using namespace std;

struct Singleton
{
    void func_a() { cout << _counter << '\n'; }
    void recycle_instance() { delete _a; _a = NULL; }
private:
    static Singleton* _a;
    static Singleton* Instance() {
        if (!_a) { _a = new Singleton; ++_counter; };
        return _a;
    }
    static int _counter;
    friend class SingletonPtr;
};
Singleton* Singleton::_a;
int Singleton::_counter;

struct SingletonPtr
{
    Singleton* operator->() { return Singleton::Instance(); }
};

int main()
{
    SingletonPtr b;
    b->func_a();
    b->recycle_instance();
    b->func_a();
}
