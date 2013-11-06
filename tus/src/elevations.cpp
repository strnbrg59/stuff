/**
Generate some elevations, for an interesting topography to fly through.
*/

#include "elevations.hpp"

#include <cassert>
#include <vector>
#include <string>
using std::string;
#include <cmath>
#include <iostream>
using std::ostream;
using std::vector;
#include "cmdline.hpp"
#include "trace.hpp"

extern Cmdline* g_cmdline;

Elevations::Elevations()
  : m_rep((g_cmdline->GridSize())/(g_cmdline->Coarsening()))
{
    if(g_cmdline->FakeScenery())
    {
        this->GenerateFakeScenery();
    } else
    {
        this->LoadRealScenery(g_cmdline->SceneryFile().c_str());
    }
}


void
Elevations::GenerateFakeScenery()
{
    int gridSize = g_cmdline->GridSize();
    int xOffset = gridSize/2;
    int yOffset = gridSize/2;
    for(int i=0; i<gridSize; ++i)
    {
        m_rep[i] = vector<double>(gridSize);
        for(int j=0; j<gridSize; ++j)
        {
            double x = i-xOffset;
            double y = j-yOffset;
            m_rep[i][j] = 10*sin(5*(x*x + y*y)*2*M_PI/(gridSize*gridSize));
        }
    }
}


void
Elevations::LoadRealScenery(char const* infilename)
{
    Trace t("Elevations::LoadRealScenery()");
    // The infile is a NASA SRTM1 file measuring 3601x3601 uint16's in
    // bigendian order.
     //

    int gridSize = g_cmdline->GridSize();
    int gridStartX = g_cmdline->GridStartX();
    int gridStartY = g_cmdline->GridStartY();
    int coarsening = g_cmdline->Coarsening();
    assert(gridStartX>=0);
    assert(gridStartY>=0);
    assert(gridStartX+gridSize < Elevations::srtmSize);
    assert(gridStartY+gridSize < Elevations::srtmSize);

    FILE* infile = fopen(infilename, "rb");
    if(!infile)
    {
        t.FatalError("File not found: %s", infilename);
    }

    uint8_t loByte, hiByte;
    int16_t dataByte;

    // We'll read all the data in the .hgt file but load only the part
    // requested via gridStart* and gridSize.
    for(int i=0;i<srtmSize;++i)
    {
        if((i>=gridStartX) && (i<gridStartX+gridSize) && (i%coarsening==0))
        {
            m_rep[(i-gridStartX)/coarsening] =
                vector<double>(gridSize/coarsening);
        }
        for(int j=0;j<srtmSize;++j)
        {
            fread(&dataByte, 2, 1, infile);
            if(((j<gridStartY)||(j>=gridStartY+gridSize)||(j%coarsening!=0))
            || ((i<gridStartX)||(i>=gridStartX+gridSize)||(i%coarsening!=0)))
            {
                continue;
            }

            loByte = (uint8_t)(dataByte >> 8);  // Could use ntohs(3) at cost of
            hiByte = (uint8_t)dataByte;         // #include'ing <arpa/inet.h>.
            double x = i;
            double y = j;
            double z = (loByte + 256*hiByte);
            if((z == -32768) || (z>10000)) // 32768 is bad data tag
            {
                z = 0;
            }
            int ii=(i-gridStartX)/coarsening;
            int jj=(j-gridStartY)/coarsening;
            m_rep[ii][jj] = z;
        }
    }

    fclose(infile);
}


ostream& operator<<(ostream& out, Elevations const& els)
{
    for(int i=0;i<g_cmdline->GridSize();++i)
    {
        for(int j=0; j<g_cmdline->GridSize(); ++j)
        {
            out << els.val(i,j) << " ";
        }
        out << '\n';
    }
    return out;
}
