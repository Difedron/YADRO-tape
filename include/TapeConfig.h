//
// Created by Diana Kholukhina on 10.05.2026.
//

#ifndef YADRO_TAPE_TAPECONFIG_H
#define YADRO_TAPE_TAPECONFIG_H

#include <string>

struct TapeConfig {
    int readDelayMs = 0;
    int writeDelayMs = 0;
    int moveDelayMs = 0;
    int rewindDelayMs = 0;
    int memoryLimitBytes = 1024;

    static TapeConfig loadFromFile(const std::string& fileName);
};

#endif // YADRO_TAPE_TAPECONFIG_H
