module;
#include <cassert>

export module basekit:fmt;

import <format>;
import <iostream>;
import :logStream;

constexpr int capacity{40};

using namespace std;

// Fmt 持有string_view，可能有生命周期风险
export class Fmt {
public:
    template<typename T>
        requires std::is_arithmetic_v<T>
    Fmt(format_string<T> fmt, T val);

    [[nodiscard]] string_view getSV() const { return {buf.data(), length}; }

    [[nodiscard]] int getLen() const { return length; }

private:
    array<char, capacity> buf{};
    string_view::size_type length{};
};

template<typename T>
    requires std::is_arithmetic_v<T>
Fmt::Fmt(format_string<T> fmt, T val) {
    try {
        // 可能超出缓冲区范围，format_to_n的模版字符串是format_string类型，构造函数为consteval
        auto ending = vformat_to(buf.begin(), fmt.get(), make_format_args(val));
        length = distance(buf.begin(), ending);
        assert(length < buf.size());
    } catch (const format_error &e) {
        println("failed format： {}", e.what());
    }
}

export inline LogStream &operator<<(LogStream &s, const Fmt &fmt) {
    s.append(fmt.getSV());
    return s;
};

template Fmt::Fmt(format_string<char> fmt, char);

template Fmt::Fmt(format_string<short> fmt, short);

template Fmt::Fmt(format_string<unsigned short> fmt, unsigned short);

template Fmt::Fmt(format_string<int> fmt, int);

template Fmt::Fmt(format_string<unsigned int> fmt, unsigned int);

template Fmt::Fmt(format_string<long> fmt, long);

template Fmt::Fmt(format_string<unsigned long> fmt, unsigned long);

template Fmt::Fmt(format_string<long long> fmt, long long);

template Fmt::Fmt(format_string<unsigned long long> fmt, unsigned long long);

template Fmt::Fmt(format_string<float> fmt, float);
