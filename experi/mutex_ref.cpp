#include <boost/thread/mutex.hpp>
#include <iostream>
#include <string>
using namespace std;

struct MutexAndString
{
    MutexAndString(boost::mutex& m_, string const& s_) : m(m_), s(s_) {}
    boost::mutex m;
    string       s;
};

void thread_func(void* data)
{
    MutexAndString* ms = (MutexAndString*)data;
    int i=0;
    while (++i < 100) {
        ms->m.lock();
        cout << ms->s << '\n';
        ms->m.unlock();
    }
}  

int main()
{
    boost::mutex m;
    MutexAndString ms1(m, "thread 1");
    MutexAndString ms2(m, "thread 2");
    pthread_t t1, t2;
    pthread_create(&t1, 0, thread_func, &ms1);
    pthread_create(&t2, 0, thread_func, &ms2);
}

