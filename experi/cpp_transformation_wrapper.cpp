#include <iostream>
#include <string>
#include <cmath>
using namespace std;

//-----------------------------------------------------
// C transformation functions
//
int first_toupper(const char* input, char** output)
{
    *output = (char*)malloc(strlen(input)+1);
    if (!*output) {
        return 1;
    }
    strcpy(*output, input);
    (*output)[0] = toupper((*output)[0]);
    return 0;
}

int int2str(int i, char** output)
{
    int n_digits = (int)log((double)i);
    *output = (char*)malloc(n_digits+1);
    if (!output) {
        return 1;
    }
    sprintf(*output, "%d", i);
    return 0;
}
//-----------------------------------------------------

template<typename T> struct XformFuncType {
    typedef int FuncType(T, char**);
};

//
// The wrapper
//
template<class T> string c_string_processor_wrapper(
    typename XformFuncType<T>::FuncType f,
    T input)
{
    char* c_output;
    int err = (*f)(input, &c_output);
    if (err) {
        throw std::exception();
    }
    string output(c_output);
    free(c_output);
    return output;
}

int main()
{
    cout << c_string_processor_wrapper<const char*>(first_toupper,
                                                    "hello_world") << '\n';
    cout << c_string_processor_wrapper<int>(int2str, 5771) << '\n';
}
