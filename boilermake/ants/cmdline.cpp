
   #include "cmdline_base.hpp"
   #include "cmdline.hpp"
   
   /*******************************************************************
    * This is a generated file.  Do not hand-edit it.
    *******************************************************************/
   
   Cmdline::Cmdline( int argc, char *argv[] )
     :
        m_rank(100),
        m_n_ants(10),
        m_delay(100),
        m_show_pheromone(true),
        m_pheromone_decay_rate(0.9),
        m_smell_radius(2),
        m_ant_life(100),
        m_gestation_period(100),
        m_feeding_freq(0.01),
        m_seed(0)
    {
        m_argmap["rank"] = new SetType(&m_rank, "field edge size");
        m_argmap["n_ants"] = new SetType(&m_n_ants, "population");
        m_argmap["delay"] = new SetType(&m_delay, "in milliseconds");
        m_argmap["show_pheromone"] = new SetType(&m_show_pheromone, "");
        m_argmap["pheromone_decay_rate"] = new SetType(&m_pheromone_decay_rate, "factor that is applied each iter");
        m_argmap["smell_radius"] = new SetType(&m_smell_radius, "Range of ant's ability to smell food or pheromone");
        m_argmap["ant_life"] = new SetType(&m_ant_life, "Life span");
        m_argmap["gestation_period"] = new SetType(&m_gestation_period, "delay between food arriving at nest, and new ant ready to go out and work");
        m_argmap["feeding_freq"] = new SetType(&m_feeding_freq, "Proportion of epochs in which food is dropped");
        m_argmap["seed"] = new SetType(&m_seed, "srand(seed)");

        CheckForCmdlineFile( std::string(
             std::string(getenv("HOME") ? getenv("HOME") : "/dev/null") + std::string("/") + std::string(
                    ".antsrc")));
        SaveCmdlineFileTimestamp();
        LoadDefaultsFromFile();
        ParseCommandLine( argc, argv );
    }
    
