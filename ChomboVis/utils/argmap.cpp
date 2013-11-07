// We're going to use a map to set run-time parameters.  The map's
// keys are strings -- the names of the parameters -- while its
// corresponding values are objects that are constructed from 
// references to those parameters and that have a member function that
// sets those parameters to what was on the command line, or what was
// read from a file.  

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <string>
#include <cassert>
#include "argmap.h"

using std::ostringstream;

// ------------------------
// SetType
//    
SetType::SetType() { }

SetType::~SetType() { }

// ------------------------
// SetInt
//    
SetInt::SetInt( void* val, string description ) :
    m_rep( (int*)val ),
    m_description( description )
{
}

SetInt::SetInt() { }

/** Set the value.  (The description member is set only in the constructor.)*/
void SetInt::set( string str )
{
    char* endptr;
    *m_rep = int(strtol( str.c_str(), &endptr, 10 ));
    assert( endptr[0]==0 );
}

string SetInt::toString()
{
    ostringstream o;
    o << *m_rep << " : " << m_description;
    return string(o.str());
} 

string SetInt::getDescription()
{
    return m_description;
}

// ------------------------
// SetShort
//    
SetShort::SetShort( void* val, string description ) :
    m_rep((short*)val),
    m_description( description )
{
}

SetShort::SetShort() { }

/** Set the value.  (The description member is set only in the constructor.)*/
void SetShort::set( string str )
{
    char* endptr;
    *m_rep = short(strtol( str.c_str(), &endptr, 10 ));
    assert( endptr[0]==0 );
}

string SetShort::toString()
{
    ostringstream o;
    o << *m_rep << " : " << m_description;
    return string(o.str());
} 

string SetShort::getDescription()
{
    return m_description;
}

// ------------------------
// SetDouble
//    
SetDouble::SetDouble( void* val, string description ) :
    m_rep((double*)val),
    m_description( description )
{
}

SetDouble::SetDouble() { }

/** Set the value.  (The description member is set only in the constructor.)*/
void SetDouble::set( string str )
{
    char* endptr;
    *m_rep = strtod( str.c_str(), &endptr );
    assert( endptr[0]==0 );
}

string SetDouble::toString()
{
    ostringstream o;
    o << *m_rep << " : " << m_description;
    return string(o.str());
} 

string SetDouble::getDescription()
{
    return m_description;
}

// ------------------------
// SetString
//    
SetString::SetString( void* val, string description ) :
    m_rep( (string*)val ),
    m_description( description )
{
}

SetString::SetString() { }

/** Set the value.  (The description member is set only in the constructor.)*/
void SetString::set( string str )
{
    *m_rep = str;
}

string SetString::toString()
{
    ostringstream o;
    o << *m_rep << " : " << m_description;
    return string(o.str());
} 

string SetString::getDescription()
{
    return m_description;
}

// ------------------------
// SetBool
//    
SetBool::SetBool( void* val, string description ) :
    m_rep( (bool*)val ),
    m_description( description )
{
}

SetBool::SetBool() { }

/** Set the value.  (The description member is set only in the constructor.)*/
void SetBool::set( string str )
{
    assert( str=="true" || str=="false" );
    if( str == "true" )       *m_rep = true;
    else                      *m_rep = false;
}

string SetBool::toString()
{
    ostringstream o;
    o << *m_rep << " : " << m_description;
    return string(o.str());
} 

string SetBool::getDescription()
{
    return m_description;
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
