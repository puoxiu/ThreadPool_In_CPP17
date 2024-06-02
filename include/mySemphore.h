#pragma once


#include <memory>
#include <mutex>
#include <condition_variable>


class Semaphore {
public:
    Semaphore(int p = 0): p_(p) {};
    ~Semaphore() = default;

    // p
    void wait() {
        std::unique_lock<std::mutex> locker(mtx_);
        conv_.wait(locker, [&](){
            return p_ > 0;
        });
        p_--;
    }
    // v
    void post() {
        std::unique_lock<std::mutex> loacker(mtx_);
        p_++;
        conv_.notify_all();
    }

private:
    int p_;

    std::mutex mtx_;
    std::condition_variable conv_;
};
