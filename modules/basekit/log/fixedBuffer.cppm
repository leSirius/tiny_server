module;

export module basekit:fixedBuffer;
import <algorithm>;
import <array>;
import <iostream>;
import <ranges>;
import <string>;
import <string_view>;
import <type_traits>;
import config;

using namespace std;

static constexpr int MaxIntSize = 20;

export template<size_t SIZE = config::LOG_BUF_SIZE>
class FixedBuffer {
public:
    FixedBuffer() = default;

    ~FixedBuffer() = default;

    void append(string_view content);

    template<integral T>
    void appendInt(T value);

    [[nodiscard]] const array<char, 4096> &getData() const;

    [[nodiscard]] string_view getSV() const;

    void setAdd(char val);

    [[nodiscard]] size_t cap() const;

    [[nodiscard]] int len() const;

    [[nodiscard]] size_t leftSpace() const;

    void reset();

    // void toZero();

private:
    array<char, SIZE> bufData{};
    string_view::size_type firstUnused{0};

    void expand(size_t len);
};

template<size_t SIZE>
template<integral T>
void FixedBuffer<SIZE>::appendInt(T value) {
    if (leftSpace() >= MaxIntSize) {
        const auto intBegin = firstUnused;
        bool isNeg = false;
        using UnsignedT = std::make_unsigned_t<T>;
        UnsignedT tempVal;
        if constexpr (std::is_signed_v<T>) {
            if (value < 0) {
                isNeg = true;
                tempVal = static_cast<UnsignedT>(0) - static_cast<UnsignedT>(value);
            } else {
                tempVal = static_cast<UnsignedT>(value);
            }
        } else {
            tempVal = static_cast<UnsignedT>(value);
        }
        do {
            const char digit = '0' + (tempVal % 10);
            setAdd(digit);
            tempVal /= 10;
        } while (tempVal != 0);
        if (isNeg) { setAdd('-'); }
        ranges::reverse(bufData.begin() + intBegin, bufData.begin() + firstUnused);
    }
}

template<size_t SIZE>
void FixedBuffer<SIZE>::append(string_view content) {
    if (leftSpace() > content.size()) {
        std::ranges::copy(content, bufData.begin() + firstUnused);
        expand(content.size());
    }
}

template<size_t SIZE>
const array<char, config::LOG_BUF_SIZE> &FixedBuffer<SIZE>::getData() const { return bufData; }

template<size_t SIZE>
string_view FixedBuffer<SIZE>::getSV() const {
    return {bufData.data(), firstUnused};
}

template<size_t SIZE>
void FixedBuffer<SIZE>::setAdd(const char val) {
    bufData[firstUnused] = val;
    firstUnused += 1;
}

template<size_t SIZE>
void FixedBuffer<SIZE>::expand(const size_t len) { firstUnused += len; }

template<size_t SIZE>
size_t FixedBuffer<SIZE>::cap() const { return bufData.size(); }

template<size_t SIZE>
int FixedBuffer<SIZE>::len() const { return firstUnused; }

template<size_t SIZE>
size_t FixedBuffer<SIZE>::leftSpace() const { return cap() - len(); }

template<size_t SIZE>
void FixedBuffer<SIZE>::reset() {
    bzero(bufData.data(), bufData.size());
    firstUnused = 0;
}



