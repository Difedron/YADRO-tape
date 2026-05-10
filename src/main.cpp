#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <stdexcept>

#include "TapeConfig.h"
#include "FileTape.h"
#include "TapeSorter.h"

int countNumbersInFile(const std::string& fileName) {
    std::ifstream file(fileName);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file");
    }

    int count = 0;
    int value;

    while (file >> value) {
        count++;
    }

    return count;
}

void prepareInputTapeFile(const std::string& inputFileName,
                          const std::string& tapeFileName) {
    std::ifstream inputFile(inputFileName);

    if (!inputFile.is_open()) {
        throw std::runtime_error("Cannot open input file");
    }

    std::ofstream tapeFile(tapeFileName);

    if (!tapeFile.is_open()) {
        throw std::runtime_error("Cannot create temporary input tape file");
    }

    int value;

    while (inputFile >> value) {
        tapeFile << std::setw(12) << value;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./YADRO_tape <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];

    try {
        TapeConfig config = TapeConfig::loadFromFile("../config.txt");

        int tapeSize = countNumbersInFile(inputFileName);

        if (tapeSize == 0) {
            std::cout << "Input file is empty" << std::endl;
            return 1;
        }

        std::string preparedInputTapeFileName = "../tmp/input_tape.txt";

        prepareInputTapeFile(inputFileName, preparedInputTapeFileName);

        FileTape inputTape(preparedInputTapeFileName, config, tapeSize);
        FileTape outputTape(outputFileName, config, tapeSize);

        TapeSorter sorter(config, "../tmp");

        sorter.sort(inputTape, outputTape);

        std::cout << "Sorting completed successfully" << std::endl;
        std::cout << "Input file: " << inputFileName << std::endl;
        std::cout << "Output file: " << outputFileName << std::endl;
        std::cout << "Numbers count: " << tapeSize << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}