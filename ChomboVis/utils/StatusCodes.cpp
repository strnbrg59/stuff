/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg
#include "StatusCodes.h"
#include <map>

using std::map;

/** Return descriptive name of arg s. */
string
StatusName( Status s )
{
    static map<Status,string> s_nameMap;
    static int initialized;
    if( !initialized )
    {
        s_nameMap[STATUS_OK] =         "STATUS_OK";
        s_nameMap[STATUS_NOT_FOUND] =  "STATUS_NOT_FOUND";
        s_nameMap[STATUS_BAD_FILE] =   "STATUS_BAD_FILE";
        s_nameMap[STATUS_DUPLICATE] =  "STATUS_DUPLICATE";
        s_nameMap[STATUS_WRONG_TYPE] = "STATUS_WRONG_TYPE";
        s_nameMap[STATUS_EOF] =        "STATUS_EOF";
        s_nameMap[STATUS_EMPTY] =      "STATUS_EMPTY";
        s_nameMap[STATUS_MISC_ERROR] = "STATUS_MISC_ERROR";

        initialized = 1;
    }

    return s_nameMap[s];
}
