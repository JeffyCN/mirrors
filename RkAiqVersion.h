/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_VERSION_H_
/*!
 * ==================== AIQ VERSION HISTORY ====================
 *
 * v0.0.9
 *  - initial version, support HDR
 *  - optimized tnr,nr in low light condition
 *  - optimized imx347 iq
 *  - FEC,ORB,LDCH not enabled
 *  - set compile optimized level to Os
 * v0.1.0
 *  - optimize nr quality under low light
 *  - optimize hdr image quality
 *  - fix circular flicker of image luma
 *  - add fec and ldch, default disabled
 * v0.1.1
 *  - fix some bugs of bayer nr, ynr, uvnr
 *  - optimize imx347 hdr mode image quality
 *  - workaround for imx347 ae flicker when the
 *    luma changed dramatically
 * v0.1.2
 *  - hdr: fix clipRatio error,and redefine tmo params
 *  - fix some bugs in ae smoot
 *  - fix high iso blc problem and uvnr / 3dnr parameters
 *  - fix mmap memory leak
 *  - fix gic bugs
 *  - add color saturation level and color inhibition level setting and getting in accm module
 *  - update imx347 and os04a10 xml
 *  - add gc4c33 xml
 * v0.1.3
 *  - IMX347: split iqfile to hdr and normal, enable fec default
 *  - add dcg setting in aiq
 *  - ablc: iq xml support diff iso diff blc value
 *  - use different iq file for mode hdr and normal
 *  - implement uapi and test
 *  - add Antiflicker-Mode
 *  - add the switch whether to enable HDR module
 *  - using mipi read back mode for normal mode
 *  - enable adebayer module
 *  - update dpcc setting in GC4C33 xml
 * v0.1.4
 * - implement module control api
 * - calibdb fast loaded
 * - afec dynamic control
 * - NR: support max 4096x gain for local gain mode
 * - add HLROIExpandEn in IQ/aiq
 * - NR,Sharp,BLC,DPCC: support 13 levels iso value
 * - ORB: bring up
 * - make sure the media link correctly when streaming on
 * - UVNR: sigmaR params change to 256/old_sigmaR
 * - gc4c33: update iqfiles v1.0.3
 * v0.1.5
 * - imx347 IQ xml v1.0.9
 * - update ahdr algo
 * - modify awb & aec runinterval para & mfnr para for gc4c33
 * - use VS as unified timestamp
 * v0.1.6
 * - gc4c33 IQ v1.0.7
 * - imx347 IQ v1.0.a
 * - NR & sharp: support free iso level on machine
 * - move paras of wbgain clip and wbgain adjustment to xml
 * - add awb chromatic adatptation gain adjust funciton
 * - add rk_aiq_uapi_sysctl_get3AStats interface
 * v0.1.7
 * - gc4c33 IQ v1.0.8
 * - sharp: fix bug of select iso level
 * - rkisp_parser_demo: parse xml and generate bin
 * - support IR-CUT&Infrared-light control
 * - add synchronization mechanism for capturing RAW and YUV images
 * - NR & sharp: fix bug for free iso level
 * - fix wrong expression in caga part
 * - modify calibdb load logic
 * v0.1.8
 * - gc4c33 iq v1.0.b
 * - demo support full/limit range
 * - fix rkisp_parse_demo can't generate bin error
 * - Add sensor dpcc setting to IQ
 * - change sensor_dpcc.enable from bool to int
 * - update DPCC setting in GC4C33 IQ
 * - format the source codes
 * - add hsnr & lsnr control from AE pre results
 * v0.1.9
 * - tnr disable/enable controlled by iq xml
 * - GC4C33 iq v1.0.c
 * - ae v0.1.3
 * - add env variable normal_no_read_back
 * - fix ahdr bug in v0.1.8
 * v1.0.0
 * - product API implement, include:
 *   - brightness/contrast/sataration/sharpeness
 *   - exposure time&gain range
 *   - white balance scene and R/G gain
 *   - noise reduction
 *   - dehaze&fec
 * - suppport cpsl(compensation light) functionality
 *   - suppport LED or IR light source
 *   - support gray mode
 *   - support auto light compensation control
 * - rkisp_parser_demo: fixup iqfile path are too long to be complete
 * - change calib parser version to v1.0.1
 * - change the name of '/tmp/capture_cnt' to '/tmp/.capture_cnt'
 * - determine isp read back times according to lumaDetect module
 * - fix sensor dpcc bug
 * v1.0.1
 * - iq parser support tag check
 *   - Calib v1.1.1 magic code 996625
 * - add following uApi
 *   - rk_aiq_uapi_getGrayMode
 *   - rk_aiq_uapi_setGrayMode
 *   - rk_aiq_uapi_setFrameRate
 *   - rk_aiq_uapi_getFrameRate
 *   - rk_aiq_uapi_sysctl_enumStaticMetas
 *   - rk_aiq_uapi_get_version_info
 * - cpsl support sensitivity and strength control
 * - add iq for OV02K10
 * - add iq for imx334
 * - fix accm-saturation bug
 * v1.0.2
 * - Calib : v1.1.3  magic code: 1003001
 * - iq_parser: fix make error for host
 * - add imx307/ov2718 xml
 * - fix Saturation_adjust_API bug
 * - support re-start and re-prepare procedure
 * - support sharp fbc rotation
 * - support VICAP MIPI + ISP, LVDS + ISP
 *   - tested on imx307, imx415, os04a10
 * - nr,sharp: add IQ para set & get interface
 * - ae: v0.1.4-20200722
 * - Fix the bug of getStaticCamHwInfo function
 * - xcore: add mutex for dq/que buffer, fix can_not_get_buffer error
 * v1.0.3
 * - Calib : v1.1.7  magic code: 1023357
 * - iqfiles:
 *   - rename all iqfiles
 *   - add imx378, imx415,s5kgm1sp,gc2035
 * - support sensor mirror and flip
 * - ae:
 *   - fix some ae uApi bugs
 *   - add EnvCalibration in AE
 * - FEC/LDCH: use resource path of user config
 * - CamHwIsp20: fix wrong mutex unlock
 * - DPCC:
 *   - Redefine DPCC in algo, add Expert Mode
 *   - Add Fast mode in DPCC
 * - fix some compatible issues of vicap and isp
 * - rkisp_demo: streaming stop after aiq
 * - fix the bug calculating the times of readback is error in lumadetect
 * v1.0.4
 * - iqfiles:
 *   - imx378/imx415/gc2053/gc4c33,HSNR<=>LSNR
 *   - imx378/s5kgm1sp, GainRange:use Linear Mode
 *   - gc2053:v0.0.2  imx415:v0.0.2  ov2718:v0.0.2
 * - ANR: add gray mode control for mfnr & uvnr param
 * - decrease AIQ heap memory usage, save 50M
 * - CamHwIsp20: move isp/ispp/mipitx,rx streaming on to prepare stage
 * - fix bugs in GainRange-dBmode
 * v1.0.5
 * - calib db: v1.1.8 magic code 1027323
 * - support dual cameras streaming concurrently
 * v1.0.6
 * - calib db: v1.1.9 magic code: 996490
 * - add backlight compasation and highlight depresion interface
 * - add enable and disable dehaze interface
 * - add asd interface to get calculated environmental luma
 * - ov2718: v0.0.4 gc2053: v0.0.3 ov02k10: v0.0.2
 * v1.0.7
 * - calib db: v1.2.0 magic code: 1006650
 * - support dependant iq for hdr/normal/gray
 * - rkisp_demo
 *   - support dual camera
 *   - add hdr x2 and x3 arg option
 * - Isp20PollThread
 *   - correct error handle in trigger_readback
 *   - fix the bug of stopping blocked by tx thread stop process
 * - ae support hdr3, add imx415 hdr3 xml
 * - imx415 anti-flicker
 * - readback two times to avoid luma detect bug
 * v1.0.8
 * - calib db: v1.2.3 magic code: 1011895
 * - add uApi
 *  - setDarkAreaBoostStrth/getDarkAreaBoostStrth
 *  - rk_aiq_uapi_sysctl_swWorkingModeDyn
 *  - rk_aiq_uapi_setFecEn/rk_aiq_uapi_setFecCorrectLevel
 * - match up with isp driver v0.1.4
 * v1.0.9
 * - calib db: v1.2.4 magic code: 1014880
 * - uApi changes:
 *   - rk_aiq_user_api_ae_queryExpResInfo
 *     modify data-type & add EnvLux in Ae-api
 *   - rk_aiq_uapi_setLdchEn
 *   - rk_aiq_uapi_setLdchCorrectLevel
 *   - rk_aiq_uapi_setFecBypass
 *   - rk_aiq_uapi_setFecEn
 * - support aiq version checking with tuning tool version
 * - fix the buf plane info changed of vb2
 * - Isp20Poll: modify the resolution of the input ISP to crop resolution
 * - awb: fix the bug in cct_lut_cfg initinalize
 * - fix flash-ir bugs
 * - switch to normal if gray mode is on
 * - add acp user interfaces
 * - fix gamma mode switching bug
 * v1.2.0
 * - calib db: v1.2.6 magic code: 1019694
 * - uApi changes:
 *    - rk_aiq_user_api_ahdr_SetAttrib
 *    - rk_aiq_user_api_adehaze_setSwAttrib
 *    - rk_aiq_user_api_adpcc_SetAttrib
 * - modify FpsSet bug in Ae
 * - user api called before sysctl prepared would cause stuck, fix it
 * - fix fec params error when switching hdr/normal
 * - disable switching working mode to normal on gray mode
 * - sharp: make more sharp strength for api
 * - add dehaze normal,HDR and night mode in algo
 * - update rkisp2x_tuner v0.2.0
 * - isp driver v0.1.6
 * - imx415 xml enable dc_en and set cfg_alpha
 * v1.2.1
 * - calib db: v1.2.7 magic code: 1021509
 * - uApi changes:
 *   - fix mwb params error after sysctl re-init
 *   - fix dehaze bugs
 * - optimize cpu usage
 *   - support buf no sync
 *   - disable Asharp_fix_Printf log
 * - isp driver v0.1.7
 * v1.2.2
 * - calib db: v1.2.9 magic code: 1018435
 *   - change imx415 hdr3 time/gain delay from 3 to 2
 *   - add gc2093/gc2053 iqfiles
 * - uApi: add rk_aiq_uapi_sysctl_setMulCamConc
 * - awb: v1.0.a
 * - update rkisp2x_tuner v0.2.1
 * - ALDCH: fix attrib has no effect setting before prepare
 * - SensorHw.cpp: fix exposure error caused by wrong dcg info
 * v1.2.3
 * - calib db: v1.3.4 magic code: 1061311
 * - fix some memory leak
 * - support Iris control
 * - support AF funtionality
 * - TMO/Dehaze: lots of modifications
 * - isp driver v0.1.8
 * v1.3.0
 * - calib db: v1.3.7 magic code: 1056480
 *   - modify sections: dehaze, TMO, AE
 * - update rkisp2x_tuner v0.3.0
 * - support Android compile
 * - uAPI changes
 *   - add blocked 3a stats uapi
 *     - rk_aiq_uapi_sysctl_get3AStatsBlk
 *     - rk_aiq_uapi_sysctl_release3AStatsRef
 *   - modify APIs:
 *     - rk_aiq_user_api_af_SetAttrib
 *     - rk_aiq_user_api_adebayer_GetAttrib
 * - cpsl: delay 2 frames to set ir on for gray mode
 *         set the cpsl to a certain status when initial
 * - change vicap tx buf num from 6 to 4
 * - AFEC: fixed bug fec can't be dynamically switched on and off
 * - fix TMO,dehaze bugs
 * - isp driver v0.1.9
 * v1.0x23.0
 * - calib db: v1.4.2 magic code: 1089142 
 * - update rkisp2x_tuner v1.0x3.0
 * - isp driver v1.0x2.0
 * - uAPI changes
 *   - add rk_aiq_uapi_sysctl_setCrop/rk_aiq_uapi_sysctl_getCrop
 *   - add rk_aiq_uapi_sysctl_preInit
 * - fix ldch/fec memleak of aiq v1.3.0 
 * - Open merge and tmo when mode is linear
 * - rk_aiq_uapi_sysctl_preInit
 * v1.0x23.1
 * - calib db: v1.4.2 magic code: 1089142, same as v1.0x23.0
 * - isp driver v1.0x2.1
 * - fix some bugs introduced by v1.0x23.0
 *   - fix normal mode noise reduction regression compared to v1.0x23.0
 *   - Add a strategy to avoid flicker in global Tmo cuased by Tmo algo
 *   - Fix bug that the wrong interpolation between dot=12 and dot=13 in AHDR
 * - some cpu usage optimization
 * v1.0x24.0
 * - calib db: v1.4.2 magic code: 1089142, same as v1.0x23.0
 * - isp driver v1.0x2.1, same as v1.0x23.1
 * - add some new iqfiles
 * - update rkisp2x_tuner v1.3.2
 * - fix some API bugs of ahdr/adpcc/adehaze
 * v1.0x24.1
 * - calib db: v1.4.3 magic code: 1123951
 *   - add cpie settings
 * - iq_parser: disable strict tag verification
 * - isp driver v1.0x2.1, same as v1.0x23.1
 * - uapi: add rk_aiq_uapi_sysctl_updateIq
 * v1.0x34.0
 * - calib db: v1.4.3 magic code: 1123951, same as v1.0x24.1
 * - isp driver v1.0x3.0
 * - Change mipi_rx buf type from USRPTR to DMABUF
 * - Open tmo enable function
 * - add exposure to ispparams
 * - fix aie gray_mode error of v1.023.3
 * - gen_mesh: v3.0.2
 * v1.0x45.1
 * - calib db: v1.4.4 magic code: 1123951 
 * - isp driver v1.0x4.1
 * - support socket IPC for toolserver
 * - support 3dnr motion detection and process 
 * v1.0x45.2
 * - isp driver v1.0x4.1
 * - fix stable bugs of 3ndr motion detection
 * - support RK-RAW data process
 * - support runtime debug log
 * - uApi support thread safe
 * v1.0x45.3
 * - optimize motion detection algo
 * v1.0x45.4
 * - motion detection stable issues
 * - system stuck issues when enable fec
 * - dump raw issues
 * v1.0x56.1
 * - isp driver v1.0x5.1
 * - ensure isp/pp params are syncronized with frame
 * - support vicap dvp interface
 * - support dynamic lsc&nr iq cell
 * v1.0x56.3
 * - update motion detection algo from jimmy
 * v2.0x60.0
 * - isp driver v1.0x6.1
 * - new tuning tool v2.0.0
 * - use json iq instead of xml
 * - reconstruct HWI & aiq_core
 *   - HWI can produce SOF, STATS, RAW/YUV image
 *   - aiq core support algo running in group threads
 * - THIS VERSION ONLY SUPPORT rk356x NOW
 * v3.0x00.1
 * - initial version for isp3x(rk3588)
 * - support 8k
 * - support camgroup
 * v3.0x8.1
 * - support custom AE/AWB
 * - expose UAPI2, hide old UAPI
 * - complete rk3588 module API
 * - add API sample code
 * - Along with ISP drv v1.0x8.0
 * v3.0x8.3
 * - complete rk3588 module camgroup API
 * - add API sample code for lsc/dpcc/ldch...
 * v3.0x8.5
 * - fix potential race condition in setSingleCamStatusReady
 * - fix some bugs in Group-Hdr mode
 * - compare with mNewAtt when setting attrib asynchronously
 * - always listen streaming on/off event in rkaiq
 * - Add configuration file rules for build
 * - add more log info for awb analysis tool
 * - add hw event callback
 * - add group module API of CP/IE/CSM
 * - notify all vicaps ready to app
 * v3.0x8.7
 * - fix compille error with gcc-4.4.7
 * - add parameters of uapi for some algos
 * - RkAiqxxxHandle: fix wrong 3a result caused by no 3a stats
 * - CamGroup: skip incomplete expoure params
 * - common: fix the compile error of 'defined but not used' with GUNC
 * - camgroup: add refCnt for shared resources
 * - reduce error log for RKAIQ running on serial port using 11520 baudrate
 * - add the function of reading/writing uapi parameters in real time for iq tools
 * v3.0x8.8
 * - hwi: optimize log printing for HWI
 * - camgroup: return error when the params of algos is incorrect
 * - PDAF: fix touchaf is abnormal
 * - AF: support optical zoom on android
 * - AF: adapt more type zoom-focus curve
 * - AE: delete extra Semicolon & add pointer release in GrpAE
 * - hwi: fix segment fault in setIspConfig when stopping AIQ
 */

#define RK_AIQ_VERSION_REAL_V "v3.0x8.8"
#define RK_AIQ_RELEASE_DATE "2022-05-05"

/******* DO NOT EDIT THE FOLLOWINGS ***********/

#define RK_AIQ_VERSION_HEAD "AIQ "
#define RK_AIQ_VERSION \
    RK_AIQ_VERSION_HEAD\
    RK_AIQ_VERSION_REAL_V

#endif
