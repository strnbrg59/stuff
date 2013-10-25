#include <list>
#include <vector>
#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
using namespace std;

/* Here's how to find_if over a BindingVector (code I used to have in
   md_cmc_license.cc):
    BindingVector licenses_on_app_bv;
    string license_key;
    bool result = find_if(licenses_on_app_bv.begin(), licenses_on_app_bv.end(),
                          bind(&Binding::value, _1) == license_key)
                != licenses_on_app_bv.end();
*/

struct MyString
{
    MyString(string s): rep_(s) {}
    bool contains_b() { return rep_.find('b') != rep_.npos; }
    string rep_;
};

ostream& operator<<(ostream& out, MyString const& ms) {
    out << ms.rep_;
    return out;
}

template<typename T> void print_list(const T& ls) {
    BOOST_FOREACH(MyString s, ls) {
        cout << s << ' ';
    }
    cout << '\n';
}

bool has_letter(char letter, const MyString& str)
{
    return str.rep_.find(letter) != str.rep_.npos;
}

bool ends_in(const MyString& str, char c)
{
    return str.rep_.find(c) == str.rep_.size()-1;
}


//-----------------------------------
typedef void (*cb)(int);
typedef boost::function<void (int)> BoostCBFunc;

void call_cb(BoostCBFunc func) {
    func(17);
}

void my_cb(int i) {
    cout << "my_cb(" << i << ")\n";
}

struct MyCBFunctor
{
    MyCBFunctor(string name) : name_(name) {}
    void func(int i) {
        cout << "MyCBFunctor(" << name_ << ")::func(" << i << ")\n";
    }
    const string name_;
};

//-----------------------------------


int main()
{
    call_cb(my_cb);
    MyCBFunctor mcbf("mcbf");
    call_cb(boost::bind(&MyCBFunctor::func, mcbf, _1));

    list<MyString> ls = boost::assign::list_of<MyString>(string("foo"))
                                                        (string("bar"))
                                                        (string("baz"));
    print_list(ls);

    ls.remove_if(boost::bind(ends_in, _1, 'z'));

/*
    ls.remove_if(boost::bind(std::equal_to<string>(),
                             boost::bind(&MyString::rep_, _1),
                             string("foo")));
*/
//  ls.remove_if(boost::bind(&MyString::contains_b, _1));


//  ls.remove_if(boost::bind(has_letter, 'b', _1));
    print_list(ls);

    return MyString("foo").rep_.empty();
}
