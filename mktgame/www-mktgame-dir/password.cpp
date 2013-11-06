/* installs and checks passwords on aqc accounts */

/**** INCLUDES ****/
#include <unistd.h> /* for sleep() */
#include <stdlib.h>
#include <string.h>
#include "aqc.hpp"
#include "xmem.hpp"

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void verify_user( String username, String pwd );
****/

/**** LOCAL FUNCTION DEFINITIONS ****/

/**** FUNCTION DEFINITIONS ****/


void verify_user( String username, String pwd ) {

  /* Check if name is among the names in the holding array (which
   *   has n elements).  You can count on holding being sorted by name.
   * Check if password is ok, too.  Fed's password works as a skeleton
   *   key, opening everyone's account.
   * Variable "result" is a vestige of the days when this function
   * 	returned something.  Here (in the www version) if anything
   *	is amiss, this function just calls exit().
  */

	int result;
	Names user;
	strncpy( user.name, username.chars(), NAMELEN );
	strncpy( user.pwd, pwd.chars(), NAMELEN );

	Names *foundit,  /* return value from bsearch() */
		*Fed,
		*key;

	// Allocate key and put latestord->name in its name field.
	key = (Names *)x_calloc( (size_t)1, sizeof(Names) );
	strcpy( key->name, user.name );

	foundit = (Names *)bsearch( (char *)key, (char *)G::names, 
		G::N_players, sizeof(Names), namescomp  );

	if( foundit == NULL )   /* didn't find name */
		error( "You don't have an account here." );

	else {
		strcpy( key->name, "Fed" );
		Fed = (Names *)bsearch( key, G::names, G::N_players,
			sizeof(Names), namescomp );

//	cout << "user->pwd = " << user->pwd << " " << strlen(user->pwd) << "<p>\n";
//	cout << "foundit->pwd = " << foundit->pwd << 
//		" " << strlen(foundit->pwd) << "<p>\n";
//	cout << "Fed->pwd = " << Fed->pwd << " " << strlen(Fed->pwd) << "<p>\n";

		// In case password has been encrypted...
//		int* serial = (int *)(char *)user.pwd; // takes first two bytes
//		String cryp_pwd = crypt_pwd( user.pwd, Fed->pwd, *serial );

		if( ( strcmp( user.pwd, foundit->pwd ) )
		&&  ( strcmp( user.pwd, Fed->pwd ) ) )
//		&&	( cryp_pwd != (user.pwd+2) ) )
			error( "Wrong password." );
		else  // now everything is fine 
			result = 1;

	}

} // verify_user()
//--------------------------


