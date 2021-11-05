#include <iostream>
#include "mod1/mod1.hpp"
#include "mod2/mod2.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    mod1_func1();
    mod1_func2();
    mod2_func1();
    mod2_func2();
    cout << "Hello world" << endl;
    return 0;
}
