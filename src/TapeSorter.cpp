#include "TapeSorter.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

TapeSorter::TapeSorter(const TapeConfig& config, const std::filesystem::path& tmpDirectory)
        : config(config), tmpDirectory(tmpDirectory) {
    ensureTemporaryDirectoryExists();
    cleanupTemporaryTapes();
}

int TapeSorter::getMaxNumbersInMemory() const {
    if (config.memoryLimitBytes < static_cast<int>(sizeof(int))) {
        throw std::runtime_error("Memory limit is too small: at least one integer must fit in memory");
    }

    int maxNumbers = config.memoryLimitBytes / static_cast<int>(sizeof(int));

    if (maxNumbers < 1) {
        maxNumbers = 1;
    }

    return maxNumbers;
}

void TapeSorter::ensureTemporaryDirectoryExists() const {
    namespace fs = std::filesystem;

    if (tmpDirectory.empty()) {
        throw std::invalid_argument("Temporary tapes directory path is empty");
    }

    std::error_code error;

    if (fs::exists(tmpDirectory, error)) {
        if (error) {
            throw std::runtime_error("Cannot check temporary tapes directory: " + error.message());
        }

        if (!fs::is_directory(tmpDirectory, error)) {
            throw std::runtime_error("Temporary tapes path is not a directory: " + tmpDirectory.string());
        }

        if (error) {
            throw std::runtime_error("Cannot check temporary tapes directory type: " + error.message());
        }

        return;
    }

    if (!fs::create_directories(tmpDirectory, error) || error) {
        throw std::runtime_error("Cannot create temporary tapes directory: " + tmpDirectory.string());
    }
}

void TapeSorter::cleanupTemporaryTapes() const {
    namespace fs = std::filesystem;

    std::error_code error;

    for (const fs::directory_entry& entry : fs::directory_iterator(tmpDirectory, error)) {
        if (error) {
            throw std::runtime_error("Cannot iterate temporary tapes directory: " + error.message());
        }

        if (!entry.is_regular_file(error)) {
            if (error) {
                throw std::runtime_error("Cannot check temporary tape file: " + error.message());
            }

            continue;
        }

        if (isTemporaryTapeFile(entry.path().filename())) {
            fs::remove(entry.path(), error);

            if (error) {
                throw std::runtime_error("Cannot remove temporary tape: " + entry.path().string());
            }
        }
    }
}

bool TapeSorter::isTemporaryTapeFile(const std::filesystem::path& fileName) const {
    std::string name = fileName.string();

    if (name == "input_tape.txt") {
        return true;
    }

    return fileName.extension() == ".txt" &&
           (name.rfind("run_", 0) == 0 || name.rfind("merge_", 0) == 0);
}

std::filesystem::path TapeSorter::makeTemporaryTapeFileName(const std::string& prefix, int index) const {
    return tmpDirectory / (prefix + "_" + std::to_string(index) + ".txt");
}

TapeRun TapeSorter::createTapeRunFromBlock(const std::vector<int>& block, int runIndex) const {
    std::filesystem::path runFileName = makeTemporaryTapeFileName("run", runIndex);

    std::filesystem::remove(runFileName);

    FileTape runTape(runFileName, config, static_cast<int>(block.size()));

    for (int i = 0; i < static_cast<int>(block.size()); ++i) {
        runTape.write(block[i]);

        if (i + 1 < static_cast<int>(block.size())) {
            runTape.moveRight();
        }
    }

    return {runFileName, static_cast<int>(block.size())};
}

std::vector<TapeRun> TapeSorter::createSortedRuns(ITape& inputTape) {
    std::vector<TapeRun> runs;

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

        runs.push_back(createTapeRunFromBlock(block, runIndex));
        runIndex++;
    }

    return runs;
}

TapeRun TapeSorter::mergeTwoRuns(const TapeRun& leftRun, const TapeRun& rightRun, int runIndex) const {
    std::filesystem::path mergedFileName = makeTemporaryTapeFileName("merge", runIndex);
    int mergedSize = leftRun.size + rightRun.size;

    std::filesystem::remove(mergedFileName);

    FileTape leftTape(leftRun.fileName, config, leftRun.size);
    FileTape rightTape(rightRun.fileName, config, rightRun.size);
    FileTape mergedTape(mergedFileName, config, mergedSize);

    bool hasLeftValue = leftRun.size > 0;
    bool hasRightValue = rightRun.size > 0;

    int leftValue = 0;
    int rightValue = 0;

    if (hasLeftValue) {
        leftValue = leftTape.read();
    }

    if (hasRightValue) {
        rightValue = rightTape.read();
    }

    int writtenCount = 0;

    while (hasLeftValue || hasRightValue) {
        int valueToWrite;

        if (!hasRightValue || (hasLeftValue && leftValue <= rightValue)) {
            valueToWrite = leftValue;

            leftTape.moveRight();

            if (leftTape.isEnd()) {
                hasLeftValue = false;
            } else {
                leftValue = leftTape.read();
            }
        } else {
            valueToWrite = rightValue;

            rightTape.moveRight();

            if (rightTape.isEnd()) {
                hasRightValue = false;
            } else {
                rightValue = rightTape.read();
            }
        }

        mergedTape.write(valueToWrite);
        writtenCount++;

        if (writtenCount < mergedSize) {
            mergedTape.moveRight();
        }
    }

    return {mergedFileName, mergedSize};
}

void TapeSorter::copyRunToOutputTape(const TapeRun& run, FileTape& outputTape) const {
    FileTape inputTape(run.fileName, config, run.size);

    inputTape.rewind();
    outputTape.rewind();

    for (int i = 0; i < run.size; ++i) {
        outputTape.write(inputTape.read());

        if (i + 1 < run.size) {
            inputTape.moveRight();
            outputTape.moveRight();
        }
    }
}

void TapeSorter::mergeRuns(const std::vector<TapeRun>& runFiles, FileTape& outputTape) {
    if (runFiles.empty()) {
        return;
    }

    if (runFiles.size() > 1 && getMaxNumbersInMemory() < 2) {
        throw std::runtime_error("Memory limit is too small for merge sorting: at least two integers must fit in memory");
    }

    std::vector<TapeRun> currentRuns = runFiles;
    int mergeIndex = 0;

    while (currentRuns.size() > 1) {
        std::vector<TapeRun> nextRuns;

        for (int i = 0; i < static_cast<int>(currentRuns.size()); i += 2) {
            if (i + 1 == static_cast<int>(currentRuns.size())) {
                nextRuns.push_back(currentRuns[i]);
            } else {
                nextRuns.push_back(mergeTwoRuns(currentRuns[i], currentRuns[i + 1], mergeIndex));
                mergeIndex++;
            }
        }

        currentRuns = nextRuns;
    }

    copyRunToOutputTape(currentRuns[0], outputTape);
}

void TapeSorter::sort(ITape& inputTape, FileTape& outputTape) {
    std::vector<TapeRun> runFiles = createSortedRuns(inputTape);
    mergeRuns(runFiles, outputTape);
}
