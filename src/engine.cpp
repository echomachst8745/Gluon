#include "engine.h"
#include "movegenerator.h"

#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include <cstdlib>

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
        if (token == "startpos") {
            board.SetupWithFEN(Gluon::BoardHelpers::STARTING_FEN);
        } else if (token == "fen") {
            std::string fen;
            for (int i = 0; i < 6; ++i)
            {
                std::string fenPart;
                ss >> fenPart;
                fen += fenPart + " ";
            }
            fen.pop_back(); // Remove trailing space
            board.SetupWithFEN(fen);
        } else if (token == "moves") {
            for (std::string moveStr; ss >> moveStr;)
            {
                MoveGenerator::GenerateMoves(board);

                for (const MoveGenerator::Move& move : MoveGenerator::GenerateMoves(board))
                {
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
    auto moves = MoveGenerator::GenerateMoves(board);

    if (moves.empty())
    {
        return;
    }

    std::cout << "bestmove " << moves[rand() % moves.size()].ToUCIString() << std::endl;
}

void Engine::HandleUCIStop()
{
    
}

void Engine::HandleUCIQuit()
{
    shouldQuit = true;
}

} // namespace Gluon::Engine