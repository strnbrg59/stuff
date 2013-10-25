#include <iostream>
#include <boost/interprocess/sync/lock_options.hpp>
#include <boost/thread/mutex.hpp>

struct ScopedTryLock
{
        ScopedTryLock(boost::mutex& m) : lock_(m) { lock_.try_lock(); }

        boost::mutex::scoped_lock lock_;
};

int main()
{
    boost::mutex m;
    ScopedTryLock stl(m);
}
