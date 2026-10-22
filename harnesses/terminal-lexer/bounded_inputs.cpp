#include "TerminalLexer.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

constexpr std::array<char, 6> alphabet{{'a', ' ', '\t', '\\', '\'', '"'}};
constexpr std::size_t maxLength = 5;
std::size_t inputCount = 0;
std::size_t cursorCount = 0;

void checkInput(const std::string& line) {
    const auto tokens = monolith::app::tokenizeCommandLine(line);
    if (!tokens.error.empty() && !tokens.args.empty()) {
        std::cerr << "lexer returned partial arguments with an error\n";
        std::abort();
    }
    ++inputCount;

    for (std::size_t cursor = 0; cursor <= line.size() + 1; ++cursor) {
        const auto context = monolith::app::completionContextAt(line, cursor);
        const std::size_t clamped = cursor < line.size() ? cursor : line.size();
        if (context.replacementStart > clamped || context.prefix.size() > clamped) {
            std::cerr << "completion context exceeds clamped cursor\n";
            std::abort();
        }
        ++cursorCount;
    }
}

void enumerate(std::string& line, std::size_t remaining) {
    checkInput(line);
    if (remaining == 0) return;
    for (char c : alphabet) {
        line.push_back(c);
        enumerate(line, remaining - 1);
        line.pop_back();
    }
}

} // namespace

int main() {
    std::string line;
    enumerate(line, maxLength);
    std::cout << "bounded lexer inputs passed: " << inputCount
              << "; completion cursors passed: " << cursorCount << '\n';
}
