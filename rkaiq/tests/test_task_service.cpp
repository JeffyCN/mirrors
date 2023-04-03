/*
 * test_task_service.h
 *
 *  Copyright (c) 2021 Rockchip Eletronics Co., Ltd.
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
 */
#include <signal.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include "task_service.h"

using namespace XCam;

class MockParam {};

struct BarTask : public ServiceTask<MockParam> {
    virtual TaskResult operator()(ServiceParam<MockParam>& p) override final {
        std::cout << "run id " << p.payload.get() << "\n" << std::flush;
        int time = std::rand() % 50 + 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return TaskResult::kSuccess;
    }
};

bool stop = false;

void worker_thread(std::shared_ptr<TaskService<MockParam>> arg) {
    std::shared_ptr<TaskService<MockParam>> svc = arg;

    while (!stop) {
        auto a = svc->dequeue();
        std::cout << "worker dequed type " << std::to_string(static_cast<int>(a.state))
                  << " handler " << static_cast<void*>(a.payload.get()) << "\n"
                  << std::flush;
        if (a.state != ParamState::kNull) {
            svc->enqueue(std::move(a));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::shared_ptr<TaskService<MockParam>> svc;

static void signal_handle(int signo) {
    stop = true;
    svc->stop();
}

int main() {
    signal(SIGINT, signal_handle);
    signal(SIGQUIT, signal_handle);
    signal(SIGTERM, signal_handle);

    auto task = std::unique_ptr<ServiceTask<MockParam>>(new BarTask);
    svc = std::make_shared<TaskService<MockParam>>(std::move(task), false);

    svc->setMaxProceedTime(TaskDuration(10));
    svc->setMaxProceedTimeByFps(40);
    svc->start();

    std::thread thr(worker_thread, svc);

    while (!stop) {
        auto p = svc->dequeue();
        std::cout << "main dequed type " << std::to_string(static_cast<int>(p.state)) << " handler "
                  << static_cast<void*>(p.payload.get()) << "\n"
                  << std::flush;
        if (p.state != ParamState::kNull) {
            svc->enqueue(p);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    svc->stop();
    thr.join();

    return 0;
}
