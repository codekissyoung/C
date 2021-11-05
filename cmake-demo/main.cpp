#include "mod1/mod1.hpp"
#include "mod2/mod2.hpp"
#include "common.h"

using namespace std;

int main(int argc, char const *argv[])
{
    mod1_func1();
    mod2_func1();
    mod2_func2();
    echo();
    return 0;
}