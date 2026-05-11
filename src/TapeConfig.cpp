#include "TapeConfig.h"

#include <fstream>
#include <sstream>

TapeConfig TapeConfig::loadFromFile(const std::filesystem::path& fileName) {
    TapeConfig config;

    std::ifstream file(fileName);

    if (!file.is_open()) {
        return config;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string key;
        std::string value;

        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            int number = std::stoi(value);

            if (key == "read_delay_ms") {
                config.readDelayMs = number;
            } else if (key == "write_delay_ms") {
                config.writeDelayMs = number;
            } else if (key == "move_delay_ms") {
                config.moveDelayMs = number;
            } else if (key == "rewind_delay_ms") {
                config.rewindDelayMs = number;
            } else if (key == "memory_limit_bytes") {
                config.memoryLimitBytes = number;
            }
        }
    }

    return config;
}
