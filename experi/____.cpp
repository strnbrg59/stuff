template<int N> struct Morse
{
    Morse<N-1> _;
    Morse<N>& operator-=(Morse<N> const& that) {
        this->_ -= that._;
        return *this;
    }

    Morse<N>& operator-=(Morse<N-1> const& that) {
        this->_ -= that;
        return *this;
    }

#ifdef BAD
    template<int M> Morse<N>& operator-=(Morse<N-M> const& that) {
        this->_.operator-=<M-1>(that);
        return *this;
    }
#endif
};

template<> struct Morse<1>
{
    int _;
    Morse<1>& operator-=(Morse<1> const& that) {
        this->_ -= that._;
        return *this;
    }
};

template<int N> Morse<N> operator-(Morse<N>& m1, Morse<N>& m2)
{
    Morse<N> result(m1);
    result -= m2;
    return result;
}

Morse<4> ____;
Morse<3> ___;
Morse<2> __;
Morse<1> _;

int main() {
    ___ = ____._;
    ___._ = __;
    ____._._._ = _;
    _._ = __._._;
    ___ = ____._ - ___;
    ___._ = __ - ____._._;
    __ = ___._ - ____._._;

    ___ -= __;    

#ifdef BAD
    ____ -= __;
#endif
}
