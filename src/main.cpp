#include "debug.h"

#include <iostream>

int main(void)
{
    for (int i = 5; i <= 5; ++i)
    {
        std::cout << "Perft( " << i << " ):" << std::endl << Gluon::Debug::RunPerftOnPosition("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", i, true, true) << std::endl;
    }

    return 0;
}