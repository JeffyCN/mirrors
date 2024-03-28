/*
 * unique_fd.h - A unique file descriptor implementation
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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
#ifndef _UNIQUE_FD_H_
#define _UNIQUE_FD_H_

#include <unistd.h>

namespace XCam {

class UniqueFd final {
 public:
    UniqueFd() : fd_(-1) {}
    explicit UniqueFd(int fd) : fd_(fd) {}

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    UniqueFd& operator=(UniqueFd&& rhs) {
        fd_ = Set(rhs.Release());
        return *this;
    }

    ~UniqueFd() {
        if (fd_ > 0) {
            close(fd_);
        }
    }

    int Release() {
        int old_fd = fd_;
        fd_        = -1;
        return old_fd;
    }

    int Get() const { return fd_; }

    int Set(int fd) {
        if (fd_ >= 0) {
            close(fd_);
        }
        fd_ = fd;
        return fd_;
    }

 private:
    int fd_;
};

}  // namespace XCam

#endif  // _UNIQUE_FD_H_
