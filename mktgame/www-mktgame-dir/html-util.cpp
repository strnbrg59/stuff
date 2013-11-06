#include <String.h>
#include <string.h>
#include <fstream.h>
#include "html-util.hpp"

Html_argument_array::~Html_argument_array() {
	delete[] rep;
	delete[] stdin_buf;
}

Html_argument_array::Html_argument_array( int n_args ) {

// Html arguments look like "name=Ted&pwd=shb".
//
// I had a helluva time parsing out the last argument without
//		obtaining extra invisible garbage: see the seconddelim 
//		variable.

	len = n_args;
	rep = new Html_argument[ len ];
	
	const int maxbuflen=2048;
	const int maxtoklen=128;

	char buf[maxbuflen];
	char tokbuf[maxtoklen];
	cin.getline( buf, maxbuflen );

	stdin_buf = new char[ strlen(buf) + 1 ];
	strcpy( stdin_buf, buf );

	char* tok;
	char seconddelim[4] = "&\n ";
	seconddelim[2] = 13;

	for( int i=0;i<len;i++ ) {

		// Get NAME field.
		if( i==0 )
			tok = strtok( buf, "= \n\t\0" );
		else
			tok = strtok( NULL, "= \n\t\0" );
		if( !tok ) {
			cout << "Html_argument_array ctor: not enough arguments!\n";
//			cout << "array was: " << stdin_buf << '\n';
			exit(0);
		}
		
		strncpy( tokbuf, tok, maxtoklen-1 );
		rep[i].name = tokbuf;

		// Get VALUE field.
		tok = strtok( NULL, seconddelim );
		if( !tok ) {
			cout << "Html_argument_array ctor: not enough arguments!\n";
			exit(0);
		}

		if( strlen(tok) > maxtoklen-1 ) {
			cout << "token was:\n" << tok << '\n';
			cout << "Html_argument_array() : overly long token.\n";
			exit(0);
		}

		strncpy( tokbuf, tok, maxtoklen-1 );
		rep[i].value = tokbuf;
	}
} // Html_argument_array::Html_argument_array( int n_args )
//----------------------

Html_argument_array::Html_argument_array() {

// Html arguments look like "name=Ted&pwd=shb".
//
// I had a helluva time parsing out the last argument without
//		obtaining extra invisible garbage: see the seconddelim 
//		variable.
//
// This version of the constructor counts the number of arguments
//		all by itself.  (It counts the number of '&'s.)

	const int maxbuflen=2048;
	const int maxtoklen=32;

	char buf[maxbuflen];
	char tokbuf[maxtoklen];
	cin.getline( buf, maxbuflen );

	stdin_buf = new char[ strlen(buf) + 1 ];
	strcpy( stdin_buf, buf );

	// Count the number of '&'s: len is that number plus 1.
	if( strlen(buf) == 0 )
		len=0;
	else {
		len = 1;
		for( int i=0;i<(int)strlen(buf);i++ ) {
			if( buf[i] == '&' )
				len ++;
		}
	}
	rep = new Html_argument[ len ];
	
	char* tok;
	char seconddelim[4] = "&\n ";
	seconddelim[2] = 13;

	for( int i=0;i<len;i++ ) {

		// Get NAME field.
		if( i==0 )
			tok = strtok( buf, "= \n\t\0" );
		else
			tok = strtok( NULL, "= \n\t\0" );
		if( !tok ) {
			cout << "Html_argument_array ctor: not enough arguments!\n";
			cout << "array was: " << stdin_buf << '\n';
			exit(0);
		}
		strncpy( tokbuf, tok, maxtoklen-1 );
		rep[i].name = tokbuf;

		// Get VALUE field.
		tok = strtok( NULL, seconddelim );
		if( !tok ) {
			cout << "Html_argument_array ctor: not enough arguments!\n";
			exit(0);
		}
		strncpy( tokbuf, tok, maxtoklen-1 );
		rep[i].value = tokbuf;
	}
} // Html_argument_array::Html_argument_array()
//----------------------

String Html_argument_array::operator[]( String name ) {
// Retrieve value member associated with name member.

	int i=0;
	String temp( name );
	while( (i<len) && (rep[i].name != name ) ) 
		i++;

	if( i<len )
		return rep[i].value;
	else
		return String( "Didn\'t find it!" );
} // operator[](String)
//-------------------------

String Html_argument_array::operator[]( int i ) {
// Retrieve value member.

	if( (i<0) || (i>=len) ) {
		cout << "html_argument_array::operator[](int): out of bounds";
		cout << " argument: " << i << '\n';
		return "Out of bounds argument";
	}
	else
		return rep[i].value;

} // operator[](int)
//-------------------------

int Html_argument_array::n_args() {
	return len;
}
//-------------------------

void Html_argument_array::show_stdin() {
	cout << "stdin was:\n" << stdin_buf << '\n';
}
//--------------------------

String Html_argument_array::name( int i ) {
	if( (i<0) || (i>=len) )
		return "Error: Html_argument_array::name(int) argument out of bounds.\n";
	else
		return rep[i].name;
}
