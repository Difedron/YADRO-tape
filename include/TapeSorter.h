//
// Created by Diana Kholukhina on 10.05.2026.
//

#ifndef YADRO_TAPE_TAPESORTER_H
#define YADRO_TAPE_TAPESORTER_H

#include "FileTape.h"
#include "TapeConfig.h"

#include <string>
#include <vector>

class TapeSorter {
private:
    TapeConfig config;
    std::string tmpDirectory;

    int getMaxNumbersInMemory() const;

public:
    TapeSorter(const TapeConfig& config, const std::string& tmpDirectory);

    std::vector<std::string> createSortedRuns(FileTape& inputTape);

    void mergeRuns(const std::vector<std::string>& runFiles, FileTape& outputTape);

    void sort(FileTape& inputTape, FileTape& outputTape);
};

#endif // YADRO_TAPE_TAPESORTER_H