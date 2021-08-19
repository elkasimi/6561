#pragma once

class RNG
{
public:
    static void randomize( );
    static uint32_t pick( int32_t min = 0, int32_t max = RAND_MAX );

private:
    static std::default_random_engine ms_engine;
};
