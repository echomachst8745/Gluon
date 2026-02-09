#include <iostream>

#include "board.h"
#include "benchmark.h"

int main()
{
	Gluon::Board b("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

	Gluon::Benchmark::Perft(b, 5);

	return 0;
}