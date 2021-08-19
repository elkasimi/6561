#include "Common.h"

#include "Board.h"
#include "Strategy.h"

RandomStrategy::RandomStrategy( )
{
}

RandomStrategy::~RandomStrategy( )
{
}

Move
RandomStrategy::get_best_move( const Board& board )
{
    return board.get_random_move( );
}
