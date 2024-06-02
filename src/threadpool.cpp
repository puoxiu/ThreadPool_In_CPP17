#include <functional>
#include <thread>
#include <iostream>
#include "threadpool.h"


const size_t TASK_MAX_THRESH_HOLD = 1024;
const size_t THREAD_MAX_THRESH_HOLD = 16;
const size_t THREAD_MAX_IDLE_TIME = 5;     // s


ThreadPool::ThreadPool():initThreadSize_(0),taskSize_(0),
                        taskSizeMAXThreshHold_(TASK_MAX_THRESH_HOLD),
                        threadSizeMAXThreshHold_(THREAD_MAX_THRESH_HOLD),
                        mode_(PoolMode::MODE_FIXED), isPoolRunning_(false), idleThreadSize_(0), curThreadSize_(0)
{

}

ThreadPool::~ThreadPool() {
    isPoolRunning_ = false;

    std::unique_lock<std::mutex> locker(taskQueueMtx_);
    notEmpty_.notify_all();
    exitCond_.wait(locker, [&](){
        return threads_.size() == 0;
    });
}


void ThreadPool::setMode(PoolMode mode) {
    if(checkRunningStatus())
        return;
    mode_ = mode;
}

void ThreadPool::setTaskQueueThreshHold(size_t threshhold) {
    taskSizeMAXThreshHold_ = threshhold;

}
void ThreadPool::setThreadSizeThreshHold(size_t threshhold) {
    if(checkRunningStatus() || mode_ == PoolMode::MODE_FIXED) 
        return;
    threadSizeMAXThreshHold_ = threshhold;
}

void ThreadPool::start(size_t initThraedSize) {
    isPoolRunning_ = true;

    initThreadSize_ = initThraedSize;
    curThreadSize_ = initThraedSize;
    idleThreadSize_ = initThraedSize;

    for(int i = 0; i < initThreadSize_; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        threads_[ptr->getThreadID()] = std::move(ptr);
        // threads_.emplace_back(std::move(ptr));
        // threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this)));
    }
    // 启动所有线程
    for(int i = 0; i < initThreadSize_; i++) {
        threads_[i]->start();
    }
}

// 各线程从任务队列中取出任务，相当于消费者
void ThreadPool::threadFunc(size_t threadID) {
    auto lastTime = std::chrono::high_resolution_clock().now();
    while(true) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> locker(taskQueueMtx_);
            std::cout << std::this_thread::get_id() <<  "等待获取任务." << std::endl;

            while(taskQueue_.size() == 0) {
                if(!isPoolRunning_) {
                    threads_.erase(threadID);
                    exitCond_.notify_all();
                    std::cout << "thread exit, id = " << std::this_thread::get_id() << std::endl;
                    return;
                }

                if(mode_ == PoolMode::MODE_CACHE) {
                    if(std::cv_status::timeout == notEmpty_.wait_for(locker, std::chrono::seconds(1))) {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);

                        if(dur.count() >= THREAD_MAX_IDLE_TIME && curThreadSize_ > initThreadSize_) {
                            // 回收当前线程
                            threads_.erase(threadID);
                            curThreadSize_--;
                            idleThreadSize_--;
                            std::cout << "thread exit, id = " << std::this_thread::get_id() << std::endl;
                            return;
                        }
                    }
                }
                else {
                    // notEmpty_.wait(locker, [&](){
                    //     return !taskQueue_.empty();
                    // });
                    notEmpty_.wait(locker);
                }
                // if(!isPoolRunning_) {
                //     threads_.erase(threadID);
                //     exitCond_.notify_all();
                //     std::cout << "thread exit, id = " << std::this_thread::get_id() << std::endl;
                //     return;
                // }
            }
            

            idleThreadSize_ --;
            task= taskQueue_.front();    
            taskQueue_.pop();
            taskSize_--;
            if(taskQueue_.size() > 0) {
                notEmpty_.notify_all();
            }
            notFull_.notify_all();
        } // 释放锁 

        // 执行任务
        if(task != nullptr)
            // task->run();
            task->exec();
        lastTime = std::chrono::high_resolution_clock().now();
        idleThreadSize_++;
    }

}

// 提交用户任务给任务队列，相当于生产者
Result ThreadPool::submitTask(std::shared_ptr<Task> task) {
    std::unique_lock<std::mutex> locker(taskQueueMtx_);
    if(!notFull_.wait_for(locker,std::chrono::seconds(1),[&]()->bool{
        return taskQueue_.size() < taskSizeMAXThreshHold_;
    })) {
        // 超时
        std::cerr << "task queue is full, submit task fail." << std::endl;
        return Result(task, false);
    }

    taskQueue_.emplace(task);
    taskSize_++;
    // notEmpty_.notify_one();
    notEmpty_.notify_all();

    // cache模式
    if(mode_ == PoolMode::MODE_CACHE && taskSize_ > idleThreadSize_ && curThreadSize_ < threadSizeMAXThreshHold_) {
        
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        auto id = ptr->getThreadID();
        threads_[id] = std::move(ptr);
        threads_[id]->start();
        curThreadSize_++;
        idleThreadSize_++;
    }


    return Result(task);
}


bool ThreadPool::checkRunningStatus() const{
    return isPoolRunning_;
}


size_t ThreadPool::getThreadSize() const {
    return curThreadSize_;
}