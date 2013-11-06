/*
  File:        BackEnd.java
  Author:      Sternberg
  Date:
  Description: Interface for any class that FrontEnd calls to do calculations.
*/

import java.util.*;

/** Interface for any class that FrontEnd calls to do calculations.
 *  <P>
 *  FrontEnd holds a reference to an instance of this interface.
*/
public interface BackEnd
{
    /** Find BHCV (busy hour call volume) given speech channels, holding
     *  time, and blocking probability.
     *  <P>
     *  Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     */
    public int solveForBHCV( TrafficParams traffic_params ) 
        throws BackEndException;

    /** Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     */
    public int solveForCarriedCalls( TrafficParams traffic_params ) 
        throws BackEndException;
    
    /** Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     */
    public int solveForOfferedTraffic( TrafficParams traffic_params ) 
        throws BackEndException;

    /** Caller is responsible for calling updateFromTextFields()
     *  and updateTextFields().
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     */
    public double solveForBlockProb( TrafficParams traffic_params ) 
        throws BackEndException;

    /** Find number of speech channels consistent with the traffic parameters
     *  selected for the other textfields in the provisioning panel.
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     */
    public int solveForSpeechChannels( TrafficParams traffic_params ) 
        throws BackEndException;

    /** Find minimum required RU per CPU.
     *
     * <P>
     * @param selected_grammars The grammars in the grammar list widget.
    */
    public double solveForMinRUPerCPU( GrammarList selected_grammars );

    /** Find RU.
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     * @param selected_grammars The grammars in the grammar list widget.
    */
    public double solveForRecognitionPower( 
        TrafficParams traffic_params,
        GrammarList selected_grammars ) throws BackEndException;

    /** Find memory requirement per server.
     *
     * <P>
     * @param traffic_params A copy of the TrafficParams object held by FrontEnd.
     * @param selected_grammars The grammars in the grammar list widget.
    */
    public double solveForMemory(
        TrafficParams traffic_params,
        GrammarList selected_grammars ) throws BackEndException;            
    
}
