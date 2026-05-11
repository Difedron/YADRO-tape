#include "FileTape.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <stdexcept>
#include <thread>

namespace {
const int CELL_WIDTH = 12;
const int RECORD_SIZE = CELL_WIDTH + 1;
}

FileTape::FileTape(const std::filesystem::path& fileName, const TapeConfig& config, int tapeSize)
        : fileName(fileName), config(config), currentPosition(0), tapeSize(tapeSize) {
    file.open(fileName, std::ios::in | std::ios::out);

    if (!file.is_open()) {
        file.clear();

        std::ofstream createFile(fileName);

        if (!createFile.is_open()) {
            throw std::runtime_error("Cannot create tape file: " + fileName.string());
        }

        for (int i = 0; i < tapeSize; ++i) {
            createFile << std::setw(CELL_WIDTH) << 0 << '\n';
        }

        createFile.close();
    } else {
        file.close();
        normalizeExistingFile();
    }

    file.open(fileName, std::ios::in | std::ios::out);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open tape file: " + fileName.string());
    }
}

void FileTape::delay(int milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::streampos FileTape::getPositionInFile(int position) const {
    return static_cast<std::streampos>(position * RECORD_SIZE);
}

std::filesystem::path FileTape::getTemporaryFileName() const {
    std::filesystem::path temporaryFileName = fileName;
    temporaryFileName += ".normalize.tmp";

    return temporaryFileName;
}

void FileTape::normalizeExistingFile() const {
    std::error_code error;
    std::uintmax_t expectedSize = static_cast<std::uintmax_t>(tapeSize * RECORD_SIZE);

    if (std::filesystem::file_size(fileName, error) == expectedSize && !error) {
        return;
    }

    std::ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        throw std::runtime_error("Cannot open tape file for normalization: " + fileName.string());
    }

    std::filesystem::path temporaryFileName = getTemporaryFileName();
    std::ofstream temporaryFile(temporaryFileName, std::ios::trunc);

    if (!temporaryFile.is_open()) {
        throw std::runtime_error("Cannot create temporary tape file: " + temporaryFileName.string());
    }

    for (int i = 0; i < tapeSize; ++i) {
        int value;

        if (!(inputFile >> value)) {
            throw std::runtime_error("Cannot normalize tape file: not enough integer values in " + fileName.string());
        }

        temporaryFile << std::setw(CELL_WIDTH) << value << '\n';
    }

    inputFile.close();
    temporaryFile.close();

    std::filesystem::rename(temporaryFileName, fileName, error);

    if (error) {
        throw std::runtime_error("Cannot replace normalized tape file: " + error.message());
    }
}

int FileTape::read() {
    if (isEnd()) {
        throw std::out_of_range("Cannot read: tape head is out of tape range");
    }

    delay(config.readDelayMs);

    file.clear();
    file.seekg(getPositionInFile(currentPosition));

    int value;

    if (!(file >> value)) {
        throw std::runtime_error("Cannot read tape value: " + fileName.string());
    }

    return value;
}

void FileTape::write(int value) {
    if (isEnd()) {
        throw std::out_of_range("Cannot write: tape head is out of tape range");
    }

    delay(config.writeDelayMs);

    file.clear();
    file.seekp(getPositionInFile(currentPosition));

    file << std::setw(CELL_WIDTH) << value << '\n';
    file.flush();
}

void FileTape::moveLeft() {
    delay(config.moveDelayMs);

    if (currentPosition > 0) {
        currentPosition--;
    }
}

void FileTape::moveRight() {
    delay(config.moveDelayMs);

    if (currentPosition < tapeSize) {
        currentPosition++;
    }
}

void FileTape::rewind() {
    delay(config.rewindDelayMs);

    currentPosition = 0;
}

bool FileTape::isEnd() const {
    return currentPosition >= tapeSize;
}

int FileTape::size() const {
    return tapeSize;
}
