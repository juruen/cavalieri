#include <iostream>
#include <functional>

using namespace std;

void foobar(function<bool ()> predicate) {
  predicate();
}
int main()
{
   auto func = [] () { cout << "Hello world"; };
   func(); // now call the function
   foobar([] () { cout << "What's up"; return true;});
}
