// We're going to use a map to set run-time parameters.  The map's
// keys are strings -- the names of the parameters -- while its
// corresponding values are objects that are constructed from 
// references to those parameters and that have a member function that
// sets those parameters to what was on the command line, or what was
// read from a file.  

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "argmap.hpp"


SetType::SetType( Anything* val, std::string description )
  : m_rep( val ),
    m_description( description )
{
}

SetType::~SetType() { }


/** Set the value.  (The description member is set only in the constructor.)*/
void SetType::set( std::string str )
{
    m_rep->set( str );
}

std::string SetType::toString()
{
    std::ostringstream o;
    o << std::string(*m_rep) << " : " << m_description;
    return std::string(o.str());
} 

std::string SetType::getDescription()
{
    return m_description;
}

//--------------------- Anything -------------------

Anything::operator bool() const
{
    if( (m_rep!="true") && (m_rep!="false") && (m_rep!="1") && (m_rep!="0") )
    {
        std::cerr << "Anything.m_rep is a logical bool, but reads|" << m_rep << "|\n";
    }
    assert( (m_rep=="true") || (m_rep=="false") || (m_rep=="1") || (m_rep=="0") );
    if( (m_rep == "true") || (m_rep == "1") )     return true;
    else                                          return false;
}


Anything::operator short() const
{
    char* endptr;
    short result = strtol( m_rep.c_str(), &endptr, 10 );
    assert( endptr[0]==0 );
    return result;
}

Anything::operator int() const
{
    char* endptr;
    int result = strtol( m_rep.c_str(), &endptr, 10 );
    assert( endptr[0]==0 );
    return result;
}

Anything::operator unsigned() const
{
    char* endptr;
    unsigned result = strtol( m_rep.c_str(), &endptr, 10 );
    assert( endptr[0]==0 );
    return result;
}

Anything::operator double() const
{
    char* endptr;
    double result = strtod( m_rep.c_str(), &endptr );
    assert( endptr[0]==0 );
    return result;
}

Anything::operator std::string() const
{
    return m_rep;
}


/** Test stub (and good example code).
main()
{
    int i=19;
    int j=37;
    double x=2.71;
    double y=3.14;
    bool t=true;
    bool f=false;
    string s1="xxx";
    string s2="xxx";

    map<string,SetType*> argmap;
    argmap["i"] = new SetInt(&i);
    argmap["j"] = new SetInt(&j);    
    argmap["x"] = new SetDouble(&x);
    argmap["y"] = new SetDouble(&y);
    argmap["t"] = new SetBool(&t);
    argmap["f"] = new SetBool(&f);
    argmap["s1"] = new SetString(&s1);
    argmap["s2"] = new SetString(&s2);

    cout << "argmap.size() = " << argmap.size() << '\n';

    argmap["x"]->set("271.314");
    argmap["y"]->set("314.271");
    argmap["i"]->set("45");
    argmap["j"]->set("98");
    argmap["t"]->set("false");
    argmap["f"]->set("true");
    argmap["s1"]->set("howdy");
    argmap["s2"]->set("doody");

    cout << "argmap[\"i\"]->toString(): " << argmap["i"]->toString() << '\n';
    cout << "argmap[\"j\"]->toString(): " << argmap["j"]->toString() << '\n';
    cout << "argmap[\"x\"]->toString(): " << argmap["x"]->toString() << '\n';
    cout << "argmap[\"y\"]->toString(): " << argmap["y"]->toString() << '\n';
    cout << "argmap[\"t\"]->toString(): " << argmap["t"]->toString() << '\n';
    cout << "argmap[\"f\"]->toString(): " << argmap["f"]->toString() << '\n';
    cout << "argmap[\"s1\"]->toString(): " << argmap["s1"]->toString() << '\n';
    cout << "argmap[\"s2\"]->toString(): " << argmap["s2"]->toString() << '\n';

    cout << "i = " << i << '\n';
    cout << "j = " << j << '\n';
    cout << "x = " << x << '\n';
    cout << "y = " << y << '\n';
    cout << "t = " << t << '\n';
    cout << "f = " << f << '\n';
    cout << "s1 = " << s1 << '\n';
    cout << "s2 = " << s2 << '\n';
    assert( i == 45 );
    assert( j == 98 );
    assert( x == 271.314 );
    assert( y == 314.271 );
    assert( t == false );
    assert( f == true );
    assert( s1 == "howdy" );
    assert( s2 == "doody" );
}
*/
