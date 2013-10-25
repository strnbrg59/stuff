#ifndef DEFINED_ASTRO_H
#define DEFINED_ASTRO_H

#include "status-codes.hpp"

#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include "boost/multi_array.hpp"
#include "ols.hpp"
using std::string;
using std::vector;
using std::pair;

class Sample;
class RawData;
class Cmdline;
class StarAccreter;
class StretchParams;

template<typename T> class Point
{
  public:
    Point(T x, T y) : _rep(x,y) { }
    Point( Point<T> const & that ) : _rep( that._rep ) {}
    Point<T> & operator=( Point<T> const & that ) {
        if( this != &that )
        {
            _rep =that._rep;
        }
        return *this;
    }
    Point() : _rep(0,0) { }


    double DistanceTo( Point<T> const & that ) {
        return pow(  pow(_rep.first - that._rep.first, 2.0)
                   + pow(_rep.second - that._rep.second, 2.0), 0.5 );
    }
    void Transform( StretchParams const & );  // implemented only for <double>
    T x() const;  // implemented only for Point<double>
    T y() const; 
    T i() const;  // implemented only for Point<int>
    T j() const;
    void x(T _x); // implemented only for Point<double>
    void y(T _y);
    void i(T _i); // implemented only for Point<int>
    void j(T _j);

  private:
    std::pair<T,T> _rep;
};


/** A rectangular region of pixels that takes in a bright star and a few pixels
 *  around it, out to the point where the pixel value is close to zero.
*/
class AiryRectangle
{
  public:
    AiryRectangle( Point<int> const & presumed_center,
                   RawData const & raw_data,
                   vector<vector<bool> > * pixels_used,
                   bool * hit_domain_edge, Cmdline const & );
    Point<double> Centroid();
    int TotalEnergy();
    bool IsInvalid() const { return _is_invalid; }
  private:
    Point<int> const & _presumed_center;
    RawData const & _raw_data;
    Point<int> _lo_corner, _hi_corner;
    Point<double> _centroid;
    int _total_energy;
    bool _is_invalid;
    Cmdline const & _cmdline;

    void _FindCorners( Point<int> const & presumed_center,
                       RawData const & raw_data,
                       vector<vector<bool> > * pixels_used,
                       bool * hit_domain_edge );
    void _FindCentroid();
    bool _TooBig() const;
    bool _EdgesNotDark( char * direction, RawData const & raw_data ) const;
    void _Expand( char direction, bool * hit_domain_edge,
                  RawData const & raw_data );

    // Deliberately unimplemented:
    AiryRectangle( AiryRectangle const & );
    AiryRectangle & operator=( AiryRectangle const & );
};


class Star
{
  public:
    Star();
    Star( double energy, Point<double> xy );
    Star( Star const & );
    Star& operator=( Star const & );
    void Transform( StretchParams const & );

    double TotalEnergy() const { return _total_energy; }
    double x() const;
    double y() const;
    Point<double> Coords() const { return _coords; }
  private:
    double _total_energy;
    Point<double> _coords;
};

bool CompareStarEnergy( Star const &, Star const & );


enum RescaleMode { trim, bleed, normalize };
typedef int PixelValue; // Reserving future flexibility to use rgb triples.
  // Not using unsigned char because RawData::operator+=() might go past that.


/** Explanation of notation: we will transform a point (x,y) to
 *  x' = ax + bxx*x + bxy*y
 *  y' = ay + byx*y + byy*y
*/   
class StretchParams
{
  public:
    StretchParams();
    StretchParams( StretchParams const & );
    StretchParams & operator=( StretchParams const & );

    double StretchNorm() const;
    double DistanceTo( StretchParams const & ) const;
    void Estimate( vector<Star> const & stars1,
                   vector<Star> const & stars2,
                   int polynomial_degree );
    void Transform( double * x, double * y ) const;
  private:
    matrix _A; // constant terms
    matrix _B; // linear terms
    matrix _C; // quadratic terms
    int _polynomial_degree;
    void _FindIntuitiveParams();
    struct {
        double shift_x;
        double shift_y;
        double stretch_x;
        double stretch_y;
        double sheer;
        double rotate;
    } _intuitive;
  friend std::ostream& operator<<( std::ostream&, StretchParams const & );
};


/** Three stars taken from a Sample instance.  We try to match them with the
 *  corresponding three from another Sample.
*/
class Triangle
{
  public:
    Triangle( Sample const &, Cmdline const & );
    Triangle( Triangle const & that );
    Triangle & operator=( Triangle const & that );

    void Advance( double dim_limit=0.0 );
    void InchForward( int bandwidth );
    bool NoMore() const { return _no_more; }
    void Reset( double bright_limit=1E123 );

    bool Matches( Triangle const & that ) const;
    int  GetMatchOffset( Triangle const & that ) const;
    Star const & GetDimmestStar() const;

    vector<Star> const & GetStars() const;
    unsigned i() const { return _i; }
    unsigned j() const { return _j; }
    unsigned k() const { return _k; }

  private:
    double DistanceTo( Triangle const & that, int * offset ) const;
    vector<double> Angles() const;
    double Perimeter() const;
    void Display() const;

    Cmdline const & _cmdline;
    vector<Star> _rep;
    Sample const * _sample;
    unsigned _i,_j,_k; // indices into _sample;
    bool _no_more;

    friend std::ostream& operator<<( std::ostream&, Triangle const & );
    friend void Display( string outfile_name, 
                         Sample const & sample,  Triangle const & triangle );
    friend void Display( string outfile_name,
                         Sample const & sample1, vector<Triangle> const & vt1 );
};




/** Typically, the Cmdline.sample_size stars whose brightest pixel exceeds
 *  Cmdline.threshold.
*/
class Sample
{
  public:
    Sample( RawData const &, Cmdline const & );
    int AlignWith( StretchParams * transform,
                   Sample const & that );
    Star const & operator[]( unsigned ) const;
    vector<Star> const & GetStars() const { return _rep; }
    unsigned Size() const { return _rep.size(); }
    void operator+=( Sample const & that );
    bool _FindNextMatchingTriangles( Triangle * this_triangle,
                                     Triangle * that_triangle,
                                     Sample const & that );
    std::pair<int,int> RawdataSize() const { return _rawdata_size; }
    bool Contains( Point<double> const & star_coords ) const;
    bool TooDim( double this_energy, double dimmest_energy,
        double mu, double sd) const;
    bool TooBright( double this_energy, double dimmest_energy,
        double mu, double sd) const;
    bool MaybeBadStar( vector<Star>::const_iterator ) const;

  private:
    Sample( Sample const & that ) : _cmdline(that._cmdline) { assert(0); }
    void operator=( Sample const & that ) { assert(0); }
    void _FindNextStarPair( pair<Star,Star> * next_pair,
                            bool * no_more_usused_stars,
                            bool * cant_find_match,
                            bool * too_dim,
                            bool * too_bright,
                            bool * maybe_bad_star,
                            vector<bool> * used_stars,
                            Sample const & that,
                            StarAccreter const & this_accreter,
                            double mu, double sd ) const;
    vector<Star>::const_iterator _FindUnmarkedCentroidStar(
        StarAccreter const &, vector<bool> * marked_stars ) const;

    void _Transform( StretchParams const & );
    Star _FindNearest( double * distance, Star star ) const;

    vector<Star> _rep;
    mutable vector<int> _bad_star_count;
    Cmdline const & _cmdline;
    std::pair<int,int> _rawdata_size;

  friend void Display( string outfile_name,
                       Sample const & sample1, Sample const & sample2 );
  friend void Display( string outfile_name,
                       Sample const & sample1, Triangle const & triangle1,
                       Sample const & sample2, Triangle const & triangle2 );
  friend void Display( string outfile_name,
                       Sample const & sample1, vector<Triangle> const & vt1 );
  friend void Display( string outfile_name,
                       Sample const & sample,  Triangle const & triangle );
};
void Display( string outfile_name,
              Sample const & sample1, Sample const & sample2 );
void Display( string outfile_name,
              Sample const & sample1, Triangle const & triangle1,
              Sample const & sample2, Triangle const & triangle2 );
void Display( string outfile_name,
              Sample const & sample1, vector<Triangle> const & vt1 );
void Display( string outfile_name,
              Sample const & sample,  Triangle const & triangle );

void FindBrightnessRatioParams( double * mu, double * sd,
    Triangle const & t_numerator, Triangle const & t_denominator,
    int offset_for_numerator );


/** All the pixels in a digital image.  For now, 8-bit greyscale. */
class RawData
{
  public:
    RawData( string ppm_file_name );
    RawData( int row_min, int row_max, int col_min, int col_max,
             PixelValue default_color = 0 );

    void Transform( StretchParams const & );
    void operator+=( RawData const & that );
    void operator-=( RawData const & that );
    void Rescale( RescaleMode, int n_layers );

    int RowMin() const { return _row_min; }
    int RowMax() const { return _row_max; }
    int ColMin() const { return _col_min; }
    int ColMax() const { return _col_max; }
    bool SameSize( RawData const& ) const;

    PixelValue PixelAt( int i, int j ) const; // i is rows, j is columns
    void PixelAt( PixelValue color, int i, int j );

    void Display() const;
    void Copy( RawData const & );


  private:
    void _Resize( int row_min, int row_max, int col_min, int col_max );

    int _row_min; int _row_max;
    int _col_min; int _col_max;

    vector<vector<PixelValue> > _rep;

    RawData( RawData const & that );  // Deliberately unimplemented
    void operator=( RawData const & that ); // Deliberately unimplemented
};


class StarAccreter
{
  public:
    void SetTriangle( Triangle const &, int offset=0 );
    void AlignWith( StarAccreter const & );
    void AddStar( Star const & );
    StretchParams GetTransform() const { return _transform; }
    Point<double> Centroid() const;

  private:
    vector<Star> _rep;
    StretchParams _transform;
};


//
// Global functions
//
std::ostream& operator<<( std::ostream&, RawData const & );
std::ostream& operator<<( std::ostream&, Sample const & );
std::ostream& operator<<( std::ostream&, Triangle const & );
std::ostream& operator<<( std::ostream&, Star const & );
std::ostream& operator<<( std::ostream&, StretchParams const & );
void TransformPoint( Point<double> *, StretchParams const & );
void FindAngles( vector<double> * angles, vector<Star> const & triangle );

#endif // DEFINED_ASTRO_H
