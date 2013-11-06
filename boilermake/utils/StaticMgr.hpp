/**
  Disciplined global object initialization and cleanup facility, based on
  "nifty counter" (Lakos, 1996, p.537) idea.

  In any header file where you declare a global object or static member
  data (objects, again -- no need to worry about integral types):

   1. #include this file.
   2. Define a static object of type StaticMgr, passing to its constructor two
      function pointers -- one for initializing your static object, the other
      for cleaning it up.
   3. For an example, see EBTools/EBIndexSpace.H.
  
  Your static objects will be initialized before entry into main(), and cleaned
  up after exit from main(), but -- crucially -- before memory usage and leakage
  accounting takes place in memtrack.cpp.

  If you have more than one static object in your header file,
  you still want to instantiate just one StaticMgr object; just make
  the two functions you pass it take care of initializing and destroying
  all the static objects in your header file.

  It doesn't matter what class you use as the template argument, i.e. to 
  instantiate StaticMgr with.  Just make sure you don't instantiate a StaticMgr
  with the same class, in another .H file.  So an ideal choice would be the
  class your respective .H file is named for.
*/

#ifndef _STATICMGR_H_
#define _STATICMGR_H_

#include <iostream>

typedef void (*PVF)();
typedef char const * (*PCF)();

template<typename T> class StaticMgr
{
public:
  StaticMgr( PVF allocator, PVF deallocator, PCF classname=0 );
  ~StaticMgr();
private:
  PVF m_deallocator;
  static int s_counter;
  PCF m_classname;
};

template<typename T>
StaticMgr<T>::StaticMgr( PVF allocator,
                         PVF deallocator,
                         PCF classname )
  : m_deallocator(deallocator),
    m_classname(classname)
{
  if( 0 == s_counter++ )
  {
    allocator();
  }
}

template<typename T>
StaticMgr<T>::~StaticMgr()
{
  if( 0 == --s_counter )
  {
    if( m_classname )
    {
//    std::cerr << "Entering StaticMgr<" << m_classname() << "> dtor...\n";
    } else
    {
//    std::cerr << "StaticMgr<unknown> dtor\n";
    }
    m_deallocator();
    if( m_classname )
    {
//     std::cerr << "Exited StaticMgr<" << m_classname() << "> dtor\n";
    } else
    {
//    std::cerr << "Exited StaticMgr<unknown> dtor\n";
    }
  }
}

template<typename T> int StaticMgr<T>::s_counter;


/**
  This guarantees that memory accounting takes place as the very last thing --
  after the cleanup of all other static objects (at least those managed by
  StaticMgr).

  No: we load OpNewTracker with LD_PRELOAD now.

static StaticMgr<OpNewTracker> s_lastThing( OpNewTracker::init,
                                            OpNewTracker::cleanup,
                                            OpNewTracker::classname );
*/
#endif // _STATICMGR_H_
