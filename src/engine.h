#pragma once

#include "position.h"
#include "search.h"

#include <string>
#include <thread>

namespace Gluon {

// [ Engine identity ]
inline const std::string ENGINE_NAME = "Gluon";
inline const std::string ENGINE_AUTHOR = "Joel Crann";

// [ Positions ]
inline const std::string START_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Engine
{
public:
    // [ Constructors ]
    Engine();

    ~Engine();

    // [ Public methods ]
    // Starts the engine and returns once the UCI "quit" command has been received.
    void Run();

    void NewGame();

    void SetPosition(const std::string& fen);

    void PlayMove(const std::string& moveString);

    // Searches a copy of the current position on its own thread, so commands can still be read.
    void StartSearch(const SearchLimits& limits);

    void StopSearch();

    void WaitForSearch();

    void SetHashSize(size_t megabytes);

    void ClearHash();

    inline const Position& GetPosition() const
    {
        return position;
    }

private:
    // [ Private methods ]
    void RunSearch(Position searchPosition, SearchLimits limits);

    // [ Data members ]
    Position position;

    Searcher searcher;

    std::thread searchThread;
};

} // namespace Gluon
