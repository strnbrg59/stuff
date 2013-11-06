/*
  File:        CallModel.java
  Author:      Sternberg
  Date:        September 2000
  Description: Calculates requests per grammar.  Think of the dialog as a
    Markov chain whose states, s_g, g=0,1,...n, are requests for recognition
    of utterances from the various grammars g.  State 0 is defined as the
    initial state.  Now let p_{ij} be the probability
    of a transition from state i to state j.  Let q_{it} be the probability
    of the system being in state i at time t, t=0,1,....  
    
    q_{it} = \sum_k p_{ki}q_{k,t-1}

    Now let c_i be the expected number of visits to state i.  We calculate
    this from

    c_i = \sum_t q_{it}
*/

import java.io.*;
import java.util.*;

public class CallModel
{
    /** Usage: either "java CallModel <transition-probabilities-filename>"
     *  or "java CallModel cgi len" (and it reads len bytes from stdin).
    */
    public static void main( String[] args )
    {
        CallModel cm = new CallModel();
        double[][] p = null;
        String[] grammar_names = null;
        try
        {
            if( args.length == 1 )
            {
                p = cm.loadTransitionProbabilitiesFromFile( args[0] );
            } else
            if( (args.length == 2) && args[0].equals("cgi") )
            {
                int content_length = Integer.parseInt( args[1] );
                byte[] put_content = new byte[content_length];
                System.in.read( put_content );
                String put_str = new String( put_content );
                p = cm.loadTransitionProbabilitiesFromCgiQueryString(
                  put_str);
                grammar_names =
                  cm.loadGrammarNamesFromCgiQueryString(put_str, p.length);
            } else
            {
                throw new Exception( "Usage: \"java CallModel " +
                  "<transition-probabilities filename>\"" );
            }
        }
        catch( Exception e ) 
        { 
            System.out.println(e.toString());
            System.exit(1);
        }

        double[] c = cm.computeExpectedRequests( p );

        System.out.println( 
          "\nExpected number of requests for each grammar:");
        for( int g=0;g<p.length;g++ )
        {
            //
            // Print expected requests.
            //
            if( grammar_names != null )
            {
                System.out.print( grammar_names[g] + ": " );
            } else
            {
                System.out.print( "grammar[" + g + "] : ");
            }
            System.out.println( Math.round( 1000*c[g] )/1000.0 );
        }
    }

    /** Return true if the probabilities q appear to have converged
     *  satisfactorily.  In the absence of a good theorem, we'll go with
     *  an ad-hoc convergence criterion: let n be the number of grammars;
     *  we consider convergence complete when the last n
     *  probabilities q (i.e. corresponding to the last n epochs of the
     *  given grammar, the last n terms added to the sum of the q's -- the
     *  expected number of visits to this grammar, which is what we're
     *  calculating) sum to less than epsilon.  We don't want to look
     *  at just the last q, as there might be some periodicities here.
    */
    boolean converged( LazyMatrix q, int grammar, int T, double epsilon )
    {
        double sum = 0;
        int k = Math.min( q.numRows(), T );
        for( int t = T-1; t >= T-k; t-- )
        {
            sum += q.get( grammar, t );
        }

        if( sum < epsilon )
        {
            //System.out.println( "converged: sum = " + sum );
            return true;
        } else
        {
            //System.out.println( "did not converge: sum = " + sum );
            return false;
        }
    }

    /** Expects to find transition probabilities in a square array, in the
     *  infile.  Ignores lines that match ^# or ^$.
     *  Assembles all the data lines into a Vector of Strings, and passes
     *  that to loadTransitionProbabilitiesFromStrings().
    */
    private double[][] 
    loadTransitionProbabilitiesFromFile( String infilename )
      throws Exception
    {
        BufferedReader reader = new BufferedReader( 
          new FileReader( infilename ));
        String one_line = null;

        Vector data_lines = new Vector();
        while( null != (one_line = reader.readLine() ))
        {
            if( (one_line.trim().length() != 0)
            &&  (one_line.trim().charAt(0) != '#'))
            {
                data_lines.addElement( one_line );
            }
        }

        return loadTransitionProbabilitiesFromStrings( data_lines );
    }

    /** The (i,j)th transition probability is the value the QUERY_STRING
     *  variable named transprob_i_j.  (Actually, we're using the POST
     *  method now, but the processing is the same once we get stdin into
     *  the variable here known as query_string.)
    */
    private double[][] 
    loadTransitionProbabilitiesFromCgiQueryString( String query_string )
      throws Exception
    {
        Hashtable transprobs = CGIForm.parseCGIQueryString( query_string );

        // Figure out the dimension of the transition prob matrix.  That'll
        // be the square root of the number of QUERY_STRING variables
        // whose names begin with "transprob_".

        Enumeration e = transprobs.keys();
        int num_transprobs = 0;
        while( e.hasMoreElements() )
        {
            String key_name = (String)e.nextElement();
            if( key_name.startsWith( "transprob_" ) )
            {
                num_transprobs ++;
            }
        }

        int num_grammars = (int)(Math.round(Math.pow( num_transprobs, 0.5)));

        double[][] p = new double[num_grammars][num_grammars];

        // Go through the QUERY_STRING again, this time extracting the
        // transition probabilities.
        e = transprobs.keys();
        while( e.hasMoreElements() )
        {
            String key_name = (String)e.nextElement();
            if( key_name.startsWith( "transprob_" ) )
            {
                StringTokenizer st = new StringTokenizer( key_name, "_" );
                String tok = st.nextToken(); // throw away

                tok = st.nextToken();
                int row = Integer.parseInt( tok );
                tok = st.nextToken();
                int col = Integer.parseInt( tok );

                String val_str = (String)(transprobs.get( key_name ));
                double val;
                if( val_str.trim().length() == 0 )
                {
                    val = 0.0;
                } else
                {
                    val = (new Double( val_str )).doubleValue();
                }
                p[row][col] = val;
            }
        }

        // Check that all rows sum to less than 1.0.  (Actually, some
        // can add up to exactly 1.0 as long as at least some other
        // row -- representing a grammar accessible with positive probability
        // from the row that sums to 1.0 -- sums to less than 1.0.
        // That's enough to ensure boundedness of the expected-visits.
        // But it's fine for our purposes to require < 1.0; after all, there
        // should always be some probability that the call ends after any
        // given utterance.
        for( int i=0;i<num_grammars;i++ )
        {
            double sum = 0;
            for( int j=0;j<num_grammars;j++ )
            {
                sum += p[i][j];
            }
            if( sum >= 1.0 )
            {
                // For output to be visible through CGI, you have to print
                // to stdout; stderr goes to some log file where no one will
                // notice it.
                System.out.println( "Error! Transition probabilities in " +
                  "row " + (i+1) + " sum to " + sum + ".  Should sum to " +
                  " less than 1." );
            }
        }

        System.out.println( 
          "Loaded the following transition prob matrix:" );
        for( int g=0;g<num_grammars;g++ )
        {
            for( int h=0;h<num_grammars;h++ )
            {
                System.out.print( p[g][h] + " " );
            }
            System.out.println("");
        }

        return p;
    }

    /** The i-th grammar name is in the hidden field called grammar_name_i.
     *  @arg query_string The QUERY_STRING environment variable in this CGI
     *                    program's environment.
     *  @arg n            The number of grammars.
     *
     *  We're actually using the POST method now, but the processing is 
     *  the same once we get stdin into the variable here known as 
     *  query_string.
    */
    private String[] 
    loadGrammarNamesFromCgiQueryString( String query_string, int n )
      throws Exception
    {
        Hashtable query_string_items = CGIForm.parseCGIQueryString( query_string );
        String[] grammar_names = new String[ n ];

        // Ideally, we'd just grab the items whose names start with "grammar_name_"
        // and return an array of them.  Unfortunately, the order of items
        // in a Hashtable is not guaranteed.  So we need to do our own sorting
        // using the numerical tag after "grammar_name_"
        
        Enumeration e = query_string_items.keys();
        while( e.hasMoreElements() )
        {
            String key_name = (String)e.nextElement();
            if( key_name.startsWith( "grammar_name_" ) )
            {
                StringTokenizer st = new StringTokenizer( key_name, "_" );
                st.nextToken();
                st.nextToken();
                int grammar_num = Integer.parseInt( st.nextToken() );
                grammar_names[ grammar_num ] = (String)(query_string_items.get(key_name));
            }
        }

        return grammar_names;
    }


    private double[][] loadTransitionProbabilitiesFromStrings( Vector data )
      throws Exception
    {   
        // Pick up first line, and count the number of items in it: that's
        // the number of grammars in the model.
        StringTokenizer st = 
          new StringTokenizer( (String)(data.elementAt(0)));
        int dim = st.countTokens();
        double[][] result = new double[dim][dim];

        // Now process all the lines of data.
        for( int i=0;i<data.size();i++ )
        {
            st = new StringTokenizer( (String)(data.elementAt( i )));
            double prob_sum = 0; // sum of probabilities (we check for < 1 )
            for( int g=0;g<dim;g++ )
            {
                String tok = null;
                try
                {
                    tok = st.nextToken();
                }
                catch( NoSuchElementException e)
                {
                    throw new Exception( "Not enough numbers in row " +
                      (i+1) + " of transition probability matrix" );
                }

                result[i][g] = 
                  Double.valueOf(tok).doubleValue();
                prob_sum += result[i][g];
            }
            
            if( prob_sum >= 1 )
            {
                throw new Exception( "Error: probabilities in row " +
                  (i+1) + " add up to " + prob_sum + 
                  " (should add up to less than 1.0)." );
            }
        }

        System.out.println( "Loaded the following transition prob matrix:" );
        for( int g=0;g<dim;g++ )
        {
            for( int h=0;h<dim;h++ ) System.out.print( result[g][h] + " " );
            System.out.println( "" );
        }

        return result;
    }

    double[] computeExpectedRequests( double[][] trans_prob_matrix )
    {
        double[][] p = trans_prob_matrix;
        double[] c = new double[ p.length ];

        LazyMatrix q = new LazyMatrix( p );
        for( int g=0; g < p.length; g ++ )
        {
            final int t_limit_increment = 20;
            final int absolute_t_limit = 1000; // ensures program exits.
            int t_limit = t_limit_increment;
            int t=0;
            while( t < t_limit )
            {
                double q_gt = q.get(g,t);
                //System.out.println("q( " + g + " , " + t + " ) = " + q_gt);
                c[g] += q.get(g,t);

                t++;
                if( (t == t_limit) && (! converged( q, g, t, 1E-10 ))
                &&  (t < absolute_t_limit) )
                {
                    t_limit += t_limit_increment;
                }
            }
        }
    
        return c;
    }
}

/** Used for storing and evaluating q_{it}, the probability of the system
 *  being in state i at time t.
*/
class LazyMatrix
{
    LazyMatrix( double[][] p )
    {
        m_p = p; // transition probability matrix.  Initial state is state 0.
        m_q = new Vector[ p.length ];
        for( int i=0;i<m_q.length;i++ )
        {
            m_q[i] = new Vector();
        }

        // Initialize with the easy, low-order, values.
        m_q[0].addElement( new Double(1.0) );
        m_q[0].addElement( new Double( m_p[0][0] ));
        for( int g=1;g<p.length;g++ )
        {
            m_q[g].addElement( new Double(0.0) );
            m_q[g].addElement( new Double( m_p[0][g] ));
        }
    }

    // If already memoized, just look it up.  Otherwise, evaluate
    // recursively (and memoize).
    double get( int i, int t )
    {
        double result = 0;
        //System.out.println("LazyMatrix.get(" + i + "," + t + ")" );

        if( m_q[i].size() > t )
        {
            result = ((Double)(m_q[i].elementAt(t))).doubleValue();
        } else
        {
            result = 0;
            for( int k=0;k<m_p.length;k++ )
            {
                result += m_p[k][i] * get(k,t-1);
            }
    
            m_q[i].addElement( new Double(result) );
//          System.out.println( "addElement, t= " + t + " size()= " +
//            m_q[i].size() );
        }

        //System.out.println("LazyMatrix.get(" + i + "," + t + ")= "+result);
        return result;
    }

    int numRows() { return m_q.length; }
    int numCols( int row ) { return m_q[row].size(); }

    private double[][] m_p;
    private Vector[] m_q;
}
