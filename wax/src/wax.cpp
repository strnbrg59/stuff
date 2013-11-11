#include "wax_tree.hpp"
#include "trace.hpp"
#include "cmdline.hpp"
#include "timer.hpp"
#include <limits.h>
#include <cstring>
#include <fstream>
using std::cin; using std::cout; using std::endl; using std::string;

NodePtr MakeMove(NodePtr& parent, int i_move, Piece::Color,
                     Cmdline const &, Position &);

// A couple timers.  99% of the time is spent in the two functions these
// timers time:
TimerContext g_legalMovesTimer("legalMovesTimer");
TimerContext g_evaluateTimer("evaluateTimer");

int
main(int argc, char * argv[])
{
    std::cout.flags(std::ios::unitbuf);

    Trace t("main()");
    Cmdline cfg(argc, argv);
    CmdlineFactory::Init(cfg);

    string initPosFile;
    if(cfg.InitPos() == "/dev/null")
    {
        initPosFile = DATADIR "/initial_position.txt";
    } else
    {
        initPosFile = cfg.InitPos();
    }
    Position currPosition(initPosFile.c_str(), cfg.NRanks());
    if(cfg.DebugLevel() > 3)
    {
        currPosition.Display();
    }

    Move nullMove;
    NodePtr nextChild(new Node(NULL, Piece::white, nullMove, 0));

    int maxMoves = (cfg.MachinePlays() == "bw") ? cfg.MaxMoves() : INT_MAX;
    for(int iter=0;iter<maxMoves;++iter)
    {
        cfg.Reload();

        //
        // White moves.
        //
        nextChild = MakeMove(nextChild, iter, Piece::white, cfg, currPosition);
        nextChild->ResetAsRoot();

        //
        // Black moves.
        //
        nextChild = MakeMove(nextChild, iter, Piece::black, cfg, currPosition);
        nextChild->ResetAsRoot();
    }
}

NodePtr
MakeMove(NodePtr& parent,
         int i_move, Piece::Color pieceColor, Cmdline const & cfg,
         Position & currPosition)
{
    Move move;
    int nLeavesEvaluated = 0;
    NodePtr bestChild;

    if(((pieceColor == Piece::white) 
             && strchr(cfg.MachinePlays().c_str(), 'w'))
    ||  ((pieceColor == Piece::black)
             && strchr(cfg.MachinePlays().c_str(), 'b')))
    {
        move = DecideMove(parent, bestChild,
                          currPosition, pieceColor, cfg, nLeavesEvaluated);

        if (pieceColor == Piece::white)
        {
            cout << (i_move+1) << ". ";
        } else {
            cout << "... ";
        }
        cout << move.Algebraic(currPosition) << "\n";
    } else
    {
        string humanMove;  // In algebraic notation
        bool legal(false);
        while(!legal)
        {
            cin >> humanMove;
            move = Move(humanMove);
            if(currPosition.IsLegalMove(move))
            {
                legal = true;
            } else
            {
                cout << "Illegal move.  Try again.\n";
            }
        }
    }
    currPosition += move;

    if (cfg.DebugLevel() > 2)
    {
        cout << "Value = " << currPosition.Evaluate() << '\n';
        cout << "Evaluated " << nLeavesEvaluated << " moves.\n";
    }

    if(cfg.DebugLevel() > 3)
    {
        currPosition.Display();
    }
    if(currPosition.NoKing())
    {
        cout << "Mate\n";
        exit(0);
    }

    return bestChild;
}
