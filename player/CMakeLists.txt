set(HEADERS
    IO.h
    RNG.h
    Board.h
    TTable.h
    Timer.h
    Configuration.h
    Strategy.h
    StrategyFactory.h
)

set(SOURCES
    IO.cc
    RNG.cc
    Board.cc
    TTable.cc
    Timer.cc
    Configuration.cc
    RandomStrategy.cc
    HumanStrategy.cc
    ExpectMaxStrategy.cc
    StrategyFactory.cc
    Competition.cc
    main.cc
)

add_executable(player
    ${HEADERS}
    ${SOURCES}
)

