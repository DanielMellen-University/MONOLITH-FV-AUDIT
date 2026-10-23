#include "Filesystem.hpp"

#include <array>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <poll.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

bool writeAll(int fd, const char* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
        const ssize_t written = ::write(fd, data + offset, size - offset);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) return false;
        offset += static_cast<std::size_t>(written);
    }
    return true;
}

bool readAll(int fd, char* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
        const ssize_t read = ::read(fd, data + offset, size - offset);
        if (read < 0 && errno == EINTR) continue;
        if (read <= 0) return false;
        offset += static_cast<std::size_t>(read);
    }
    return true;
}

bool readReadyWithTimeout(int fd, char& value) {
    pollfd descriptor{fd, POLLIN, 0};
    int result;
    do {
        result = ::poll(&descriptor, 1, 10000);
    } while (result < 0 && errno == EINTR);
    return result == 1 && (descriptor.revents & POLLIN) != 0
        && readAll(fd, &value, 1);
}

bool resultWithTimeout(int fd, std::array<char, 2>& results) {
    pollfd descriptor{fd, POLLIN, 0};
    int result;
    do {
        result = ::poll(&descriptor, 1, 10000);
    } while (result < 0 && errno == EINTR);
    return result == 1 && (descriptor.revents & POLLIN) != 0
        && readAll(fd, results.data(), results.size());
}

} // namespace

int main() {
    namespace fs = std::filesystem;
    std::signal(SIGPIPE, SIG_IGN);
    const fs::path root = fs::path("/tmp")
        / ("monolith-fv-cooperating-writers-" + std::to_string(::getpid()));
    std::error_code ec;
    require(fs::create_directory(root, ec) && !ec,
            "could not create unique temporary filesystem root");

    monolith::fs::Filesystem filesystem(root.string());
    require(filesystem.initialize(), "parent filesystem initialization failed");
    const std::string path = "/shared.bin";
    require(filesystem.writeFile(path, "baseline"), "baseline write failed");
    monolith::fs::FileStamp expected;
    require(filesystem.fileStamp(path, expected), "baseline file stamp unavailable");

    int readyPipe[2]{};
    int releasePipe[2]{};
    int resultPipe[2]{};
    require(::pipe(readyPipe) == 0 && ::pipe(releasePipe) == 0
                && ::pipe(resultPipe) == 0,
            "could not create process coordination pipes");

    std::array<pid_t, 2> children{};
    for (std::size_t writer = 0; writer < children.size(); ++writer) {
        const pid_t child = ::fork();
        require(child >= 0, "fork failed");
        if (child == 0) {
            ::close(readyPipe[0]);
            ::close(releasePipe[1]);
            ::close(resultPipe[0]);

            const char marker = static_cast<char>('A' + writer);
            const std::string payload(256 * 1024, marker);
            monolith::fs::Filesystem childFilesystem(root.string());
            int resultCode = 3;
            if (childFilesystem.initialize()) {
                const auto producer = [&](std::ostream& output) {
                    const std::size_t midpoint = payload.size() / 2;
                    output.write(payload.data(), static_cast<std::streamsize>(midpoint));
                    if (!output || !writeAll(readyPipe[1], "R", 1)) return false;
                    char release = '\0';
                    if (!readAll(releasePipe[0], &release, 1)) return false;
                    output.write(payload.data() + midpoint,
                                 static_cast<std::streamsize>(payload.size() - midpoint));
                    return static_cast<bool>(output);
                };
                using Result = monolith::fs::ConditionalWriteResult;
                const Result result = childFilesystem.writeFileWithProducerIfStampMatches(
                    path, producer, expected);
                if (result == Result::Written) resultCode = 1;
                else if (result == Result::Conflict) resultCode = 2;
            }

            const char code = static_cast<char>(resultCode);
            const bool reported = writeAll(resultPipe[1], &code, 1);
            ::close(readyPipe[1]);
            ::close(releasePipe[0]);
            ::close(resultPipe[1]);
            ::_exit(reported ? 0 : 1);
        }
        children[writer] = child;
    }

    ::close(readyPipe[1]);
    ::close(releasePipe[0]);
    ::close(resultPipe[1]);

    bool bothReady = true;
    for (std::size_t writer = 0; writer < children.size(); ++writer) {
        char marker = '\0';
        if (!readReadyWithTimeout(readyPipe[0], marker) || marker != 'R') {
            bothReady = false;
            break;
        }
    }

    const std::array<char, 2> release{{'G', 'G'}};
    const bool released = writeAll(releasePipe[1], release.data(), release.size());
    ::close(readyPipe[0]);
    ::close(releasePipe[1]);

    std::array<char, 2> results{};
    const bool receivedResults = resultWithTimeout(resultPipe[0], results);
    ::close(resultPipe[0]);

    bool childrenSucceeded = true;
    for (const pid_t child : children) {
        int status = 0;
        pid_t waited;
        do {
            waited = ::waitpid(child, &status, 0);
        } while (waited < 0 && errno == EINTR);
        childrenSucceeded = childrenSucceeded && waited == child
            && WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }

    const std::size_t writtenCount = static_cast<std::size_t>(results[0] == 1)
        + static_cast<std::size_t>(results[1] == 1);
    const std::size_t conflictCount = static_cast<std::size_t>(results[0] == 2)
        + static_cast<std::size_t>(results[1] == 2);
    const std::string published = filesystem.readFile(path);
    const bool completePayload = published == std::string(256 * 1024, 'A')
        || published == std::string(256 * 1024, 'B');
    std::size_t unexpectedEntries = 0;
    for (const auto& entry : fs::directory_iterator(root)) {
        if (entry.path().filename() != "shared.bin") ++unexpectedEntries;
    }
    const bool cleaned = unexpectedEntries == 0;
    fs::remove_all(root, ec);

    require(bothReady, "writers did not both reach their producer barrier");
    require(released, "could not release both staged writers");
    require(receivedResults, "did not receive both writer results");
    require(childrenSucceeded, "a writer child exited unsuccessfully");
    require(writtenCount == 1 && conflictCount == 1,
            "expected exactly one conditional publish and one stale-stamp conflict");
    require(completePayload, "visible target is not one complete writer payload");
    require(cleaned, "writer race left unexpected staging entries");
    require(!ec, "temporary filesystem cleanup failed");

    std::cout << "cooperating writer race passed: 2 processes, both staged before release, "
                 "one Written, one Conflict, complete 262144-byte target\n";
}
