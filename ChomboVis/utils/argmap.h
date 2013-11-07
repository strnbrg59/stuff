#ifndef DEFINED_ARGMAP_HPP
#define DEFINED_ARGMAP_HPP

#include <string>
using std::string;

/** A wrapper for a pointer to an arbitrary data type.  You need to define
 *  a subclass for each data type you want to support.
 *
 *  We have to use inheritance, rather than templates, because we need to
 *  convert from string to the specific type; for ints we need atoi(), for
 *  doubles we need atof(), etc.  If we only cared about user-defined types
 *  we could require those types to have a from-string conversion member.
*/
class SetType
{
  public:
    SetType( void*, string description="no description available" );
    SetType();
    virtual ~SetType();
    virtual string toString()=0;
    virtual string getDescription()=0;
    virtual void set( string ) = 0;
};

class SetInt : public SetType
{
  public:
    SetInt( void*, string description="no description available" );
    SetInt();
    void set( string );
    string toString();
    string getDescription();
  private:
    int* m_rep;
    string m_description;
};

class SetShort : public SetType
{
  public:
    SetShort( void*, string description="no description available" );
    SetShort();
    void set( string );
    string toString();
    string getDescription();
  private:
    short* m_rep;
    string m_description;
};

class SetDouble : public SetType
{
  public:
    SetDouble( void*, string description="no description available" );
    SetDouble();
    void set( string );
    string toString();
    string getDescription();
  private:
    double* m_rep;
    string m_description;
};

class SetString : public SetType
{
  public:
    SetString( void*, string description="no description available" );
    SetString();
    void set( string );
    string toString();
    string getDescription();
  private:
    string* m_rep;
    string m_description;
};

class SetBool : public SetType
{
  public:
    SetBool( void*, string description="no description available" );
    SetBool();
    void set( string );
    string toString();
    string getDescription();
  private:
    bool* m_rep;
    string m_description;
};

#endif // DEFINED_ARGMAP_HPP
