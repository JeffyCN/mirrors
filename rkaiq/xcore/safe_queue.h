/*
 * safe_queue.h - safe queue template
 *
 *  Copyright (c) 2022 Rockchip ISP Team
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

#ifndef ROCKCHIP_ISP_SAFE_QUEUE_H
#define ROCKCHIP_ISP_SAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

namespace RkCam {

template <typename T>
class SafeQueue {
 public:
    SafeQueue()  = default;
    ~SafeQueue() = default;

    void push(T&& t) {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            q_.push_back(t);
        }
        cv_.notify_one();
    }

    T& front() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [&]() { return !q_.empty(); });
        return q_.front();
    }

    void pop() {
        std::lock_guard<std::mutex> lk(mtx_);
        q_.pop_front();
    }

    void clear() {
        std::lock_guard<std::mutex> lk(mtx_);
        q_.clear();
    }

 private:
    std::deque<T> q_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

}  // namespace RkCam

#endif  // ROCKCHIP_ISP_SAFE_QUEUE_H
