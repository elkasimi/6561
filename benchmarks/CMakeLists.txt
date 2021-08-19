set(SOURCES
	../player/Board.h
	../player/Board.cc
	../player/Timer.h
	../player/Timer.cc
	../player/Configuration.h
	../player/Configuration.cc
	../player/RNG.h
	../player/RNG.cc
	BoardBenchmark.cc
	main.cc
)

add_executable(PlayerBenchmarks
    ${SOURCES}
)

target_link_libraries(PlayerBenchmarks
    benchmark
)
