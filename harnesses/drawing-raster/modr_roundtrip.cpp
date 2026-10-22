#include "DrawingRaster.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::abort();
    }
}

void checkDecoded(const std::vector<uint8_t>& expected, int width, int height,
                  int actualWidth, int actualHeight,
                  const std::vector<uint8_t>& actual) {
    require(actualWidth == width && actualHeight == height, "dimensions changed");
    require(actual.size() == expected.size(), "decoded buffer size mismatch");
    for (std::size_t i = 0; i < expected.size(); i += 4) {
        require(actual[i] == expected[i] && actual[i + 1] == expected[i + 1]
                && actual[i + 2] == expected[i + 2] && actual[i + 3] == 255,
                "decoded pixel mismatch");
    }
}

void checkRoundTrip(int width, int height, std::size_t& splitCases) {
    std::vector<uint8_t> rgba(static_cast<std::size_t>(width * height * 4));
    for (std::size_t i = 0; i < rgba.size(); i += 4) {
        rgba[i] = static_cast<uint8_t>(i * 3 + width);
        rgba[i + 1] = static_cast<uint8_t>(i * 5 + height);
        rgba[i + 2] = static_cast<uint8_t>(i * 7 + width + height);
        rgba[i + 3] = static_cast<uint8_t>(i + 17);
    }

    const std::string encoded = monolith::drawing::encodeModr(width, height, rgba);
    require(encoded.size() == monolith::drawing::kModrHeaderBytes
            + static_cast<std::size_t>(width * height) * 3,
            "encoded length mismatch");

    int decodedWidth = -1;
    int decodedHeight = -1;
    std::vector<uint8_t> decoded{9, 8, 7};
    require(monolith::drawing::decodeModr(encoded, decodedWidth, decodedHeight, decoded),
            "whole-buffer decode failed");
    checkDecoded(rgba, width, height, decodedWidth, decodedHeight, decoded);

    for (std::size_t split = 0; split <= encoded.size(); ++split) {
        monolith::drawing::ModrStreamDecoder decoder(encoded.size());
        require(decoder.consume(std::string_view(encoded.data(), split)),
                "first decoder chunk failed");
        require(decoder.consume(std::string_view(encoded.data() + split,
                                                  encoded.size() - split)),
                "second decoder chunk failed");
        decodedWidth = -1;
        decodedHeight = -1;
        decoded.assign({9, 8, 7});
        require(decoder.finish(decodedWidth, decodedHeight, decoded),
                "split decoder finish failed");
        checkDecoded(rgba, width, height, decodedWidth, decodedHeight, decoded);
        ++splitCases;
    }
}

std::size_t checkMalformedLengths() {
    const std::vector<uint8_t> rgba{1, 2, 3, 4};
    const std::string valid = monolith::drawing::encodeModr(1, 1, rgba);
    std::size_t cases = 0;
    for (std::size_t size = 0; size < valid.size(); ++size) {
        const std::string truncated = valid.substr(0, size);
        int width = 91;
        int height = 92;
        std::vector<uint8_t> output{6, 5, 4};
        require(!monolith::drawing::decodeModr(truncated, width, height, output),
                "truncated stream accepted");
        require(width == 91 && height == 92 && output == std::vector<uint8_t>({6, 5, 4}),
                "failed decode modified caller outputs");
        ++cases;
    }

    std::string extended = valid;
    extended.push_back('x');
    int width = 91;
    int height = 92;
    std::vector<uint8_t> output{6, 5, 4};
    require(!monolith::drawing::decodeModr(extended, width, height, output),
            "extra payload accepted");
    require(width == 91 && height == 92 && output == std::vector<uint8_t>({6, 5, 4}),
            "failed extended decode modified caller outputs");
    return cases + 1;
}

} // namespace

int main() {
    std::size_t splitCases = 0;
    for (int width = 1; width <= 3; ++width) {
        for (int height = 1; height <= 3; ++height) {
            checkRoundTrip(width, height, splitCases);
        }
    }
    const std::size_t malformedCases = checkMalformedLengths();
    std::cout << "small .modr canvases passed: 9; chunk split cases passed: "
              << splitCases << "; malformed length cases passed: " << malformedCases << '\n';
}
