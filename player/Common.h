#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <random>
#include <sstream>
#include <string>

/*
    move is encoded like that:
        if sliding move
            move = 1 | (direction << 1);
            where direction = 0..3 for R, L, U and D
        else
            move = place << 1
            where place = 0..15

        board is like this:
            list of 16 fields
                every field = 2 bits for color + 4 bits for exponent of the
                power of 3

*/
using Move = uint32_t;

#define INVALID_MOVE 0xffffffffu
#define FIELD_MASK 0x1fu
#define MAX_RELEVANCE 10000
#define TTABLE_SIZE 524288
#define MAX_ROW 1048576
#define MAX_PHASE 1000

#define FIELD_VALUE( f ) ( ( f ) >> 2 )
#define FIELD_COLOR( f ) ( (f)&3 )

#pragma GCC optimize( "unroll-loops" )
