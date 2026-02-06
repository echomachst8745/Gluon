#include <iostream>

#include "board.h"
#include "movegenerator.h"

int main()
{
	Gluon::Board b;

	auto moves = Gluon::GeneratePseudoLegalMoves(b);

	for (const auto& move : moves)
	{
		std::cout << move.ToUCIString() << std::endl;
	}

	return 0;
}