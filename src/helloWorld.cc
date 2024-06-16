// Hello World program
#include <iostream>

// user headers
#include "BasicSPSC.hh"
#include "BasicSPSCWithoutModulo.hh"
using namespace std;

int main(int argc, char** argv)
{
    using spsc10 = BasicSPSC<int, 10>;
    using spsc32 = BasicSPSCWithoutModulo<int, 32>;
    std::unique_ptr<spsc10> spsc = std::make_unique<spsc10>();
    std::unique_ptr<spsc32> spsc_ = std::make_unique<spsc32>();
    std::cout << "Hello World" << std::endl;
    return 0;
}