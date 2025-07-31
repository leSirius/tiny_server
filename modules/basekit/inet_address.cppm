module;
#include <string_view>
#include <arpa/inet.h>

export module basekit:inetAddress;
import <string>;
import utils;

using namespace std;

namespace basekit {
    export class InetAddress {
    public:
        InetAddress() = default;

        InetAddress(string_view ip, int port);

        [[nodiscard]] const sockaddr_in *getPointer() const;

        [[nodiscard]] const sockaddr *getReinter() const;

        [[nodiscard]] socklen_t getLen() const;

        [[nodiscard]] sockaddr *getReinterCC() const;

        socklen_t *getLenPtr();

        [[nodiscard]] string getAddress() const;

        [[nodiscard]] int getPort() const;

    private:
        sockaddr_in addr{};
        socklen_t addrLen{sizeof(addr)};
    };

    InetAddress::InetAddress(const string_view ip, const int port) {
        addr.sin_family = AF_INET;
        utils::errIf(inet_pton(AF_INET, ip.data(), &addr.sin_addr) != 1, "failed to bind address");
        addr.sin_port = htons(static_cast<uint16_t>(port));
        addrLen = sizeof(addr);
    }

    const sockaddr_in *InetAddress::getPointer() const {
        return &addr;
    }

    const sockaddr *InetAddress::getReinter() const {
        return reinterpret_cast<const sockaddr *>(getPointer());
    }

    socklen_t InetAddress::getLen() const {
        return addrLen;
    }

    // ::accept 使用
    sockaddr *InetAddress::getReinterCC() const {
        return const_cast<sockaddr *>(getReinter());
    }

    // ::accept 使用
    socklen_t *InetAddress::getLenPtr() {
        return &addrLen;
    }

    string InetAddress::getAddress() const {
        char buffer[INET_ADDRSTRLEN];
        utils::errIf(
            inet_ntop(AF_INET, &(addr.sin_addr), buffer, sizeof(buffer)) == nullptr,
            "inet_ntop failed"
        );
        return {buffer};
    }

    int InetAddress::getPort() const {
        return ntohs(addr.sin_port);
    }
}
