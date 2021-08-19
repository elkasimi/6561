#pragma once

class Board;

class Timer
{
public:
    explicit Timer( int max_total_time = 0 );
    ~Timer( );

    void start( );
    void stop( );
    int32_t get_delta_time( ) const;
    int32_t get_total_time( ) const;
    int32_t get_max_time( const Board& board ) const;
    void set_alarm( uint32_t milliseconds );
    void clear_alarm( );
    bool is_time_over( ) const;

private:
    using SystemClock = std::chrono::system_clock;
    using TimePoint = SystemClock::time_point;

    TimePoint m_start;
    TimePoint m_end;
    uint32_t m_delta_time;
    uint32_t m_total_time;
    uint32_t m_max_total_time;
    TimePoint m_alarm_start_time;
    uint32_t m_alarm_delta_time;
    bool m_alarm_set;
    mutable bool m_time_over;
};
