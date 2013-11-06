#include <string>
#include <stdlib.h>
#include <string.h>
#include <iostream>

char *
StrtokXML(char **lasts,
          char* last_ch,
          bool* is_within_angle_brackets );

main()
{
    char buf[] = "<HTML> data1 <HEAD> <allow> 209.24.199 </Allow></Head></Html>";
    char * buf_copy = new char[strlen(buf)+1];
    strcpy( buf_copy, buf );
    char * lasts = buf_copy;
    char last_ch;
    bool is_within_angle_brackets;
    char * tok = StrtokXML( &lasts, &last_ch, &is_within_angle_brackets );
    while( tok )
    {
        cout << tok << endl;
        tok = StrtokXML( &lasts, &last_ch, &is_within_angle_brackets );
    }
}

/** Tokenizes at delimiters "><".  Indicates if token was found surrounded
 *  by angle brackets.  Throws away comments.  Doesn't understand escaped
 *  characters, so don't use '<' or '>' anywhere unless they really enclose
 *  a tag.
 *
 *  MT-safe.
 *
 *  @param lasts As in strtok_r (position to start from on next call).
 *         On first call, lasts should point to the beginning of the
 *         string you want to tokenize.
 *  @param last_ch Delimiter at which previous call ended.
 *  @param is_within_angle_brackets Indicates XML element.
*/
char *
StrtokXML(char **lasts,
          char* last_ch,
          bool* is_within_angle_brackets )
{
    bool found_left_angle = false;
    bool found_right_angle = false;
    char const * delim = "<>\0";

    // If starting at a delimiter, move past that.
    char * c = *lasts;

    c += strspn(c, delim);
    if(*c == '\0') return 0;
    if( (( c > *lasts ) && (*(c-1) == '<') )
    ||  (*last_ch == '<') )
    {
        found_left_angle = true;
    }

    // Find next delimiter and move just past that.
    // FIXME: We are going to fail if there's a <> inside a comment.
    char * d  = c + strcspn(c, delim);
    if(*d != '\0')
    {
        if( *d == '>' )
        {
            found_right_angle = true;
        }

        if( ( found_left_angle && !found_right_angle )
        ||  ( !found_left_angle && found_right_angle ) )
        {
            cerr << "Bad format -- unmatched angle brackets at "
                 << *last_ch << string(c) <<  endl;
            return 0;
        }

        *last_ch = *d;
        *d++ = '\0';
    }

    *lasts = d;
    *is_within_angle_brackets = found_left_angle && found_right_angle;

    if( c && *is_within_angle_brackets )
    {
        // Convert to uppercase
        if( c )
        {
            for( char * i = c; *i; i++ )
            {
                *i = toupper(*i);
            }
        }
    }

    return c;
}
