#include "Common.h"

#include "Board.h"
#include "Competition.h"
#include "Configuration.h"
#include "IO.h"
#include "StrategyFactory.h"

namespace
{
const uint32_t FAIL_THRESHOLD = 2187;

double
run_game( int32_t game_id,
          Strategy::Type first_player_type,
          Strategy::Type second_player_type )
{
    auto first_player = StrategyFactory::create( first_player_type );
    auto second_player = StrategyFactory::create( second_player_type );
    Board board;
    bool first = true;
    //    std::cerr << "Game " << game_id << " started .." << std::endl;
    int32_t m = 0;
    while ( !board.end_game( ) )
    {
        if ( first )
        {
            Move first_player_move = first_player->get_best_move( board );
            board.do_move( first_player_move );
            first = false;
        }
        else
        {
            Move second_player_move = second_player->get_best_move( board );
            board.do_move( second_player_move );
            first = true;
        }

        ++m;
        //        std::cerr << "In game " << game_id << " " << m << " moves
        //        played .." << std::endl;
    }

    //    std::cerr << "Game " << game_id << " ended." << std::endl;

    double score = board.get_best_score( );
    return score;
}
}

Competition::Competition( const Strategy::Type first_strategy_type,
                          const Strategy::Type second_strategy_type,
                          const int32_t iterations,
                          const int32_t simultaneous )
    : m_first_strategy_type( first_strategy_type )
    , m_second_strategy_type( second_strategy_type )
    , m_iterations( iterations )
    , m_simultaneous( simultaneous )
{
}

Competition::~Competition( )
{
}

Competition::Result
Competition::run( )
{
    std::vector< double > values;
    int32_t game_id = 0;

    for ( int32_t i = 0; i < m_iterations; ++i )
    {
        std::cerr << "i=" << i << std::endl;
        std::vector< std::future< double > > futures;

        for ( int32_t l = 0; l < m_simultaneous; ++l )
        {
            ++game_id;

            if ( l % 2 == 0 )
                futures.push_back( std::async( std::launch::async, run_game,
                                               game_id, m_first_strategy_type,
                                               m_second_strategy_type ) );
            else
                futures.push_back( std::async( std::launch::async, run_game,
                                               game_id, m_second_strategy_type,
                                               m_first_strategy_type ) );
        }

        for ( auto& future : futures )
        {
            auto value = future.get( );
            std::cerr << "\t" << value;
            values.push_back( value );
        }
        std::cerr << std::endl;
    }

    Result result;

    result.fails = std::count_if(
        values.cbegin( ), values.cend( ),
        []( double value ) { return value < FAIL_THRESHOLD; } );

    auto values_count = values.size( );
    double mean = std::accumulate( values.cbegin( ), values.cend( ), 0.0 );
    mean /= values_count;
    result.mean_score = mean;

    result.min_score = *std::min( values.cbegin( ), values.cend( ) );

    result.max_score = *std::max( values.cbegin( ), values.cend( ) );

    std::vector< double > diff( values_count );
    std::transform( values.cbegin( ), values.cend( ), diff.begin( ),
                    [mean]( double x ) { return x - mean; } );
    double square_sum = std::inner_product( diff.cbegin( ), diff.cend( ),
                                            diff.cbegin( ), 0.0 );
    double std_deviation = std::sqrt( square_sum / values_count );
    double min_estimated_value
        = mean - 2.0 * std_deviation / std::sqrt( values_count );
    double max_estimated_value
        = mean + 2.0 * std_deviation / std::sqrt( values_count );
    result.confidence_interval.first = min_estimated_value;
    result.confidence_interval.second = max_estimated_value;

    return result;
}
