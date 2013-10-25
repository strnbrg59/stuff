void get_int(int*);
void get_double(double*);

template<typename T> struct GetterTraits
{
    typedef void (*F)(T*);
    static F getter;
};

template<typename T> void get_generic(T* t) {
    GetterTraits<T>::getter(t);
}

template<> GetterTraits<int>::F GetterTraits<int>::getter = get_int;
template<> GetterTraits<double>::F GetterTraits<double>::getter = get_double;
