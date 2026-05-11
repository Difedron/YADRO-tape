//
// Created by Diana Kholukhina on 10.05.2026.
//

#ifndef YADRO_TAPE_FILETAPE_H
#define YADRO_TAPE_FILETAPE_H

#include "ITape.h"
#include "TapeConfig.h"

#include <filesystem>
#include <fstream>

class FileTape : public ITape {
private:
    std::fstream file;
    std::filesystem::path fileName;
    TapeConfig config;

    int currentPosition;
    int tapeSize;

    void delay(int milliseconds) const;
    std::streampos getPositionInFile(int position) const;
    std::filesystem::path getTemporaryFileName() const;
    void normalizeExistingFile() const;

public:
    FileTape(const std::filesystem::path& fileName, const TapeConfig& config, int tapeSize);

    int read() override;
    void write(int value) override;

    void moveLeft() override;
    void moveRight() override;
    void rewind() override;

    bool isEnd() const override;

    int size() const;
};

#endif // YADRO_TAPE_FILETAPE_H
