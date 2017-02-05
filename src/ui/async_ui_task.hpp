// *****************************************************************************
//
// ui/async_ui_task.hpp
//
// Async task system to allow running an async task with a user supplied
// callback.
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#ifndef _UI_ASYNCUITASK_HPP_
#define _UI_ASYNCUITASK_HPP_

#include <QtConcurrent/QtConcurrent>
#include <deque>

// -----------------------------------------------------------------------------
//
template <typename Result>
class async_ui_task
{
public:

    template <typename CompletionHandler>
    async_ui_task(CompletionHandler on_complete)
        : on_complete_(std::move(on_complete))
    {
        result_watcher_.connect(
            &result_watcher_,
            &QFutureWatcher<Result>::finished,
            [this](){ task_complete(); });
    }

    template<typename Function, typename... Params>
    void run_or_enqueue(Function fun, Params... params)
    {
        if(work_queue_.size() > 1)
        {
            work_queue_.pop_back();
        }

        work_queue_.push_back(
            [=]()
            {
                return fun(params...);
            }
        );

        if(work_queue_.size() == 1)
        {
            result_watcher_.setFuture(QtConcurrent::run(work_queue_.front()));
        }
    }

private:

    void task_complete()
    {
        on_complete_(result_watcher_.future().result());
        dequeue_and_run();
    }

    void dequeue_and_run()
    {
        work_queue_.pop_front();
        if(!work_queue_.empty())
        {
            result_watcher_.setFuture(QtConcurrent::run(work_queue_.front()));
        }
    }

    QFutureWatcher<Result> result_watcher_;
    std::deque<std::function<Result()>> work_queue_;
    std::function<void(Result)> on_complete_;
};

#endif // _UI_ASYNCUITASK_HPP_
