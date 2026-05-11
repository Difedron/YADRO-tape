#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "FileTape.h"
#include "TapeConfig.h"
#include "TapeSorter.h"

namespace {
namespace fs = std::filesystem;

void writeValuesToTape(FileTape& tape, const std::vector<int>& values) {
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        tape.write(values[i]);

        if (i + 1 < static_cast<int>(values.size())) {
            tape.moveRight();
        }
    }
}

std::vector<int> readValuesFromTape(FileTape& tape, int size) {
    std::vector<int> values;

    tape.rewind();

    for (int i = 0; i < size; ++i) {
        values.push_back(tape.read());

        if (i + 1 < size) {
            tape.moveRight();
        }
    }

    return values;
}

int countLines(const fs::path& fileName) {
    std::ifstream file(fileName);
    int lineCount = 0;
    std::string line;

    while (std::getline(file, line)) {
        lineCount++;
    }

    return lineCount;
}

TapeConfig makeFastConfig(int memoryLimitBytes) {
    TapeConfig config;
    config.readDelayMs = 0;
    config.writeDelayMs = 0;
    config.moveDelayMs = 0;
    config.rewindDelayMs = 0;
    config.memoryLimitBytes = memoryLimitBytes;

    return config;
}
}

TEST_CASE("TapeConfig loads values from file", "[TapeConfig]") {
    const fs::path configFileName = fs::path("..") / "tmp" / "test_config.txt";

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

    fs::remove(configFileName);
}

TEST_CASE("FileTape writes and reads values", "[FileTape]") {
    const fs::path tapeFileName = fs::path("..") / "tmp" / "test_file_tape.txt";

    fs::remove(tapeFileName);

    TapeConfig config = makeFastConfig(1024);

    FileTape tape(tapeFileName, config, 3);

    writeValuesToTape(tape, {10, -5, 7});

    REQUIRE(readValuesFromTape(tape, 3) == std::vector<int>{10, -5, 7});
    REQUIRE(countLines(tapeFileName) == 3);
    REQUIRE(fs::file_size(tapeFileName) == 39);

    tape.moveRight();
    REQUIRE(tape.isEnd());

    fs::remove(tapeFileName);
}

TEST_CASE("FileTape reads numbers separated by arbitrary whitespace", "[FileTape]") {
    const fs::path tapeFileName = fs::path("..") / "tmp" / "test_whitespace_tape.txt";

    std::ofstream inputFile(tapeFileName);
    inputFile << "1 8\t7\n12 3 4\n88 88 -124 99 1000 0 17 16";
    inputFile.close();

    FileTape tape(tapeFileName, makeFastConfig(1024), 14);

    REQUIRE(readValuesFromTape(tape, 14) == std::vector<int>{1, 8, 7, 12, 3, 4, 88, 88, -124, 99, 1000, 0, 17, 16});
    REQUIRE(countLines(tapeFileName) == 14);
    REQUIRE(fs::file_size(tapeFileName) == 182);

    fs::remove(tapeFileName);
}

TEST_CASE("TapeSorter creates temporary directory when it is missing", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_created_tapes_dir";
    fs::remove_all(tmpDirectory);

    TapeSorter sorter(makeFastConfig(16), tmpDirectory);

    REQUIRE(fs::is_directory(tmpDirectory));

    fs::remove_all(tmpDirectory);
}

TEST_CASE("TapeSorter removes stale temporary tapes on creation", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_cleanup_tapes_dir";
    fs::remove_all(tmpDirectory);
    fs::create_directories(tmpDirectory);

    std::ofstream(tmpDirectory / "run_0.txt") << "old run";
    std::ofstream(tmpDirectory / "merge_0.txt") << "old merge";
    std::ofstream(tmpDirectory / "input_tape.txt") << "old input tape";
    std::ofstream(tmpDirectory / ".gitkeep") << "";
    std::ofstream(tmpDirectory / "notes.txt") << "not a tape";

    TapeSorter sorter(makeFastConfig(16), tmpDirectory);

    REQUIRE_FALSE(fs::exists(tmpDirectory / "run_0.txt"));
    REQUIRE_FALSE(fs::exists(tmpDirectory / "merge_0.txt"));
    REQUIRE_FALSE(fs::exists(tmpDirectory / "input_tape.txt"));
    REQUIRE(fs::exists(tmpDirectory / ".gitkeep"));
    REQUIRE(fs::exists(tmpDirectory / "notes.txt"));

    fs::remove_all(tmpDirectory);
}

TEST_CASE("TapeSorter creates sorted runs as temporary tapes", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_runs_as_tapes";
    fs::remove_all(tmpDirectory);

    TapeConfig config = makeFastConfig(static_cast<int>(sizeof(int)) * 2);
    TapeSorter sorter(config, tmpDirectory);

    const fs::path inputTapeFileName = tmpDirectory / "input_tape.txt";
    FileTape inputTape(inputTapeFileName, config, 5);
    writeValuesToTape(inputTape, {4, 1, 3, -2, 0});

    std::vector<TapeRun> runs = sorter.createSortedRuns(inputTape);

    REQUIRE(runs.size() == 3);
    REQUIRE(runs[0].size == 2);
    REQUIRE(runs[1].size == 2);
    REQUIRE(runs[2].size == 1);

    for (const TapeRun& run : runs) {
        REQUIRE(countLines(run.fileName) == run.size);
        REQUIRE(fs::file_size(run.fileName) == static_cast<std::uintmax_t>(run.size * 13));

        FileTape runTape(run.fileName, config, run.size);
        std::vector<int> values = readValuesFromTape(runTape, run.size);

        REQUIRE(std::is_sorted(values.begin(), values.end()));
    }

    fs::remove_all(tmpDirectory);
}

TEST_CASE("TapeSorter sorts values using temporary tapes", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_sort_tapes";
    fs::remove_all(tmpDirectory);

    const fs::path inputTapeFileName = tmpDirectory / "input_tape.txt";
    const fs::path outputTapeFileName = tmpDirectory / "output_tape.txt";

    TapeConfig config = makeFastConfig(12);
    TapeSorter sorter(config, tmpDirectory);

    std::vector<int> inputValues = {15, -3, 8, 8, 0, 42, -10, 7, 1, 15};
    std::vector<int> expectedValues = {-10, -3, 0, 1, 7, 8, 8, 15, 15, 42};

    FileTape inputTape(inputTapeFileName, config, static_cast<int>(inputValues.size()));
    FileTape outputTape(outputTapeFileName, config, static_cast<int>(inputValues.size()));

    writeValuesToTape(inputTape, inputValues);

    sorter.sort(inputTape, outputTape);

    REQUIRE(readValuesFromTape(outputTape, static_cast<int>(expectedValues.size())) == expectedValues);

    fs::remove_all(tmpDirectory);
}

TEST_CASE("TapeSorter sorts input FileTape without preparing temporary input tape", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_direct_input_sort_tapes";
    fs::remove_all(tmpDirectory);
    fs::create_directories(tmpDirectory);

    const fs::path inputFileName = tmpDirectory / "input.txt";
    const fs::path outputTapeFileName = tmpDirectory / "output_tape.txt";

    TapeConfig config = makeFastConfig(static_cast<int>(sizeof(int)) * 2);
    std::vector<int> inputValues = {6, 1, -4, 6, 0, 3};

    std::ofstream inputFile(inputFileName);
    inputFile << "6 1\t-4\n6 0 3";
    inputFile.close();

    FileTape inputTape(inputFileName, config, static_cast<int>(inputValues.size()));
    FileTape outputTape(outputTapeFileName, config, static_cast<int>(inputValues.size()));
    TapeSorter sorter(config, tmpDirectory);

    sorter.sort(inputTape, outputTape);

    REQUIRE(readValuesFromTape(outputTape, static_cast<int>(inputValues.size())) == std::vector<int>{-4, 0, 1, 3, 6, 6});

    REQUIRE_FALSE(fs::exists(tmpDirectory / "input_tape.txt"));

    fs::remove_all(tmpDirectory);
}

TEST_CASE("TapeSorter handles many runs with a two-integer memory limit", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_low_memory_sort_tapes";
    fs::remove_all(tmpDirectory);

    TapeConfig config = makeFastConfig(static_cast<int>(sizeof(int)) * 2);
    TapeSorter sorter(config, tmpDirectory);

    std::vector<int> inputValues = {9, 3, 7, 1, 8, 2, 6, 4, 5, 0, -1, 12, 11, 10, -3, -2};
    std::vector<int> expectedValues = inputValues;
    std::sort(expectedValues.begin(), expectedValues.end());

    const fs::path inputTapeFileName = tmpDirectory / "input_tape.txt";
    const fs::path outputTapeFileName = tmpDirectory / "output_tape.txt";

    FileTape inputTape(inputTapeFileName, config, static_cast<int>(inputValues.size()));
    FileTape outputTape(outputTapeFileName, config, static_cast<int>(inputValues.size()));

    writeValuesToTape(inputTape, inputValues);

    sorter.sort(inputTape, outputTape);

    REQUIRE(readValuesFromTape(outputTape, static_cast<int>(expectedValues.size())) == expectedValues);

    fs::remove_all(tmpDirectory);
}

TEST_CASE("TapeSorter rejects merge sorting when memory fits only one integer", "[TapeSorter]") {
    const fs::path tmpDirectory = fs::path("..") / "tmp" / "test_too_low_memory_sort";
    fs::remove_all(tmpDirectory);

    TapeConfig config = makeFastConfig(static_cast<int>(sizeof(int)));
    TapeSorter sorter(config, tmpDirectory);

    std::vector<int> inputValues = {3, 1, 2};

    const fs::path inputTapeFileName = tmpDirectory / "input_tape.txt";
    const fs::path outputTapeFileName = tmpDirectory / "output_tape.txt";

    FileTape inputTape(inputTapeFileName, config, static_cast<int>(inputValues.size()));
    FileTape outputTape(outputTapeFileName, config, static_cast<int>(inputValues.size()));

    writeValuesToTape(inputTape, inputValues);

    REQUIRE_THROWS(sorter.sort(inputTape, outputTape));

    fs::remove_all(tmpDirectory);
}
