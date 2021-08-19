#include "Common.h"

#include "Board.h"
#include "Timer.h"

Timer::Timer( int max_total_time )
    : m_delta_time( 0 )
    , m_total_time( 0 )
    , m_max_total_time( max_total_time )
    , m_alarm_delta_time( 0 )
    , m_alarm_set( false )
    , m_time_over( false )
{
}

Timer::~Timer( )
{
}

void
Timer::start( )
{
    m_start = SystemClock::now( );
}

void
Timer::stop( )
{
    m_end = SystemClock::now( );
    auto dur = std::chrono::duration_cast< std::chrono::milliseconds >(
        m_end - m_start );
    m_delta_time = (uint32_t)dur.count( );
    m_total_time += m_delta_time;
}

int32_t
Timer::get_delta_time( ) const
{
    return m_delta_time;
}

int32_t
Timer::get_total_time( ) const
{
    return m_total_time;
}

int32_t
Timer::get_max_time( const Board& board ) const
{
    uint32_t p = board.get_phase( );
    int32_t remaining_moves = ( 1000 - p ) / 2;

    if ( remaining_moves <= 0 )
    {
        remaining_moves = 1;
    }

    int32_t max_time = 0;

    if ( m_max_total_time > m_total_time )
    {
        max_time = ( m_max_total_time - m_total_time ) / remaining_moves;
    }

    return max_time;
}

void
Timer::set_alarm( uint32_t milliseconds )
{
    m_alarm_start_time = SystemClock::now( );
    m_alarm_delta_time = milliseconds;
    m_time_over = false;
    m_alarm_set = true;
}

void
Timer::clear_alarm( )
{
    m_alarm_set = false;
}

bool
Timer::is_time_over( ) const
{
    if ( !m_time_over )
    {
        auto now = SystemClock::now( );
        auto dur = std::chrono::duration_cast< std::chrono::milliseconds >(
            now - m_alarm_start_time );
        auto delta_time = dur.count( );
        m_time_over = delta_time >= m_alarm_delta_time;
    }

    return m_time_over;
}
