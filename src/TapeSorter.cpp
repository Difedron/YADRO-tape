#include "TapeSorter.h"

#include <algorithm>
#include <fstream>
#include <queue>

TapeSorter::TapeSorter(const TapeConfig& config, const std::string& tmpDirectory)
        : config(config), tmpDirectory(tmpDirectory) {
}

int TapeSorter::getMaxNumbersInMemory() const {
    int maxNumbers = config.memoryLimitBytes / static_cast<int>(sizeof(int));

    if (maxNumbers < 1) {
        maxNumbers = 1;
    }

    return maxNumbers;
}

std::vector<std::string> TapeSorter::createSortedRuns(FileTape& inputTape) {
    std::vector<std::string> runFiles;

    int maxNumbers = getMaxNumbersInMemory();
    int runIndex = 0;

    inputTape.rewind();

    while (!inputTape.isEnd()) {
        std::vector<int> block;

        while (!inputTape.isEnd() && static_cast<int>(block.size()) < maxNumbers) {
            block.push_back(inputTape.read());
            inputTape.moveRight();
        }

        std::sort(block.begin(), block.end());

        std::string runFileName = tmpDirectory + "/run_" + std::to_string(runIndex) + ".txt";

        std::ofstream runFile(runFileName);

        for (int value : block) {
            runFile << value << " ";
        }

        runFile.close();

        runFiles.push_back(runFileName);
        runIndex++;
    }

    return runFiles;
}

void TapeSorter::mergeRuns(const std::vector<std::string>& runFiles, FileTape& outputTape) {
    struct HeapElement {
        int value;
        int fileIndex;

        bool operator>(const HeapElement& other) const {
            return value > other.value;
        }
    };

    std::vector<std::ifstream> files;

    for (const std::string& fileName : runFiles) {
        files.emplace_back(fileName);
    }

    std::priority_queue<
            HeapElement,
            std::vector<HeapElement>,
            std::greater<>
    > minHeap;

    for (int i = 0; i < static_cast<int>(files.size()); ++i) {
        int value;

        if (files[i] >> value) {
            minHeap.push({value, i});
        }
    }

    outputTape.rewind();

    while (!minHeap.empty()) {
        HeapElement current = minHeap.top();
        minHeap.pop();

        outputTape.write(current.value);
        outputTape.moveRight();

        int nextValue;

        if (files[current.fileIndex] >> nextValue) {
            minHeap.push({nextValue, current.fileIndex});
        }
    }
}

void TapeSorter::sort(FileTape& inputTape, FileTape& outputTape) {
    std::vector<std::string> runFiles = createSortedRuns(inputTape);
    mergeRuns(runFiles, outputTape);
}