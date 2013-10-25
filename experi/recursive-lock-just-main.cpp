#include <boost/thread/mutex.hpp>
#include <iostream>
#include <pthread.h>

void* thread_func(void*)
{
    boost::mutex m;
    boost::mutex::scoped_lock outer(m);
    for (int i=0;i<2;++i) {
        std::cerr << "outer " << i << '\n';
        boost::mutex::scoped_lock inner(m);
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
