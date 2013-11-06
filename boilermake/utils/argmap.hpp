#ifndef DEFINED_ARGMAP_HPP
#define DEFINED_ARGMAP_HPP

#include <string>
#include <sstream>

class Anything
{
  public:
    Anything( std::string val ) : m_rep( val ) { }
    template<typename T> Anything( T );
    template<typename T> Anything& operator=(T t);
    void set( std::string val ) { m_rep = val; }
    operator bool()        const;
    operator int()         const;
    operator unsigned()    const;
    operator std::string() const;
    operator short()       const;
    operator double()      const;
  private:
    std::string m_rep;
};

template<typename T> Anything::Anything( T t )
{
    *this = t;
}

template<typename T> Anything&
Anything::operator=( T t )
{
    std::ostringstream o;
    o << t;
    m_rep = o.str();
    return *this;
}

class SetType
{
  public:
    SetType( Anything* val, std::string description );
    SetType();
    ~SetType();
    std::string toString();
    std::string getDescription();
    void set( std::string );
  private:
    Anything*   m_rep; // not owned
    std::string m_description;
};

#endif // DEFINED_ARGMAP_HPP
