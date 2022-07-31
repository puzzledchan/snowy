/**
 * @file ThreadPool.hpp
 * @author JDongChen
 * @brief 线程池
 * @version 0.1
 * @date 2022-07-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>

#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>

#include <functional>
#include <future>

#include <stdexcept>


class ThreadPool {
private:
    unsigned short _initSize;       //初始化线程数量
    bool _autoGrow; //
    unsigned short _maxThrNum; //

    using Task = std::function<void()>; // 任务类型
    std::vector<std::thread> _pool; // 线程池
    std::queue<Task> _tasks; // 任务队列

    std::mutex _lockTask; // 任务队列锁
    std::mutex _lockGrow;  // 自动增长锁
    std::condition_variable _task_cv;

    std::atomic<bool> _running{ true };     //线程池是否执行
    std::atomic<int>  _idlThrNum{ 0 };  //空闲线程数量



public:
    ThreadPool(unsigned short poolSize, bool autoGrow = true, unsigned short maxthreads = 16) {
        _initSize = poolSize;
        _autoGrow = autoGrow;
        _maxThrNum = maxthreads;
        _AddThread(_initSize);
    } 
    ~ThreadPool() {
        _running = false;
        _task_cv.notify_all(); // 唤醒
        for(std::thread& thread : _pool) {
            //thread.detach(); // 让线程 自我处理
            if (thread.joinable())
                thread.join(); // 等待任务结束， 前提：线程一定会执行完
        }
    };     //

public:
    template<class F, class... Args>
    auto Submit(F&& f, Args&&... args)-> std::future<decltype(f(args...))> {
        if(!_running) {
            throw std::runtime_error("Submit on ThreadPool is stopped.");
        }
        using RetType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock(_lockTask);
            _tasks.emplace([task](){
               (*task)();
            });
        }
        if(_autoGrow && (_idlThrNum < 1 && _pool.size() < _maxThrNum)) {
            this->_AddThread(1);
        }

        _task_cv.notify_one(); // 唤醒一个工作线程
        return future;
    }

    int GetIdlCount() { return _idlThrNum; }
    int GetThrCount() { return _pool.size(); }

private:
    void _AddThread(unsigned short size) {
        if (!_running)    // stoped ??
            throw std::runtime_error("Grow on ThreadPool is stopped.");
        std::unique_lock<std::mutex> lockGrow{ _lockGrow }; //自动增长锁

        for(;_pool.size() < _maxThrNum && size > 0; size --) {

            std::thread thr([this](){this->_WorkRoutine();});
            _pool.emplace_back(std::move(thr));

            {
                std::unique_lock<std::mutex> gurad{_lockTask};
                _idlThrNum++;
            }
        }

    }
    /**
     * @brief 线程循环、等待任务
     * 
     */
    void _WorkRoutine() {
        while(true) {
            Task task;
            {
                std::unique_lock<std::mutex> guard{_lockTask};  
                _task_cv.wait(guard,[this] {
                            return !_running || !_tasks.empty();
                });  
                if(!_running && _tasks.empty()) {
                    return;         // 停止运行 且 任务队列为空线程池返回
                } 
                _idlThrNum--;
                task = std::move(_tasks.front());
                _tasks.pop();       
            }

            task();

            if (_idlThrNum > 0 && _pool.size() > _initSize) {//支持自动释放空闲线程,避免峰值过后大量空闲线程
                return;
            }
            
            {
                std::unique_lock<std::mutex> guard{_lockTask}; 
                _idlThrNum++;
            }
        }
    }
};

#endif 