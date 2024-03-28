#include "rkaiq_manager.h"
#include "rkaiq_media.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_device_id;
extern int g_width;
extern int g_height;
extern int g_rtsp_en;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;
extern std::string iqfile;
extern std::string g_sensor_name;

RKAiqToolManager::RKAiqToolManager()
{
}

RKAiqToolManager::~RKAiqToolManager()
{
}

void RKAiqToolManager::SaveExit()
{
}

int RKAiqToolManager::IoCtrl(int id, void* data, int size)
{
    int retValue = 0;

    LOG_INFO("IoCtrl id: 0x%x\n", id);
    /*
    if (id > ENUM_ID_AWB_START && id < ENUM_ID_AWB_END)
    {
        retValue = AWBIoCtrl(id, data, size);
    }
    else if (id > ENUM_ID_CPROC_START && id < ENUM_ID_CPROC_END)
    {
        retValue = CPROCIoCtrl(id, data, size);
    }
    */
    LOG_INFO("IoCtrl id: 0x%x exit\n", id);
    LOG_INFO("IoCtrl retValue: %d \n", retValue);
    return retValue;
}
