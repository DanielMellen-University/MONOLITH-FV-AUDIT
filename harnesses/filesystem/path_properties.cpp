#include "Filesystem.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

constexpr std::array<char, 4> alphabet{{'a', 'b', '.', '/'}};
std::size_t pathsChecked = 0;
std::size_t pairsChecked = 0;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::abort();
    }
}

void checkCanonical(const monolith::fs::Filesystem& fs, const std::string& path) {
    const std::string normalized = fs.normalize(path);
    require(!normalized.empty() && normalized.front() == '/',
            "normalized path is not absolute");
    require(fs.normalize(normalized) == normalized, "normalization is not idempotent");
    std::size_t start = 1;
    while (start < normalized.size()) {
        const std::size_t end = normalized.find('/', start);
        const std::string component = normalized.substr(
            start, end == std::string::npos ? std::string::npos : end - start);
        require(!component.empty() && component != "." && component != "..",
                "normalized path contains a non-canonical component");
        if (end == std::string::npos) break;
        start = end + 1;
    }
    ++pathsChecked;
}

void enumeratePaths(const monolith::fs::Filesystem& fs, std::string& path,
                    std::size_t remaining) {
    checkCanonical(fs, path);
    if (remaining == 0) return;
    for (char c : alphabet) {
        path.push_back(c);
        enumeratePaths(fs, path, remaining - 1);
        path.pop_back();
    }
}

void checkEntryNames(const monolith::fs::Filesystem& fs) {
    const std::array<std::string, 6> names{{"", ".", "..", "a", "a/b",
                                            std::string("a\0b", 3)}};
    for (const std::string& name : names) {
        const bool expected = name == "a";
        require(monolith::fs::Filesystem::isValidEntryName(name) == expected,
                "entry-name validity mismatch");
    }
    const std::string parent = fs.normalize("/alpha/../beta");
    for (const std::string name : {"a", "b", "file.txt"}) {
        const std::string child = fs.join(parent, name);
        require(child == fs.normalize(child), "join did not return canonical path");
        require(fs.isSameOrDescendant(parent, child), "joined name escaped parent");
    }
}

void checkComponentContainment(const monolith::fs::Filesystem& fs) {
    require(fs.isSameOrDescendant("/a", "/a"), "same path not included");
    require(fs.isSameOrDescendant("/a", "/a/b"), "child path not included");
    require(!fs.isSameOrDescendant("/a", "/ab"), "sibling prefix treated as child");
    require(!fs.isSameOrDescendant("/a", "/a-sibling/file"),
            "sibling subtree treated as child");
    require(fs.isSameOrDescendant("/", "/anything"), "root containment failed");

    for (const std::string ancestor : {"/a", "/a/b", "/ab", "/x.y"}) {
        for (const std::string candidate : {"/a", "/a/b", "/a/bc", "/ab", "/x.y/z"}) {
            const bool expected = candidate == ancestor || ancestor == "/"
                || (candidate.size() > ancestor.size()
                    && candidate.compare(0, ancestor.size(), ancestor) == 0
                    && candidate[ancestor.size()] == '/');
            require(fs.isSameOrDescendant(ancestor, candidate) == expected,
                    "component-aware descendant result mismatch");
            ++pairsChecked;
        }
    }
}

} // namespace

int main() {
    const monolith::fs::Filesystem fs("/tmp/monolith-fv-path-root");
    std::string path;
    enumeratePaths(fs, path, 5);
    checkEntryNames(fs);
    checkComponentContainment(fs);
    std::cout << "canonical paths checked: " << pathsChecked
              << "; explicit containment pairs checked: " << pairsChecked << '\n';
}
