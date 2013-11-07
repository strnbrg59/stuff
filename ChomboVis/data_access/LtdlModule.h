#include <ltdl.h>
#include <string>

/** Object-oriented dynamic module operations.  For use by class ChomboBridge.
*/
class LtdlModule
{
  public:
    typedef void FuncSignature( void * );

    LtdlModule( char const * moduleName );
    ~LtdlModule();

    int Apply( char const * funcName, void * funcArg ) const;
    bool HasSymbol( char const * symbolName ) const;
    bool IsGood() const;

  private:
    bool Save_dlerror() const;

    lt_dlhandle m_rep;
    mutable std::string m_errormsg;
    bool m_isgood;
};
