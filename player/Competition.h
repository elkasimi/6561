#pragma once

#include "Strategy.h"

class Competition
{
public:
    using Interval = std::pair< double, double >;
    struct Result
    {
        int32_t fails;
        double mean_score;
        double min_score;
        double max_score;
        Interval confidence_interval;
    };

    Competition( const Strategy::Type first_strategy_type,
                 const Strategy::Type second_strategy_type,
                 const int32_t iterations,
                 const int32_t simultaneous );
    ~Competition( );

    Result run( );

private:
    Strategy::Type m_first_strategy_type;
    Strategy::Type m_second_strategy_type;
    int32_t m_iterations;
    int32_t m_simultaneous;
};
