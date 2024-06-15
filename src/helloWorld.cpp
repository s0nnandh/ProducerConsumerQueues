// Hello World program
#include <iostream>

// user headers
#include "BasicSPSC.hpp"
#include "BasicSPSCWithoutModulo.hpp"
using namespace std;

int main()
{
    using spsc32 = BasicSPSCWithoutModulo<int, 32>;
    std::unique_ptr<BasicSPSC<int>> spsc = std::make_unique<BasicSPSC<int>>(10);
    std::unique_ptr<spsc32> spsc_ = std::make_unique<spsc32>();
    std::cout << "Hello World" << std::endl;
    return 0;
}