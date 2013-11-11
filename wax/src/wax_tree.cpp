#include "wax_tree.hpp"
#include "cmdline.hpp"
#include <values.h>
#include <vector>
#include <cassert>
#include <cmath>
#include <deque>
#include <boost/foreach.hpp>
using namespace std;

const double g_bigval[2] = {MAXDOUBLE, -MAXDOUBLE};

Move
DecideMove(NodePtr& parent,
           NodePtr& bestChild,
           Position& currPosition, Piece::Color color,
           Cmdline const & cmdline,
           int & nLeavesEvaluated)
{
    Trace t("DecideMove()");
    Move bestMove;
    double bestValue;
    double alphaBetaPruneValue = g_bigval[color];
    bestChild = parent->MinMaxValue(bestValue, &bestMove, currPosition,
                                    cmdline.MaxDepth(),
                                    alphaBetaPruneValue, nLeavesEvaluated);
    return bestMove;
}


void
Node::ResetAsRoot()
{
    m_parent = NULL;
    DecrementDepth();
}

void
Node::DecrementDepth()
{
    --m_depth;
    for (vector<NodePtr>::iterator i=m_children.begin();
         i!=m_children.end(); ++i)
    {
        (*i)->DecrementDepth();
    }
}

    
void
Node::PrintChildValues(Position const& pos) const
{
    BOOST_FOREACH(NodePtr child, m_children)
    {
        cout << "[" << child->m_value << ", "
             << child->m_move.Algebraic(pos) << "], \n";
    }
}

void
Node::Sort()
{
    Trace t("Node::Sort()");
    assert(!m_children.empty());

    if(m_color == Piece::white)
    {
        std::sort(m_children.begin(), m_children.end(), NodeGT);
    } else
    {
        std::sort(m_children.begin(), m_children.end(), NodeLT);
    }
}


/** Throw away all but quantile*100% of the children. 
 *  Must run this after Node::Sort().
 */
void
Node::PruneLowValueChildren(double quantile)
{
    int keepers = int(m_children.size() * quantile + 0.5);
    vector<NodePtr>::iterator first_discard = m_children.begin() + keepers;

    // Now that you've freed their memory, remove discards from the vector.
    m_children.erase(first_discard, m_children.end());
}


Node::Node(Node const * parent,
                   Piece::Color color,
                   Move const & move, int depth)
  : m_color(color),
    m_move(move),
    m_parent(parent),
    m_isACapture(move.IsACapture()),
    m_depth(depth),
    m_value(-g_bigval[color])
{
}


Node::~Node()
{
    Trace t("Node dtor");
}


bool
Node::operator<(Node const & that) const
{
    return m_value < that.m_value;
}
bool
Node::operator>(Node const & that) const
{
    return m_value > that.m_value;
}

/* The segfaults were because std::sort requires a *strict* ordering, and
 * I was giving it >=.  See usr/local/src/experi/std_sort_segfault.cpp.
 */
bool NodeLT(NodePtr pt1, NodePtr pt2)
{
    return pt1->operator<(*pt2);
}
bool NodeGT(NodePtr pt1, NodePtr pt2)
{
    return pt1->operator>(*pt2);
}


/** @bestMove is non-null when this function is called by something else, null
 *  when it's called recursively.
 */
NodePtr
Node::MinMaxValue(double& value,
                  Move* bestMove, Position& position,
                  int maxDepth,
                  double alphaBetaPruneValue,
                  int& nLeavesEvaluated)
{
    Trace t("Node::MinMaxValue()");
    Move nullMove;
    RepetitionPreventer repetitionPreventer;
    NodePtr result;
    
    if(m_depth == maxDepth)  // Evaluate me, I'm a leaf.
    {
        assert((!bestMove) || (*bestMove!=nullMove));
        ++nLeavesEvaluated;
        value = position.Evaluate();
        return result; // NULL
    }

    //
    // Grow tree, if empty.
    //
    if(m_children.empty())
    {
        MovesVectPtr legalMoves(position.GetLegalMoves(m_color));

        if(legalMoves->empty())
        {
            assert((!bestMove) || (*bestMove!=nullMove));
            value = position.Evaluate();
            return result; // NULL.  This won't be the best child (and so cause
            // trouble in DecideMove) unless there are literally no possible
            // moves, but that may be impossible; it's not even true in a
            // stalemate.
        }

        for(unsigned m=0; m<legalMoves->size(); ++m)
        {
            assert((*legalMoves)[m] != nullMove);
            assert((*legalMoves)[m].From() != Board::nullSquare);

            NodePtr child(new Node(this, Piece::OtherColor(m_color),
                                   (*legalMoves)[m], m_depth+1));
            // TODO: upgrade boost so you can use shared_ptr_from_this<>.
            m_children.push_back(child);
        }
    } else
    {
        // Make alpha-beta pruning more likely to prune.
        this->Sort();
/*
        // FIXME: plays stupid even at prune=0.7 max_depth=5, and throws out
        // "repetitions" that never were!  Part of the problem is that alpha-
        // beta pruning leaves many child values set at MAXDOUBLE
        this->PruneLowValueChildren(
            CmdlineFactory::TheCmdline().PruneQuantile());
*/
    }

    double bestChildValue = -g_bigval[m_color];
    BOOST_FOREACH(NodePtr child, m_children)
    {
        Position newPosition(position);
        newPosition += child->m_move;

        double childValue;
        child->MinMaxValue(childValue, 0, newPosition, maxDepth, bestChildValue,
                           nLeavesEvaluated);
        child->SetValue(childValue);

        //
        // Alpha-beta pruning
        if(((m_color==Piece::white) && (childValue > alphaBetaPruneValue))
        || ((m_color==Piece::black) && (childValue < alphaBetaPruneValue)))
        {
            value = alphaBetaPruneValue;
            return child;
        }

        if(((m_color==Piece::white) && (childValue > bestChildValue))
        || ((m_color==Piece::black) && (childValue < bestChildValue)))
        {
            if(bestMove)
            {
                if (repetitionPreventer.IsRepetition(child->GetMove(),
                                                     m_color))
                {
                    t.Info() << "Excluding repetition "
                             << child->GetMove().Algebraic(position)<<'\n';
                    continue;
                }
                else
                {
                    *bestMove = child->GetMove();
                    result = child;
                }
            }
            assert((!bestMove) || (*bestMove != nullMove));

            // This has to go after the check for repetitions or we risk
            // finding no "best move".
            bestChildValue = childValue;
        }
    }
    assert((!bestMove) || (*bestMove != nullMove));
    if (bestMove)
    {
        repetitionPreventer.AddMove(*bestMove, m_color);
    }

    value = bestChildValue;
    return result;
}

std::ostream& operator<<(std::ostream& out, Node const& tn)
{
    out << tn.m_value;
    return out;
}


std::vector<Move>&
RepetitionPreventer::TheVector(Piece::Color color)
{
    static vector<Move> moves[2];
    return moves[color];
}

void
RepetitionPreventer::AddMove(const Move& m, Piece::Color color)
{
    TheVector(color).push_back(m);
}

bool
RepetitionPreventer::IsRepetition(const Move& m, Piece::Color color)
{
    Trace t("IsRepetition");
    vector<Move>& v(TheVector(color));
    unsigned int n = v.size();
    bool result = false;
    int const max_lookback = 10;

    /* It's a repetition if m was repeated 2 moves back, or m was repeated
     * 3 moves back and the move just 1 ago was repeated 4 moves back, etc.
     */
    for (int i=2; i< ((n/2 < max_lookback) ? n/2 : max_lookback); ++i)
    {
        result = (v[n-i] == m);
        if (result)
        {
            for (int j=1; j<i; ++j)
            {
                if (v[n-i-j] != v[n-j])
                {
                    result = false;
                    if ((i>3) && (j == (i-1)))
                    {
                        t.Info() << "Disqualifying repetition at lags "
                                 << "(" << i << ", " << j << ")\n";
                    }
                    break;
                }
            }
        }
        if (result)
        {
            return true;
        }
    }

    return false;
}
