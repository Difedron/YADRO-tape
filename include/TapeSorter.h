//
// Created by Diana Kholukhina on 10.05.2026.
//

#ifndef YADRO_TAPE_TAPESORTER_H
#define YADRO_TAPE_TAPESORTER_H

#include "FileTape.h"
#include "ITape.h"
#include "TapeConfig.h"

#include <filesystem>
#include <string>
#include <vector>

struct TapeRun {
    std::filesystem::path fileName;
    int size;
};

class TapeSorter {
private:
    TapeConfig config;
    std::filesystem::path tmpDirectory;

    int getMaxNumbersInMemory() const;
    void ensureTemporaryDirectoryExists() const;
    void cleanupTemporaryTapes() const;
    bool isTemporaryTapeFile(const std::filesystem::path& fileName) const;
    std::filesystem::path makeTemporaryTapeFileName(const std::string& prefix, int index) const;
    TapeRun createTapeRunFromBlock(const std::vector<int>& block, int runIndex) const;
    TapeRun mergeTwoRuns(const TapeRun& leftRun, const TapeRun& rightRun, int runIndex) const;
    void copyRunToOutputTape(const TapeRun& run, FileTape& outputTape) const;

public:
    TapeSorter(const TapeConfig& config, const std::filesystem::path& tmpDirectory);

    std::vector<TapeRun> createSortedRuns(ITape& inputTape);

    void mergeRuns(const std::vector<TapeRun>& runFiles, FileTape& outputTape);

    void sort(ITape& inputTape, FileTape& outputTape);
};

#endif // YADRO_TAPE_TAPESORTER_H
