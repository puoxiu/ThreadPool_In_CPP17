# ThreadPool_In_CPP17

## 线程池特点
* 使用智能指针管理资源，防止资源泄漏
* 基于C++17,Any封装任务返回值以实现不同类型返回值的包装
* 基于互斥锁、原子类型atomic、条件变量、信号量实现线程间通信
* 实现两种运行方式：cahce-线程数量可变、fixed-线程数量固定

## 使用方式

* 只作测试/学习则可直接编译运行

* 依据Task接口实现自己的任务类，class MyTask : public Task
* 修改main函数测试代码

## 运行

### 我的编译环境

* OS：Deepin23
* Linux内核版本：6.1.32-amd64-desktop-hwe
* g++版本：13.2.0

### 编译

```shell
mkdir build
cd build
cmake .
make
```

### 运行

```c++
./threadPool.out
```

