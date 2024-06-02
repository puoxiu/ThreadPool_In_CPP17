#include "thread.h"

#include <iostream>


Thread::Thread(ThreadFunc func): func_(func), threadID_(generatID++) {

}

Thread::~Thread() {

}
size_t Thread::generatID = 0;

void Thread::start() {
    std::thread t(func_, threadID_);
    t.detach();             // 设置分离线程
}

size_t Thread::getThreadID() const {
    return threadID_;
}


Task::Task() : result_(nullptr){

}
void Task::exec() {
    if(result_ != nullptr)
        result_->setVal(run());
    
}

void Task::setResult(Result *res) {
    result_ = res;
}

// MyTask
any MyTask::run() {    
    std::cout << std::this_thread::get_id() <<  "正在执行任务" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    int sum = 0;
    for(int i = begin_; i <= end_; i++) {
        sum += i;
    }

    return sum;
}

// Result
Result::Result(std::shared_ptr<Task> task, bool isValid ) : task_(task), isValid_(isValid) {
    task_->setResult(this);
}

void Result::setVal(any any) {
    any_ = std::move(any);
    // 任务执行结束--p操作
    sem_.post();
}

any Result::get() {
    if(!isValid_) {
        return "";
    }
    sem_.wait();        // 等待任务执行结束
    return std::move(any_);
}