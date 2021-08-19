#include "Common.h"

#include "Board.h"
#include "Configuration.h"
#include "RNG.h"
#include "Timer.h"

Board::Board( )
    : m_score( 0 )
    , m_best_score( 0 )
    , m_phase( 0 )
{
    m_rows[ 0 ] = 0x0u;
    m_rows[ 1 ] = 0x0u;
    m_rows[ 2 ] = 0x0u;
    m_rows[ 3 ] = 0x0u;
}

namespace
{
uint32_t ROWS_SLIDE_LEFT[ MAX_ROW ];
uint32_t ROWS_SLIDE_RIGHT[ MAX_ROW ];
uint32_t ROWS_SCORE[ MAX_ROW ];
bool ROWS_CAN_BE_SLIDED_LEFT[ MAX_ROW ];
bool ROWS_CAN_BE_SLIDED_RIGHT[ MAX_ROW ];

uint32_t ROWS_EMPTY[ MAX_ROW ];
uint8_t ROWS_SUM[ 3 ][ MAX_ROW ];
uint8_t ROWS_MERGES[ 3 ][ MAX_ROW ];
uint8_t ROWS_BAD_MERGES[ 3 ][ MAX_ROW ];
uint8_t ROWS_DESTROYS[ 3 ][ MAX_ROW ];
uint8_t ROWS_BAD_DESTROYS[ 3 ][ MAX_ROW ];
uint16_t ROWS_MONOTONOCITY[ 3 ][ MAX_ROW ];

uint32_t ZOBRIST_HASH_INIT;
uint32_t ZOBRIST_HASH_FIELD[ 4 ][ 2 ][ 1024 ];
uint32_t ZOBRIST_HASH_PHASE[ 1001 ];

uint32_t ZOBRIST_LOCK_INIT;
uint32_t ZOBRIST_LOCK_FIELD[ 4 ][ 2 ][ 1024 ];
uint32_t ZOBRIST_LOCK_PHASE[ 1001 ];

const uint8_t WHITE = 0;
const uint8_t RED = 1;
const uint8_t BLUE = 2;
const uint8_t GRAY = 3;

const double EVAL_MIN = 1000000.0;

const uint32_t POW_3[] = {1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683};

uint32_t
reverse( uint32_t row )
{
    return ( ( row >> 15 ) & FIELD_MASK ) << 0
           | ( ( row >> 10 ) & FIELD_MASK ) << 5
           | ( ( row >> 5 ) & FIELD_MASK ) << 10
           | ( ( row >> 0 ) & FIELD_MASK ) << 15;
}

void
rotate_90( const uint32_t rows[ 4 ], uint32_t rotated_rows[ 4 ] )
{
    rotated_rows[ 0 ] = ( ( rows[ 0 ] >> 15 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 1 ] >> 15 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 2 ] >> 15 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 3 ] >> 15 ) & FIELD_MASK ) << 15;

    rotated_rows[ 1 ] = ( ( rows[ 0 ] >> 10 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 1 ] >> 10 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 2 ] >> 10 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 3 ] >> 10 ) & FIELD_MASK ) << 15;

    rotated_rows[ 2 ] = ( ( rows[ 0 ] >> 5 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 1 ] >> 5 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 2 ] >> 5 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 3 ] >> 5 ) & FIELD_MASK ) << 15;

    rotated_rows[ 3 ] = ( ( rows[ 0 ] >> 0 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 1 ] >> 0 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 2 ] >> 0 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 3 ] >> 0 ) & FIELD_MASK ) << 15;
}

void
rotate_270( const uint32_t rows[ 4 ], uint32_t rotated_rows[ 4 ] )
{
    rotated_rows[ 0 ] = ( ( rows[ 3 ] >> 0 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 2 ] >> 0 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 1 ] >> 0 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 0 ] >> 0 ) & FIELD_MASK ) << 15;

    rotated_rows[ 1 ] = ( ( rows[ 3 ] >> 5 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 2 ] >> 5 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 1 ] >> 5 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 0 ] >> 5 ) & FIELD_MASK ) << 15;

    rotated_rows[ 2 ] = ( ( rows[ 3 ] >> 10 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 2 ] >> 10 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 1 ] >> 10 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 0 ] >> 10 ) & FIELD_MASK ) << 15;

    rotated_rows[ 3 ] = ( ( rows[ 3 ] >> 15 ) & FIELD_MASK ) << 0
                        | ( ( rows[ 2 ] >> 15 ) & FIELD_MASK ) << 5
                        | ( ( rows[ 1 ] >> 15 ) & FIELD_MASK ) << 10
                        | ( ( rows[ 0 ] >> 15 ) & FIELD_MASK ) << 15;
}

bool
row_can_be_slided_left( uint32_t row )
{
    // pack the row
    uint32_t p = 0;
    uint32_t s = 0;
    uint32_t r = row;
    while ( r > 0 )
    {
        if ( r & FIELD_MASK )
        {
            p |= ( r & FIELD_MASK ) << s;
            s += 5;
        }
        else
        {
            return true;
        }

        r >>= 5;
    }

    // slide the row
    while ( p > 0 )
    {
        if ( ( ( ( p & FIELD_MASK ) >> 2 ) == ( ( p & 0x3e0 ) >> 7 ) )
             && ( p & 0x3e0 ) )
        {
            return true;
        }
        else
        {
            p >>= 5;
        }
    }

    return false;
}

bool
row_can_be_slided_right( uint32_t row )
{
    return row_can_be_slided_left( reverse( row ) );
}

uint32_t
row_pack_left( uint32_t row )
{
    uint32_t p = 0;
    uint32_t s = 0;
    uint32_t r = row;
    while ( r > 0 )
    {
        if ( r & FIELD_MASK )
        {
            p |= ( r & FIELD_MASK ) << s;
            s += 5;
        }
        r >>= 5;
    }
    return p;
}

uint32_t
row_slide_left( uint32_t row )
{
    uint32_t x = 0;
    uint32_t p = row;
    uint32_t s = 0;
    while ( p > 0 )
    {
        if ( ( ( ( p & FIELD_MASK ) >> 2 ) == ( ( p & 0x3e0 ) >> 7 ) )
             && ( p & 0x3e0 ) )
        {
            if ( ( p & 0x3 ) == ( ( p & 0x60 ) >> 5 ) )
            {
                x |= ( ( p & FIELD_MASK ) + 4 ) << s;
                s += 5;
            }
            p >>= 10;
        }
        else
        {
            x |= ( p & FIELD_MASK ) << s;
            s += 5;
            p >>= 5;
        }
    }

    return x;
}

uint32_t
row_score( uint32_t row )
{
    uint32_t s = 0;
    while ( row > 0 )
    {
        uint32_t f = row & FIELD_MASK;
        if ( f & 3 )
        {
            s += POW_3[ f >> 2 ];
        }
        row >>= 5;
    }

    return s;
}

uint32_t
row_empty( uint32_t row )
{
    uint32_t e = 0;
    for ( uint32_t i = 0; i < 4; ++i )
    {
        e += ( row & 3 ) == 0;
        row >>= 5;
    }

    return e;
}

uint8_t
row_sum( uint32_t row, uint8_t color )
{
    uint8_t sum = 0;
    while ( row > 0 )
    {
        uint32_t f = row & FIELD_MASK;

        if ( FIELD_COLOR( f ) == color )
        {
            sum += 1 << FIELD_VALUE( f );
        }

        row >>= 5;
    }

    return sum;
}

uint8_t
row_merges( uint32_t row, uint8_t color )
{
    uint8_t merges = 0;
    uint8_t count = 0;
    uint8_t prev = 0;
    while ( row > 0 )
    {
        uint8_t f = row & FIELD_MASK;
        if ( f > 0 )
        {
            if ( f == prev )
            {
                ++count;
            }
            else if ( count > 0 )
            {
                if ( FIELD_COLOR( prev ) == color )
                {
                    merges += count + 1;
                }
                count = 0;
            }
            prev = f;
        }
        row >>= 5;
    }

    if ( count > 0 && FIELD_COLOR( prev ) == color )
    {
        merges += count + 1;
    }

    return merges;
}

uint8_t
row_bad_merges( uint32_t row, uint8_t color )
{
    uint8_t merges = 0;
    uint8_t count = 0;
    uint8_t prev = 0;
    while ( row > 0 )
    {
        uint8_t f = row & FIELD_MASK;
        if ( f > 0 )
        {
            if ( f == prev )
            {
                ++count;
            }
            else if ( count > 0 )
            {
                if ( FIELD_COLOR( prev ) != color )
                {
                    merges += count + 1;
                }
                count = 0;
            }
            prev = f;
        }
        row >>= 5;
    }

    if ( count > 0 && FIELD_COLOR( prev ) != color )
    {
        merges += count + 1;
    }

    return merges;
}

uint8_t
row_destroys( uint32_t row, uint8_t color )
{
    uint8_t destroys = 0;
    uint8_t prev = 0;
    while ( row > 0 )
    {
        uint8_t f = row & FIELD_MASK;
        if ( f > 0 )
        {
            destroys += ( prev != 0 )
                        && ( FIELD_VALUE( f ) == FIELD_VALUE( prev ) )
                        && ( FIELD_COLOR( f ) != FIELD_COLOR( prev ) )
                        && ( FIELD_COLOR( f ) != color )
                        && ( FIELD_COLOR( prev ) != color );
            prev = f;
        }
        row >>= 5;
    }

    return destroys;
}

uint8_t
row_bad_destroys( uint32_t row, uint8_t color )
{
    uint8_t destroys = 0;
    uint8_t prev = 0;
    while ( row > 0 )
    {
        uint8_t f = row & FIELD_MASK;
        if ( f > 0 )
        {
            destroys += ( prev != 0 )
                        && ( FIELD_VALUE( f ) == FIELD_VALUE( prev ) )
                        && ( FIELD_COLOR( f ) != FIELD_COLOR( prev ) )
                        && ( FIELD_COLOR( f ) == color
                             || FIELD_COLOR( prev ) == color );
            prev = f;
        }
        row >>= 5;
    }

    return destroys;
}

uint16_t
row_monotonocity( uint32_t row, uint8_t color )
{
    uint16_t p[] = {0, 1, 8, 27, 64, 125, 217, 343};

    uint32_t fields[ 4 ]
        = {( row >> 0 ) & FIELD_MASK, ( row >> 5 ) & FIELD_MASK,
           ( row >> 10 ) & FIELD_MASK, ( row >> 15 ) & FIELD_MASK};

    uint32_t mr = 0;
    for ( uint32_t i = 1; i < 4; ++i )
    {
        uint32_t f = fields[ i ];
        uint32_t prev = fields[ i - 1 ];
        if ( FIELD_COLOR( f ) == color )
        {
            uint32_t vf = FIELD_VALUE( f );
            uint32_t vp = FIELD_VALUE( prev );
            if ( FIELD_COLOR( prev ) == 0 || FIELD_COLOR( prev ) == color )
            {
                if ( vf > vp )
                    mr += p[ vf ] - p[ vp ];
            }
        }
    }

    uint32_t reversed_fields[ 4 ]
        = {( row >> 15 ) & FIELD_MASK, ( row >> 10 ) & FIELD_MASK,
           ( row >> 5 ) & FIELD_MASK, ( row >> 0 ) & FIELD_MASK};
    uint32_t ml = 0;
    for ( uint32_t i = 1; i < 4; ++i )
    {
        uint32_t f = reversed_fields[ i ];
        uint32_t prev = reversed_fields[ i - 1 ];
        if ( FIELD_COLOR( f ) == color )
        {
            uint32_t vf = FIELD_VALUE( f );
            uint32_t vp = FIELD_VALUE( prev );
            if ( FIELD_COLOR( prev ) == 0 || FIELD_COLOR( prev ) == color )
            {
                if ( vf > vp )
                    ml += p[ vf ] - p[ vp ];
            }
        }
    }

    return ml < mr ? ml : mr;
}
}

void
Board::init_data( )
{
    Timer timer;
    timer.start( );
    for ( uint32_t r = 0; r < MAX_ROW; ++r )
    {
        uint32_t p = row_pack_left( r );
        ROWS_SLIDE_LEFT[ r ] = row_slide_left( p );
        p = row_pack_left( reverse( r ) );
        ROWS_SLIDE_RIGHT[ r ] = reverse( row_slide_left( p ) );
        ROWS_SCORE[ r ] = row_score( r );
        ROWS_EMPTY[ r ] = row_empty( r );
        ROWS_CAN_BE_SLIDED_LEFT[ r ] = row_can_be_slided_left( r );
        ROWS_CAN_BE_SLIDED_RIGHT[ r ] = row_can_be_slided_right( r );

        for ( uint8_t color = 1; color <= 3; ++color )
        {
            auto c = color - 1;
            ROWS_SUM[ c ][ r ] = row_sum( r, color );
            ROWS_MERGES[ c ][ r ] = row_merges( r, color );
            ROWS_BAD_MERGES[ c ][ r ] = row_bad_merges( r, color );
            ROWS_DESTROYS[ c ][ r ] = row_destroys( r, color );
            ROWS_MONOTONOCITY[ c ][ r ] = row_monotonocity( r, color );
            ROWS_BAD_DESTROYS[ c ][ r ] = row_bad_destroys( r, color );
        }
    }

    ZOBRIST_HASH_INIT = RNG::pick( 0, TTABLE_SIZE - 1 );
    ZOBRIST_LOCK_INIT = RNG::pick( );

    for ( uint32_t i = 0; i <= MAX_PHASE; ++i )
    {
        ZOBRIST_HASH_PHASE[ i ] = RNG::pick( 0, TTABLE_SIZE - 1 );
        ZOBRIST_LOCK_PHASE[ i ] = RNG::pick( );
    }

    for ( uint32_t i = 0; i < 4; ++i )
    {
        for ( uint32_t j = 0; j < 2; ++j )
        {
            for ( uint32_t f = 0; f < 1024; ++f )
            {
                ZOBRIST_HASH_FIELD[ i ][ j ][ f ]
                    = RNG::pick( 0, TTABLE_SIZE - 1 );
                ZOBRIST_LOCK_FIELD[ i ][ j ][ f ] = RNG::pick( );
            }
        }
    }

    double s = 0.0;

    s += sizeof( ROWS_SLIDE_LEFT );
    s += sizeof( ROWS_SLIDE_RIGHT );
    s += sizeof( ROWS_SCORE );
    s += sizeof( ROWS_CAN_BE_SLIDED_LEFT );
    s += sizeof( ROWS_CAN_BE_SLIDED_RIGHT );
    s += sizeof( ROWS_EMPTY );
    s += sizeof( ROWS_SUM );
    s += sizeof( ROWS_MERGES );
    s += sizeof( ROWS_BAD_MERGES );
    s += sizeof( ROWS_DESTROYS );
    s += sizeof( ROWS_BAD_DESTROYS );
    s += sizeof( ROWS_MONOTONOCITY );
    s += sizeof( ZOBRIST_HASH_INIT );
    s += sizeof( ZOBRIST_HASH_FIELD );
    s += sizeof( ZOBRIST_HASH_PHASE );
    s += sizeof( ZOBRIST_LOCK_INIT );
    s += sizeof( ZOBRIST_LOCK_FIELD );
    s += sizeof( ZOBRIST_LOCK_PHASE );
    s += sizeof( POW_3 );
    s += sizeof( WHITE );
    s += sizeof( RED );
    s += sizeof( BLUE );
    s += sizeof( GRAY );
    s += sizeof( EVAL_MIN );

    timer.stop( );
    std::cerr << "data init took " << timer.get_delta_time( ) / 1000.0
              << " second" << std::endl;

    s /= 1024 * 1024;
    std::cerr << "data size = " << s << "M" << std::endl;
}

bool
Board::can_be_slided_left( ) const
{
    return ROWS_CAN_BE_SLIDED_LEFT[ m_rows[ 0 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ m_rows[ 1 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ m_rows[ 2 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ m_rows[ 3 ] ];
}

bool
Board::can_be_slided_right( ) const
{
    return ROWS_CAN_BE_SLIDED_RIGHT[ m_rows[ 0 ] ]
           || ROWS_CAN_BE_SLIDED_RIGHT[ m_rows[ 1 ] ]
           || ROWS_CAN_BE_SLIDED_RIGHT[ m_rows[ 2 ] ]
           || ROWS_CAN_BE_SLIDED_RIGHT[ m_rows[ 3 ] ];
}

bool
Board::can_be_slided_up( ) const
{
    uint32_t rotated_rows[ 4 ];
    rotate_90( m_rows, rotated_rows );

    return ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 0 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 1 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 2 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 3 ] ];
}

bool
Board::can_be_slided_down( ) const
{
    uint32_t rotated_rows[ 4 ];
    rotate_270( m_rows, rotated_rows );

    return ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 0 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 1 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 2 ] ]
           || ROWS_CAN_BE_SLIDED_LEFT[ rotated_rows[ 3 ] ];
}

bool
Board::end_game( ) const
{
    return begin_move( ) == end_move( );
}

void
Board::do_move( const Move& move )
{
    if ( move & 1 )
    {
        auto dir = move >> 1;

        if ( dir == RIGHT )
        {
            m_rows[ 0 ] = ROWS_SLIDE_RIGHT[ m_rows[ 0 ] ];
            m_rows[ 1 ] = ROWS_SLIDE_RIGHT[ m_rows[ 1 ] ];
            m_rows[ 2 ] = ROWS_SLIDE_RIGHT[ m_rows[ 2 ] ];
            m_rows[ 3 ] = ROWS_SLIDE_RIGHT[ m_rows[ 3 ] ];
        }
        else if ( dir == LEFT )
        {
            m_rows[ 0 ] = ROWS_SLIDE_LEFT[ m_rows[ 0 ] ];
            m_rows[ 1 ] = ROWS_SLIDE_LEFT[ m_rows[ 1 ] ];
            m_rows[ 2 ] = ROWS_SLIDE_LEFT[ m_rows[ 2 ] ];
            m_rows[ 3 ] = ROWS_SLIDE_LEFT[ m_rows[ 3 ] ];
        }
        else if ( dir == UP )
        {
            uint32_t rotated_rows[ 4 ];
            rotate_90( m_rows, rotated_rows );
            rotated_rows[ 0 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 0 ] ];
            rotated_rows[ 1 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 1 ] ];
            rotated_rows[ 2 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 2 ] ];
            rotated_rows[ 3 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 3 ] ];
            rotate_270( rotated_rows, m_rows );
        }
        else if ( dir == DOWN )
        {
            uint32_t rotated_rows[ 4 ];
            rotate_270( m_rows, rotated_rows );
            rotated_rows[ 0 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 0 ] ];
            rotated_rows[ 1 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 1 ] ];
            rotated_rows[ 2 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 2 ] ];
            rotated_rows[ 3 ] = ROWS_SLIDE_LEFT[ rotated_rows[ 3 ] ];
            rotate_90( rotated_rows, m_rows );
        }
        else
        {
            assert( false );
        }
    }
    else
    {
        uint32_t place = move >> 1;
        uint32_t u = 0;
        uint32_t a = m_phase % 10;
        if ( a == 0 || a == 5 )
        {
            u = 2;
        }
        else if ( a == 1 || a == 6 )
        {
            u = 1;
        }
        else if ( a == 2 || a == 7 )
        {
            u = 3;
        }
        else
        {
            std::cerr << "phase=" << m_phase << " a=" << a << std::endl;
            assert( false );
        }

        m_rows[ place >> 2 ] |= u << ( 5 * ( place & 3 ) );
    }

    update( );
}

Move
Board::get_random_move( ) const
{
    return *begin_move( );
}

uint32_t
Board::get_score( ) const
{
    return m_score;
}

uint32_t
Board::get_best_score( ) const
{
    return m_best_score;
}

int32_t
Board::get_phase( ) const
{
    return m_phase;
}

Board::MoveIterator
Board::begin_move( ) const
{
    return MoveIterator( this );
}

Board::MoveIterator
Board::end_move( ) const
{
    return MoveIterator( this, true );
}

void
Board::update( )
{
    uint32_t p = m_phase % 10;

    if ( p == 0 || p == 5 )
    {
        m_score++;
    }
    else if ( p == 1 || p == 6 )
    {
        m_score++;
    }
    else if ( p == 2 || p == 7 )
    {
        m_score++;
    }
    else
    {
        m_score = 0;

        for ( uint32_t i = 0; i < 4; ++i )
        {
            uint32_t r = m_rows[ i ];
            m_score += ROWS_SCORE[ r ];
        }
    }

    m_best_score = std::max( m_best_score, m_score );

    ++m_phase;
}

uint32_t
Board::get_hash( ) const
{
    uint32_t h = ZOBRIST_HASH_INIT;

    for ( uint32_t i = 0; i < 4; ++i )
    {
        uint32_t r = m_rows[ i ];
        uint32_t lo = r & 1023;
        uint32_t hi = r >> 10;
        if ( lo != 0 )
            h ^= ZOBRIST_HASH_FIELD[ i ][ 0 ][ lo ];
        if ( hi != 0 )
            h ^= ZOBRIST_HASH_FIELD[ i ][ 1 ][ hi ];
    }

    h ^= ZOBRIST_HASH_PHASE[ m_phase ];
    return h;
}

uint32_t
Board::get_lock( ) const
{
    uint32_t l = ZOBRIST_LOCK_INIT;

    for ( uint32_t i = 0; i < 4; ++i )
    {
        uint32_t r = m_rows[ i ];
        uint32_t lo = r & 1023;
        uint32_t hi = r >> 10;
        if ( lo != 0 )
            l ^= ZOBRIST_LOCK_FIELD[ i ][ 0 ][ lo ];
        if ( hi != 0 )
            l ^= ZOBRIST_LOCK_FIELD[ i ][ 1 ][ hi ];
    }

    l ^= ZOBRIST_LOCK_PHASE[ m_phase ];
    return l;
}

double
Board::evaluate( const Configuration& configuration ) const
{
    uint8_t red_sum = 0;
    uint8_t gray_sum = 0;
    uint8_t blue_sum = 0;

    uint8_t best_sum = 0;
    uint8_t bad_sum = 0;

    for ( uint32_t i = 0; i < 4; ++i )
    {
        uint32_t r = m_rows[ i ];
        red_sum += ROWS_SUM[ RED - 1 ][ r ];
        gray_sum += ROWS_SUM[ GRAY - 1 ][ r ];
        blue_sum += ROWS_SUM[ BLUE - 1 ][ r ];
    }

    uint32_t best_color = WHITE;
    if ( red_sum < gray_sum )
    {
        if ( gray_sum < blue_sum )
        {
            best_color = BLUE;
            best_sum = blue_sum;
            bad_sum = red_sum + gray_sum;
        }
        else
        {
            best_color = GRAY;
            best_sum = gray_sum;
            bad_sum = red_sum + blue_sum;
        }
    }
    else if ( red_sum < blue_sum )
    {
        best_color = BLUE;
        best_sum = blue_sum;
        bad_sum = red_sum + gray_sum;
    }
    else
    {
        best_color = RED;
        best_sum = red_sum;
        bad_sum = gray_sum + blue_sum;
    }

    --best_color;

    uint32_t merges = 0;
    uint32_t destroys = 0;
    uint32_t monotonocity = 0;
    uint32_t empties = 0;
    uint32_t bad_merges = 0;
    uint32_t bad_destroys = 0;

    // compute merges for the best color
    uint32_t rotated_rows[ 4 ];
    rotate_90( m_rows, rotated_rows );

    uint8_t* rows_merges = ROWS_MERGES[ best_color ];
    uint8_t* rows_bad_merges = ROWS_BAD_MERGES[ best_color ];
    uint8_t* rows_destroys = ROWS_DESTROYS[ best_color ];
    uint8_t* rows_bad_destroys = ROWS_BAD_DESTROYS[ best_color ];
    uint16_t* rows_monotonocity = ROWS_MONOTONOCITY[ best_color ];

    for ( uint32_t i = 0; i < 4; ++i )
    {
        uint32_t r = m_rows[ i ];
        uint32_t ro = rotated_rows[ i ];
        empties += ROWS_EMPTY[ r ];
        merges += rows_merges[ r ];
        merges += rows_merges[ ro ];
        bad_merges += rows_bad_merges[ r ];
        bad_merges += rows_bad_merges[ ro ];
        destroys += rows_destroys[ r ];
        destroys += rows_destroys[ ro ];
        bad_destroys += rows_bad_destroys[ r ];
        bad_destroys += rows_bad_destroys[ ro ];
        monotonocity += rows_monotonocity[ r ];
        monotonocity += rows_monotonocity[ ro ];
    }

    const auto& weights = configuration.get_weights( );
    const auto& empties_weight = weights[ 0 ];
    const auto& merges_weight = weights[ 1 ];
    const auto& best_sum_weight = weights[ 2 ];
    const auto& destroys_weight = weights[ 3 ];
    const auto& monotonocity_weight = weights[ 4 ];
    const auto& bad_merges_weight = weights[ 5 ];
    const auto& bad_sum_weight = weights[ 6 ];
    const auto& bad_destroys_weight = weights[ 7 ];

    double eval = EVAL_MIN + empties_weight * empties + merges_weight * merges
                  + best_sum_weight * best_sum + destroys_weight * destroys
                  + monotonocity_weight * monotonocity
                  + bad_merges_weight * bad_merges + bad_sum_weight * bad_sum
                  + bad_destroys_weight * bad_destroys;

    return eval;
}

int32_t
Board::get_branching_factor_at_depth( uint32_t depth ) const
{
    uint32_t l = m_phase + depth;
    uint32_t p = l % 10;
    switch ( p )
    {
    case 0:
    case 1:
    case 2:
    case 5:
    case 6:
    case 7:
        return 10;
    default:
        return 4;
    }
}

Board::MoveIterator::MoveIterator( const Board* board, bool end )
    : m_board( board )
{
    if ( m_board->m_phase == MAX_PHASE )
    {
        end = true;
    }

    switch ( m_board->m_phase % 10 )
    {
    case 0:
    case 1:
    case 2:
    case 5:
    case 6:
    case 7:
        m_slide = false;
        m_curr = end ? 16 : 0;
        m_length = 16;
        m_offset = rand( ) & 15;
        advance( );
        break;

    default:
        m_slide = true;
        m_curr = end ? 4 : 0;
        m_length = 4;
        m_offset = rand( ) & 3;
        advance( );
        break;
    }
}

const Move& Board::MoveIterator::operator*( ) const
{
    return m_move;
}

Board::MoveIterator& Board::MoveIterator::operator++( )
{
    if ( m_curr < m_length )
    {
        ++m_curr;
        advance( );
    }

    return *this;
}

Board::MoveIterator Board::MoveIterator::operator++( int /*dummy*/ )
{
    MoveIterator tmp = *this;
    ++*this;
    return tmp;
}

bool
Board::MoveIterator::operator==( const MoveIterator& move_iterator ) const
{
    return ( m_board == move_iterator.m_board )
           && ( m_curr == move_iterator.m_curr );
}

bool
Board::MoveIterator::operator!=( const MoveIterator& move_iterator ) const
{
    return ( m_board != move_iterator.m_board )
           || ( m_curr != move_iterator.m_curr );
}

void
Board::MoveIterator::advance( )
{
    m_move = INVALID_MOVE;
    if ( m_slide )
    {
        while ( m_curr < m_length && m_move == INVALID_MOVE )
        {
            uint32_t i = ( m_curr + m_offset ) & 3;
            switch ( i )
            {
            case 0:
                if ( m_board->can_be_slided_right( ) )
                    m_move = 1 | ( RIGHT << 1 );
                else
                    ++m_curr;
                break;

            case 1:
                if ( m_board->can_be_slided_left( ) )
                    m_move = 1 | ( LEFT << 1 );
                else
                    ++m_curr;
                break;

            case 2:
                if ( m_board->can_be_slided_up( ) )
                    m_move = 1 | ( UP << 1 );
                else
                    ++m_curr;
                break;

            case 3:
                if ( m_board->can_be_slided_down( ) )
                    m_move = 1 | ( DOWN << 1 );
                else
                    ++m_curr;
                break;
            }
        }
    }
    else
    {
        while ( m_curr < m_length && m_move == INVALID_MOVE )
        {
            uint32_t i = ( m_curr + m_offset ) & 15;
            uint32_t f = ( m_board->m_rows[ i >> 2 ] >> ( 5 * ( i & 3 ) ) )
                         & FIELD_MASK;
            if ( f == 0 )
            {
                m_move = i << 1;
                break;
            }
            else
            {
                ++m_curr;
            }
        }
    }
}

bool
operator==( const Board& lhs, const Board& rhs )
{
    if ( lhs.m_phase % 10 != rhs.m_phase % 10 )
        return false;

    for ( uint32_t i = 0; i < 4; ++i )
    {
        if ( lhs.m_rows[ i ] != rhs.m_rows[ i ] )
            return false;
    }

    return true;
}
