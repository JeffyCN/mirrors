/***********************************************
v1.0.0:
  release init camera hal version.
v1.1.0:
  1. fix some ae bugs
  2. fix manual isp bugs
v1.2.0:
  1. support dump RAW file.
v1.3.0:
  1. isp tunning tool function has been verified.
  2. optimize aec algorithm.
  3. disable lots of info logs.
v1.4.0:
  1. ov2710 & ov4689 optimized IQ
v1.5.0:
  1. hst weight and flt setting are not expected,fix it
  2. update ov4689 xml
v1.6.0:
  1. fix get metadata bug
  2. fix mapSensorExpToHal bug
  3. add imx323 tunning file
  4. fix ae bugs
  5. wdr & adpf can config in tunning file
v1.7.0:
  1. ov2710 & ov4689 WDR and adpf off, GOC on
v1.8.0:
  1. fix calibdb memory leak
  2. CameraBuffer support timestamp
  3. update im3x3 tunning file
v1.9.0:
  1. can set  wb & fliker mode & ae bias
     through user interface
  2. ae was stopped for ov4689 under paticular condition,
     fix it.
v1.a.0:
  1. imx323,ov4689,ov2710 wdr on,goc off
v1.b.0:
  1. use isp_sensor_output_width instead of
     isp_sensor_input_width
v1.c.0:
  1. use different goc curve for wdr on or off
  2. auto detect isp,cif,usb video devices
v1.d.0:
  1.
        IQ: imx323 v0.3.0
            Gamma on.
            WDR on.
            saturation set at [80 80 80 70]
            ov4689 v0.4.0
  2. support isp driver v0.1.7
v1.e.0:
  1.

      AEC: Add AOE algorithm and AOE parameter parse.
     -----------------------------------
      IQ:  imx323 v0.3.1
     Gamma on;Gamma curve change.
     WDR on.
     CC offset set at [0 0 0].
     CC sat set at [100 100 100 80].
     add AOE parameter but still can not enable.
     -----------------------------------
     ov4689 v0.4.1
     Gamma on;Gamma curve change.
     WDR on.
     CC offset set at [0 0 0].
     CC sat set at [100 100 100 90].
     add AOE parameter but still can not enable.
  2. fix calibdb memory leak
v1.e.1:
  1. initial awb mode doesn't take effect,fix it
  2. fix adpf deNoiseLevel pointer bug
  3. sensor connected to isp adapter support fps control
v1.e.2:
  1. add 3A alogrithm result to medta data
v1.e.3:
  1. add open flag O_CLOEXEC for device
  2. support awdr
  3. select IQ file from /tmp/ prior to /etc/cam_iq
  4. update IQ file: imx323 v0.3.5
         ov4689 v0.4.9
v1.e.4:
  1. fix some ae bugs
  2. update tunning files
v1.e.5:
  1. support setting cproc parameters in xml
v1.e.6:
  1. flt_result->grn_stage1 6->8 when flt_cfg->denoise_level is 0;
  2. WDR used default after getAWDRResults, but only modify wdr_gain_max_value, this is temporary!
  3. update ov4689 tunning xml to v55
  4. fix imx323 ecm lock range
v1.e.7:
  1. config 3dnr in xml
  2. fix "QBUF failed" error log
  3. add thread name
  4. commit ipc imx323 xml, customer should select CVR OR IPC xml according to the sensor used,
     and should rename "imx323_LA6114PA_<xxx>.xml" to "imx323_LA6114PA.xml".
  5. usb camera support wb and ae bias setting
v1.e.8:
  1. use AStyle to format codes,
     AStyle.exe --options=c:\zyc\AStyle\file\chromium.ini --exclude=Android.mk -n -z2 -R	
v1.e.9:
  1. add ircut & led control for IPC
  2. can read lightsensor stats from kernel, used to check day or night for IPC
  3. fix 16.04 build errors
  4. modify aec exposure limits for afps
  5. print PU name in no buffer msg
  6. update imx323_LA6271.xml to version 0.03, support 3dnr controls in xml
  7. add Y8/Y10/Y12 format.
v1.e.a:
  1. update imx323_LA6271.xml to version 0.09
  2. add Y8/Y10/Y12 format
  3. fix cppcheck errors and warnings
  4. fix mp & sp concurrent used bug
  5. disable lsc,wb gain, ccm when night mode, gamma changed with wdr.
v1.e.b:
  1. fix detached thread bugs
  2. fix ipc manual wb mode bug
v1.e.c
  1. modify sharp & denoise level value for ipc
v1.e.d
  1. implement ISP brightness,contrast,saturation,hue interfaces
  2. fix awdr bug, awdr havn't been disabled actually in night mode.
  3. modify Copyright info.
  4. update imx323_LA6271.xml to version 0.0c:
  	1) modify filter & sharpness level
  	2) modify whitepoint region condition
v1.e.e
  1. some files missing coypyright info in last version, fix it
  2. apply awb_errcode_metadata.patch 
v1.e.f
  1. TimeDot can be less than 1/flicker.
  2. add wb type HAL_WB_INVAL
v1.f.0
  1. IMX323_LA6271: v0.1.1
v1.f.1
  1. imx323_LA6271: v0.1.2
  2. add fps control in IQ xml
  3. 3dnr: remove old parameters, add new para in IQ xml.
v1.f.2
  1. support YUV range config
v1.f.3
  1. fix dumpsys raw failed
  2. use different gamma curve for night and day mode.
v1.f.4.0
  1. add a version number at the end of version string, HAL user should 
     only update this version number if neccesary.
  2. IMX323_LA6271: v0.1.4
  3. add BufferBase clasee
  4. fix some return value or limits which may cause system stop running
  5. add some items in xml file for 3dnr & gamma
v1.f.5.0
  1. add more paras in xml for demosaic and 3dnr 
v1.f.6.0
  1. modify aec algorithm
  2. fix awdr & adpf errors
  3. disable dsp 3dnr uv denoise when BW mode
v1.f.7.0
  1. fix memory leak in cam_ia_api
  2. modify code & para in IQ xml to get right filter
v1.f.8.0
  1. fix high cpu usage error
***********************************************/
#define CAMHALVERSION "1.f.8.0"
