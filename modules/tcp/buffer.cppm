module ;

#include <cstdio>

export module tcp:buffer;

import <iostream>;
import <string>;

using namespace std;

namespace tcp {
    export class Buffer {
    public:
        // Buffer();

        // ~Buffer();

        void append(string_view _str);

        [[nodiscard]] size_t size() const;

        [[nodiscard]] const char *c_str() const;

        void clearBuf();

        void getLineBuf();

        void setBuf(string_view _buf);

    private:
        string buf{};
    };

    void Buffer::append(const string_view _str) {
        buf.append(_str);
        // println("appended size {}: {}", _str.length(), _str);
    }

    size_t Buffer::size() const {
        return buf.size();
    }

    const char *Buffer::c_str() const {
        return buf.c_str();
    }

    void Buffer::clearBuf() {
        buf.clear();
    }

    void Buffer::getLineBuf() {
        clearBuf();
        getline(cin, buf);
    }

    void Buffer::setBuf(const string_view _buf) {
        clearBuf();
        append(_buf);
    }
}
