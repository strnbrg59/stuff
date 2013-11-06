/*
  File:        PKC.java
  Author:      Sternberg
  Date:        May 2000
  Description: Public key cryptography stuff.  License file contains 
     "Licensed to <somebody> at <some company>" text, encrypted with
     private key known only to Ted's development machine.  This .java
     file decrypts that text, using the public key and modulus, which
     are hard-coded here.  Obviously, an attacker could generate his
     own license using his own private key, and then surgically insert
     the corresponding public key into the .class file here.

*/

import java.io.*;
import java.math.*;
import java.util.*;

public class PKC
{
    /** Usage:  If args[0]=="encrypt", then it prints out encrypted
     *  args[1].  If args[0]=="decrypt", then it decrypts what it
     *  finds in the file identified by args[1].
     *  Takes p and e from files named "modulus" and 
     *  "exponent" in the current working directory.
     *
    */
    public static void main( String[] args )
    {
        BigInteger p = PKC.getBigIntegerFromFile( "modulus");
        BigInteger d = 
          PKC.getBigIntegerFromFile( "public_exponent");
        BigInteger e = 
          PKC.getBigIntegerFromFile( "exponent" );
        PKC rsa = new PKC();

        if( args[0].equals( "encrypt" ) && args.length == 2 )
        {
            System.out.println( rsa.encrypt( args[1], p, e ));
            System.exit(0);
        }
        if( args[0].equals( "encryptCGI" ) && args.length == 2 )
        {
            CGIForm cgi_form = new CGIForm();
            Hashtable query_hash = cgi_form.parseCGIQueryString( args[1] );
            String licensee = (String)(query_hash.get("licensee"));
            System.out.println( "License for "+ licensee + "<BR>" );
            System.out.println( "Cut as indicated and save in a file called "+
              "license.dat, in the directory where you intend to run " +
              "Dimensioner" );
            System.out.println( "-------- cut here --------\n" );
            System.out.println( rsa.encrypt(licensee, p, e) );
            System.out.println( "-------- cut here --------" );
            System.exit(0);
        }

        if( args[0].equals( "decrypt" ) && args.length == 2 )
        {
            System.out.println( rsa.decryptFromFile( 
              args[1], 
              new BigInteger("d12afb16c43ea524b190dbc792cedcb07e2954a2f0c0fd5a277c7d7e186ab95592d46cae3bb1648f421e2478c3c15cf9a0f73a983191f27a347b67aa1bf6ed39",16),
            //p, 
              new BigInteger("010001",16)
              //d
            ));
            System.exit(0);
        } else
        {
            String cypher = rsa.encrypt( args[0], p, e );
            System.err.println( "cypher = " + cypher );

            String plain_text = rsa.decrypt( cypher, p, d );
            System.err.println( "plain_text = " + plain_text );
        }
    }

    String decryptFromFile( String filename, 
                            BigInteger p, 
                            BigInteger d )
    {
      String plain_text = new String();

      try
      {
        BufferedReader reader = 
          new BufferedReader( new FileReader( filename ));
        StringBuffer cypher = new StringBuffer();
        String one_line = reader.readLine();
        while( one_line != null )
        {
            cypher.append( one_line.trim() + "\n");
            one_line = reader.readLine();
        }
        
        plain_text = decrypt( new String(cypher), p, d );
      }
      catch( Exception e ) 
      { 
        System.err.println(e.toString());
        plain_text = "nobody! (I.e. in use now without a valid license)";
      }

      return plain_text;
    }

    /** If necessary, break plain_text up into blocks no longer than 
     *  p.  Blocks' cyphertexts are separated by '\n';
    */
    String encrypt( String plain_text, 
                    BigInteger p,
                    BigInteger e )
    {
        final int block_size = 64;
        int p_length = p.toString(16).length()/2; // # bytes
        if( p_length < block_size )
        {
            System.err.println( "Do not use modulus < " + block_size +
              " bytes long!" );
            System.exit(1);
        }

        String result = new String("");

        for( int start=0; 
             start < plain_text.length(); 
             start += block_size )
        {
            int end = Math.min( start + block_size, 
                                plain_text.length() );
            if( end > start )
            {
                result +=
                  encrypt_block( plain_text.substring(start, end), p, e ) +
                   "\n";
            }
        }

        return result;
    }

    private String encrypt_block( String plaintext_block,
                                  BigInteger p,
                                  BigInteger e )
    {
        final char[] hex_digits = {'0','1','2','3','4','5','6','7',
                                   '8','9','a','b','c','d','e','f'};

        byte[] byte_array = plaintext_block.getBytes();

        String[] hex_array = new String[ plaintext_block.length() ];
        for( int i=0;i<plaintext_block.length();i++ )
        {
            int hex = (new Byte(byte_array[i])).intValue();
            char[] char_array = new char[2];
            char_array[0] = hex_digits[ hex/16 ];
            char_array[1] = hex_digits[ hex - 16*(hex/16) ];
            hex_array[i] = new String( char_array );
        }

        BigInteger plaintext_bigint = hex2bigint( hex_array );
        BigInteger cypher_bigint = plaintext_bigint.modPow( e, p );
        String result = cypher_bigint.toString(16);
        return result;
    }

    /** @arg cypher is the hexadecimal representation of the cyphertext.
     *  If there are '\n' characters, they delimit the bounds of individually
     *  encrypted blocks.
    */
    String decrypt( String cypher, BigInteger p, BigInteger d )
    {
        StringBuffer result = new StringBuffer();

        StringTokenizer st = new StringTokenizer( cypher, "\n" );
        while( st.hasMoreTokens() )
        {
            String block = st.nextToken();
            
            BigInteger cypher_bigint = new BigInteger( block, 16 );
            BigInteger plain_bigint = cypher_bigint.modPow( d, p );
            byte[] plain_byte = plain_bigint.toByteArray();

            for( int j=0;j<plain_byte.length;j++ )
            {
                char c = (char)plain_byte[j];
                result.append(c);
            }
        }
        return new String(result);
    }
        

    /** Convert a bigendian hexadecimal string, e.g. b5 4c bf eb 48 ...
     *  into a Java BigInteger.
    */
    public BigInteger hex2bigint( String[] str )
    {
        String smash = new String("");
        for( int i=0;i<str.length;i++ )
        {
            smash += str[i];
        }

        BigInteger result = new BigInteger( smash, 16 );
        return result;
    }

    /** Format is bigendian hexadecimal, e.g. b5 4c bf eb 48 ... */
    public static BigInteger getBigIntegerFromFile( String filename )
    {
      BigInteger result = new BigInteger("0");

      try
      {
        BufferedReader reader = 
          new BufferedReader( new FileReader( filename));
        String the_one_line = reader.readLine();
        StringTokenizer st = new StringTokenizer( the_one_line );
        String smash = new String("");
        while( st.hasMoreTokens() )
        {
            smash += st.nextToken();
        }

        result = new BigInteger( smash, 16 );
      }
      catch( Exception e )
      {
          e.printStackTrace();
          System.exit(1);
      }

      return result;
    }
}
