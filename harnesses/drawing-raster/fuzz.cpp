#include "DrawingRaster.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    std::string encoded;
    if (size > 0) encoded.assign(reinterpret_cast<const char*>(data), size);
    int width = -7;
    int height = -9;
    std::vector<std::uint8_t> rgba{3, 1, 4};
    const bool decoded = monolith::drawing::decodeModr(encoded, width, height, rgba);

    int streamWidth = -7;
    int streamHeight = -9;
    std::vector<std::uint8_t> streamRgba{3, 1, 4};
    monolith::drawing::ModrStreamDecoder decoder(size);
    bool consumed = true;
    std::size_t offset = 0;
    while (offset < size) {
        const std::size_t chunk = std::min<std::size_t>(1 + (offset % 23), size - offset);
        if (!decoder.consume(std::string_view(encoded.data() + offset, chunk))) {
            consumed = false;
            break;
        }
        offset += chunk;
    }
    const bool streamDecoded = consumed
        && decoder.finish(streamWidth, streamHeight, streamRgba);

    if (decoded != streamDecoded) std::abort();
    if (!decoded) {
        if (width != -7 || height != -9 || rgba != std::vector<std::uint8_t>({3, 1, 4})
            || streamWidth != -7 || streamHeight != -9
            || streamRgba != std::vector<std::uint8_t>({3, 1, 4})) {
            std::abort();
        }
        return 0;
    }

    if (width <= 0 || height <= 0 || width > monolith::drawing::kMaxModrDimension
        || height > monolith::drawing::kMaxModrDimension
        || streamWidth != width || streamHeight != height || streamRgba != rgba) {
        std::abort();
    }
    const std::size_t expectedBytes = static_cast<std::size_t>(width)
        * static_cast<std::size_t>(height) * 4;
    if (rgba.size() != expectedBytes) std::abort();
    for (std::size_t alpha = 3; alpha < rgba.size(); alpha += 4) {
        if (rgba[alpha] != 255) std::abort();
    }
    if (monolith::drawing::encodeModr(width, height, rgba) != encoded) std::abort();
    return 0;
}
