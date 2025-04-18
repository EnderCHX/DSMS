//
// Created by c on 2025/4/16.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

namespace CHX {
class ThreadPool {
    public:
        // 构造函数，传入线程数
        ThreadPool(size_t);
        // 入队任务(传入函数和函数的参数)
        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args)
            -> std::future<std::invoke_result_t<F(Args...)>>;   //利用尾置限定符  std::future用来获取异步任务的结果
        // 析构
        ~ThreadPool();
    private:
        // 工作线程组 need to keep track of threads so we can join them
        std::vector< std::thread > workers;
        // 任务队列 the task queue
        std::queue< std::function<void()> > tasks;

        // 异步 synchronization
        std::mutex queue_mutex; // 队列互斥锁
        std::condition_variable condition;  // 条件变量
        bool stop;	// 停止标志
};

}

#endif //THREAD_POOL_H
