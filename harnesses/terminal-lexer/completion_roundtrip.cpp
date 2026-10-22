#include "TerminalLexer.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

namespace {

constexpr std::array<char, 6> alphabet{{'a', ' ', '\t', '\\', '\'', '"'}};
constexpr std::array<char, 3> quoteModes{{'\0', '\'', '"'}};

void checkValue(const std::string& value, std::size_t& cases) {
    for (char quote : quoteModes) {
        const std::string escaped = monolith::app::escapeCompletion(value, quote);
        std::string line;
        if (quote != '\0') line.push_back(quote);
        line += escaped;
        if (quote != '\0') line.push_back(quote);

        const auto tokens = monolith::app::tokenizeCommandLine(line);
        if (!tokens.error.empty() || tokens.args.size() != 1 || tokens.args[0] != value) {
            std::cerr << "round trip failed for quote=" << static_cast<int>(quote)
                      << " value-size=" << value.size() << '\n';
            std::abort();
        }
        ++cases;
    }
}

void enumerate(std::string& value, std::size_t remaining, std::size_t& cases) {
    if (!value.empty()) checkValue(value, cases);
    if (remaining == 0) return;
    for (char c : alphabet) {
        value.push_back(c);
        enumerate(value, remaining - 1, cases);
        value.pop_back();
    }
}

} // namespace

int main() {
    std::string value;
    std::size_t cases = 0;
    enumerate(value, 3, cases);
    std::cout << "completion round trips passed: " << cases << '\n';
}
