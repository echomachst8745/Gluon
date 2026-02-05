#include "uci.h"

#include "benchmark.h"
#include "engine.h"
#include "evaluation.h"
#include "transposition.h"

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace Gluon::UCI {

// !EXPLAIN!
static std::mutex outputMutex;

static void PrintLine(const std::string& line)
{
    const std::lock_guard<std::mutex> outputLock(outputMutex);

    std::cout << line << std::endl;
}

static void HandlePositionCommand(Engine& engine, std::istringstream& commandStream)
{
    std::string token;
    commandStream >> token;

    // !EXPLAIN!
    if (token == "startpos")
    {
        engine.SetPosition(START_POSITION_FEN);

        commandStream >> token;
    }
    else if (token == "fen")
    {
        std::string fen;

        while (commandStream >> token && token != "moves")
        {
            fen += token + ' ';
        }

        engine.SetPosition(fen);
    }

    if (token == "moves")
    {
        while (commandStream >> token)
        {
            engine.PlayMove(token);
        }
    }
}

static void HandleGoCommand(Engine& engine, std::istringstream& commandStream)
{
    SearchLimits limits;

    std::string token;

    while (commandStream >> token)
    {
        if      (token == "depth")     { commandStream >> limits.depth; }
        else if (token == "movetime")  { commandStream >> limits.moveTimeMilliseconds; }
        else if (token == "wtime")     { commandStream >> limits.remainingTimeMilliseconds[WHITE]; }
        else if (token == "btime")     { commandStream >> limits.remainingTimeMilliseconds[BLACK]; }
        else if (token == "winc")      { commandStream >> limits.incrementMilliseconds[WHITE]; }
        else if (token == "binc")      { commandStream >> limits.incrementMilliseconds[BLACK]; }
        else if (token == "movestogo") { commandStream >> limits.movesToGo; }
        else if (token == "infinite")  { limits.infinite = true; }
    }

    engine.StartSearch(limits);
}

static void HandleSetOptionCommand(Engine& engine, std::istringstream& commandStream)
{
    std::string token;
    std::string optionName;
    std::string optionValue;

    commandStream >> token;

    // !EXPLAIN!
    while (commandStream >> token && token != "value")
    {
        optionName += optionName.empty() ? token : ' ' + token;
    }

    while (commandStream >> token)
    {
        optionValue += optionValue.empty() ? token : ' ' + token;
    }

    if (optionName == "Hash")
    {
        std::istringstream optionValueStream(optionValue);

        size_t megabytes = 0;

        if (optionValueStream >> megabytes)
        {
            engine.SetHashSize(megabytes);
        }
    }
    else if (optionName == "Clear Hash")
    {
        engine.ClearHash();
    }
}

static void HandleBenchCommand(std::istringstream& commandStream)
{
    int depth = DEFAULT_BENCHMARK_DEPTH;
    int requestedDepth = 0;

    if (commandStream >> requestedDepth)
    {
        depth = requestedDepth;
    }

    RunBenchmark(depth);
}

static void HandleSearchBenchCommand(std::istringstream& commandStream)
{
    int depth = DEFAULT_SEARCH_BENCHMARK_DEPTH;
    int requestedDepth = 0;

    if (commandStream >> requestedDepth)
    {
        depth = requestedDepth;
    }

    RunSearchBenchmark(depth);
}

void RunLoop(Engine& engine)
{
    std::string commandLine;

    while (std::getline(std::cin, commandLine))
    {
        std::istringstream commandStream(commandLine);

        std::string command;
        commandStream >> command;

        if (command == "uci")
        {
            PrintLine("id name " + ENGINE_NAME + '\n' +
                      "id author " + ENGINE_AUTHOR + '\n' +
                      "option name Hash type spin default " + std::to_string(DEFAULT_HASH_SIZE_MEGABYTES) +
                      " min " + std::to_string(MIN_HASH_SIZE_MEGABYTES) +
                      " max " + std::to_string(MAX_HASH_SIZE_MEGABYTES) + '\n' +
                      "option name Clear Hash type button" + '\n' +
                      "uciok");
        }
        else if (command == "setoption")
        {
            HandleSetOptionCommand(engine, commandStream);
        }
        else if (command == "isready")
        {
            PrintLine("readyok");
        }
        else if (command == "ucinewgame")
        {
            engine.StopSearch();

            engine.NewGame();
        }
        else if (command == "position")
        {
            HandlePositionCommand(engine, commandStream);
        }
        else if (command == "go")
        {
            HandleGoCommand(engine, commandStream);
        }
        else if (command == "stop")
        {
            engine.StopSearch();
        }
        else if (command == "d")
        {
            PrintLine(engine.GetPosition().ToString());
        }
        else if (command == "bench")
        {
            HandleBenchCommand(commandStream);
        }
        else if (command == "searchbench")
        {
            HandleSearchBenchCommand(commandStream);
        }
        else if (command == "evaltest")
        {
            RunEvaluationTest();
        }
        else if (command == "quit")
        {
            break;
        }
    }
}

void PrintSearchInfo(const SearchResult& result)
{
    const int64_t nodesPerSecond = result.timeMilliseconds > 0
                                   ? int64_t(result.nodes) * 1000 / result.timeMilliseconds
                                   : int64_t(result.nodes) * 1000;

    std::ostringstream infoStream;

    infoStream << "info depth " << result.depth;

    if (IsMateScore(result.score))
    {
        infoStream << " score mate " << MateScoreToMoves(result.score);
    }
    else
    {
        infoStream << " score cp " << result.score;
    }

    infoStream << " nodes " << result.nodes
               << " nps " << nodesPerSecond
               << " time " << result.timeMilliseconds
               << " hashfull " << result.hashFull
               << " pv " << result.bestMove.ToString();

    PrintLine(infoStream.str());
}

void PrintBestMove(const SearchResult& result)
{
    PrintLine("bestmove " + (result.bestMove.IsNull() ? std::string("0000")
                                                      : result.bestMove.ToString()));
}

} // namespace Gluon::UCI
