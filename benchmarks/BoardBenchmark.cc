#include "../player/Common.h"

#include "../player/Board.h"

#include <benchmark/benchmark.h>

class BoardFixture : public benchmark::Fixture
{
};

BENCHMARK_F( BoardFixture, get_random_move )( benchmark::State& state )
{
    Board board;

    while ( state.KeepRunning( ) )
    {
        Move move = board.get_random_move( );
        (void)move;
    }
}
