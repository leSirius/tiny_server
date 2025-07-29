module ;

#include <cstdio>

export module basekit:buffer;

import <iostream>;
import <string>;

using namespace std;

namespace basekit {
    export class Buffer {
    public:
        // Buffer();

        // ~Buffer();

        void append(string_view _str);

        [[nodiscard]] size_t size() const;

        [[nodiscard]] const char *c_str() const;

        void clear();

        void getLine();

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

    void Buffer::clear() {
        buf.clear();
    }

    void Buffer::getLine() {
        buf.clear();
        getline(cin, buf);
    }

    void Buffer::setBuf(const string_view _buf) {
        clear();
        append(_buf);
    }
}
