#ifndef _OPNEWTRACKER_H_
#define _OPNEWTRACKER_H_

/****************************************************************************
This file redefines the global operators new, delete, new[] and
delete[] so as to keep track of all calls to the new and delete
operators in any program run while the environment variable LD_PRELOAD
is set to libopnewtracker.so.  Upon exit, the program will print a
list of numbers that help identify which new'ed pointers didn't get
delete'd.  You pick any one of those numbers and set the environment
variable CH_OPNEWTRACKER_COUNTER_TRAP to it.  Then, next time you run
your program, it aborts right where that call to new takes place, and
you can look at the stack trace to find the exact spot in your code.

Any attempts to delete a pointer not allocated with new, or call delete[]
when you should have called delete (or vice versa), or delete a non-NULL
pointer twice, are caught immediately, causing a program abort.

Caveats:

1. This won't work right with programs that redefine the global
operators new, new[], delete or delete[], since we already redefine
those here.


2. The global OpNewTracker object has to be constructed before any
user-defined static data whose constructor might result in a call to
new.  We can't guarantee that this takes place, but we can detect when
it doesn't, and in those cases we exit with an appropriate message.
The only way to really guarantee that the OpNewTracker object gets
constructed before any other static data is to use the StaticMgr class
for all your static data (at least that static data whose constructors
might result in a call to new or new[].)

******************************************************************************/


#include <map>

#if  __GNUC__ == 3  &&  __GNUC_MINOR__ == 3
    #include <bits/stl_alloc.h>
#else // This will work with gcc3.4 and 4.0.  Anything else I don't know.
    #include <ext/malloc_allocator.h>
#endif

using std::map;

class OpNewTracker
{
public:
  OpNewTracker();
  ~OpNewTracker();
  void insert( void* p, size_t size, bool brackets );
  void remove( void* p, bool brackets );

  static void init();
  static void cleanup();
  static char const * classname() { return "OpNewTracker"; }

  // These are strictly for new'ing the OpNewTracker singleton.
  static void * operator new( size_t ) throw (std::bad_alloc);
  static void   operator delete( void * ) throw ();

private:
  typedef std::pair<long,int> map_value_T;
#if  __GNUC__ == 3  &&  __GNUC_MINOR__ == 3
      typedef std::allocator<map<std::string,std::string> > map_alloc_T;
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
      I haven't figured out what to do about malloc_allocator, on g++ 4.3+
#else // This will work with gcc3.4 and 4.0.  Anything else I don't know.
  typedef malloc_allocator<map<std::string,std::string> > map_alloc_T;
#endif
  typedef map<void*,map_value_T,std::less<void*>,map_alloc_T> map_T;
  typedef map_T::iterator iterator_T;
  typedef map_T::const_iterator const_iterator_T;

  map_T m_news;        // for calls to new
  map_T m_bracketNews; // for calls to new[]
  int m_counter;       // serial number of call to new or new[]

  // Useful statistics
  long m_numNews;
  long m_numBracketNews;
  long m_numDeletes;
  long m_numBracketDeletes;
  long m_currHeapInUse;
  long m_maxHeapInUse;
};

#endif // _OPNEWTRACKER_H_
