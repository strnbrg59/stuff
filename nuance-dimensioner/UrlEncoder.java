/*
  File:        UrlEncoder.java
  Author:      Sternberg
  Date:
  Description: Takes a String and converts all its spaces to 
*/

public class UrlEncoder
{
    public static void main( String[] args )
    {
        Character ch = new Character( args[0].charAt(0) );
        System.out.println( "You typed " + ch );
        System.out.println( "Hex is " + UrlEncoder.char2hex(ch.charValue()) );
    }

    /** Return hex representation of argument. */
    public static String char2hex( char c )
    {
        int numvalue = (new Byte( (byte)c )).byteValue();

        StringBuffer hexrep = new StringBuffer(2);
        hexrep.insert(0, m_hex[numvalue/16]);
        hexrep.insert(1, m_hex[numvalue % 16]);
        return new String(hexrep);
    }

    /** Return a copy of str, after converting all non-alphanumeric characters
     *  to '%' + their hex codes.
    */
    public static String encode( String str )
    {
        StringBuffer sb = new StringBuffer();
        for( int i=0; i<str.length(); i++ )
        {
            if( Character.isLetterOrDigit( str.charAt( i )))
            {
                sb.append( str.charAt( i ) );
            }
            else
            {
                sb.append( "%" + UrlEncoder.char2hex( str.charAt( i )));
            }
        }

        return new String( sb );
    }

    private static char[] m_hex = 
      {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
}

    
