#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <fstream>
#include <vector>
#include <string>

#include "TapeConfig.h"
#include "FileTape.h"
#include "TapeSorter.h"

TEST_CASE("TapeConfig loads values from file", "[TapeConfig]") {
std::string configFileName = "../tmp/test_config.txt";

std::ofstream configFile(configFileName);
configFile << "read_delay_ms=1\n";
configFile << "write_delay_ms=2\n";
configFile << "move_delay_ms=3\n";
configFile << "rewind_delay_ms=4\n";
configFile << "memory_limit_bytes=16\n";
configFile.close();

TapeConfig config = TapeConfig::loadFromFile(configFileName);

REQUIRE(config.readDelayMs == 1);
REQUIRE(config.writeDelayMs == 2);
REQUIRE(config.moveDelayMs == 3);
REQUIRE(config.rewindDelayMs == 4);
REQUIRE(config.memoryLimitBytes == 16);

std::remove(configFileName.c_str());
}

TEST_CASE("FileTape writes and reads values", "[FileTape]") {
std::string tapeFileName = "../tmp/test_file_tape.txt";

std::remove(tapeFileName.c_str());

TapeConfig config;
config.readDelayMs = 0;
config.writeDelayMs = 0;
config.moveDelayMs = 0;
config.rewindDelayMs = 0;

FileTape tape(tapeFileName, config, 3);

tape.write(10);
tape.moveRight();

tape.write(-5);
tape.moveRight();

tape.write(7);

tape.rewind();

REQUIRE(tape.read() == 10);
tape.moveRight();

REQUIRE(tape.read() == -5);
tape.moveRight();

REQUIRE(tape.read() == 7);
tape.moveRight();

REQUIRE(tape.isEnd());

std::remove(tapeFileName.c_str());
}

TEST_CASE("TapeSorter sorts values using temporary tapes", "[TapeSorter]") {
std::string inputTapeFileName = "../tmp/test_sort_input_tape.txt";
std::string outputTapeFileName = "../tmp/test_sort_output_tape.txt";

std::remove(inputTapeFileName.c_str());
std::remove(outputTapeFileName.c_str());

TapeConfig config;
config.readDelayMs = 0;
config.writeDelayMs = 0;
config.moveDelayMs = 0;
config.rewindDelayMs = 0;
config.memoryLimitBytes = 12;

std::vector<int> inputValues = {15, -3, 8, 8, 0, 42, -10, 7, 1, 15};
std::vector<int> expectedValues = {-10, -3, 0, 1, 7, 8, 8, 15, 15, 42};

FileTape inputTape(inputTapeFileName, config, static_cast<int>(inputValues.size()));
FileTape outputTape(outputTapeFileName, config, static_cast<int>(inputValues.size()));

for (int i = 0; i < static_cast<int>(inputValues.size()); ++i) {
inputTape.write(inputValues[i]);

if (i + 1 < static_cast<int>(inputValues.size())) {
inputTape.moveRight();
}
}

TapeSorter sorter(config, "../tmp");
sorter.sort(inputTape, outputTape);

outputTape.rewind();

for (int i = 0; i < static_cast<int>(expectedValues.size()); ++i) {
REQUIRE(outputTape.read() == expectedValues[i]);

if (i + 1 < static_cast<int>(expectedValues.size())) {
outputTape.moveRight();
}
}

std::remove(inputTapeFileName.c_str());
std::remove(outputTapeFileName.c_str());
}