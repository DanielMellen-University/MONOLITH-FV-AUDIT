#include "TerminalLexer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    std::string line;
    if (size > 0) line.assign(reinterpret_cast<const char*>(data), size);
    const auto tokens = monolith::app::tokenizeCommandLine(line);
    if (!tokens.error.empty() && !tokens.args.empty()) std::abort();

    const std::size_t middle = size / 2;
    const std::size_t cursors[] = {0, middle, size, size + 1};
    for (const std::size_t cursor : cursors) {
        const auto context = monolith::app::completionContextAt(line, cursor);
        const std::size_t clamped = std::min(cursor, size);
        if (context.replacementStart > clamped || context.prefix.size() > clamped) {
            std::abort();
        }
        if (context.quote != '\0' && context.quote != '\'' && context.quote != '"') {
            std::abort();
        }
    }
    return 0;
}
