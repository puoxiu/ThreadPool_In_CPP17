#pragma once

#include <functional>
#include <atomic>
#include <any>
#include <thread>
#include "mySemphore.h"


using std::any;

class Result;

// 线程类
class Thread {
public:
    using ThreadFunc = std::function<void(size_t)>;

    Thread(ThreadFunc func);
    ~Thread();


    void start();
    size_t getThreadID() const;

private:
    ThreadFunc func_;
    size_t threadID_;
    static size_t generatID;
};

// 任务基类
class Task {
public:
    Task() ;
    ~Task() =default;
    void exec();
    void setResult(Result *res);
    virtual any run() = 0;
private:
    Result *result_;
};

using ulong = unsigned long ;
// 用户任务类
class MyTask : public Task {
public:
    MyTask(ulong begin, ulong end): begin_(begin), end_(end) { }

    any run() override ;
private:
    ulong begin_;
    ulong end_;
};


// 提交到线程池的执行返回结果类
class Result {
public:
    Result(std::shared_ptr<Task> task, bool isValid = true) ;
    ~Result() = default;

    void setVal(any any);

    any get();
private:
    any any_;
    Semaphore sem_;
    std::shared_ptr<Task> task_;  // 指向对应任务对象
    std::atomic_bool isValid_;  
};



