#pragma once

class IO
{
public:
    static Move read_move( const std::string& string );
    static void display_move( std::ostream& out, const Move& move );
    static void display_move_ln( std::ostream& out, const Move& move );
};