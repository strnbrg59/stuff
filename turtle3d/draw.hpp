// Author: Ted Sternberg, strnbrg@trhj.homeunix.net

#include <vector>
#include <string>
#include <list>

void Display();

void IdleFunc();

void Reshape( int w, int h );

void PrintMatrix( float const m[16] );
void PrintModelviewMatrix();
void InitFog();
void InitLighting();
void DrawSphere( bool withLighting );

std::vector<std::string> TokenizeCommand( char * buf );

template<typename T> struct Coord
{
    T x;
    T y;
    T z;
    Coord() : x(-1), y(-1), z(-1) { }
    Coord( T _x, T _y, T _z ) : x(_x), y(_y), z(_z) { }
};

struct Event
{
    enum {nothing=-3, pd=-2, pu=-1};

    Event( float x, float y, float z, int _modifier )
      : position(x,y,z), modifier(_modifier) { }
    Coord<float> position;
    int modifier;
};


typedef std::list< Event > EventList;

class Drawing
{
  public:
    Drawing( int argc, char * * argv );

    void Forward( float x );
    void Right( float degrees );
    void Up( float degrees );
    void Roll( float degrees );
    void SetTurtleVisible( bool );
    void SetPenColor( float r, float g, float b );
    void PenDown();
    void PenUp();
    void Clean();

    void ShowSphere( bool b );
    void SetLighting( bool b );
    void SetSphereRadius( float r );
    void SphereForward( float degrees );

    void Redraw() const;
    void Reshape( int, int ) const;
    void Interpreter( std::vector<std::string> const & cmd_words );

  private:
    void Goto( float x, float y, float z );
    void DrawTurtle() const;

    int        m_windowSize; // square, to simplify int2float conversion in
                             // Forward().
    EventList  m_nodes;
    bool       m_turtleVisible;

    float      m_turtleMatrix[16];
    bool       m_lighting;

    bool       m_sphereVisible;
    float      m_sphereRadius;
};
