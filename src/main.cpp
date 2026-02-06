#include <iostream>

#include "board.h"
#include "movegenerator.h"

int main()
{
	Gluon::Board b;

	for (int i = 0; i < 10; ++i)
	{
		auto moves = Gluon::GeneratePseudoLegalMoves(b);

		b.MakeMove(moves[0]);

		std::cout << b.GetBoardString() << std::endl;
	}

	return 0;
}