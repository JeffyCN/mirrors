#ifndef _TOOL_RKAIQ_API_MANAGER_H_
#define _TOOL_RKAIQ_API_MANAGER_H_

#include <memory>
#include <thread>

#include "logger/log.h"
#include "rkaiq_cmdid.h"

class RKAiqToolManager
{
  public:
    RKAiqToolManager();
    virtual ~RKAiqToolManager();
    int IoCtrl(int id, void* data, int size);
    void SaveExit();

  private:
    std::string iqfiles_path_;
    std::string sensor_name_;
    std::thread* rkaiq_engine_thread_;
    static int thread_quit_;
};

#endif // _TOOL_RKAIQ_API_MANAGER_H_
