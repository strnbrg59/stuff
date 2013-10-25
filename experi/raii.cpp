#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

struct A
{
    mutex m;
    condvar c;

    void lock() { m.lock(); }
    void unlock() { m.unlock(); }

    boost::shared_ptr<B> getB() { return boost::shared_ptr<B>(new B(m,c);) }
    struct B
    {
        B(mutex& m_, condvar& c_) : m(m_), c(c_) { m.lock(); }
        B(B const& b) : this->m(b.m), this->c(b.c) {}
        ~B() { cleanup(); }
        void cleanup() { m.unlock; c.notify_all(); }

        mutex& m;
        condvar& c;
    };
};

threadfunc1(A& a)
{
    a.lock();
    ...do stuff...
    a.unlock();
    ...do more stuff...
}

threadfunc2(A& a)
{
    boost::shared_ptr<A::B> bp(a.getB());
    ...do yet other stuff...
}


int main()
{
    A a;
    pthread_create(threadfunc1(a));
    pthread_create(threadfunc2(a));
}

