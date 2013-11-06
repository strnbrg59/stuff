
    #ifndef INCLUDED_CMDLINE_HPP
    #define INCLUDED_CMDLINE_HPP
    #include "cmdline_base.hpp"
    /*******************************************************************
     * This is a generated file.  Do not hand-edit it.
    *******************************************************************/
     
    class Cmdline : public CmdlineBase
    { 
      public: 
         
        Cmdline( int argc, char *argv[] );
        virtual ~Cmdline() {}
    
        int Rank() const { return m_rank; }
        void Rank( int x ) {m_rank = x; }

        int NAnts() const { return m_n_ants; }
        void NAnts( int x ) {m_n_ants = x; }

        int Delay() const { return m_delay; }
        void Delay( int x ) {m_delay = x; }

        bool ShowPheromone() const { return m_show_pheromone; }
        void ShowPheromone( bool x ) {m_show_pheromone = x; }

        double PheromoneDecayRate() const { return m_pheromone_decay_rate; }
        void PheromoneDecayRate( double x ) {m_pheromone_decay_rate = x; }

        int SmellRadius() const { return m_smell_radius; }
        void SmellRadius( int x ) {m_smell_radius = x; }

        int AntLife() const { return m_ant_life; }
        void AntLife( int x ) {m_ant_life = x; }

        int GestationPeriod() const { return m_gestation_period; }
        void GestationPeriod( int x ) {m_gestation_period = x; }

        double FeedingFreq() const { return m_feeding_freq; }
        void FeedingFreq( double x ) {m_feeding_freq = x; }

        int Seed() const { return m_seed; }
        void Seed( int x ) {m_seed = x; }

      private:
        Anything m_rank; // "field edge size"
        Anything m_n_ants; // "population"
        Anything m_delay; // "in milliseconds"
        Anything m_show_pheromone; // ""
        Anything m_pheromone_decay_rate; // "factor that is applied each iter"
        Anything m_smell_radius; // "Range of ant's ability to smell food or pheromone"
        Anything m_ant_life; // "Life span"
        Anything m_gestation_period; // "delay between food arriving at nest, and new ant ready to go out and work"
        Anything m_feeding_freq; // "Proportion of epochs in which food is dropped"
        Anything m_seed; // "srand(seed)"

    }; 
    #endif // INCLUDED_CMDLINE_HPP    
    
