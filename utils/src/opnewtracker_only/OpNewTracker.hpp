#ifndef _OPNEWTRACKER_H_
#define _OPNEWTRACKER_H_

/****************************************************************************
This file redefines the global operators new, delete, new[] and delete[] so
as to keep track of all calls to the new and delete operators in any program
run while the environment variable LD_PRELOAD is set to libopnewtracker.so.
Upon exit, the program will print a
list of numbers that help identify which new'ed pointers didn't get delete'd.
You pick any one of those numbers and set the environment variable
CH_OPNEWTRACKER_COUNTER_TRAP to it.  Then, next time you run your program, it
aborts right where that call to new takes place, and you can look at the stack
trace to find the exact spot in your code.

The one caveat is that the global OpNewTracker object has to be constructed
before any user-defined static data whose constructor might result in a call to
new.  We can't guarantee that this takes place, but we can detect when it
doesn't, and in those cases we exit with an appropriate message.  The only way
to really guarantee that the OpNewTracker object gets constructed before any
other static data is to use the StaticMgr class for all your static data (whose
constructors might result in a call to new or new[].)
******************************************************************************/


#include <map>
#include <ext/malloc_allocator.h>
using std::map;

class OpNewTracker
{
public:
  OpNewTracker();
  ~OpNewTracker();
  void insert( void* p, bool brackets );
  void remove( void* p, bool brackets );

  static void init();
  static void cleanup();
  static char const * classname() { return "OpNewTracker"; }

  // These are strictly for new'ing the OpNewTracker singleton.
  static void * operator new( size_t ) throw (std::bad_alloc);
  static void   operator delete( void * ) throw ();

private:
  typedef __gnu_cxx::malloc_allocator<map<void*,long> > map_alloc_T;
  typedef map<void*,long,std::less<void*>,map_alloc_T> map_T;
  typedef map_T::iterator iterator_T;
  typedef map_T::const_iterator const_iterator_T;

  map_T m_news;        // for calls to new
  map_T m_bracketNews; // for calls to new[]
  int m_counter;
};

#endif // _OPNEWTRACKER_H_
