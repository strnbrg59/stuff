#include "fingers.hpp"
#include "trace.hpp"
#include "cmdline.hpp"
#include <values.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
using namespace std;

void
Node::Print(ostream& out) const
{
    out << "[" << *m_noteIter << " : f" << m_f << " : s" << m_s << "]";
}


Node::Node(NoteIterator noteIter)
  : m_f(0), m_s(0), m_cost(0),
    m_accumulatedCost(0), 
    m_position(-999),
    m_depth(0),
    m_noteIter(noteIter),
    m_parent(0), m_isUrRoot(true)
{
    assert(!s_urRootConstructed);  // Ensure this function is only called once.
    s_urRootConstructed = true;
}

Node::Node(Node const& that)
  : m_f(that.m_f), m_s(that.m_s), m_cost(that.m_s),
    m_accumulatedCost(that.m_accumulatedCost),
    m_position(that.m_position),
    m_depth(that.m_depth),
    m_noteIter(that.m_noteIter),
    m_parent(that.m_parent), m_isUrRoot(that.m_isUrRoot)
{
    m_lastFingeredNode = m_parent->m_lastFingeredNode;
}

Node::Node(Finger const& f, Chorda const& s, Node const* parent,
           NoteIterator noteIter)
  : m_f(f), m_s(s), m_cost(0),
    m_accumulatedCost(0),
    m_position(-999), 
    m_depth(0),
    m_noteIter(noteIter),
    m_parent(parent), m_isUrRoot(false)
{
    Trace t("Node::Node()");
    assert(m_parent);

    m_lastFingeredNode = m_parent->m_lastFingeredNode;

    Note note(*noteIter);
    m_position = position(m_f, m_s, note);
    m_depth = m_parent->m_depth+1;
    m_cost = 0;
    double rawCost = 0.0;
    Cmdline const& cmdline(CmdlineFactory::TheCmdline());

    if(f==0)
    {
        rawCost += cmdline.OpenStringPenalty();
    } else
    {
        int shift_size = ShiftSize(m_lastFingeredNode.get());
        if(shift_size != 0)
        {
            double incrViola(DBL_MAX), incrStretch(DBL_MAX),
                   incrGeneric(DBL_MAX), incrBasePos(DBL_MAX);
            if(m_position == 2)
            {
                incrBasePos = cmdline.BasePosShiftPenalty();
            }
            if(m_position == 7)
            {
                incrViola = cmdline.ViolaPosShiftPenalty();
            }
            if(abs(shift_size) == 1)
            {
                incrStretch = cmdline.StretchPenalty();
            }
            incrGeneric = cmdline.GenericShiftPenalty();
            rawCost += min(incrViola,
                            min(incrStretch,
                                min(incrBasePos, incrGeneric)));
        }
        if(m_position > 7)
        {
            rawCost += cmdline.ThumbPosPenalty();
            if(f==4) rawCost += cmdline.PinkyInThumbPosPenalty();
        }
        rawCost += cmdline.PerStringChangePenalty()
                *  abs(m_parent->m_s - m_s);
    }

    m_cost = rawCost * pow(cmdline.DiscountFactor(), double(m_depth));
    m_accumulatedCost = m_parent->m_accumulatedCost + m_cost;

    if(((m_depth == CmdlineFactory::TheCmdline().MaxDepth())
     || (m_noteIter + 1 == s_noteIterEnd))
    && (m_accumulatedCost < Tree::s_minCost.second))
    {
        Tree::s_minCost = std::make_pair(this, m_accumulatedCost);
    }

    if(f != 0)
    {
        m_lastFingeredNode.reset(new Node(*this));
    }
}


Node::~Node()
{
    for(ChildVector::iterator iter=m_children.begin();
        iter!=m_children.end();
        ++iter)
    {
        delete *iter;
    }
}

void
Node::MakeChildren()
{
    Trace t("Node::MakeChildren()");

    NoteIterator nextIter(m_noteIter);
    if(m_parent) ++nextIter;

    ostringstream ost;
    for(unsigned d=0;d<m_depth;++d)
    {
        ost << "    ";
    }
    ost  << "Node" << *this << ": cost="
         << "(" << m_cost << ", "
         << m_accumulatedCost << ")"
         << ", depth=" << m_depth
         << ", note=" << *m_noteIter;
    t.Info("%s", ost.str().c_str());    

    if((m_depth == CmdlineFactory::TheCmdline().MaxDepth())
    || (m_accumulatedCost >= Tree::s_minCost.second)
    || (nextIter == s_noteIterEnd))
    {
        return;
    }

    if(m_children.empty())
    {
        Note nextNote(*nextIter);
        bool impossibleNote = true;
        for(int f=0; f<=4; ++f)
        {
            for(int s=1; s<=4; ++s)
            {
                if(isFeasible(f, s, nextNote))
                {
                    impossibleNote = false;
                    Node* newNode = new Node(f, s, this, nextIter);
                    /* This stops us cold after a single iteration!?
                    if(newNode->m_accumulatedCost >= Tree::s_minCost.second)
                    {
                        delete newNode;
                        return;
                    }
                    */
                    m_children.push_back(newNode);
                    newNode->MakeChildren();
                }
            }
        }
        assert(!impossibleNote);
    } else // Leaves left from last iteration.
    {
        for(ChildVector::iterator i=m_children.begin(); i!=m_children.end();
            ++i)
        {
            (*i)->MakeChildren();
        }
    }
}


Tree::Tree(Infile const& infile)
{
    Node::s_noteIterEnd = infile.end();
    m_root = new Node(infile.begin());
}


Tree::~Tree()
{
}


/** Decrement by 1 the depth of this node and all its children. */
void
Node::ResetDepths()
{
    --m_depth;
    double discountFactor = CmdlineFactory::TheCmdline().DiscountFactor();
    m_cost /= discountFactor;
    m_accumulatedCost /= discountFactor;
    for(ChildVector::iterator i=m_children.begin(); i!=m_children.end(); ++i)
    {
        (*i)->ResetDepths();
    }
}


/** Remove child from m_children, but without erasing it. */
void
Node::DeparentChild(Node const* child)
{
    ChildVector::iterator pchild;
    for(ChildVector::iterator i=m_children.begin(); i!=m_children.end(); ++i)
    {
        if(*i == child)
        {
            pchild = i;
        }
    }
    (*pchild)->m_parent = 0;
    m_children.erase(pchild);
}

/** Shift size from *this to *that.
 *  No stretches yet -- just shift or no shift.
*/
int
Node::ShiftSize(Node const* that) const
{
    if (!that) {
        return 0;
    } else {
        Finger f0(this->m_f), f1(that->m_f);
        Chorda s0(this->m_s), s1(that->m_s);
        Note n0(*(this->m_noteIter)), n1(*(that->m_noteIter));
        assert(f0>=0); assert(f0>=0);
        assert(f0<=4); assert(f1<=4);
        assert(s0>=1); assert(s1>=1);
        assert(s0<=4); assert(s1<=4);
        int p1 = position(f0, s0, n0);
        int p2 = position(f1, s1, n1);
        return p2 - p1;
    }
}


void
Tree::Grow()
{
    Trace t("Tree::Grow()");
    assert(CmdlineFactory::TheCmdline().MaxDepth() >= 2);

    do
    {
        double savedMinCost = s_minCost.second;
        s_minCost.second = DBL_MAX;
        m_root->MakeChildren();
        if(s_minCost.second > DBL_MAX/2) 
        {   // Bottom of tree reached end of score
            s_minCost.second = savedMinCost;
        }
        // Trace back from mincost node and reset m_root to its first decision.
        Node const* currChild = s_minCost.first;
        while(currChild->m_parent != m_root)
        {
            currChild = currChild->m_parent;
        }
        ostringstream ost;
        ost  << "Mincost = (" << *currChild
             << ", ["
             << "pos" << currChild->m_position
             << ", " << setw(8) << currChild->m_cost
             << ", " << setw(8) << s_minCost.second
             << "])";
        t.Notice("%s", ost.str().c_str());
    
        const_cast<Node*>(currChild)->ResetDepths(); // And of all children.
        m_root->DeparentChild(currChild);
        delete m_root;
        m_root = const_cast<Node*>(currChild);
    } while(!m_root->m_children.empty());
}

std::pair<Node const*, double> Tree::s_minCost(std::make_pair((Node const*)0,
                                                               DBL_MAX));

NoteIterator Node::s_noteIterEnd;
bool Node::s_urRootConstructed = false;
