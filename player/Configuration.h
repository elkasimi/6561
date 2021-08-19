#pragma once

class Configuration
{
public:
    explicit Configuration( const std::vector< double >& weights );

    const std::vector< double >& get_weights( ) const;
    int32_t get_min_depth( ) const;
    int32_t get_max_depth( ) const;
    int32_t get_max_total_time( ) const;
    static const Configuration& get_best_configuration( );
    static const Configuration& get_lightweight_configuration( );

private:
    std::vector< double > m_weights;
    int32_t m_min_depth;
    int32_t m_max_depth;
    int32_t m_max_total_time;
};
