#include <condition_variable>
#include <mutex>
#include <queue>
#include <iostream>
#include <random>
#include <cmath>

#define HOW_MANY_DESKS 5
#define HOW_MANY_CUSTOMERS 30

class logger {
    std::mutex m_mt;

protected:
    logger() = default;

public:
    static logger &instance() {
        static logger lg;
        return lg;
    }

    logger(logger const &) = delete;

    logger &operator=(logger const &) = delete;

    void log(std::string_view mess) {
        std::lock_guard<std::mutex> lock(m_mt);
        std::cout << "LOG: " << mess << "\n";
    }
};


class ticketing_machine {
    int m_first_ticket{};
    int m_last_ticket{};

public:
    explicit ticketing_machine(int const start)
        : m_first_ticket(start)
          , m_last_ticket(start) {
    }

    int next() { return m_last_ticket++; }
    [[nodiscard]] int last() const { return m_last_ticket - 1; }
    void reset() { m_last_ticket = m_first_ticket; }
};

class customer {
    int m_number{};

    friend bool operator<(customer const &a, customer const &b);

public:
    explicit customer(int const n)
        : m_number(n) {
    }

    [[nodiscard("some info")]] int get_number() const noexcept { return m_number; }
};

bool operator<(customer const &a, customer const &b) {
    return a.m_number > b.m_number;
}

int main() {
    std::priority_queue<customer> customers;
    bool is_office_open{true};
    std::mutex mt;
    std::condition_variable sv;

    std::vector<std::thread> desks;
    for (int i{1}; i <= HOW_MANY_DESKS; ++i) {
        desks.emplace_back([&]() {
            std::random_device rd;
            auto seed = std::array<int, std::mt19937::state_size>{};
            std::generate(std::begin(seed), std::end(seed), std::ref(rd));
            std::seed_seq seq(std::begin(seed), std::end(seed));
            std::mt19937 end(seq);
            std::uniform_int_distribution<> uid(2000, 3000);

            logger::instance().log("desk " + std::to_string(i) + " open.");

            while (is_office_open || !customers.empty()) {
                std::unique_lock<std::mutex> locker(mt);

                sv.wait_for(locker, std::chrono::seconds(1), [&customers]() { return !customers.empty(); });

                if (!customers.empty()) {
                    auto top = customers.top();
                    customers.pop();

                    logger::instance().log(
                        "- desks" + std::to_string(i) + " handling customer: " + std::to_string(top.get_number()));

                    logger::instance().log("Queue size: " + std::to_string(customers.size()));

                    locker.unlock();
                    sv.notify_one();
                    std::this_thread::sleep_for(std::chrono::milliseconds(uid(end)));

                    logger::instance().log(
                        "desk " + std::to_string(i) + " fulfilled customer " + std::to_string(top.get_number()));
                }
            }
            logger::instance().log("desk " + std::to_string(i) + " closed");
        });
    }

    std::thread store([&]() {
        ticketing_machine tm(100);
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data),
                      std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 end(seq);
        std::uniform_int_distribution<> uid(200, 500);

        for (int i{1}; i <= HOW_MANY_CUSTOMERS; ++i) {
            customer c(tm.next());
            customers.push(c);

            logger::instance().log("+ new customer with ticket " + std::to_string(c.get_number()));
            logger::instance().log("Queue size: " + std::to_string(customers.size()));

            sv.notify_one();
            std::this_thread::sleep_for(std::chrono::milliseconds(uid(end)));
        }
        is_office_open = false;
    });
    store.join();
    for (auto &i: desks) i.join();
}




