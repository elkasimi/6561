find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(SOURCES
	../player/Board.h
	../player/Board.cc
	../player/Timer.h
	../player/Timer.cc
	../player/Configuration.h
	../player/Configuration.cc
	../player/RNG.h
	../player/RNG.cc
	BoardTest.cc
	main.cc
)

add_executable(PlayerTests
    ${SOURCES}
)

target_link_libraries(PlayerTests
    ${GTEST_LIBRARIES}
    pthread
)
