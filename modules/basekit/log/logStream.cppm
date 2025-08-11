module;


export module basekit:logStream;

import :fixedBuffer;
import  <format>;
import  <iostream>;

export class LogStream {
public:
    using Buffer = FixedBuffer<>;
    typedef LogStream self;

    LogStream() = default;

    ~LogStream() = default;

    void append(string_view str);

    [[nodiscard]] const Buffer &getBuffer() const;

    void resetBuffer();

    self &operator<<(bool v);

    self &operator<<(short num);

    self &operator<<(unsigned short num);

    self &operator<<(int num);

    self &operator<<(unsigned int num);

    self &operator<<(long num);

    self &operator<<(unsigned long num);

    self &operator<<(long long num);

    self &operator<<(unsigned long long num);

    self &operator<<(const float &num);

    self &operator<<(const double &num);

    self &operator<<(char v);

    self &operator<<(const char *str);

    self &operator<<(const std::string &v);

    LogStream &operator<<(const std::string_view &v);

private:
    Buffer buffer;
};


void LogStream::append(const string_view str) {
    buffer.append(str);
}

const LogStream::Buffer &LogStream::getBuffer() const {
    return buffer;
}

void LogStream::resetBuffer() {
    // buffer.toZero();
    buffer.reset();
}

LogStream &LogStream::operator<<(const bool v) {
    buffer.append(v ? "1" : "0");
    return *this;
}

LogStream &LogStream::operator<<(const short num) {
    return (*this) << static_cast<int>(num);
}

LogStream &LogStream::operator<<(const unsigned short num) {
    return (*this) << static_cast<unsigned int>(num);
}

LogStream &LogStream::operator<<(const int num) {
    buffer.appendInt(num);
    return *this;
}

LogStream &LogStream::operator<<(const unsigned int num) {
    buffer.appendInt(num);
    return *this;
}

LogStream &LogStream::operator<<(const long num) {
    buffer.appendInt(num);
    return *this;
}

LogStream &LogStream::operator<<(const unsigned long num) {
    buffer.appendInt(num);
    return *this;
}

LogStream &LogStream::operator<<(const long long num) {
    buffer.appendInt(num);
    return *this;
}

LogStream &LogStream::operator<<(const unsigned long long num) {
    buffer.appendInt(num);
    return *this;
}

LogStream &LogStream::operator<<(const float &num) {
    return (*this) << static_cast<const double>(num);
}

LogStream &LogStream::operator<<(const double &num) {
    array<char, 32> buf{};
    const auto [out, size] = format_to_n(buf.data(), 32, "{}", num);
    buffer.append(string_view(buf.data(), size));
    return *this;
}

LogStream &LogStream::operator<<(const char v) {
    buffer.append(&v);
    return *this;
}


LogStream &LogStream::operator<<(const char *str) {
    buffer.append(str == nullptr ? "(null)" : str);
    return *this;
}

LogStream &LogStream::operator<<(const std::string &v) {
    buffer.append(v);
    return *this;
}

LogStream &LogStream::operator<<(const std::string_view &v) {
    buffer.append(v);
    return *this;
}

