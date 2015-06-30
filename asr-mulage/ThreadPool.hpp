//#include <thread>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
//#include <future>
#include <mutex>
#include <memory>
//#include <thread>
#include <type_traits>
#include <limits>
#include <functional>

#include "ThreadSafePriorityQueue.hpp"

template <typename F, typename R>
void set_promise_value(boost::promise<F>& promise, boost::function<R()> fn) {
    promise.set_value(fn());
}

template <typename R>
void set_promise_value(boost::promise<void>& promise, boost::function<R()> fn) {
    fn();
    promise.set_value();
}

template <typename T>
class ThreadPool {
    struct Task {
        Task(boost::function<T()> fn_, size_t priority_) : fn(fn_), priority(priority_) { }
        Task(const Task&) = delete;
        bool operator<(const Task& task) const { return (this->priority > task.priority); }

        boost::function<T()> fn;
        boost::promise<T> promise;
        size_t priority;
    };

    std::vector<boost::thread> _threads;
    ThreadSafePriorityQueue<Task> _tasks;

    bool _joined;

public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(size_t threads_num) : _joined(false) {
        while (threads_num--) {
            _threads.emplace_back([this]{
                while (true) {
                    auto taskptr = this->_tasks.wait_and_pop();
                    if (taskptr == nullptr)
                        break;
                    try {
                        set_promise_value(taskptr->promise, taskptr->fn);
                    } catch (...) {
                        taskptr->promise.set_exception(boost::current_exception());
                    }
                }
            });
        }
    }
    
    ~ThreadPool() {
        if (!_joined)
            this->join();
    }
    
    void join() {
        _tasks.stop();
        for (auto &thread : _threads) {
            thread.join();
        }
        _joined = true;
    }

    boost::shared_future<T> enqueue(boost::function<T()> f, size_t priority = std::numeric_limits<size_t>::min()) {
        auto task = std::make_shared<Task>(f, priority);
        _tasks.push(task);
        return task->promise.get_future();
    }
};
