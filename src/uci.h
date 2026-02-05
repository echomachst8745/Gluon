#pragma once

#include "search.h"

namespace Gluon {

class Engine;

namespace UCI {

// Reads commands from standard input until "quit" is received.
void RunLoop(Engine& engine);

void PrintSearchInfo(const SearchResult& result);

void PrintBestMove(const SearchResult& result);

} // namespace UCI

} // namespace Gluon
