#pragma once

struct Entry
{
    Entry( )
        : lock( 0 )
        , relevance( 0 )
        , move( INVALID_MOVE )
        , value( 0.0 )
    {
    }
    uint32_t lock;
    int32_t relevance;
    Move move;
    double value;
};

class Board;
class TTable
{
public:
    TTable( );
    ~TTable( );

    void reset_counters( );
    uint32_t get_search_hits( ) const;
    uint32_t get_search_misses( ) const;

    Entry* lookup( const Board& board, int32_t relevance );
    void store( const Board& board,
                const double& value,
                const Move& move,
                int32_t relevance );

private:
    uint32_t m_search_hits;
    uint32_t m_search_misses;
    std::vector< Entry > m_entries;
};
