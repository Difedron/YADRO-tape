#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <stdexcept>

#include "TapeConfig.h"
#include "FileTape.h"
#include "TapeSorter.h"

namespace {
const std::filesystem::path DEFAULT_CONFIG_FILE_NAME = std::filesystem::path("..") / "config.txt";
const std::filesystem::path DEFAULT_TMP_DIRECTORY = std::filesystem::path("..") / "tmp";

void printUsage() {
    std::cout << "Usage: ./YADRO_tape <input_file> <output_file> [config_file] [tmp_directory]" << std::endl;
}

void ensureDirectoryExists(const std::filesystem::path& directoryName) {
    namespace fs = std::filesystem;

    if (directoryName.empty()) {
        throw std::invalid_argument("Temporary tapes directory path is empty");
    }

    std::error_code error;

    if (fs::exists(directoryName, error)) {
        if (error) {
            throw std::runtime_error("Cannot check temporary tapes directory: " + error.message());
        }

        if (!fs::is_directory(directoryName, error)) {
            throw std::runtime_error("Temporary tapes path is not a directory: " + directoryName.string());
        }

        if (error) {
            throw std::runtime_error("Cannot check temporary tapes directory type: " + error.message());
        }

        return;
    }

    if (!fs::create_directories(directoryName, error) || error) {
        throw std::runtime_error("Cannot create temporary tapes directory: " + directoryName.string());
    }
}

int countNumbersInFile(const std::filesystem::path& fileName) {
    std::ifstream file(fileName);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file");
    }

    int count = 0;
    int value;

    while (file >> value) {
        count++;
    }

    if (!file.eof()) {
        throw std::runtime_error("Invalid integer value in input file");
    }

    return count;
}
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 5) {
        printUsage();
        return 1;
    }

    std::filesystem::path inputFileName = argv[1];
    std::filesystem::path outputFileName = argv[2];
    std::filesystem::path configFileName = argc >= 4 ? std::filesystem::path(argv[3]) : DEFAULT_CONFIG_FILE_NAME;
    std::filesystem::path tmpDirectory = argc >= 5 ? std::filesystem::path(argv[4]) : DEFAULT_TMP_DIRECTORY;

    try {
        ensureDirectoryExists(tmpDirectory);

        TapeConfig config = TapeConfig::loadFromFile(configFileName);

        int tapeSize = countNumbersInFile(inputFileName);

        if (tapeSize == 0) {
            std::cout << "Input file is empty" << std::endl;
            return 1;
        }

        FileTape inputTape(inputFileName, config, tapeSize);
        TapeSorter sorter(config, tmpDirectory);

        std::filesystem::remove(outputFileName);
        FileTape outputTape(outputFileName, config, tapeSize);

        sorter.sort(inputTape, outputTape);

        std::cout << "Sorting completed successfully" << std::endl;
        std::cout << "Input file: " << inputFileName.string() << std::endl;
        std::cout << "Output file: " << outputFileName.string() << std::endl;
        std::cout << "Config file: " << configFileName.string() << std::endl;
        std::cout << "Temporary tapes directory: " << tmpDirectory.string() << std::endl;
        std::cout << "Numbers count: " << tapeSize << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
