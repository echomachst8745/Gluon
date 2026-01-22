#include "debug.h"

#include <iostream>

int main(void)
{
    for (int i = 1; i <= 6; ++i)
    {
        std::cout << "Perft( " << i << " ) = " << Gluon::Debug::RunPerftOnPosition(Gluon::BoardHelpers::STARTING_FEN, i) << std::endl;
    }

    return 0;
}