#pragma once

class Configuration;
class Board
{
public:
    enum Direction
    {
        RIGHT = 0,
        LEFT = 1,
        UP = 2,
        DOWN = 3
    };

public:
    class MoveIterator : public std::iterator< std::forward_iterator_tag, Move >
    {
    public:
        MoveIterator( const Board* board, bool end = false );

        const Move& operator*( ) const;
        MoveIterator& operator++( );
        MoveIterator operator++( int dummy );

        bool operator==( const MoveIterator& move_iterator ) const;
        bool operator!=( const MoveIterator& move_iterator ) const;

    private:
        void advance( );

        const Board* m_board;
        uint32_t m_curr;
        uint32_t m_length;
        Move m_move;
        bool m_slide;
        uint32_t m_offset;
    };

public:
    Board( );
    virtual ~Board( )
    {
    }

    bool end_game( ) const;

    void do_move( const Move& move );

    Move get_random_move( ) const;

    uint32_t get_score( ) const;

    uint32_t get_best_score( ) const;

    int32_t get_phase( ) const;

    uint32_t get_hash( ) const;

    uint32_t get_lock( ) const;

    double evaluate( const Configuration& configuration ) const;

    int32_t get_branching_factor_at_depth( uint32_t depth ) const;

    static void init_data( );

    MoveIterator begin_move( ) const;

    MoveIterator end_move( ) const;

    friend bool operator==( const Board& lhs, const Board& rhs );

private:
    uint32_t m_score;
    uint32_t m_best_score;
    uint32_t m_phase;
    uint32_t m_rows[ 4 ];

    void update( );

    bool can_be_slided_right( ) const;
    bool can_be_slided_left( ) const;
    bool can_be_slided_up( ) const;
    bool can_be_slided_down( ) const;
};

bool operator==( const Board& lhs, const Board& rhs );
