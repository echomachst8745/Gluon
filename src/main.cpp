#include "debug.h"

#include <iostream>

int main(void)
{
    for (int i = 1; i <= 6; ++i)
    {
        std::cout << "Perft( " << i << " ):" << std::endl << Gluon::Debug::RunPerftOnPosition(Gluon::BoardHelpers::STARTING_FEN, i, false) << std::endl;
    }

    return 0;
}