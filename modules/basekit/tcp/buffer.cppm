module ;
#include <cassert>

export module basekit:buffer;
import <algorithm>;
import <iostream>;
import <string>;
import <vector>;

using namespace std;

namespace basekit {
    constexpr int PrePendIndex = 8;
    constexpr int InitialSize = 1024;

    export class Buffer {
    public:
        Buffer();

        ~Buffer();

        char *vecBegin();

        // const对象的begin函数，使得const对象调用begin函数时
        // 所得到的迭代器只能对数据进行读操作，而不能进行修改
        [[nodiscard]] const char *vecBegin() const;

        char *readBegin();

        [[nodiscard]] const char *readBegin() const;

        char *writeBegin();

        [[nodiscard]] const char *writeBegin() const;

        void append(string_view message);

        [[nodiscard]] int readableBytes() const;

        [[nodiscard]] int writableBytes() const;

        [[nodiscard]] int prependableBytes() const;

        char *peek();

        [[nodiscard]] const char *peek() const;

        [[nodiscard]] string peekAsString(size_t len) const;

        [[nodiscard]] string peekAllAsString() const;

        // 取数据，取出后更新read_index,相当于不可重复取
        void retrieve(int len);

        string retrieveAsString(int len);

        void retrieveAll();

        string retrieveAllAsString();

        void retrieveUntil(const char *end);

        std::string retrieveUntilAsString(const char *end);

        void ensureWritableBytes(int len);

    private:
        vector<char> buffer{vector<char>(InitialSize)};
        int readIndex{PrePendIndex};
        int writeIndex{PrePendIndex};
    };

    Buffer::Buffer() = default;

    Buffer::~Buffer() = default;

    int Buffer::readableBytes() const { return writeIndex - readIndex; }

    int Buffer::writableBytes() const { return static_cast<int>(buffer.size()) - writeIndex; }

    int Buffer::prependableBytes() const { return readIndex; }

    char *Buffer::vecBegin() { return &*buffer.begin(); }

    const char *Buffer::vecBegin() const { return &*buffer.begin(); }

    char *Buffer::readBegin() { return vecBegin() + readIndex; }

    const char *Buffer::readBegin() const { return vecBegin() + readIndex; }

    char *Buffer::writeBegin() { return vecBegin() + writeIndex; }

    const char *Buffer::writeBegin() const { return vecBegin() + writeIndex; }

    void Buffer::append(const string_view message) {
        const auto len = static_cast<int>(message.size());
        ensureWritableBytes(len);
        std::copy_n(message.data(), len, writeBegin());
        writeIndex += len;
    }

    char *Buffer::peek() { return readBegin(); }

    const char *Buffer::peek() const { return readBegin(); }

    string Buffer::peekAsString(const size_t len) const { return {peek(), len}; }

    string Buffer::peekAllAsString() const { return peekAsString(readableBytes()); }

    void Buffer::retrieve(const int len) {
        assert(readableBytes() > len);
        if (len + readIndex < writeIndex) {
            // 如果读的内容不超过可读空间，则只用更新read_index_
            readIndex += len;
        } else {
            // 否则就是正好读完，需要同时更新write_index_
            retrieveAll();
        }
    }

    string Buffer::retrieveAsString(const int len) {
        assert(readIndex + len <= writableBytes());
        string ret = std::move(peekAsString(len));
        retrieve(len);
        return ret;
    }

    void Buffer::retrieveAll() {
        writeIndex = PrePendIndex;
        readIndex = writeIndex;
    }

    string Buffer::retrieveAllAsString() {
        assert(readableBytes() > 0);
        string ret = std::move(peekAllAsString());
        retrieveAll();
        return ret;
    }

    void Buffer::retrieveUntil(const char *end) {
        assert(writeBegin() >= end);
        readIndex += static_cast<int>(end - readBegin());
    }

    string Buffer::retrieveUntilAsString(const char *end) {
        assert(writeBegin() >= end);
        string ret = std::move(peekAsString(static_cast<int>(end - readBegin())));
        retrieveUntil(end);
        return ret;
    }

    void Buffer::ensureWritableBytes(const int len) {
        if (writableBytes() >= len) { return; }
        if (writableBytes() + prependableBytes() >= PrePendIndex + len) {
            // 如果此时writable和prepenable的剩余空间超过写的长度，则先将已有数据复制到初始位置，
            // 将不断读导致的read_index_后移使前方没有利用的空间利用上。
            std::copy_n(readBegin(), writeIndex - readIndex, vecBegin() + PrePendIndex);
            writeIndex = PrePendIndex + readableBytes();
            readIndex = PrePendIndex;
        } else {
            buffer.resize(writeIndex + len);
        }
    }
}
