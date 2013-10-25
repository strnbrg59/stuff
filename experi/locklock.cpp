//
// Demonstrates fatal error that results when the boost::mutex
// dtor is called, after unlock() has been called twice.
//

#include <boost/thread/mutex.hpp>
#include <iostream>

int main()
{
    boost::mutex mut;
    mut.lock();
    mut.unlock();
    mut.unlock();
}
