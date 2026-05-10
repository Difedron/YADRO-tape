#include "FileTape.h"

#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <thread>

FileTape::FileTape(const std::string& fileName, const TapeConfig& config, int tapeSize)
        : fileName(fileName), config(config), currentPosition(0), tapeSize(tapeSize) {
    file.open(fileName, std::ios::in | std::ios::out);

    if (!file.is_open()) {
        file.clear();

        std::ofstream createFile(fileName);

        for (int i = 0; i < tapeSize; ++i) {
            createFile << std::setw(12) << 0;
        }

        createFile.close();

        file.open(fileName, std::ios::in | std::ios::out);
    }
}

void FileTape::delay(int milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::streampos FileTape::getPositionInFile(int position) const {
    return static_cast<std::streampos>(position * 12);
}

int FileTape::read() {
    if (isEnd()) {
        throw std::out_of_range("Cannot read: tape head is out of tape range");
    }

    delay(config.readDelayMs);

    file.clear();
    file.seekg(getPositionInFile(currentPosition));

    int value;
    file >> value;

    return value;
}

void FileTape::write(int value) {
    if (isEnd()) {
        throw std::out_of_range("Cannot write: tape head is out of tape range");
    }

    delay(config.writeDelayMs);

    file.clear();
    file.seekp(getPositionInFile(currentPosition));

    file << std::setw(12) << value;
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