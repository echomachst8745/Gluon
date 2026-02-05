#include "engine.h"

#include "movegenerator.h"
#include "movelist.h"
#include "uci.h"

namespace Gluon {

// [ Constructors ]
Engine::Engine()
{
    NewGame();
}

Engine::~Engine()
{
    StopSearch();
}

// [ Public methods ]
void Engine::Run()
{
    UCI::RunLoop(*this);

    StopSearch();
}

void Engine::NewGame()
{
    SetPosition(START_POSITION_FEN);

    ClearHash();
}

void Engine::SetPosition(const std::string& fen)
{
    position.SetupWithFEN(fen);
}

void Engine::PlayMove(const std::string& moveString)
{
    const MoveList moves = GenerateLegalMoves(position);

    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        if (moves[moveIndex].ToString() == moveString)
        {
            PositionState state;

            position.MakeMove(moves[moveIndex], state);

            return;
        }
    }
}

void Engine::StartSearch(const SearchLimits& limits)
{
    StopSearch();

    searchThread = std::thread(&Engine::RunSearch, this, position, limits);
}

void Engine::StopSearch()
{
    searcher.RequestStop();

    WaitForSearch();
}

void Engine::WaitForSearch()
{
    if (searchThread.joinable())
    {
        searchThread.join();
    }
}

void Engine::SetHashSize(size_t megabytes)
{
    StopSearch();

    searcher.ResizeTranspositionTable(megabytes);
}

void Engine::ClearHash()
{
    StopSearch();

    searcher.ClearTranspositionTable();
}

// [ Private methods ]
void Engine::RunSearch(Position searchPosition, SearchLimits limits)
{
    const SearchResult result = searcher.Run(searchPosition, limits, UCI::PrintSearchInfo);

    UCI::PrintBestMove(result);
}

} // namespace Gluon
