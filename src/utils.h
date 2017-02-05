#pragma once
#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string> split(const std::string& line, char delim) {
    std::vector<std::string> v;
    std::stringstream ss;
    ss.str(line);
    std::string next;
    while (std::getline(ss, next, delim)) {
        v.push_back(next);
    }
    return v;
}

// Note: consumes "\r\n" before returning
inline std::string getCRLFLine(std::stringstream& ss) {
    std::string line;
    if (std::getline(ss, line, '\n')) {
        if (line[line.size()-1] == '\r') {
            line.resize(line.size() - 1);
        }
        return line;
    }
    return "";
}
