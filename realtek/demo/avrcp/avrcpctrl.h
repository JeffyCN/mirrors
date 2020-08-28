#ifndef AVRCPCTRL_H
#define AVRCPCTRL_H
//play status 
#define AVRCP_PLAY_STATUS_STOPPED	0x00 // 停止
#define AVRCP_PLAY_STATUS_PLAYING	0x01 //正在播放
#define AVRCP_PLAY_STATUS_PAUSED	0x02 //暂停播放
#define AVRCP_PLAY_STATUS_FWD_SEEK	0x03 //快进
#define AVRCP_PLAY_STATUS_REV_SEEK	0x04 //重播
#define AVRCP_PLAY_STATUS_ERROR		0xFF //错误状态


#ifdef __cplusplus
extern "C" {
#endif
    /**
    * 初始化 蓝牙音频反向控制模块
    */
    int init_avrcp_ctrl();
    /**
    * 释放蓝牙音频反向控制相关资源
    */
    int release_avrcp_ctrl();
    /**
    * 播放
    */
    void play_avrcp();
     /**
    * 暂停播放
    */
    void pause_avrcp();
    /**
    * 停止播放
    */
    void stop_avrcp();
    /**
    * 下一首
    */
    void next_avrcp();
    /**
    * 上一首
    */
    void previous_avrcp();
    /**
    * 获取当前蓝牙音频状态
    */
    int getstatus_avrcp();

#ifdef __cplusplus
} /* extern "C" */
#endif /* C++ */

#endif 
