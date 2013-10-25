#include <queue>
#include <cassert>
#include <iostream>
using namespace std;

int main() {
  priority_queue<int> Q;
  Q.push(1);
  Q.push(4);
  Q.push(2);
  Q.push(8);
  Q.push(5);
  Q.push(2);
  Q.push(7);
  
  assert(Q.size() == 7);

  assert(Q.top() == 8);
}
