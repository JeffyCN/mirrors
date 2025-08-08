/*
 * task_service.h
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#ifndef XCORE_TASK_SERVICE_H
#define XCORE_TASK_SERVICE_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include "task_traits.h"

#define DEBUG 1
#ifndef TASK_LOG
#define TASK_LOG(format, ...) \
    printf("DEBUG: " #format "\n", ##__VA_ARGS__)
#endif

namespace XCam {

// SERVICE CONCEPTS:
// * A aynchronous threaded service to execute specific tasks that deal with
//   INPUTS and OUTPUTS parameters
// * Service will calculate the MAX_PROCESS_TIME of the task consumed
// * Support run ONCE/SYNC/ASYNC on a task
//   * ASYNC: Run task in a threaded loop
//   * SYNC: Call ENQUEUE/DEQUEUE synchronous by user
//   * ONCE: RunOnce for one param input
// * Support ENQUEUE/DEQUEUE methods
//   * ENQUEUE will signal loop immediately to run task
//   * DEQUEUE will wait until results not empty and the MAY_BLOCK is true
//   * DEQUEUE may return empty param if no result and the MAY_BLOCK is false
// * Support buffer overrun/underrun behaviors on
//   * BUSY (fifo full)
//   * IDLE (fifo empty for longer than 2 period)
//   * LATE (execution time longer than 1 period)
//   * ERROR (process error)
// * Support config and config change event ???
//   to negotiate with other service ???
//
// * Users can ENQUEUE new params, DEQUEUE PROCESSED/ERROR/SKIPPED params
// * Users can trigger START, STOP the service
// * Users can FLUSH the proccessing queue, mark param to SKIPPED
// * Users can specific the MAXIMUM PROCESS TIME
//
// * Parameters are UNIQUE, FIFO even on ERROR/LATE happened
// * Parameters have a BUFFER POOL, with MAX_PARAM_COUNT
// * Parameters can be ALLOCATED inside or outside of Service
// * Last result will be HOLD A COPY if current task failed
//
// * Task is a FUNCTOR object
// * Task does NOT OWN the param
// * Task may HOLD several input buffers
// * Task may return process result
//   * SUCCESS - returned, successful processed
//   * SKIPPED - returned, but this param skipped
//   * FAILED - returned, but failed in processing
//   * AGAIN - required more inputs
// * Task inputs and outputs may have different id
// * Task outputs may have inputs param
//

template <typename T, class Container = std::deque<ServiceParam<T>>>
class TaskService {
 public:
    typedef TaskService<T> value_type;
    typedef ServiceTask<T> task_type;
    typedef ServiceParam<T> param_type;

    TaskService()                      = delete;
    TaskService(const TaskService<T>&) = delete;
    const TaskService<T>& operator=(const TaskService<T>&) = delete;

    explicit TaskService(std::unique_ptr<ServiceTask<T>> task,
                         const bool may_block                = default_may_block,
                         const uint8_t max_param_count       = default_max_param_count,
                         const TaskDuration max_process_time = default_process_time)
        : max_param_count_(max_param_count),
          max_process_time_(max_process_time),
          may_block_(may_block),
          started_(false),
          task_(std::move(task)) {
        assert(task_.get() != nullptr);
        allocServiceParam();
    }

    virtual ~TaskService() {
        stop();
        clear();
    }

    void loop() {
        while (started_) {
            std::unique_lock<std::mutex> in_lock(in_mutex_);
            auto wait_ret = in_cond_.wait_for(in_lock, max_process_time_, [this]() {
                return (!started_ || (!in_params_.empty() &&
                                      in_params_.front().state == ParamState::kReadyForProcess));
            });
            if (wait_ret == false) {
                // IDLE
                continue;
            }
            if (!started_) break;

            auto param = std::move(in_params_.front());
            in_params_.pop_front();
            in_lock.unlock();
            param.state = ParamState::kProcessing;
            const auto start = std::chrono::steady_clock::now();

            auto ret = (*task_)(param);
            if (ret == TaskResult::kSuccess) {
                param.state = ParamState::kProcessedSuccess;
            } else {
                param.state = ParamState::kProcessedError;
                TASK_LOG("task processs failed");
#ifdef USE_TASK_CALLBACK
                callbacks_.onError();
#endif
            }

            {
                std::lock_guard<std::mutex> out_lock(out_mutex_);
                // TODO(Cody): if we have alllocated params in front,
                // this will block the result until these params are
                // dequeued. if we use push_front, we break the sequence
                // of params
                out_params_.push_back(std::move(param));
                out_cond_.notify_one();
            }

            const auto end = std::chrono::steady_clock::now();

            const std::chrono::duration<double, std::milli> elapsed = end - start;
            if (elapsed >= max_process_time_) {
                // TODO(Cody): match the frame id ?
                TASK_LOG("params processs elapsed %lf exceeds %lf", elapsed.count(),
                     max_process_time_.count());
#if USE_TASK_CALLBACK
                callbacks_.onLate();
#endif
            }
        }
    }

    void start() {
        if (started_) {
            return;
        }
        started_  = true;
        executor_ = std::thread([this] { loop(); });
        // executor_.detach();
#if USE_TASK_CALLBACK
        callbacks_.onStarted();
#endif
    }

    void stop() {
        if (started_) {
            started_ = false;
            executor_.join();
        }
#if USE_TASK_CALLBACK
        callbacks_.onStoped();
#endif
    }

    void allocServiceParam() noexcept {
        std::lock_guard<std::mutex> lock(out_mutex_);
        while (out_params_.size() < max_param_count_) {
            // TODO(Cody): Must add try block if -fexceptions enabled
            // Ensure new object does not throw any exceptions
            ServiceParam<T> param;
            param.state = ParamState::kAllocated;
            param.unique_id = -1;
            param.payload = std::make_shared<T>();
            out_params_.push_back(std::move(param));
            out_cond_.notify_one();
        }
    }

    void enqueue(ServiceParam<T>& input) {
        std::lock_guard<std::mutex> lock(in_mutex_);
        // TODO(Cody): deal with busy case
#if 0
        if (in_params_.size() == max_param_count_) {
            TASK_LOG("input params exceeds max count!");
#if USE_TASK_CALLBACK
            callbacks_.onFull(input);
#endif
            return;
        }
#endif
        input.state = ParamState::kReadyForProcess;
        in_params_.push_back(input);
        in_cond_.notify_one();
    }

    void enqueue(ServiceParam<T>&& input) {
        std::lock_guard<std::mutex> lock(in_mutex_);
        // TODO(Cody): deal with busy case
#if 0
        if (in_params_.size() == max_param_count_) {
            TASK_LOG("input params exceeds max count!");
#if USE_TASK_CALLBACK
            callbacks_.onFull(input);
#endif
            return;
        }
#endif
        input.state = ParamState::kReadyForProcess;
        in_params_.push_back(std::move(input));
        in_cond_.notify_one();
    }

    ServiceParam<T> dequeue() {
        std::unique_lock<std::mutex> lock(out_mutex_);
        out_cond_.wait(lock, [this]() {
            if ((!out_params_.empty() &&
                 (out_params_.front().state == ParamState::kAllocated ||
                  out_params_.front().state == ParamState::kProcessedSuccess ||
                  out_params_.front().state == ParamState::kProcessedError))) {
                return true;
            } else if (!may_block_) {
                ServiceParam<T> empty = {
                    .state = ParamState::kNull,
                    .unique_id = -1,
                    .payload = std::shared_ptr<T>(nullptr),
                };
                out_params_.push_front(std::move(empty));
                return true;
            } else {
                return false;
            }
        });
        auto p = std::move(out_params_.front());
        out_params_.pop_front();
        return p;
    }

    void flush() {
        std::lock(in_mutex_, out_mutex_);
        std::lock_guard<std::mutex> in_lock(in_mutex_, std::adopt_lock);
        std::lock_guard<std::mutex> out_lock(in_mutex_, std::adopt_lock);
        if (in_params_.empty()) {
            return;
        }
        auto paramIt =
            std::remove_if(in_params_.begin(), in_params_.end(), [](const ServiceParam<T>& p) {
                return (p.state != ParamState::kReadyForProcess) &&
                       (p.state != ParamState::kAllocated);
            });
        for (auto param = paramIt; param != in_params_.end(); param++) {
            param.state = ParamState::kAllocated;
            param.unique_id = -1;
            out_params_.push_back(param);
            out_cond_.notify_one();
        }
    }

    void clear() {
        std::lock(in_mutex_, out_mutex_);
        std::lock_guard<std::mutex> in_lock(in_mutex_, std::adopt_lock);
        std::lock_guard<std::mutex> out_lock(in_mutex_, std::adopt_lock);
        in_params_.clear();
        out_params_.clear();
    }

    uint8_t getMaxParamCount() const { return max_param_count_; }
    void setMaxParamCount(const uint8_t count) { max_param_count_ = count; }

    const TaskDuration getMaxProceedTime() { return max_process_time_; }
    void setMaxProceedTime(const TaskDuration duration) { max_process_time_ = duration; }
    void setMaxProceedTimeByFps(const int fps) {
        max_process_time_ = TaskDuration(std::chrono::milliseconds(1000 / fps));
    }

    // TODO(Cody): add support link to other services
#if 0
    template <typename O>
    friend void link(TaskService<O>* other);
    template <typename O>
    friend void unlink(TaskService<O>* other);
#endif

 private:
    uint8_t max_param_count_;
    TaskDuration max_process_time_;
    bool may_block_;
    bool started_;

    std::mutex in_mutex_;
    std::condition_variable in_cond_;
    std::mutex out_mutex_;
    std::condition_variable out_cond_;
    std::unique_ptr<ServiceTask<T>> task_;
    std::thread executor_;

    Container in_params_;
    Container out_params_;

#if USE_TASK_CALLBACK
    TaskCallbacks callbacks_;
#endif
};

}  // namespace XCam

#endif  // ALGOS_AEIS_TASK_SERVICE_H
