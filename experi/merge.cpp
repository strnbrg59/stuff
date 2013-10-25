#include <iostream>
using namespace std;

struct Node
{
    Node(int v) : value(v) { }
    int value;
    Node* next;
};


void PrintList(Node* head)
{
    Node* curr = head;
    while(curr)
    {
        cout << curr->value << " ";
        curr = curr->next;
    }
    cout << '\n';
}


void DeleteList(Node* head)
{
    Node* curr = head;
    while(curr)
    {
        Node* temp = curr;
        curr = curr->next;
        delete temp;
    }
}


/* Set inPlace to false if you want the result in a brand-new list.
 * Set inPlace to true if you want to use the existing lists (and just
 * reset their links to get a unified, sorted, list).
*/
Node* sortMerge(Node* head0, Node* head1, bool inPlace)
{
    /* Initialize */
    Node* currLowest = head0->value < head1->value ? head0 : head1;
    Node* headNew;
    if(inPlace)
    {
        headNew = currLowest;
    } else
    {
        headNew = new Node(currLowest->value);
    }
    Node* currNew = headNew;
    Node* curr0;
    Node* curr1;
    if(currLowest == head0)
    {
        curr0 = head0->next;
        curr1 = head1;
    } else
    {
        curr0 = head0;
        curr1 = head1->next;
    }
    
    /* Sort until hit end of one list */
    while(curr0 && curr1)
    {
        Node* tempNew = currNew;
        Node** smaller;
        if(curr0->value < curr1->value)
        {
            smaller = &curr0;
        } else
        {
            smaller = &curr1;
        }
        if(inPlace)
        {
            currNew = *smaller;
        } else
        {
            currNew = new Node((*smaller)->value);
        }
        *smaller = (*smaller)->next;
        tempNew->next = currNew;
    }
    
    /* Add remainder of the other list. */
    Node* remainingList = curr0 ? curr0 : curr1;
    while(remainingList)
    {
        Node* tempNew = currNew;
        if(inPlace)
        {
            currNew = remainingList;
        } else
        {
            currNew = new Node(remainingList->value);
        }
        tempNew->next = currNew;
        remainingList = remainingList->next;
    }
    
    return headNew;
}


int main()
{
    int const len0 = 4;
    int const len1 = 3;
    int listValues0[len0] = {5,5,5,8};
    int listValues1[len1] = {2,2,9};

    /* Initialize the two lists we want to merge. */
    Node* head0 = new Node(listValues0[0]);
    head0->next = 0;
    Node* prev = head0;
    for(int i=1;i<len0;++i)
    {
        Node* curr = new Node(listValues0[i]);
        prev->next = curr;
        prev = curr;
    }

    Node* head1 = new Node(listValues1[0]);
    head1->next = 0;
    prev = head1;
    for(int i=1;i<len1;++i)
    {
        Node* curr = new Node(listValues1[i]);
        prev->next = curr;
        prev = curr;
    }

    PrintList(head0);
    PrintList(head1);


    /* Do the merge. */
    bool inPlace = true;
    Node* merged = sortMerge(head0, head1, inPlace);
    PrintList(merged);

    if(!inPlace)
    {
        DeleteList(head0);
        DeleteList(head1);
    }
    DeleteList(merged);
}
