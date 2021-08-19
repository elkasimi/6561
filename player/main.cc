#include "Common.h"

#include "Board.h"
#include "Competition.h"
#include "Configuration.h"
#include "IO.h"
#include "RNG.h"
#include "Strategy.h"

void
play_with_myself( const Configuration& configuration )
{
    Competition competition{Strategy::Type::EXPECT_MAX_LIGHTWEIGHT,
                            Strategy::Type::EXPECT_MAX_LIGHTWEIGHT, 50, 2};
    const auto result = competition.run( );
    std::cout << "mean-score=" << result.mean_score << std::endl;
}

void
play_with_random_player( const Configuration& configuration )
{
    Competition competition{Strategy::Type::EXPECT_MAX_LIGHTWEIGHT,
                            Strategy::Type::RANDOM, 50, 2};
    const auto result = competition.run( );
    std::cout << "mean-score=" << result.mean_score << std::endl;
}

void
main_loop( )
{
    std::cout.sync_with_stdio( false );
    std::cerr.sync_with_stdio( false );

    Board board;
    const auto& configuration = Configuration::get_best_configuration( );
    ExpectMaxStrategy player( configuration );
    player.enable_logging( );
    std::string s;

    std::cin >> s;
    while ( s != "Quit" )
    {
        if ( s == "B" )
        {
            // Nothing to do
        }
        else
        {
            if ( s != "A" )
            {
                Move opponent_move = IO::read_move( s );
                board.do_move( opponent_move );
                player.change_root( opponent_move );
            }

            Move my_move = player.get_best_move( board );
            board.do_move( my_move );
            player.change_root( my_move );
            IO::display_move_ln( std::cerr, my_move );
            IO::display_move_ln( std::cout, my_move );
        }

        std::cin >> s;
    }
}

int
main( int argc, char* argv[] )
{
    RNG::randomize( );
    Board::init_data( );

    if ( argc > 1 )
    {
        std::string s = argv[ 1 ];

        if ( s == "--best-configuration-self-play" )
        {
            const auto& best_configuration
                = Configuration::get_best_configuration( );
            play_with_myself( best_configuration );
        }
        else if ( s == "--best-lightweight-configuration-self-play" )
        {
            const auto& lightweight_configuration
                = Configuration::get_lightweight_configuration( );
            play_with_myself( lightweight_configuration );
        }
        else if ( s == "--best-configuration-with-random" )
        {
            const auto& best_configuration
                = Configuration::get_best_configuration( );
            play_with_random_player( best_configuration );
        }
        else if ( s == "--best-lightweight-configuration-with-random" )
        {
            const auto& lightweight_configuration
                = Configuration::get_lightweight_configuration( );
            play_with_random_player( lightweight_configuration );
        }
        else if ( s == "--configuration-with-random" )
        {
            std::vector< double > weights( argc - 2 );
            for ( int32_t i = 2; i < argc; ++i )
            {
                std::istringstream iss( argv[ i ] );
                iss >> weights[ i - 2 ];
            }
            Configuration configuration( weights );
            play_with_random_player( configuration );
        }
        else
        {
            std::cerr << "player usage:" << std::endl;
            std::cerr << "player --best-configuration-self-play or"
                      << std::endl;
            std::cerr << "player --best-lightweight-configuration-self-play or"
                      << std::endl;
            std::cerr << "player --best-configuration-with-random or"
                      << std::endl;
            std::cerr
                << "player --best-lightweight-configuration-with-random or"
                << std::endl;
            std::cerr << "player --configuration-with-random" << std::endl;
        }
    }
    else
    {
        main_loop( );
    }

    return 0;
}
