#pragma once

#include "board.h"

#include <string>

namespace Gluon::Engine {

constexpr const char ENGINE_NAME[] = "Gluon";
constexpr const char ENGINE_AUTHOR[] = "BittenPi824";

class Engine
{
public:
    Engine();
    ~Engine() = default;

    bool EngineShouldQuit();

    void HandleUCICommand(const std::string& command);

private:
    bool shouldQuit;

    Board board;

    void HandleUCI();
    void HandleIsReady();
    void HandleUCINewGame();
    void HandleUCIPosition(const std::string& positionCommand);
    void HandleUCIGo(const std::string& goCommand);
    void HandleUCIStop();
    void HandleUCIQuit();
};

} // namespace Gluon::Engine