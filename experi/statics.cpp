#include "StaticMgr.H"

struct IntVect
{
    IntVect( int i_, int j_ ) : i(i_), j(j_) {;}
    IntVect() {;}
    int i;
    int j;
    static IntVect * ps_Unit;
    static IntVect   s_Unit;
};

IntVect * IntVect::ps_Unit;
IntVect   IntVect::s_Unit;

void allocStatic()
{
    IntVect::ps_Unit = new IntVect(7,9);
    IntVect::s_Unit = *IntVect::ps_Unit;
}

void freeStatic()
{
    //delete IntVect::ps_Unit;
}

static StaticMgr<IntVect> s_IntVectStaticMgr( allocStatic, freeStatic );

int main()
{
    std::cerr << "Entered main()\n";
}
