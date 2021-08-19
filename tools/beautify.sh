#! /bin/sh

clang-format -i -style=file player/*.h player/*.cc tests/*.cc benchmarks/*.cc
rm -f player/*.orig tests/*.orig benchmarks/*.orig
