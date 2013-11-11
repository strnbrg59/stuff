#ifndef _INCLUDED_WAX_TREE_H_
#define _INCLUDED_WAX_TREE_H_

#include "piece.hpp"
#include "board.hpp"
#include "position.hpp"
#include "trace.hpp"
#include <deque>
#include <boost/shared_ptr.hpp>
class Cmdline;
struct Node;

using boost::shared_ptr;
typedef shared_ptr<Node> NodePtr;

struct Node
{
    Node(Node const* parent, Piece::Color color, Move const & move,
             int depth);
    ~Node();
    NodePtr MinMaxValue(double& value,
                       Move * bestMove, Position& rootPosition,
                       int maxDepth, double alphaBetaPruneValue,
                       int & nLeavesEvaluated);
    bool IsACapture() const { return m_isACapture; }
    void Sort();
    void PruneLowValueChildren(double quantile);
    Move const & GetMove() const { return m_move; }
    double GetValue() const { return m_value; }
    void   SetValue(double x) { m_value = x; }
    bool operator<(Node const & that) const;
    bool operator>(Node const & that) const;
    void PrintChildValues(Position const&) const;
    void ResetAsRoot();
    void DecrementDepth();

    Piece::Color             m_color;
    Move                     m_move; // that got us here
    Node const *         m_parent;
    bool                     m_isACapture;
    int                      m_depth;
    std::vector<NodePtr> m_children;
    double                   m_value;

  private:
    Node(Node const &);
    Node& operator=(Node const &);
    friend std::ostream& operator<<(std::ostream&, Node const&);
};

std::ostream& operator<<(std::ostream&, Node const&);

bool NodeLT(NodePtr pt1, NodePtr pt2);
bool NodeGT(NodePtr pt1, NodePtr pt2);

Move DecideMove(NodePtr& parent, NodePtr& bestChild,
                Position&, Piece::Color, Cmdline const &,
                int & nLeavesEvaluated);


class RepetitionPreventer
{
public:
    bool IsRepetition(const Move&, Piece::Color);
    void AddMove(const Move&, Piece::Color);

private:
    std::vector<Move>& TheVector(Piece::Color);
};


#endif // include guard
