// Kill threads gracefully.
// Main thread calls pthread_kill().
// Worker threads register a signal handler that decides what to
// do.
//
// Build with "g++ thread_kill.cpp -lpthread -o thread_kill"

#include <iostream>
#include <vector>
#include <cstdio>
#include <pthread.h>
#include <signal.h>
#include <boost/shared_ptr.hpp>
using std::cerr;
using std::vector;

struct ThreadData
{
    ThreadData() : done_(false) {}
    bool done_;

};
typedef boost::shared_ptr<ThreadData> ThreadDataPtr;

void* thread_func(void *arg)
{
    signal(SIGSTOP, SIG_DFL);

    ThreadData* data = static_cast<ThreadData*>(arg);
    cerr << "Thread " << pthread_self() << " starting, gonna spin...\n";
    while (!data->done_) {
        usleep(100);
    }

    cerr << "Thread " << pthread_self() << " exiting.\n";
}

int main()
{
    int const n_threads = 5;
    int err = 0;
    pthread_t thread_id;
    vector<pthread_t> thread_ids;
    vector<ThreadDataPtr> thread_datas;
    for (int i=0;i<n_threads;++i) {
        ThreadDataPtr td(new ThreadData());
        thread_datas.push_back(td);
        err = pthread_create(&thread_id, NULL, thread_func, td.get());
        if (err) {
            perror("!!!");
        }
        thread_ids.push_back(thread_id);
        usleep(250);
    }

    sleep(5);
    pthread_kill(thread_ids[0], SIGSTOP);
    for (int i=1; i<n_threads; ++i) {
        thread_datas[i]->done_ = true;
    }
}
