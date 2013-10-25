#include <iostream>
using namespace std;

struct Link
{
    Link(int id) : _id(id) { }
    int _id;
    Link* _next;
};


Link* reverseLinks(Link* head)
{
    Link* prev = head;
    Link* curr = head->_next;
    head->_next = 0;
    while(curr)
    {
        Link* next = curr->_next;
        curr->_next = prev;
        prev = curr;
        curr = next;
    }

    return prev;
}

void printList(Link* head)
{
    Link* curr = head;
    while(curr)
    {
        cout << curr->_id << ' ';
        curr = curr->_next;
    }
    cout << '\n';
}


int main()
{
    Link* head = new Link(0);
    Link* curr = head;
    for(int i=1;i<5;++i)
    {
        Link* next = new Link(i);
        curr->_next = next;
        curr = next;
    }
    curr->_next = 0;


    printList(head);
    Link* tail = reverseLinks(head);
    printList(tail);
}
