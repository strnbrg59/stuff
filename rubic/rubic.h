#ifndef RUBIC_H
#define RUBIC_H

#include <map>
#include <vector>
#include <fstream>
#include <string>

template<int N> class CubeColoring;

template<int N> void permuteColoring(std::string moves);

template<int N> bool cubeIsUnscrambled();

struct Printable
{
    virtual std::ostream& print(std::ostream& out) const = 0;
};


template<int N> struct Face
{
    typedef typename std::vector<int> Row;
    typedef typename std::vector<Row> Array;
    Array rep_;
public:
    Face();
    Face& turn();
    Row& operator[](int i) { return rep_[i]; }
};

template<int N> struct Faces
{
    std::vector< Face<N> > rep_;
    Face<N>& operator[](int i) { return rep_[i]; }
    Faces();
};

template<int N> class Permutation : public Printable
{
private:
    std::vector<int> rep_;
public:
    Permutation();
    Permutation(std::vector<int> const&);
    Permutation(Faces<N>&);
    Permutation& permute(std::string moves);
    Permutation operator*(Permutation const&) const;
    void operator*=(Permutation const&);
    bool operator==(Permutation const&) const;
    bool operator!=(Permutation const&) const;
    bool operator<(Permutation const&) const;
    std::ostream& print(std::ostream& out) const;
    friend class CubeColoring<N>;
};

template<int N> Permutation<N> pow(Permutation<N> const&, int);

template<int N> class Turns
{
    enum FaceName {faceL, faceR, faceD, faceU, faceB, faceF};
    Permutation<N> forwardFacingTurn(FaceName selfFace,
                                     FaceName otherFaces[4][2]) const;
    Permutation<N> backwardFacingTurn(FaceName selfFace,
                                      FaceName otherFaces[4][2]) const;
    Permutation<N> TurnL() const;
    Permutation<N> TurnR() const;
    Permutation<N> TurnD() const;
    Permutation<N> TurnU() const;
    Permutation<N> TurnB() const;
    Permutation<N> TurnF() const;
public:
    Turns();
    const Permutation<N> L, R, D, U, B, F,
                         l, r, d, u, b, f;
};


template<int N> class CubeColoring : public Printable
{
    std::vector<int> rep_;
    std::ostream& print(std::ostream& out) const;
public:
    CubeColoring();
    CubeColoring& permute(Permutation<N> const&);
    bool operator==(CubeColoring const&) const;
    int operator[](int i) const { return rep_[i]; }
    int operator[](int i) { return rep_[i]; }
    static CubeColoring& instance();
    friend struct Printable;
};

std::string invert_turnstring(std::string const& turnstring);

std::ostream& operator<<(std::ostream&, Printable const&);

#include "rubic.h_impl"

#endif
