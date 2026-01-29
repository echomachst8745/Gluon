#include "debug.h"

#include <iostream>

int main(void)
{
    for (int i = 1; i <= 9; ++i)
    {
        std::cout << "Perft( " << i << " ):" << std::endl;
        Gluon::Debug::RunPerftOnPosition("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", i, true, true);
    }

    return 0;
}