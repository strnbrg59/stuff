#ifndef __INCLUDED_RCM_TEMPLATE_CLASSES_H_
#define __INCLUDED_RCM_TEMPLATE_CLASSES_H_

template<class T> class M;
template<class T> class Mrep;
template<class T> class V;

class ostream;

enum VectorOrientation { horizontal, vertical }; // row vs column vector

template<class T> class Vrep
{
  private:
    friend class V<T>;

    Vrep(int size, VectorOrientation ori=horizontal);
    ~Vrep();

    int m_size;
    int refCount;
    VectorOrientation orientation;
    T* data;
};

template<class T> class V
{
public:
    V();  // size is set in emptyCtorArg.
    V( int size, VectorOrientation ori ); 
    V(const V<T>&);
    V(const M<T>&);
    V(const T& );
    ~V();

    inline const T& operator[](int i) const;   // getter
    inline       T& operator[](int i);         // setter

    int size() const { return rep->m_size; }

    VectorOrientation getOrientation() const { return rep->orientation; }
    void setOrientation( VectorOrientation ori ) { rep->orientation = ori; }

    V<T>& operator=(const V<T>&);

private:
    friend class M<T>;
    friend class Mrep<T>;

    bool outOfBounds( int r ) const;

    static int emptyCtorArg;
    Vrep<T>* rep;
 };

template<class T> class Mrep
{
  private:
    friend class M<T>;

    Mrep(int r, int c);
    ~Mrep();

    int m_rows, m_cols;
    int refCount;
    V<T>* data;
};

template<class T> class M
{
  public:
    M(int r, int c);
    M(const M<T>&);
    M(const V<T>&);
    M(T);

    ~M();

    M<T>& operator=(const M<T>&);

    inline const V<T>& operator[](int r) const;   // getter
    inline       V<T>& operator[](int r);         // setter
    bool outOfBounds( int r ) const;

    int rows() const { return rep->m_rows; }
    int cols() const { return rep->m_cols; }

    enum FormatMode { plain, neat, TeX };
    static void setFormatMode(FormatMode);

  private:
    Mrep<T>* rep;
    static FormatMode m_formatMode;            
};

typedef M<double> Md;
typedef V<double> Vd;

#include "rcm-template-classes.hImpl"

#endif
