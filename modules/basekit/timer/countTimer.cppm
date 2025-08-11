module;

export module basekit:countTimer;
import <chrono>;
import <functional>;

import :timestamp;

using namespace std;

namespace basekit {
    export class CountTimer {
    public:
        CountTimer(Timestamp timestamp, function<void()> cb);

        template<typename Rep, typename Period>
        CountTimer(Timestamp timestamp, function<void()> cb, chrono::duration<Rep, Period> _interval);

        void reStart(const Timestamp &now);

        void run() const;

        [[nodiscard]] const Timestamp &getExpiration() const;

        [[nodiscard]] bool isRepeat() const;

    private:
        Timestamp expiration;
        function<void()> callback;
        Timestamp::Duration interval;
        bool repeat;
    };

    CountTimer::CountTimer(const Timestamp timestamp, function<void()> cb)
        : expiration(timestamp),
          callback(std::move(cb)),
          interval(Timestamp::Duration::zero()),
          repeat(interval.count() > 0) {
    }

    template<typename Rep, typename Period>
    CountTimer::CountTimer(const Timestamp timestamp, function<void()> cb, chrono::duration<Rep, Period> _interval)
        : expiration(timestamp),
          callback(std::move(cb)),
          interval(duration_cast<Timestamp::Duration>(_interval)),
          repeat(interval.count() > 0) {
    }


    void CountTimer::reStart(const Timestamp &now) { expiration = now + interval; }

    void CountTimer::run() const { callback(); }

    const Timestamp &CountTimer::getExpiration() const { return expiration; }

    bool CountTimer::isRepeat() const { return repeat; }
}

