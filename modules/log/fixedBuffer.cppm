module;

export module log:fixedBuffer;
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

export class FixedBuffer {
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

    void clear();

private:
    array<char, config::LOG_BUF_SIZE> bufData{};
    string_view::size_type firstUnused{0};

    void expand(size_t len);
};

template<integral T>
void FixedBuffer::appendInt(T value) {
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

void FixedBuffer::append(string_view content) {
    if (leftSpace() > content.size()) {
        std::ranges::copy(content, bufData.begin() + firstUnused);
        expand(content.size());
    }
}

const array<char, config::LOG_BUF_SIZE> &FixedBuffer::getData() const { return bufData; }

string_view FixedBuffer::getSV() const {
    return {bufData.data(), firstUnused};
}


void FixedBuffer::setAdd(const char val) {
    bufData[firstUnused] = val;
    firstUnused += 1;
}

void FixedBuffer::expand(const size_t len) { firstUnused += len; }

size_t FixedBuffer::cap() const { return bufData.size(); }

int FixedBuffer::len() const { return firstUnused; }

size_t FixedBuffer::leftSpace() const { return cap() - len(); }

void FixedBuffer::reset() { firstUnused = 0; }

void FixedBuffer::clear() { bzero(bufData.data(), bufData.size()); }


