#include "Filesystem.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

monolith::fs::Filesystem::FileContentProducer content(const std::string& value) {
    return [&value](std::ostream& output) {
        output.write(value.data(), static_cast<std::streamsize>(value.size()));
        return static_cast<bool>(output);
    };
}

} // namespace

int main() {
    namespace fs = std::filesystem;
    const fs::path root = "/tmp/monolith-fv-conditional-write";
    std::error_code ec;
    fs::remove_all(root, ec);

    monolith::fs::Filesystem filesystem(root.string());
    require(filesystem.initialize(), "filesystem initialization failed");

    using Result = monolith::fs::ConditionalWriteResult;
    const std::optional<monolith::fs::FileStamp> absent;
    require(filesystem.writeFileWithProducerIfStampMatches("/note.txt", content("first"), absent)
                == Result::Written,
            "expected-absent write did not publish");
    require(filesystem.readFile("/note.txt") == "first", "published bytes mismatch");

    monolith::fs::FileStamp firstStamp;
    require(filesystem.fileStamp("/note.txt", firstStamp), "initial stamp unavailable");
    require(filesystem.writeFileWithProducerIfStampMatches(
                "/note.txt", content("second"), firstStamp) == Result::Written,
            "matching-stamp write did not publish");
    require(filesystem.readFile("/note.txt") == "second", "replacement bytes mismatch");

    require(filesystem.writeFileWithProducerIfStampMatches(
                "/note.txt", content("stale"), firstStamp) == Result::Conflict,
            "stale-stamp write did not conflict");
    require(filesystem.readFile("/note.txt") == "second", "conflict changed target bytes");

    monolith::fs::FileStamp secondStamp;
    require(filesystem.fileStamp("/note.txt", secondStamp), "replacement stamp unavailable");
    const auto failedProducer = [](std::ostream& output) {
        output << "partial";
        return false;
    };
    require(filesystem.writeFileWithProducerIfStampMatches(
                "/note.txt", failedProducer, secondStamp) == Result::Failed,
            "failing producer was not reported as failed");
    require(filesystem.readFile("/note.txt") == "second", "producer failure changed target bytes");

    require(filesystem.writeFileWithProducerIfStampMatches(
                "/note.txt", content("unexpected"), absent) == Result::Conflict,
            "expected-absent write replaced an existing target");
    require(filesystem.readFile("/note.txt") == "second", "expected-absent conflict changed target");

    fs::remove_all(root, ec);
    require(!ec, "temporary filesystem cleanup failed");
    std::cout << "conditional write cases passed: 5\n";
}
