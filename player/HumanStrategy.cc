#include "Common.h"

#include "Board.h"
#include "IO.h"
#include "Strategy.h"

HumanStrategy::HumanStrategy( )
{
}

HumanStrategy::~HumanStrategy( )
{
}

Move
HumanStrategy::get_best_move( const Board& board )
{
    auto begin = board.begin_move( ), end = board.end_move( );
    assert( begin != end && "Game ended!" );

    std::cout << "possible moves are:" << std::endl;
    std::for_each( begin, end, []( Move move ) {
        IO::display_move_ln( std::cout, move );
    } );

    for ( ;; )
    {
        std::cout << "Please choose one valid move .." << std::endl;
        std::string s;
        std::cin >> s;
        Move human_move = IO::read_move( s );
        if ( std::find( begin, end, human_move ) != end )
            return human_move;
    }
}
