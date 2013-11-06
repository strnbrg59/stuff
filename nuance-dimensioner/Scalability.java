/*
  File:        Scalability.java
  Author:      Sternberg
  Date:        October 20, 2000
  Description: Study RU/BHCV, RU/port etc
*/

import java.util.*;

public class Scalability
{
    public static void main( String[] args )
    {
        //
        // Initialize grammar model.
        // Grammar constructor arguments are: name, LU, duration, "".
        // GrammarList.addElement args are Grammar, requests, false.
        GrammarList gl = new GrammarList(1);
        gl.addElement( new Grammar("grammar1",2.4,2.0), 5.0, false );

        AnalyticalBackEnd backend = new AnalyticalBackEnd();
        Scalability scalability = new Scalability();

        TrafficParams tp = new TrafficParams();
        tp.set_holding_time( 91 );
        tp.set_block_prob( 0.02 );
        tp.set_latency_threshold( 2.0 );
        tp.set_latency_prob( 95.0 );

        int bhcv[] = {500,1000,2000,5000,10000,20000,50000,100000,500000};

        try
        {
            for( int i=0;i<bhcv.length;i++ )
            {
                tp.set_bhcv( bhcv[i] );
                int ports = backend.solveForSpeechChannels( tp );
                tp.set_speech_channels( ports );
                double RU = backend.solveForRecognitionPower( tp, gl);

                System.out.println( 
                    "bhcv= " + bhcv[i] + " " +

                    "ports= " + ports + " " +

                    "RU= " + RU + " " +
                    
                    "ports/bhcv= " + Math.round(10000.0*ports/bhcv[i])/10000. +" "+

                    "RU/bhcv= " + Math.round(10000*RU/bhcv[i])/10000.0 + " " +

                    "RU/port= " + Math.round(10000*RU/ports)/10000.0 );
            }
        } catch ( Exception e ) { System.out.println(e.toString()); }
    }
}




