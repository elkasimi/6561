#include "Common.h"

#include "Board.h"
#include "TTable.h"

TTable::TTable( )
    : m_search_hits( 0 )
    , m_search_misses( 0 )
    , m_entries( TTABLE_SIZE )
{
}

TTable::~TTable( )
{
}

void
TTable::reset_counters( )
{
    m_search_hits = 0;
    m_search_misses = 0;
}

uint32_t
TTable::get_search_hits( ) const
{
    return m_search_hits;
}

uint32_t
TTable::get_search_misses( ) const
{
    return m_search_misses;
}

Entry*
TTable::lookup( const Board& board, int32_t relevance )
{
    auto h = board.get_hash( );
    auto l = board.get_lock( );
    auto i = h;
    if ( m_entries[ i ].lock == l )
    {
        ++m_search_hits;
        if ( m_entries[ i ].relevance >= relevance )
        {
            return &m_entries[ i ];
        }
    }
    else
    {
        ++m_search_misses;
    }

    return nullptr;
}

void
TTable::store( const Board& board,
               const double& value,
               const Move& move,
               int32_t relevance )
{
    auto h = board.get_hash( );
    auto i = h;
    auto e = &m_entries[ i ];

    if ( e->relevance < relevance )
    {
        e->lock = board.get_lock( );
        e->relevance = relevance;
        e->move = move;
        e->value = value;
    }
}
