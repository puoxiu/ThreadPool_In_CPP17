#include "threadpool.h"
#include <chrono>
#include <thread>
#include <iostream>
// #include <memory>
// #include <any>

// 测试代码
void test() {
    ThreadPool *pool = new ThreadPool();
    pool->setMode(PoolMode::MODE_CACHE);

    pool->start();
    
    Result res1 = pool->submitTask(std::make_shared<MyTask>(1,100));
    Result res2 = pool->submitTask(std::make_shared<MyTask>(101,200));
    Result res3 = pool->submitTask(std::make_shared<MyTask>(201,300));
    Result res4 = pool->submitTask(std::make_shared<MyTask>(301,400));
    Result res5 = pool->submitTask(std::make_shared<MyTask>(201,300));
    Result res6 = pool->submitTask(std::make_shared<MyTask>(301,400));
    Result res7 = pool->submitTask(std::make_shared<MyTask>(201,300));
    Result res8 = pool->submitTask(std::make_shared<MyTask>(301,400));
    Result res9 = pool->submitTask(std::make_shared<MyTask>(301,400));

    auto sum1 = std::any_cast<int>(res1.get());
    auto sum2 = std::any_cast<int>(res2.get());
    auto sum3 = std::any_cast<int>(res3.get());
    auto sum4 = std::any_cast<int>(res4.get());
    std::cout << sum1 + sum2 + sum3 + sum4<< std::endl;

    std::cout <<"count : " <<  pool->getThreadSize() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(7));
    std::cout <<"count : " <<  pool->getThreadSize() << std::endl;

    delete pool;
}

int main()
{
    test();

    std::cout <<"main over!" << std::endl;
    return 0;
}
