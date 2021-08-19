#include "Common.h"

#include "Configuration.h"
#include "StrategyFactory.h"

std::unique_ptr< Strategy >
StrategyFactory::create( Strategy::Type strategy_type )
{
    switch ( strategy_type )
    {
    case Strategy::Type::RANDOM:
        return std::make_unique< RandomStrategy >( );

    case Strategy::Type::HUMAN:
        return std::make_unique< HumanStrategy >( );

    case Strategy::Type::EXPECT_MAX:
        return std::make_unique< ExpectMaxStrategy >(
            Configuration::get_best_configuration( ) );

    case Strategy::Type::EXPECT_MAX_LIGHTWEIGHT:
        return std::make_unique< ExpectMaxStrategy >(
            Configuration::get_lightweight_configuration( ) );
    }

    return std::unique_ptr< Strategy >( );
}
