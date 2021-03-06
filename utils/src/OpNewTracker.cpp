#include "OpNewTracker.hpp"
#include "StaticMgr.hpp"
#include <string>
#include <cstdlib>
#include <iostream>
#include <cassert>
#define CH_assert( a ) assert( a )

using std::pair;
using std::make_pair;

static OpNewTracker * s_opNewTracker;


OpNewTracker::OpNewTracker()
  : m_counter(0),
    m_numNews(0),
    m_numBracketNews(0),
    m_numDeletes(0),
    m_numBracketDeletes(0),
    m_currHeapInUse(0),
    m_maxHeapInUse(0)
{
#ifdef NDEBUG
  std::cerr << "OpNewTracker won't work properly if compiled with -DNDEBUG.\n";
  abort();
#endif
}

OpNewTracker::~OpNewTracker()
{
  std::cout << "==========================================\n";
  if( m_news.empty() && m_bracketNews.empty() )
    {
        std::cout << "OpNewTracker: no memory leaks.\n";
    }
  else
    {
      std::cout << "OpNewTracker found these unmatched calls to new "
                   "or new[].\n";
      std::cout << "To get a core dump of right where one of these calls\n"
                   "takes place, set an environment variable,\n"
                   "CH_OPNEWTRACKER_COUNTER_TRAP, to one of the listed "
                   "serial numbers.\n"
                   "Then rerun your program.\n\n";
      if( ! m_news.empty() )
        {
          std::cout << "***Unmatched new's (serial_number size):\n";
          for( const_iterator_T iter=m_news.begin();
               iter!=m_news.end();
               ++iter )
            {
              std::cout << "( " << iter->second.first << " "
                                << iter->second.second << " )\n";
            }
        }

      if( ! m_bracketNews.empty() )
        {
          std::cout << "***Unmatched new[]'s (serial_number size):\n";
          for( const_iterator_T iter=m_bracketNews.begin();
               iter!=m_bracketNews.end();
               ++iter )
            {
              std::cout << "( " << iter->second.first << " "
                                << iter->second.second << " )\n";
            }
        }

      std::cout << "Number of calls to...\n"
                << "  new:      " << m_numNews << '\n'
                << "  delete:   " << m_numDeletes << '\n'
                << "  new[]:    " << m_numBracketNews << '\n'
                << "  delete[]: " << m_numBracketDeletes << '\n'
                << "Unfreed memory at exit: " << m_currHeapInUse << '\n';
    }
  std::cout << "Maximum heap in use:    " << m_maxHeapInUse << '\n';
  std::cout << "==========================================\n";
}

void
OpNewTracker::insert( void* p, size_t size, bool brackets )
{
  CH_assert( p );
  pair<const_iterator_T,bool> retval;
  if( brackets )
    {
      retval = m_bracketNews.insert( make_pair(p, make_pair(m_counter,size)));
      ++m_numBracketNews;
    }
  else
    {
      retval = m_news.insert( make_pair(p, make_pair(m_counter,size)));
      ++m_numNews;
    }
  CH_assert( retval.second ); // Error in this file, if assertion fails.

  //
  // If the OpNewTracker report says newed pointer #17 was left
  // undeleted, then in your environment do a
  // setenv CH_OPNEWTRACKER_COUNTER_TRAP 17
  // and the program will abort when the counter reaches 17.
  //
  char* counterTrap = getenv( "CH_OPNEWTRACKER_COUNTER_TRAP" );
  if( counterTrap )
    {
      CH_assert( m_counter != atoi(counterTrap) );
    }

  ++m_counter;
  m_currHeapInUse += size;
  m_maxHeapInUse = std::max(m_currHeapInUse, m_maxHeapInUse);
}

void
OpNewTracker::remove( void* p, bool brackets )
{
  CH_assert( p );

  // These pairs of assertions test, first, that we're not calling delete[]
  // on a pointer allocated with new (or vice versa); and second, that the
  // pointer has been allocated with (the appropriate form of new/new[]) at
  // all and hasn't been deleted already.
  int size;
  iterator_T iter;
  if( brackets )
    {
      CH_assert( m_news.find( p ) == m_news.end() );
      iter = m_bracketNews.find( p );
      CH_assert( iter != m_bracketNews.end() );
      size = iter->second.second;
      m_bracketNews.erase( p );
      ++m_numBracketDeletes;
    }
  else
    {
      CH_assert( m_bracketNews.find( p ) == m_bracketNews.end() );
      iter = m_news.find( p );
      CH_assert( iter != m_news.end() );
      size = iter->second.second;
      m_news.erase( p );
      ++m_numDeletes;
    }

  m_currHeapInUse -= size;
}


/*static*/ void
OpNewTracker::init()
{
  s_opNewTracker = new OpNewTracker;
}
/*static*/ void
OpNewTracker::cleanup()
{
  delete s_opNewTracker;
}


#define FREEBSD_MALLOC_ZERO 0x800

void * operator new(size_t size) throw (std::bad_alloc)
{
  void * retval = malloc(size);
  CH_assert( retval );

  // On FreeBSD (but not on Linux) malloc(0) returns the same address over
  // and over (and not even anything we can use -- typically, 0x800).
  // We can decline to insert these addresses into OpNewTracker, but then
  // we won't find them when op delete() is called.
  // So when size==0, whatever retval is, we always set it to 0x800.  We don't
  // insert that into s_opNewTracker, but we do return it from this function.
  // Then, when the client calls delete on that pointer, we recognize it as
  // 0x800, and we make sure not to delete it.
  //
  // We do the same in operator new[] and operator delete[]
  if( size == 0 )
    {
      return (void *)FREEBSD_MALLOC_ZERO;
    }
  else
    {
      if( !s_opNewTracker )
        {
          std::cerr << "===================================================\n"
                       "Fatal error: static object being constructed before\n"
                       "there's an OpNewTracker.\n"
                       "Forget about using libopnewtracker for this program\n"
                       "or redesign the program so it uses StaticMgr.H.\n"
                       "===================================================\n";
          exit(1);
        }
      s_opNewTracker->insert( retval, size, false );
      return retval;
    }
}


void operator delete( void * p ) throw ()
{
  if( p && (p!=(void*)FREEBSD_MALLOC_ZERO) )
    {
      s_opNewTracker->remove( p, false );
      free( p );
    }
}  


void * operator new[](size_t size) throw (std::bad_alloc)
{
  void * retval = malloc(size);
  CH_assert( retval );

  if( size == 0 )
    {
      return (void *)FREEBSD_MALLOC_ZERO;
    }
  else
    {
      s_opNewTracker->insert( retval, size, true );
      return retval;
    }
}


void operator delete[]( void * p ) throw ()
{
  if( p && (p!=(void*)FREEBSD_MALLOC_ZERO) )
    {
      s_opNewTracker->remove( p, true );
      free( p );
    }
}  

//
// These are just for OpNewTracker; we redefine the global operators new and
// delete separately.
//
void *
OpNewTracker::operator new( size_t size ) throw (std::bad_alloc)
{
  void * retval = malloc(size);
  return retval;
}
void
OpNewTracker::operator delete( void * buf ) throw ()
{
  free( buf );
}


#ifdef DO_DEMO

struct StaticThing
{
  StaticThing()
    {
      std::cerr << "Entering StaticThing ctor\n";
      themap["foo"] = "bar";
      std::cerr << "Leaving StaticThing ctor\n";
    }

  ~StaticThing()
    {
      std::cerr << "StaticThing dtor\n";
    }
#if  __GNUC__ == 3  &&  __GNUC_MINOR__ == 3
      typedef std::allocator<map<std::string,std::string> > ALLOC_T;
#else // This will work with gcc3.4 and 4.0.  Anything else I don't know.
  typedef __gnu_cxx::malloc_allocator<map<std::string,std::string> > ALLOC_T;
#endif

  map<std::string,std::string> themap;
};

StaticThing g_staticThing;

int main()
{
    double * must_not_compile_with_DNDEBUG = new double;
    assert( must_not_compile_with_DNDEBUG );
//  Throw in a deliberate memory leak...
    delete must_not_compile_with_DNDEBUG;

    int * pi2 = new int;
    delete pi2;

    int * a = new int[10000];
    a = a;
//  Another deliberate memory leak...
  delete[] a;

    std::string * str = new std::string;
    *str = "foobarbaz";
//  And another...
  delete str;

    map<std::string, std::string> strmap;
    strmap["foo"] = "bar";
}
#endif // DO_DEMO
