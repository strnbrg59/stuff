#include <list>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
using namespace std;

template<typename T> struct Link
{
    typedef Link<T>* Ptr;

    Link(T const& t) : next(NULL), data(t) {}
    Ptr next;
    T data;

    void print(ostream& out) {
    {
        typename Link<T>::Ptr curr = this;
        while (curr) {
            out << curr->data << ' ';
            curr = curr->next;
        }
        out << '\n';
    }
}



};

Link<char>::Ptr make_char_linked_list(string const& str)
{
    Link<char>::Ptr prev(NULL), head(NULL);
    for (string::const_iterator i=str.begin(); i!=str.end(); ++i) {
        Link<char>::Ptr curr(new Link<char>(*i));
        if (prev) {
            prev->next = curr;
        } else {
            head = prev = curr;
        }
        prev = curr;
    }

    return head;
}

template<typename T> typename Link<T>::Ptr
reverse_list(typename Link<T>::Ptr head)
{
    typename Link<T>::Ptr curr, next, nextnext;
    curr = head;
    next = curr->next;
    while (next) {
        nextnext = next->next;
        next->next = curr;
        curr = next;
        next = nextnext;
    }
    head->next = NULL;
    return curr;
}

template<typename T> typename Link<T>::Ptr
delete_list(typename Link<T>::Ptr head)
{
    Link<char>::Ptr curr = head;
    Link<char>::Ptr next = curr->next;
    while (curr) {
        next = curr->next;
        delete curr;
        curr = next;
    }
}
    

int main()
{
    Link<char>::Ptr head = make_char_linked_list("abcde");
    head->print(cout);

    Link<char>::Ptr tail = reverse_list<char>(head);
    tail->print(cout);

    delete_list<char>(tail);
}
