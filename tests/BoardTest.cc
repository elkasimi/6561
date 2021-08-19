#include "../player/Common.h"

#include "../player/Board.h"

#include <gtest/gtest.h>

TEST( BoardTest, check_board_initialization )
{
    Board board;
    ASSERT_FALSE( board.end_game( ) );
}
