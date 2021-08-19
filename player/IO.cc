#include "Common.h"

#include "Board.h"
#include "IO.h"

Move
IO::read_move( const std::string& string )
{
    Move move;
    if ( string == "R" )
    {
        move = 1 | ( Board::RIGHT << 1 );
    }
    else if ( string == "L" )
    {
        move = 1 | ( Board::LEFT << 1 );
    }
    else if ( string == "U" )
    {
        move = 1 | ( Board::UP << 1 );
    }
    else if ( string == "D" )
    {
        move = 1 | ( Board::DOWN << 1 );
    }
    else
    {
        uint32_t row = string[ 0 ] - '0';
        uint32_t column = string[ 1 ] - '0';
        uint32_t place = 4 * ( row - 1 ) + ( column - 1 );
        move = place << 1;
    }

    return move;
}

void
IO::display_move( std::ostream& out, const Move& move )
{
    if ( move & 1 )
    {
        uint32_t dir = move >> 1;
        if ( dir == Board::RIGHT )
        {
            out << "R";
        }
        else if ( dir == Board::LEFT )
        {
            out << "L";
        }
        else if ( dir == Board::UP )
        {
            out << "U";
        }
        else if ( dir == Board::DOWN )
        {
            out << "D";
        }
        else
        {
            out << "invalid move!";
        }
    }
    else
    {
        uint32_t place = move >> 1;
        uint32_t row = ( place >> 2 ) + 1;
        uint32_t column = ( place & 3 ) + 1;
        if ( row >= 1 && row <= 4 && column >= 1 && column <= 4 )
        {
            out << row << column;
        }
        else
        {
            out << "invalid move!";
        }
    }
}

void
IO::display_move_ln( std::ostream& out, const Move& move )
{
    IO::display_move( out, move );
    out << std::endl;
}
