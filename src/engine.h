#pragma once

#include "board.h"
#include "search.h"
#include "transpositiontable.h"

#include <string>
#include <thread>
#include <atomic>

namespace Gluon::Engine {

constexpr const char ENGINE_NAME[] = "Gluon";
constexpr const char ENGINE_AUTHOR[] = "BittenPi824";

class Engine
{
public:
    Engine();
    ~Engine() = default;

    bool EngineShouldQuit();

    bool SearchShouldStop() const noexcept;

    void HandleUCICommand(const std::string& command);

private:
    bool shouldQuit;

    std::thread searchThread;

    Board board;

    Search::TranspositionTable::TranspositionTable transpositionTable; // 1 GB transposition table
    
    void HandleUCI();
    void HandleIsReady();
    void HandleUCINewGame();
    void HandleUCIPosition(const std::string& positionCommand);
    void HandleUCIGo(const std::string& goCommand);
    void HandleUCIStop();
    void HandleUCIQuit();
};

} // namespace Gluon::Engine