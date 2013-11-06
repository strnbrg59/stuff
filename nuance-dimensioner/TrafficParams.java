/*
  File:        TrafficParams.java
  Author:      Sternberg
  Date:
  Description: Structure, which holds the variables that correspond to
               what we display in the provisioning panel.
*/

import java.util.*;

/** Structure to hold bhcv, holding_time, block_prob, etc; and getters and
    setters for them.
    This class has almost 70 lines, but there's little here besides getters
    and setters, and a toString() function.

    <P>
    The only purpose of grouping
    these "traffic parameters" into a class is to make it easier for a 
    reader to keep track of where the various parameters are set, and where
    they are not set but merely used.

    <P>
    The variables are not initialized here.  They are initialized when we
    construct the TextFields in which their values appear.
*/
public class TrafficParams
{
    // ---------------------------
    // getters
    //

    // provisioning_panel
    public int   get_bhcv()               { return bhcv; }
    public int   get_carried_calls()      { return carried_calls; }
    public int   get_holding_time()       { return holding_time; }
    public double get_offered_traffic()    { return offered_traffic; }
    public double get_block_prob()         { return block_prob; }
    public int   get_speech_channels()    { return speech_channels; }

    // call_model_panel
    public double get_requests_per_call()  { return requests_per_call; }

    // rl_panel
    public double get_latency_threshold()  { return latency_threshold; }
    public double get_latency_prob()        { return latency_prob; }

    // ru_panel
    public double get_required_ru()        { return required_ru; }
    public double get_min_ru_per_cpu()     { return min_ru_per_cpu; }
    public double get_memory()             { return memory; }
    public double get_ru_per_server()      { return ru_per_server; }

    public boolean get_twopass() { return twopass; }

    // ---------------------------
    // setters
    //

    // provisioning_panel
    public void set_bhcv(int a)                { bhcv = a; }
    public void set_carried_calls(int a)       { carried_calls = a; }
    public void set_holding_time(int a)        { holding_time = a; }
    public void set_offered_traffic(double x)   { offered_traffic = x; }
    public void set_block_prob(double x)        { block_prob = x; }
    public void set_speech_channels(int a)     { speech_channels = a; }

    // call_model_panel
    public void set_requests_per_call(double x) { requests_per_call = x;}

    // rl_panel
    public void set_latency_threshold(double x) { latency_threshold = x;}
    public void set_latency_prob(double x)       { latency_prob = x; }

    // ru_panel
    public void set_required_ru(double x)       { required_ru = x; }
    public void set_min_ru_per_cpu(double x)    { min_ru_per_cpu = x; }
    public void set_ru_per_server(double x)     { ru_per_server = x; }
    public void set_memory(double x)            { memory = x; }

    public void set_twopass( boolean b ) { twopass = b; }

    // ---------------------------
    // data members -- private
    //

    // provisioning_panel
    private int   bhcv;
    private int   carried_calls;
    private int   holding_time;
    private double offered_traffic;
    private double block_prob;
    private int   speech_channels;

    // call_model_panel
    private double requests_per_call;

    // rl_panel
    private double latency_threshold;
    private double latency_prob;

    // ru_panel
    private double required_ru;
    private double min_ru_per_cpu;
    private double ru_per_server; // set by user
    private double memory;

    private boolean twopass;  // 2-pass acoustic model

    // -----------------------------
    // copy constructor (Java doesn't generate one automatically.  If it did,
    // that would be all we needed.
    //
    public TrafficParams( TrafficParams tp )
    {
        this.bhcv = tp.bhcv;
        this.carried_calls = tp.carried_calls;
        this.holding_time = tp.holding_time;
        this.offered_traffic = tp.offered_traffic;
        this.block_prob = tp.block_prob;
        this.speech_channels = tp.speech_channels;
        this.requests_per_call = tp.requests_per_call;
        this.latency_threshold = tp.latency_threshold;
        this.latency_prob = tp.latency_prob;
        this.required_ru = tp.required_ru;
        this.min_ru_per_cpu = tp.min_ru_per_cpu;
        this.ru_per_server = tp.ru_per_server;
        this.memory = tp.memory;
        this.twopass = tp.twopass;
    }

    // string constants for use in toString() and fromString().
    private static final String BHCV = "Busy hour call attempts: ";
    private static final String HOLDING = "Call holding time (sec): ";
    private static final String BLOCK_PROB = "Probability of blocking: ";
    private static final String CHANNELS = "Speech Channels:         ";
    private static final String THRESHOLD = "Latency threshold (sec): ";
    private static final String LATENCY_PROB = "Prob(latency<threshold): ";
    private static final String REQUIRED_RU = "Recognition power (RU):  ";
    private static final String TWOPASS = "Using 2-pass:            ";
    private static final String MIN_RU_PER_CPU = "Minimum RU per server :  ";
    private static final String RU_PER_SERVER = "RU per server:           ";
    private static final String MEMORY = "Memory per server:       ";

    public String toString()
    {
        String result = new String(
          BHCV + bhcv + "\n" +
          HOLDING + holding_time + "\n" +
          BLOCK_PROB + block_prob + "\n" +
          CHANNELS + speech_channels + "\n" +
          THRESHOLD + latency_threshold + "\n" +
          LATENCY_PROB + latency_prob + "\n" +
          REQUIRED_RU  + required_ru + "\n" +
          TWOPASS + twopass + "\n" +
          MIN_RU_PER_CPU + min_ru_per_cpu + "\n" +
          RU_PER_SERVER + ru_per_server + "\n" +
          MEMORY + memory );
        return result;
    }

    /** Support for the "Load" menu option.  Meant to be used after some
     *  header material has already been removed from the summary file.
     *  @arg summary_line is, e.g. "Minimum RU per server : 2.32".  This is a typical
     *       line from the output produced by the "Save as..." menu option.
     */
    public void fromString( String summary ) throws BackEndException
    {
        StringTokenizer st = new StringTokenizer( summary, "\n" );
        bhcv = summary2int( BHCV, st );
        holding_time = summary2int( HOLDING, st );
        block_prob = summary2double( BLOCK_PROB, st );
        speech_channels = summary2int( CHANNELS, st );
        latency_threshold = summary2double( THRESHOLD, st );
        latency_prob = summary2double( LATENCY_PROB, st );
        required_ru = summary2double( REQUIRED_RU, st );
        twopass = summary2boolean( TWOPASS, st );
        min_ru_per_cpu = summary2double( MIN_RU_PER_CPU, st );
        ru_per_server = summary2double( RU_PER_SERVER, st );
        memory = summary2double( MEMORY, st );
    }

    /** Support for fromString().
     *  @arg label is there to make it hard to code improperly. 
     *  @arg line_tokenizer breaks multiline summary at the newlines.
     */
    private int summary2int( String label, StringTokenizer per_line_tokenizer )
      throws BackEndException
    {
        int result = -999;
        
        //
        // Check format of summary_line.
        //
        if( ! per_line_tokenizer.hasMoreTokens() )
        {
            throw new BackEndException(
              "Error: no " + label + " line in load file." );
        }
        String one_line = per_line_tokenizer.nextToken();
        StringTokenizer st = new StringTokenizer( one_line, ":" );
        if( st.countTokens() != 2 )
        {
            throw new BackEndException(
              "Bad format in loaded file at line " + one_line );
        }
        String line_label = st.nextToken() + ":";
        if( ! label.trim().equals( line_label.trim() ) )
        {
            throw new BackEndException(
              "Expected label--" + label + "--but found--" + line_label + "--");
        }

        String str_value = st.nextToken();
        try
        {
            result = Integer.parseInt( str_value.trim() );
        } 
        catch( Exception e )
        {
            throw new BackEndException(
              "Invalid format, expected int, found--" + str_value + "--" );
        }
        return result;
    }

    /** Support for fromString().
     *  @arg label is there to make it hard to code improperly. 
     *  @arg line_tokenizer breaks multiline summary at the newlines.
     */
    private double summary2double( String label, StringTokenizer per_line_tokenizer )
      throws BackEndException
    {
        double result = -99.99;
        
        //
        // Check format of summary_line.
        //
        if( ! per_line_tokenizer.hasMoreTokens() )
        {
            throw new BackEndException(
              "Error: no " + label + " line in load file." );
        }
        String one_line = per_line_tokenizer.nextToken();
        StringTokenizer st = new StringTokenizer( one_line, ":" );
        if( st.countTokens() != 2 )
        {
            throw new BackEndException(
              "Bad format in loaded file at line " + one_line );
        }
        String line_label = st.nextToken() + ":";
        if( ! label.trim().equals( line_label.trim() ) )
        {
            System.err.println(
              "Expected label--" + label + "--but found--" + line_label + "--");
            throw new BackEndException(
              "Expected label--" + label + "--but found--" + line_label + "--");
        }

        String str_value = st.nextToken();
        try
        {
            result = Double.parseDouble( str_value.trim() );
        } 
        catch( Exception e )
        {
            throw new BackEndException(
              "Invalid format, expected int, found--" + str_value + "--" );
        }
        return result;
    }

    /** Support for fromString().
     *  @arg label is there to make it hard to code improperly. 
     *  @arg line_tokenizer breaks multiline summary at the newlines.
     */
    private boolean summary2boolean( String label, StringTokenizer per_line_tokenizer )
      throws BackEndException
    {
        boolean result = false;
        
        //
        // Check format of summary_line.
        //
        if( ! per_line_tokenizer.hasMoreTokens() )
        {
            throw new BackEndException(
              "Error: no " + label + " line in load file." );
        }
        String one_line = per_line_tokenizer.nextToken();
        StringTokenizer st = new StringTokenizer( one_line, ":" );
        if( st.countTokens() != 2 )
        {
            throw new BackEndException(
              "Bad format in loaded file at line " + one_line );
        }
        String line_label = st.nextToken() + ":";
        if( ! label.trim().equals( line_label.trim() ) )
        {
            System.err.println(
              "Expected label--" + label + "--but found--" + line_label + "--");
            throw new BackEndException(
              "Expected label--" + label + "--but found--" + line_label + "--");
        }

        String str_value = st.nextToken();
        try
        {
            Boolean b = Boolean.valueOf( str_value.trim() );
            result = b.booleanValue();
        } 
        catch( Exception e )
        {
            throw new BackEndException(
              "Invalid format, expected int, found--" + str_value + "--" );
        }
        return result;
    }

    // Empty constructor; now that we have a copy constructor, Java won't
    // generate the empty constructor for us.
    public TrafficParams() 
    {}
}
