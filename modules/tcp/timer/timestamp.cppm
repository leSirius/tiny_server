module;

export module tcp:timestamp;
import <array>;
import <chrono>;
// import <format>;
import <iostream>;

using namespace std;

namespace tcp {
    export class Timestamp {
    public:
        using TimePoint = chrono::time_point<chrono::system_clock>;
        using Duration = chrono::microseconds;

        explicit Timestamp(TimePoint _us);

        auto operator<=>(const Timestamp &rhs) const;

        bool operator==(const Timestamp &rhs) const;

        [[nodiscard]] string toString() const;

        template<size_t N>
            requires (N >= 19)
        size_t toString(array<char, N> &ptr) const;

        [[nodiscard]] Duration getDuration() const;

        [[nodiscard]] TimePoint getTimePoint() const;

        static Timestamp getNow();

        template<typename Rep, typename Period>
        static Timestamp getAfter(chrono::duration<Rep, Period> dur);

    private:
        TimePoint timePoint{chrono::system_clock::now()};
    };

    Timestamp::Timestamp(const TimePoint _us) : timePoint(_us) {
    }

    auto Timestamp::operator<=>(const Timestamp &rhs) const {
        return timePoint.time_since_epoch().count() <=> rhs.timePoint.time_since_epoch().count();
    }

    bool Timestamp::operator==(const Timestamp &rhs) const {
        return timePoint.time_since_epoch().count() == rhs.timePoint.time_since_epoch().count();
    }


    string Timestamp::toString() const {
        return format("{:%Y-%m-%d %H:%M:%S}", timePoint);
    }

    template<size_t N>
        requires(N >= 19)
    size_t Timestamp::toString(array<char, N> &ptr) const {
        auto tp = chrono::time_point_cast<chrono::seconds>(timePoint);
        return std::format_to_n(
            ptr.data(), ptr.size(), "{:%Y-%m-%d %H:%M:%S}", tp
        ).size;
    }

    Timestamp::Duration Timestamp::getDuration() const {
        return timePoint.time_since_epoch();
    }

    Timestamp::TimePoint Timestamp::getTimePoint() const {
        return timePoint;
    }

    Timestamp Timestamp::getNow() {
        return Timestamp(chrono::system_clock::now());
    }

    template<typename Rep, typename Period>
    Timestamp Timestamp::getAfter(const chrono::duration<Rep, Period> dur) {
        return Timestamp(chrono::system_clock::now() + dur);
    }

    export template<typename Rep, typename Period>
    Timestamp operator+(const Timestamp &lhs, const std::chrono::duration<Rep, Period> &dur) {
        return Timestamp(lhs.getTimePoint() + dur);
    }
}

