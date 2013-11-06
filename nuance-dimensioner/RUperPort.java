/*
  File:        RUperPort.java
  Author:      Sternberg
  Date:        March 2, 2000
  Description: Empirical investigation of RU/port requirements.
*/

import java.util.*;

public class RUperPort
{
    public static void main( String[] args )
    {
        AnalyticalBackEnd backend = new AnalyticalBackEnd();
        TrafficParams traffic_params = new TrafficParams();
        initializeTrafficParams( traffic_params );

        try
        {
            // Recognition power.  We need to pass a GrammarList object.
            GrammarList gl = new GrammarList(1);
            gl.addElement( new Grammar("foo",1.0,1.0), 1.0, false );

            int speech_channels;
            double required_ru;

            for( int bhcv = 100; bhcv < 30100; bhcv += 100 )
            {
                for( int holding = 10; holding < 190; holding += 10 )
                {
                    traffic_params.set_bhcv( bhcv );
                    traffic_params.set_holding_time( holding );

                    speech_channels =
                      backend.solveForSpeechChannels( traffic_params);
                    traffic_params.set_speech_channels( speech_channels );
                    required_ru =
                      backend.solveForRecognitionPower( traffic_params, gl);

                    System.out.println( "" + bhcv + " " + holding + " " +
                      speech_channels + " " + required_ru );
                }
            }
        }
        catch( BackEndException bee ) 
        {
            System.exit(1);
        }
    }

    private static void initializeTrafficParams( TrafficParams tp )
    {
        tp.set_block_prob( 0.02 );
        tp.set_latency_threshold( 2.0 );
        tp.set_latency_prob( 95.0 );
    }

}
