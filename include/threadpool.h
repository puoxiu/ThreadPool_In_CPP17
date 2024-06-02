#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "thread.h"


enum class PoolMode {
    MODE_FIXED,
    MODE_CACHE,
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void start(size_t initThraedSize = std::thread::hardware_concurrency());    // 开始运行-创建并初始化所有线程
    void setMode(PoolMode mode);        // 设置工作模式
    void setTaskQueueThreshHold(size_t threshhold);
    void setThreadSizeThreshHold(size_t threshhold);

    size_t getThreadSize() const;

    Result submitTask(std::shared_ptr<Task> task);        // 提交任务给任务队列


    ThreadPool(const ThreadPool&) = delete;             // 禁止拷贝构造和赋值构造
    ThreadPool& operator= (const ThreadPool&) = delete;

private:
    // 线程函数
    void threadFunc(size_t threadID);

    bool checkRunningStatus() const;

private:
    // std::vector<std::unique_ptr<Thread>> threads_;
    std::unordered_map<size_t, std::unique_ptr<Thread>> threads_;       // 所有线程

    size_t initThreadSize_;                 // 初始化线程数量
    size_t threadSizeMAXThreshHold_;        // 最大线程数量
    std::atomic_size_t idleThreadSize_;        // 空闲线程数量
    std::atomic_size_t curThreadSize_;         // 当前线程总数量
    

    // std::queue<Task*> tasks;
    std::queue<std::shared_ptr<Task>> taskQueue_;   // 任务队列
    std::atomic_uint taskSize_;
    size_t taskSizeMAXThreshHold_;       // 任务队列数量最大阈值

    // 线程安全
    std::mutex taskQueueMtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    std::condition_variable exitCond_;
    
    PoolMode mode_;
    std::atomic_bool isPoolRunning_;
};

