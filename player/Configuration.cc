#include "Common.h"

#include "Configuration.h"

Configuration::Configuration( const std::vector< double >& weights )
    : m_weights( weights )
    , m_min_depth( 0 )
    , m_max_depth( 0 )
    , m_max_total_time( 0 )
{
}

const std::vector< double >&
Configuration::get_weights( ) const
{
    return m_weights;
}

const Configuration&
Configuration::get_best_configuration( )
{
    static Configuration best_configuration( {2.67989, 4.1556, 13.1196, 5.98808,
                                              -0.0133713, 2.09843, -4.45752,
                                              -5.7916} );
    best_configuration.m_min_depth = 1;
    best_configuration.m_max_depth = 20;
    best_configuration.m_max_total_time = 58000;

    return best_configuration;
}

const Configuration&
Configuration::get_lightweight_configuration( )
{
    static Configuration lightweight_configuration(
        {2.67989, 4.1556, 13.1196, 5.98808, -0.0133713, 2.09843, -4.45752,
         -5.7916} );
    lightweight_configuration.m_min_depth = 1;
    lightweight_configuration.m_max_depth = 4;
    lightweight_configuration.m_max_total_time = 10000;

    return lightweight_configuration;
}

int32_t
Configuration::get_min_depth( ) const
{
    return m_min_depth;
}

int32_t
Configuration::get_max_depth( ) const
{
    return m_max_depth;
}

int32_t
Configuration::get_max_total_time( ) const
{
    return m_max_total_time;
}
