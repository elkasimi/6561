#include "Common.h"

#include "RNG.h"

std::default_random_engine RNG::ms_engine;

void
RNG::randomize( )
{
    std::random_device rd{};
    auto s = rd( );
    ms_engine.seed( s );
}

uint32_t
RNG::pick( int32_t min, int32_t max )
{
    std::uniform_int_distribution<> d{};
    return d( ms_engine,
              std::uniform_int_distribution<>::param_type{min, max} );
}
