//
// You get a deadlock regardless of whether you define THREADFUL,
// *as long as you link to -lpthread*.  If you don't link to -lpthread
// (and of course don't define THREADFUL either) then there's seemingly
// no locking going on at all.
//

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <iostream>
#include <pthread.h>

struct MyLock
{
    MyLock(boost::recursive_mutex& m) : m_(m) { m.lock(); }
    ~MyLock() { m_.unlock(); }
    boost::recursive_mutex& m_;
};

//typedef MyLock scoped_lock;
typedef boost::mutex::

void* thread_func(void*)
{
    boost::recursive_mutex m;
    MyLock outer(m);
    for (int i=0;i<2;++i) {
        std::cerr << "outer " << i << '\n';
        MyLock inner(m);
        for (int j=0;j<3;++j) {
            std::cerr << "  inner " << j << '\n';
        }
    }
    return NULL;
}

int main()
{
#ifdef THREADFUL
    std::cerr << "Threadful...\n";
    pthread_t thread;
    int ret1 = pthread_create(&thread, NULL, thread_func, NULL);
    std::cerr << "pthread_create() returned " << ret1 << '\n';
    int ret2 = pthread_join(thread, NULL);
    std::cerr << "pthread_join() returned " << ret2 << '\n';
#else
    std::cerr << "No thread...\n";
    thread_func(NULL);
#endif
}
