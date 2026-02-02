#include "engine.h"
#include "movegenerator.h"
#include "search.h"
#include "debug.h"

#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <thread>
#include <atomic>

namespace Gluon::Engine {

Engine::Engine()
{
    shouldQuit = false;

    board = Board();
}

bool Engine::EngineShouldQuit()
{
    return shouldQuit;
}

void Engine::HandleUCICommand(const std::string& command)
{
    if (command == "uci") {
        HandleUCI();
    } else if (command == "isready") {
        HandleIsReady();
    } else if (command == "ucinewgame") {
        HandleUCINewGame();
    } else if (command.find("position") == 0) {
        HandleUCIPosition(command.substr(std::strlen("position ")));
    } else if (command.find("go") == 0) {
        HandleUCIGo(command.substr(std::strlen("go ")));
    } else if (command == "stop") {
        HandleUCIStop();
    } else if (command == "quit") {
        HandleUCIQuit();
    }
}

void Engine::HandleUCI()
{
    std::cout << "id name " << ENGINE_NAME << std::endl;
    std::cout << "id author " << ENGINE_AUTHOR << std::endl;
    std::cout << "uciok" << std::endl;
}

void Engine::HandleIsReady()
{
    std::cout << "readyok" << std::endl;
}

void Engine::HandleUCINewGame()
{
    board.SetupWithFEN(Gluon::BoardHelpers::STARTING_FEN);
}

void Engine::HandleUCIPosition(const std::string& positionCommand)
{
    std::stringstream ss(positionCommand);

    for (std::string token; ss >> token;) {
        if (token == "startpos")
        {
            board.SetupWithFEN(Gluon::BoardHelpers::STARTING_FEN);
        }
        else if (token == "fen")
        {
            std::string fen;
            for (int i = 0; i < 6; ++i)
            {
                std::string fenPart;
                ss >> fenPart;
                fen += fenPart + " ";
            }
            fen.pop_back(); // Remove trailing space
            board.SetupWithFEN(fen);
        }
        else if (token == "moves")
        {
            for (std::string moveStr; ss >> moveStr;)
            {
                auto moves = MoveGenerator::GenerateMoves(board);

                for (int i = 0; i < moves.moveCount; ++i)
                {
                    const auto& move = moves.moves[i];

                    if (move.ToUCIString() == moveStr)
                    {
                        board.MakeMove(move);
                        break;
                    }
                }
            }
        }
    }
}

void Engine::HandleUCIGo(const std::string& goCommand)
{
    std::stringstream ss(goCommand);

    for (std::string token; ss >> token;)
    {
        if (token == "perft")
        {
            int depth;
            ss >> depth;
            Debug::RunPerft(board, depth, true, true);
        }
        else if (token == "infinite")
        {
            Search::SearchMaxTimeSeconds = std::numeric_limits<double>::infinity();
            Search::SearchMaxDepth = 8;
        }
        else if (token == "movetime")
        {
            int moveTimeMs;
            ss >> moveTimeMs;
            Search::SearchMaxTimeSeconds = static_cast<double>(moveTimeMs) / 1000.0;
            Search::SearchMaxDepth = 20;
        }
        else if (token == "depth")
        {
            int depth;
            ss >> depth;
            Search::SearchMaxTimeSeconds = std::numeric_limits<double>::infinity();
            Search::SearchMaxDepth = depth;
        }
    }

    if (searchThread.joinable())
    {
        searchThread.join();
    }

    searchInProgress = true;
    Search::SearchStopped = false;

    searchThread = std::thread([this]()
    {
        lastSearchResult = Search::StartSearch(board, Search::SearchMaxTimeSeconds == std::numeric_limits<double>::infinity());
        searchInProgress = false;
        std::cout << "bestmove " << lastSearchResult.bestMove.ToUCIString() << std::endl;
    });
}

void Engine::HandleUCIStop()
{
    if (searchInProgress)
    {
        Search::SearchStopped = true;
        if (searchThread.joinable())
        {
            searchThread.join();
        }
        searchInProgress = false;
    }
}

void Engine::HandleUCIQuit()
{
    shouldQuit = true;
}

} // namespace Gluon::Engine