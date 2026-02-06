#include <iostream>

#include "board.h"

int main()
{
	Gluon::Board b;

	std::cout << b.GetBoardString() << std::endl;

	return 0;
}