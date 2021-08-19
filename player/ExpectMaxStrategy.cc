#include "Common.h"

#include "Board.h"
#include "Configuration.h"
#include "Strategy.h"
#include "TTable.h"
#include "Timer.h"

ExpectMaxStrategy::ExpectMaxStrategy( const Configuration& configuration )
    : m_nodes( 0 )
    , m_total_nodes( 0 )
    , m_total_time( 0 )
    , m_use_ttable( true )
    , m_can_abort( false )
    , m_aborted( false )
    , m_solo( false )
    , m_log( false )
    , m_min_depth( configuration.get_min_depth( ) )
    , m_configuration( configuration )
{
    if ( m_use_ttable )
    {
        m_ttable = std::make_unique< TTable >( );
    }
    m_timer = std::make_unique< Timer >( configuration.get_max_total_time( ) );
}

ExpectMaxStrategy::~ExpectMaxStrategy( )
{
}

void
ExpectMaxStrategy::enable_solo_mode( )
{
    m_solo = true;
}

void
ExpectMaxStrategy::enable_logging( )
{
    m_log = true;
}

void
ExpectMaxStrategy::disable_logging( )
{
    m_log = false;
}

double
ExpectMaxStrategy::depth_first_search( const Board& board,
                                       uint32_t depth,
                                       uint32_t max_depth,
                                       bool& out_exact,
                                       Move* out_best_move )
{
    // check if we don't run out of time
    if ( m_can_abort && m_timer->is_time_over( ) )
    {
        m_aborted = true;
        return 0.0;
    }

    // lookup in the ttable
    auto relevance = max_depth - depth + board.get_phase( );

    if ( m_use_ttable )
    {
        auto entry = m_ttable->lookup( board, relevance );

        if ( entry != nullptr )
        {
            if ( entry->relevance != MAX_RELEVANCE )
            {
                out_exact = false;
            }
            if ( out_best_move != nullptr )
            {
                if ( entry->move != INVALID_MOVE )
                {
                    *out_best_move = entry->move;
                    ++m_nodes;
                    return entry->value;
                }
            }
            else
            {
                ++m_nodes;
                return entry->value;
            }
        }
    }

    const auto begin = board.begin_move( ), end = board.end_move( );

    if ( begin == end )
    {
        ++m_nodes;
        return board.get_best_score( );
    }

    if ( depth == max_depth )
    {
        out_exact = false;
        ++m_nodes;
        return board.evaluate( m_configuration );
    }

    if ( m_solo || ( depth % 2 == 0 ) )
    {
        auto value = std::numeric_limits< double >::min( );
        Move best_move = INVALID_MOVE;
        Move alternate_move = INVALID_MOVE;
        bool exact = true;

        std::for_each( begin, end, [&]( Move move ) {
            if ( m_aborted )
            {
                return;
            }

            auto tmp_board = board;
            tmp_board.do_move( move );
            auto v
                = depth_first_search( tmp_board, depth + 1, max_depth, exact );

            if ( value < v )
            {
                value = v;
                alternate_move = best_move;
                best_move = move;
            }
        } );

        if ( m_aborted )
        {
            return 0.0;
        }

        if ( out_best_move != nullptr )
        {
            if ( best_move != m_forbidden_move
                 || alternate_move == INVALID_MOVE )
            {
                *out_best_move = best_move;
            }
            else
            {
                *out_best_move = alternate_move;
            }
        }

        if ( exact )
        {
            relevance = MAX_RELEVANCE;
        }
        else
        {
            out_exact = false;
        }

        if ( m_use_ttable )
        {
            m_ttable->store( board, value, best_move, relevance );
        }

        return value;
    }
    else
    {
        auto sum_values = 0.0;
        auto sum_weights = 0.0;
        bool exact = true;
        std::for_each( begin, end, [&]( Move move ) {
            if ( m_aborted )
            {
                return;
            }
            auto tmp_board = board;
            tmp_board.do_move( move );
            auto value
                = depth_first_search( tmp_board, depth + 1, max_depth, exact );
            sum_values += value;
            sum_weights += 1;
        } );

        if ( m_aborted )
        {
            return 0.0;
        }

        auto value = sum_values / sum_weights;

        if ( exact )
        {
            relevance = MAX_RELEVANCE;
        }
        else
        {
            out_exact = false;
        }

        if ( m_use_ttable )
        {
            m_ttable->store( board, value, INVALID_MOVE, relevance );
        }

        return value;
    }
}

Move
ExpectMaxStrategy::get_best_move( const Board& board )
{
    m_forbidden_move = INVALID_MOVE;

    auto history_node = (HistoryNode*)nullptr;

    const auto iterator = std::find_if(
        m_history.cbegin( ), m_history.cend( ),
        [&board]( const HistoryNode& node ) { return node.first == board; } );

    if ( iterator != m_history.cend( ) )
    {
        m_forbidden_move = iterator->second;
        if ( m_log )
        {
            std::cerr
                << "position already seen! trying another move if possible"
                << std::endl;
        }
    }

    Move best_move;
    int32_t t = 0;
    int32_t p = board.get_phase( );

    if ( m_log )
    {
        std::cerr << "p=" << p << std::endl;
    }

    int32_t max_time = m_timer->get_max_time( board );
    m_timer->set_alarm( max_time );
    m_aborted = false;
    int32_t mind = 1;
    int32_t maxd = m_configuration.get_max_depth( );
    if ( m_use_ttable )
    {
        auto entry = m_ttable->lookup( board, mind );

        if ( entry != nullptr )
        {
            auto entry_depth = entry->relevance - board.get_phase( );
            if ( entry_depth <= maxd )
            {
                mind = entry_depth;
            }
        }

        m_ttable->reset_counters( );
    }

    if ( m_log )
    {
        std::cerr << "maxt=" << ( max_time / 1000.0 ) << std::endl;
    }

    for ( int32_t d = mind; d <= maxd; ++d )
    {
        m_can_abort = d > m_min_depth;
        m_nodes = 0;
        bool exact = true;
        m_timer->start( );
        auto value = depth_first_search( board, 0, d, exact, &best_move );
        m_timer->stop( );
        int32_t dt = m_timer->get_delta_time( );
        t += dt;
        m_total_time += dt;
        m_total_nodes += m_nodes;
        auto f = board.get_branching_factor_at_depth( d + 1 );

        if ( value > 1000000.0 )
        {
            value -= 1000000.0;
        }

        if ( m_log )
        {
            std::cerr << "\td=" << d << " n=" << m_nodes << " v=" << value
                      << " dt=" << ( dt / 1000.0 ) << " t=" << ( t / 1000.0 )
                      << " f=" << f << " tt=" << ( m_total_time / 1000.0 )
                      << std::endl;
        }

        if ( m_aborted )
        {
            if ( m_log )
            {
                std::cerr << "aborted!" << std::endl;
            }

            break;
        }

        if ( exact )
        {
            if ( m_log )
            {
                std::cerr << "exact!" << std::endl;
            }

            break;
        }

        if ( ( d >= m_min_depth ) && ( t + f * dt > max_time ) )
        {
            break;
        }
    }

    m_timer->clear_alarm( );

    if ( m_use_ttable )
    {
        if ( m_log )
        {
            uint32_t h = m_ttable->get_search_hits( );
            uint32_t m = m_ttable->get_search_misses( );
            auto r = 100.0 * h / ( 1.0 + m + h );
            std::cerr << "\tr=" << r << "%" << std::endl;
        }
    }

    if ( history_node == nullptr )
    {
        m_history.push_back( std::make_pair( board, best_move ) );
    }
    else
    {
        history_node->second = best_move;
    }

    return best_move;
}
