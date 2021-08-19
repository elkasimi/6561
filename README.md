# 6561
my entry in the CodeCup 2016 competition that wins the first place

Please have a look at:
http://www.codecup.nl
for more information

## to build the player:

$cmake .

$make

## Ranking
http://www.codecup.nl/competition.php?comp=183

## Game rules

http://www.codecup.nl/rules_6561.php

## Install gtest on ubuntu:
sudo apt-get install libgtest-dev

cd /usr/src/gtest

sudo cmake CMakeLists.txt

sudo make
 
# copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp *.a /usr/lib

## Install google benchmark on ubuntu:
git clone https://github.com/google/benchmark.git

cd benchmark/

cmake .

make

sudo make install

