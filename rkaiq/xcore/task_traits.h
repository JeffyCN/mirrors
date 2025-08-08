/*
 * task_taits.h
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
#ifndef TASK_TRAITS_H
#define TASK_TRAITS_H

#include <chrono>
#include <memory>

namespace XCam {

enum class TaskResult {
    kSuccess,  // Output valid
    kAgain,    // Need more inputs
    kSkipped,  // The result is skipped
    kFailed,   // Process failed
};

template <typename R, typename... Ts>
struct Task {
    R operator()(Ts... args);
};

enum class ParamState {
    kNull,              // Holds empty value
    kAllocated,         // Initial state, allocated but no content
    kReadyForProcess,   // Filled with content, waiting to be processed
    kProcessing,        // Being proccessing by task
    kProcessedError,    // Be Processed but failed
    kProcessedSuccess,  // Be Processed, waiting to be dequeued
    kProcessedDequed,   // Be processed and dequed, canbe either error or
                        // success,
    kMaxState,
};
// template <typename T>
// using ServiceParam = std::pair<ParamState, std::shared_ptr<T>>;

template <typename T>
struct ServiceParam {
    ParamState state;            // params state
    int32_t unique_id;           // same as frame id
    std::shared_ptr<T> payload;  // holds actual params
};

template <typename T>
struct ServiceTask : public Task<TaskResult, ServiceParam<T>> {
    ServiceTask() = default;
    // Allow inherit class to dtor
    virtual ~ServiceTask() = default;
    virtual TaskResult operator()(ServiceParam<T>& p) { return TaskResult::kSkipped; }
};

using TaskDuration  = std::chrono::duration<double, std::milli>;
using TaskTimePoint = std::chrono::time_point<std::chrono::system_clock>;

constexpr uint8_t default_max_param_count   = 5;
constexpr TaskDuration default_process_time = TaskDuration(33);
constexpr bool default_may_block            = false;

}  // namespace XCam

#endif  // TASK_TRAITS_H
