#include <assert.h>

extern "C" void __CPROVER_assume(int condition);

int normalizeIndex(int index, int count) {
    if (index < 0 || index >= count) return -1;
    return index;
}

int main() {
    constexpr int count = 4;
    int index;
    __CPROVER_assume(index >= -1 && index <= count);

    const int normalized = normalizeIndex(index, count);
    if (normalized == -1) {
        assert(index < 0 || index >= count);
    } else {
        assert(normalized == index);
        assert(normalized >= 0 && normalized < count);
    }
}
