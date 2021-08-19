#pragma once

#include "Strategy.h"

class StrategyFactory
{
public:
    static std::unique_ptr< Strategy > create( Strategy::Type strategy_type );
};
