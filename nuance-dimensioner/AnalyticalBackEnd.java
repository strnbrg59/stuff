/*
  File:        AnalyticalBackEnd.java
  Author:      Sternberg
  Date:
  Description: This is where the computations take place.  
*/

import java.util.*;

/** This is where the calculations take place.  
 *  <P>
 *  FrontEnd holds a reference to an instance of this class.
*/
public class AnalyticalBackEnd implements BackEnd
{
    /** Find BHCV (busy hour call volume) given speech channels, holding
        time, and blocking probability.
        <P>
        Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params Copy of TrafficParams object held by FrontEnd.
     */
    public int solveForBHCV( TrafficParams traffic_params ) 
        throws BackEndException
    {
        final int   channels   = traffic_params.get_speech_channels();
        final int h            = traffic_params.get_holding_time();
        final double block_prob = traffic_params.get_block_prob();

        // Check for invalid values.
        if( (channels <= 0) || (channels > 1000000) )
        {
            throw new BackEndException("Invalid number of speech channels" );
        }
        if( (h <= 0) || (h>3600) )
        {
            throw new BackEndException( "Invalid call holding time" );
        }
        if( (block_prob <= 0) || (block_prob > 0.99) )
        {
            throw new BackEndException("Invalid blocking probability");
        }

        int newBHCV;

        newBHCV=(int)(channels*0.99*3600.0/h);
        //Dbg.println( "mCallHoldingTime="+h );
        int i=0;
        while ( GosB(channels,newBHCV,h) > block_prob - 1.0e-16 ) 
        {
            //Dbg.println( "i=" + (++i) );
            newBHCV = (int)(0.99*newBHCV);
        }

        while ( GosB(channels,newBHCV,h) <= block_prob + 1.0e-16 )
        { 
            newBHCV++;
        }
        
        return newBHCV - 1;
    }

    /** Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params Copy of the TrafficParams object held by FrontEnd.
     */
    public int solveForCarriedCalls( TrafficParams traffic_params ) 
        throws BackEndException
    {
        final int   channels   = traffic_params.get_speech_channels();        
        final double bhcv       = traffic_params.get_bhcv();
        final double block_prob = traffic_params.get_block_prob();

        // Check for invalid values.
        if( (channels <= 0) || (channels > 1000000) )
        {
            throw new BackEndException( "Invalid number of speech channels" );
        }
        if( (bhcv <= 0) || (bhcv > 36000000) )
        {
            throw new BackEndException( "Invalid call volume" );
        }
        if( (block_prob <= 0) || (block_prob > 0.99) )
        {
            throw new BackEndException("Invalid blocking probability");
        }

        return (int)((1-block_prob)*bhcv);
    }
    
    /** Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params Copy of the TrafficParams object held by FrontEnd.
     */
    public int solveForOfferedTraffic( TrafficParams traffic_params ) 
        throws BackEndException
    {
        final double h               = traffic_params.get_holding_time();
        final int   bhcv            = traffic_params.get_bhcv();

        // Check for invalid values.
        if( (bhcv <= 0) || (bhcv > 36000000) )
        {
            throw new BackEndException( "Invalid call volume" );
        }
        if( (h <= 0) || (h>3600) )
        {
            throw new BackEndException( "Invalid call holding time" );
        }

        return (int)(Math.round(100*h*bhcv/3600.0)/100.0);
    }

    /** Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params Copy of the TrafficParams object held by FrontEnd.
     */
    public double solveForBlockProb( TrafficParams traffic_params ) 
        throws BackEndException
    {
        final int   h               = traffic_params.get_holding_time();
        final int   bhcv            = traffic_params.get_bhcv();
        final int   channels        = traffic_params.get_speech_channels();

        // Check for invalid values.
        if( (h <= 0) || (h>3600) )
        {
            throw new BackEndException( "Invalid call holding time" );
        }
        if( (bhcv <= 0) || (bhcv > 36000000) )
        {
            throw new BackEndException("Invalid call volume");
        }
        if( (channels <= 0) || (channels > 1000000) )
        {
            throw new BackEndException( "Invalid number of speech channels" );
        }

        int N;

        double A = h*bhcv/3600.0;

        double E=1.0;
        for (N=1; N<=channels; N++) E=A*E/(N+A*E);

        // Round it off so it fits into a textfield, but don't round to 
        // anything bigger the 1/100000, or you'll experience puzzling
        // behavior; set the BHCV textfield and then click on blockprob,
        // then on channels, then again on blockprob, then on BHCV...it'll
        // keep on changing.
        return Math.round(100000.0*E)/100000.0;
    }

    /** Find number of speech channels consistent with the traffic parameters
     *  selected for the other textfields in the provisioning panel.
     *
     * <P>
     * @param traffic_params Copy of TrafficParams object held by FrontEnd.
     */
    public int solveForSpeechChannels( TrafficParams traffic_params ) 
        throws BackEndException
    {
        final int   h               = traffic_params.get_holding_time();
        final int   bhcv            = traffic_params.get_bhcv();
        final double block_prob      = traffic_params.get_block_prob();

        // Check for invalid values.
        if( (h <= 0) || (h>3600) )
        {
            throw new BackEndException( "Invalid call holding time" );
        }
        if( (bhcv <= 0) || (bhcv > 36000000) )
        {
            throw new BackEndException("Invalid call volume");
        }
        if( (block_prob <= 0) || (block_prob > 0.99) )
        {
            throw new BackEndException( "Invalid blocking probability" );
        }

        int N;
        double A = h*bhcv/3600.0;

        double E = 1.0;        // Flood p.98
        for (N = 1; E > block_prob + 1.0e-16; N++) 
            E=A*E/(N+A*E);

        return N-1;
    }

    /** Find minimum required RU per CPU : it's 1.33 times the greatest
     *  load of any single grammar of those selected.
     * <P>
     * @param selected_grammars The grammars in the grammar list widget.
    */
    public double solveForMinRUPerCPU( GrammarList selected_grammars )
    {
        double max_gram_load = 0.0;

        // Find the grammar with the greatest load.
        for (int i=0; i < selected_grammars.getItemCount(); i++ )
        {
            Grammar currGrammar = selected_grammars.grammarAt(i);
            double verificationOverhead = 0.0;
            if( selected_grammars.verifyAt( i ) == true )
            {
                verificationOverhead = 0.5;
            }
            if (max_gram_load < currGrammar.load + verificationOverhead)
            {
                max_gram_load = currGrammar.load + verificationOverhead;
            }
        }

        double result = max_gram_load*1.3333; // 3/4 threshold
        return Math.round( result * 100 )/100.0;
    }

    /** We use the model in Flood section 4.7, interpreting a request as an
     *  utterance.  Unfortunately, Flood contemplates iid requests, whereas
     *  ours are heterogeneous (whenever there's more than one grammar in our
     *  grammar model).  So we pretend our utterances are iid, with expected
     *  service time sum(LU_g*Dur_g*Req_g)/sum(Req_g), where LU_g is the LU 
     *  of the g-th grammar in our grammar model, Dur_g is its duration and 
     *  Req_g its requests (the expected number of times it comes up per call).
     *  This, unfortunately, leads to a paradox: add an easy grammar to a model
     *  that already has a hard grammar, and total required RUs goes down.
     * <P>
     * @param traffic_params Copy of the TrafficParams object held by FrontEnd.
     * @param selected_grammars The grammars in the grammar list widget.
    */
    public double solveForRecognitionPower( 
        TrafficParams traffic_params,
        GrammarList selected_grammars ) throws BackEndException
    {
        final double lat_thresh = traffic_params.get_latency_threshold();
        final double lat_prob    = traffic_params.get_latency_prob();

        // Sanity check on parameters.
        if( (lat_thresh <= 0) || (lat_thresh > 20) )
        {
            throw new BackEndException( "Invalid latency threshold" );
        }
        if( (lat_prob < 0.001) || (lat_prob > 99.999) )
        {
            throw new BackEndException( 
                "Invalid latency probability");
        }

        // We set N -- Flood's number-of-servers -- to 1, since that's the
        // only case where we have useful results for the M/G queue.

        Ah ah=computeAh( traffic_params, selected_grammars );

        // Find cpu_ru such that Gos(lat_thresh,ah.A,ah.h,ah.sd_h) == lat_prob
        class GosFunctor implements NewtonFunctor
        {
            GosFunctor( double lat_thresh, double A, double h, double sd_h )
            {
                m_lat_thresh = lat_thresh;
                m_A = A;
                m_h = h;
                m_sd_h = sd_h;
            }

            public double f( double cpu_ru )
            {
                double g = 
                  Gos( m_lat_thresh, m_A/cpu_ru, m_h/cpu_ru, m_sd_h/cpu_ru );
                return g - lat_prob;
            }

            private double m_lat_thresh;
            private double m_A;
            private double m_h;
            private double m_sd_h;
        }

        GosFunctor gos_functor = 
          new GosFunctor( lat_thresh,ah.A,ah.h,ah.sd_h );
        double initial_cpu_ru = ah.A;
        double cpu_ru = Newton.findZero( gos_functor, initial_cpu_ru );
        return Math.round(cpu_ru * 100 )/100.0;
    }

    /** Memory required per server.
     *  Algorithm by Chris Toulson.  Good for one-package servers.
     * <P>
     * @param traffic_params Copy of the TrafficParams object held by
     *    FrontEnd.
     * @param selected_grammars The grammars in the grammar list widget.
    */
    public double solveForMemory(
        TrafficParams traffic_params,
        GrammarList selected_grammars ) throws BackEndException
    {
        double mem_per_channel = 0;
        double mem_for_hmm_models = 0;
        double mem_for_node_array = 17;
        double typical_server_ru = 20.0; // good guess as of April 2000.

        for (int g=0; g < selected_grammars.getItemCount(); g++) 
        {
            Grammar curr_grammar = selected_grammars.grammarAt(g);
            double lu = curr_grammar.load;

            double tmp1 = Math.min( 7.5, 2 + 0.85 * lu );
            mem_per_channel = Math.max( mem_per_channel, tmp1 );

            double tmp2 = Math.min( 15, 2 * (lu - 0.5) );
            mem_for_hmm_models = Math.max( mem_for_hmm_models, tmp2 );

            mem_for_node_array += 1.2 * (lu - 0.5);
        }

        int n_ports = solveForSpeechChannels( traffic_params );
        int n_servers = (new Double( Math.ceil(
          solveForRecognitionPower( traffic_params,
                                    selected_grammars ) /
          traffic_params.get_ru_per_server() ))).intValue();
        double result = mem_for_node_array +
                        mem_for_hmm_models +
                        mem_per_channel * (n_ports + n_servers - 1)/n_servers;
        if( traffic_params.get_twopass() == true )
        {
            result *= 3.0;
        }
        
        return Math.round(100*result)/100.0;
    }


    /** A = Erlangs, 1/h = hazard rate for utt termination.  See comments before
     *  solveForRecognitionPower() for more details on how we interpret the model
     *  in Flood section 4.7.
     * <P>
     * @param traffic_params Copy of the TrafficParams object held by FrontEnd.
     * @param selected_grammars The lines in the Grammar-list widget.
     */
    private Ah computeAh( 
        TrafficParams traffic_params,
        GrammarList selected_grammars )
    {
        // Loop over all the grammars selected in the List widget; these
        // are the individual grammars which we combine to construct the
        // "grammar model" for which we wish to calculate required RU.

        double agg_req=0;  // agg stands for "aggregate".
        double agg_lu=0;
        double agg_lu_sqr=0;
        double agg_dur=0;

        for (int g=0; g < selected_grammars.getItemCount(); g++) 
        {
            Grammar currGrammar = selected_grammars.grammarAt(g);
            double verificationOverhead = selected_grammars.verifyAt( g ) ?
                                          0.5 : 0.0;
            double req = selected_grammars.requestsAt(g);
            double lu = currGrammar.load + verificationOverhead;
            double dur = currGrammar.duration;

            agg_lu += lu * dur * req;
            agg_lu_sqr += Math.pow(lu * dur,2) * req;
            agg_dur += dur * req;
            agg_req += req;
        }

        final int bhcv         = traffic_params.get_bhcv();
        final double block_prob = traffic_params.get_block_prob();
        // Should we calculate block_prob first, to make sure we use a 
        // number consistent with the BHCV and ports numbers
        // currently in the textfields?  No.  If the user doesn't like
        // our speech channel provisioning model, he should be able to force 
        // the RU calculation to use the numbers he thinks are correct for
        // ports, bhcv and block_prob.

        double A = agg_lu*bhcv*(1-block_prob)/3600; 
          // A is offered traffic
        double h = agg_lu/agg_req; 
          // h is mean service time per utterance
        double sd_h = Math.pow( 2*agg_lu_sqr/agg_req - 
                                Math.pow(agg_lu/agg_req,2), 
                                0.5 );  // sd of mixture of exponentials

        return (new Ah(A,h,sd_h));
    }

    /**
     * @ret Probability (pct) of delay less than t.  We set Flood's N,
     * the number of servers, to 1; there aren't any good results for
     * M/G/c queues.  With N=1, all the formulas become much simpler.
     *
     * @arg t Latency threshold.
     * @arg A Erlangs (lambda/mu in more standard queueing theory notation)
     * @arg h expected service time (1/mu)
     * @arg sd_h std dev of h -- used for M/G/1 adjustment.
    */
    private double Gos(double t, double A, double h, double sd_h)
    {
        // M/G/1 adjustment (Gross&Harris Eqtn 5.11).  Reduces to
        // unity, when there's only one grammar in the grammar model (in
        // that case, sd_h = h -- exponential dsn).
        double foo = 1.5;  // otherwise still too low -- try 95%/97.5% test
        double mg1_adj = 1 + (A/2)*( Math.pow(sd_h/h,2) - 1 ) * foo;

        // Tpb ("T-prime-bar" -- expected delay, given delay>0) --
        // Flood's Eqtn 4.20a -- times M/G/1 adjustment.
        double Tpb = h/(1-A) * mg1_adj;

        // P(TD>=t) -- prob(delay>=t) -- Flood's Eqtn 4.21b :
        double PrGrtrDel = A * Math.exp( -t/Tpb );

        // Adjust PrGrtrDel for computation time, h, by adding h to delay
        double adjusted = PrGrtrDel * Math.exp( h/Tpb );

        return ( 1 - (adjusted>1.0?1.0:adjusted) ) * 100;
    }

    private double GosB(int ports, int BHCV, int holding)
    {
        int N;
        double E=1.0;
        double A=(holding+0.0)*BHCV/3600.;

        for (N=1;N<=ports;N++) {
            E=A*E/(N+A*E);
            if (E==0) break;
        }
        return E;
    }

    /** For testing this class. */
    public static void main( String[] args )
    {
        AnalyticalBackEnd backend = new AnalyticalBackEnd();
        TrafficParams traffic_params = new TrafficParams();
        backend.initializeTrafficParams( traffic_params );

        // Loop.  Each time, tryNewValues() makes some kind of modification
        // to the data in traffic_params.
        for( int i=0;i<5;i++ )
        {
            backend.modifyTrafficParams( traffic_params );        
        
            try
            {
                // Recognition power.  We need to pass a GrammarList object.
                GrammarList gl = new GrammarList(1);
                gl.addElement( GrammarDatabase.elementAt(0), 1.0, false );
                System.out.println( "recognition_power=" + 
                    backend.solveForRecognitionPower( traffic_params, gl));
                System.out.println( "min_RU_per_CPU=" +
                    backend.solveForMinRUPerCPU( gl ));

                traffic_params.set_latency_prob( 97.5 );
                gl.addElement( GrammarDatabase.elementAt(1), 1.0, false );
                System.out.println( "recognition_power=" + 
                    backend.solveForRecognitionPower( traffic_params, gl));
                System.out.println( "min_RU_per_CPU=" +
                    backend.solveForMinRUPerCPU( gl ));

                // Stress test:
                gl.addElement( GrammarDatabase.elementAt(2), 100.0, false );
                System.out.println( "recognition_power=" + 
                    backend.solveForRecognitionPower( traffic_params, gl));
                System.out.println( "min_RU_per_CPU=" +
                    backend.solveForMinRUPerCPU( gl ));

                int bhcv, carried, offered;
    
                System.out.println( "bhcv=" + 
                    (bhcv = backend.solveForBHCV(traffic_params)));
                traffic_params.set_bhcv( bhcv );
    
                System.out.println( "carried_calls=" + 
                    (carried = backend.solveForCarriedCalls(traffic_params)));
                traffic_params.set_carried_calls( carried );
    
                System.out.println( "offered_traffic=" + 
                    (offered = backend.solveForOfferedTraffic(traffic_params)));
                traffic_params.set_offered_traffic( offered );
    
                System.out.println( "block_prob=" + 
                    backend.solveForBlockProb( traffic_params));
    
                System.out.println( "speech_channels=" + 
                    backend.solveForSpeechChannels( traffic_params));

                System.out.println( "memory_requirement=" + 
                    backend.solveForMemory( traffic_params, gl));
            }
            catch( BackEndException bee ) 
            {
                System.exit(1);
            }

            System.out.println("");
        }
    }

    /** Used in main(), the unit testing stub. */
    private void initializeTrafficParams( TrafficParams tp )
    {
        tp.set_bhcv( 5000 );
        tp.set_holding_time( 10 );
        tp.set_block_prob( 0.02 );
        tp.set_speech_channels( 1 );

        tp.set_latency_threshold( 2.0 );
        tp.set_latency_prob( 95.0 );
        tp.set_ru_per_server( 20 );
    }

    /** Used in main(), the unit testing stub.
     *  Modify the data members of the TrafficParams instance, so we can 
     *  try BackEnd out on different configurations.
    */
    private void modifyTrafficParams( TrafficParams tp )
    {
        m_number_of_calls_to_modifyTrafficParams ++;
        int k=m_number_of_calls_to_modifyTrafficParams;
  
        tp.set_speech_channels( tp.get_speech_channels() + k );
        System.out.println( 
            "speech_channels reset to " + tp.get_speech_channels());
    }

    /** Used in modifyTrafficParams(), which is used for unit testing. */
    private static int m_number_of_calls_to_modifyTrafficParams;
}

class Ah
{
    double A;
    double h;
    double sd_h; // standard deviation of individual grammar h's weighted
                 // by their requests in the grammar model.  Used for the
                 // M/G/1 queue adjustment (Gross&Harris eqtn 5.11).
 
    Ah( double A, double h, double sd_h )
    {
        this.A = A;
        this.h = h;
        this.sd_h = sd_h;
    }
}
