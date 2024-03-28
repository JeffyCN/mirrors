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
 * v3.0x8.11
 * - sysctl: remove atrib constructor of rk_aiq_init_lib
 * - support compile aiq as static lib
 * - hwi: optimize the bug of vicap drop frame
 * - xcore: support setting the policy/priority of scheduling before creating thread
 * - uAPI: print the verison of RKAIQ as default
 * - fix the bug that ISP params are asynchronous in group cam
 * - 8_camera_aiq_stable_ver1.0
 * - dpcc: fix params init
 * - Adehaze: update api struct and fuctional api
 * - A3DLUT: some optimization
 * - ACCM: some optimization
 * - ASD: fix some bugs
 * - fix bugs of colorasgray and calibtuning
 * - tuning: won't free calib witch from iqfiles.
 * - use json format iqfile in default.
 * - tuning: update calib async instead stop/start.
 * - custom AE: adapt for isp2x
 * - AF: fix can not get soft stats on 356x
 * - Fix compile bug in android
 * - Tuning: fix memory leak.
 * - uAPI: support for updating calib in cam group mode
 * - compile: use ccache to speed up rebuiding.
 * - tuning: fix wrong CMD size checking.
 * - csm: unsupport limit range
 * - Tuning: add CMD for get 3Astats.
 * - support acgc
 * v3.0x9.1
 * - force using FakeCamera0.json at offline mode
 * - awb otp : apply otp in awb stat
 * - awb: translator awb stat based on pre_wbgain_inv_r and bls1
 * - AF: skip the same motor position in ipc search
 * - AF: update calulation of shift value
 * - iqfiles: isp21: change default af value
 * - fix memleak for some modules
 * - IspParamsAssembler: protect from "no free xxx buf" of core
 * - v4l2_device: fix wrong dequeued flag of params buf
 * - xcore: set buffer QUEUED flag when get buffer.
 * - Update the lost isp params by driver again
 * - iqfiles: af: change for fullsweep and pdaf
 * - update 356x json files to 3.9.1
 * - PDAF: update to v0.1.5
 * - kernel headers: Sync with driver
 * - CamHwIsp20: set memory mode to word align
 * - 1: ynr support sigma fomula and subsample point in auto
 * - HWI: modify AE/HIST HW setting
 * - support otp for awb/alsc
 * - Hwi: open all AE hw modules
 * - CCM/3DLUT: del mode & manual cfg in json
 * - support the function that reset VICAP
 * - add uapi for turn on/off socket server.
 * - PDAF: update to v0.1.1
 * - hwi: update lsc only when status change.
 * - CAC/LDCH: Support mesh buffer count
 * - Debayer:params change with ISO
 * - tuning: create socket for each camera.
 * - AF: optimize optical zoom
 * - PDAF: update to v0.1.0
 * - PDAF: optimize pdaf
 * - support for loading iq from buffer address.
 * - tuning: support for group camera.
 * - Amerge/drc/dehaze: support iso as ctrldata
 * - HWI: support the funciton of setting isp dgain
 * - algo: alsc: adjust the position of crop for lsc otp
 * - merge rk3588 with rk356x base on release v3.0x8.8
 * v3.0x9.3
 * iqfiles: add the json of gc8034/ov02b10/s5kjn1
 * aiq_core: transfer initial exposure to algos
 * AF: support change resolution
 * aiq_core: restore the flag of conf_type after switching scene
 * ipc_server: use 0 be camid when no physical camera.
 * camgroupManager: multichannel camera write i2C in parallel
 * aiq offline base on remove rkstream
 * remove stream code.
 * aiq_core: keep same iso for Android picture taken when resolution changed
 * hwi: remove configuring isp params from trigger_isp_readback
 * def variable(RKAIQ_INSTALL_ALGOS_LIB) to intall lib
 * Adrc, amerge: fix init expo bug
 * alsc: fix can't turn on when disabled by IQ.
 * aiq_core: add initial params of ispDgain/dgain in init
 * Adrc, amerge and adehaze: use RK_AIQ_ALGO_CONFTYPE_CHANGERES for capture
 * AE: fix reg2float accuracy problem
 * AF: Fix custom af build error on Android
 * AF: fix "Can't release AF (3)" in camera monkey test
 * AF: fix can not change statics setting when first enter
 * PDAF: fix memleak when enable pdaf
 * PDAF: add set memory mode for pdaf video device
 * PDAF: fix no fine search in low light env when first enter
 * PDAF: add pdaf.a for 356x
 * AF: fix crash in af search
 * PDAF: support fine search extended mode
 * PDAF: support reshape mode
 * AF: fix lens pos is changed when no af stats
 * PDAF: lock ae in fine search to speed up
 * algos: alsc: apply lsc otp in processing
 * algo: alsc: lsc otp compatible with isp-unite mode
 * fix force config LSC when enable blc1
 * Fix compile error on Android
 * sysctl: should not release all sensor's calibs when deinit
 * v3.0x9.4
 * bbbeacb Revert "hwi: add the interface of set_csi_mem_word_big_align"
 * ecb78ae Revert "AIE: fix bug that param not taking effect"
 * 8e3551d Revert "API: add enum static info by physical id"
 * 75ea4d3 Revert "Adehaze: add XCAM_MESSAGE_YNR_V3_PROC_RES_OK in grpDhazConds, and use ynr proc res"
 * b5acf48 AF: change some log from LOGE_AEC to LOGE_AF
 * f257548 PDAF: add librkaiq_pdaf.a for 356x linux
 * 1ac8977 algos: alsc: adjust Vignetting for lsc OTP
 * 15d5011 AE: add envChange for android
 * 65c1304 iqfiles: update xunfei s5kjin1 iq
 * e2caf1c Adehaze: add XCAM_MESSAGE_YNR_V3_PROC_RES_OK in grpDhazConds, and use ynr proc res
 * 3e81d4e AF: set lockae to false in pdaf search
 * 22e30f9 AF: support AF_MODE_EDOF mode
 * 2d8d0dc PDAF: fix stop_stream is called twice sometimes
 * 8a8a8e5 fix touchae/touchaf failure
 * 35ebba0 AF: fix af is stopped after do touchaf/capture
 * 0d2a231 iqfiles: add pdTargetOffset
 * f987eaa PDAF: update pdaflib to v0.1.8
 * 00b23e9 AF: reset/start search when user change af win config
 * ed914df AF: fix fps is dropped in dual camera case on android platform
 * 5b2c616 PDAF: fix cannot do pdaf search when enter camera first time
 * c7c53b9 PDAF: fix lost af/pdaf stats in readback mode sometimes
 * bf96150 API: add enum static info by physical id
 * dc9e59b Amerge: update isCapture function
 * 338519c Adrc: update isCapture function
 * 95bb11e Adehaze: fix dehaze and hist capturing bug in 8k mode
 * fda5a8c awb : Increase the decimal digits for lineRgProjCCT
 * 39cf59a AIE: fix bug that param not taking effect
 * 50d3dbf rkrawstream: add api mutex.
 * 24f446c rkrawstream: add rkrawstream Android build script.
 * 8d09ff3 xcore: fix buf cache flag.
 * 4239d10 hwi: add the interface of set_csi_mem_word_big_align
 * a80bc53 iqfiles: update gc8034/ov02b10 json
 * 07a180a iqfiles: update xunfei s5kjin1 iq
 * ebd231d AF: fix motor is moved after capture
 * ccf8ba2 uApi2: add interface rk_aiq_uapi2_sysctl_setIspParamsDelayCnts
 * 9290261 aiq_core: add setDelayCnt interface
 * 3677cb2 uApi: rawReproc_genIspParams support params sync for semi mode
 * 54de8ac xcore: add log module rkrawstream.
 * d611e50 do not qbuf at prepare when use dmabuf.
 * 58351eb split v4l2 device stop to streamoff and release buffer. for iflytek project.
 * 2fa7642 add rkrawstream and rkvi_demo.
 * 439b562 RkAiqManager: compatible with the calling method of mMetasCb on android hal
 * 754c3c2 awb: update s5kjn1_default_default.json
 * 66d1bcf awb : Accelerated convergence speed when wbgain is approaching to target value
 *
 *
 * v4.0x8.3
 * - initial version for isp32(rv1106)
 * v4.0x8.5
 * - Merge isp3x branch v3.0x8.5
 * v4.0x8.6
 * - Add support blcv32 and predgain
 * - Add support isp32 api for merge/drc/dehaze
 * - Fix blc sychronization issue
 * - Fix a few build error
 * v4.0x8.7
 * - Fix some bug during ip verify
 * v4.0x8.8
 * - Support nr/sharp/cpsl/cp/ie module APIs
 * - Fix dehaze/drc/merge/drc api bugs
 * - Fix ae/debayer/nr/sharp params
 * - Optimized heap buffer usage a bit
 * v4.0x9.8-rc0
 * - Support write AWB debug buffer to file
 *   driver version should match with kernel
 * - Support switch sub-scene
 * - Support cutomer ae/awb
 * - Optimized code size
 * - Optimized memory usages
 * - Optimized cpu ultilization
 * - Fix params bug for 3dlut/ccm/gic..
 * v4.0x9.8-rc1
 * - Fix a few IQ tunning stable issues
 * - Support static link library
 * - AWB ：Fix smart run judging scheme
 * - AE : Fix stats translate error
 * - IE : Fix BW not working for one frame during tunning
 * - CAC : Fix buffer not freed when using ctrl-c to exit app
 * - CAC : Limit expo_thed/adj value ranges
 * - CAC : Support related path for psf map binary
 * - GAMMA/MERGE/DRC: Rename and Optimize cpu usage
 * - DEHAZE: Support hist wr
 * - Update os04a10/sc4336/sc500ai IQ json
 * v4.0x9.8-rc2
 * - rkisp_demo: Support build without drm and rga
 * - 3DLUT: Fix memory leak
 * - DRC/MERGE/DEHAZE: Fix a few issue in uAPI and tool API
 * - AF: Fix a few functional issues
 * - AWB: Fix memory not initilized
 * v4.0x9.8-rc3
 * - DRC/DEHAZE/MERGE/NR/SHARP: Consider PreDGain and OB offset
 * - Optimize switch scene for switching between night and day
 * - Optimize CPU usage of CSM and 3DLUT
 * - IQ: Add and Update sc4336, sc230ai, gc4653 and jx_k17
 * v4.0x9.8
 * - Fix aiq crashed on IQTool read/write
 * - CAC: correct HDR ratio value
 * - 3DLUT: Fix a few issues
 * - NR/SHARP: Support tools ui controls
 * v4.0x9.9-rc1
 * - Fix CPU stalled when LSC dynamic on and off
 * - Fix ISP Stream stopped if switching CAC of to on
 * - FIx 3DLUT damp issue
 * - Correct DRC value range
 * - Correct CAC HDR ratio
 * - Optimize CAC tuning UX
 * - Add check BLC0 calibration value
 * - Support IQ json2bin feature
 * - Support new IQ gc2093
 * - Support get 3A stats when tuning
 * v4.0x9.9-rc2
 * - Support fast ae/awb
 * - Support convert iq from json to bin
 * - CCM/3DLUT use pre-dgain
 * - AWB fixes two issues about wbgain
 * - IQ update SC500AI for isp32
 * - Optimze SmartIR
 * - Fix crashed in GrpMsgHdl thread
 * - Dehaze support hist semiauto mode
 * v4.0x9.9-rc3
 * - Dehaze fix stats effect delay
 * - AWB add new strategy to optimzie fast awb
 * - CAC reduce buffer count
 * - j2s4b fix compile error on old system
 * v4.0x9.9-rc4
 * - AWB: update a few tool struct
 * - DRC/DEHAZE: fix a few API issues
 * - ThunderBoot(TB): support trigger first frame twice
 * - TB: Use flock to make aiq run as a single instance
 * - IQ: add sc3338, gc4023, update sc230ai, os04a10
 * - API: add set IQ bin by buffer, add set TB info
 * - API: update AWB struct, users need to rebuild APP
 * - HWI: Fix a few issues
 * v4.0x9.9-rc5
 * - DRC/DEHAZE/AE: Fix a few issues
 * - TB: Fix offline AWB issues
 * - Multi-Cam: Fix AE stats lost in mutli-cam case
 * - FakeSensor: Fix 8bit raw simualtion
 * - IQ: Update sc230ai/sc3338/gc2093/sc031gs
 * - IQ Bin: Reduce binary size
 * v4.0x9.9
 * - AEC: Fix params wrong if stats invalid
 * - j2s4b: Fix installation step on low version cmake
 * - IQ: jx_k17: Fix wrong enum value for hdr line mode
 * - DRC: fix some issues, and modify IIR frame value
 * v4.0x9.a-rc1
 * - hwi: Export device buf count for user to config
 * - Adrc, Amerge, adehaze: fix init expo bug
 * - Adehaze: update functional api
 * - Adehaze: use ynr proc res for dehaze local gain
 * - aiq_core: transmit the proc result of YNR V22 and V3 to algos
 * - aiq: support thunder boot for product doorlock
 * - cac: Fix getting none lut buffers for thunder boot product
 * - Use initial awb cfg for server of thunder boot product
 * - iqfiles: Fix warning reported by jsonlint -s -W xxx.json
 * - Revert "fakesensor: Open used tx/rx devs only"
 * - hwi: Use unordered_map instead of map for  static infos
 * - AF: merge isp3x modification
 * v4.0x9.a
 * - iq_parser: increase the range of distortion params
 * - algos: ldchV21: support for dynamic switching
 * - rkisp_demo: update CompileOptions.cmake from rkaiq
 * - iqfiles: update sc200ai json for Tonly
 * - build: Enable build iq binrary remove unused RKPLATFORM
 * - cmake: Change to compatible with cmake's future version
 * - rkisp_demo: Fix build error on IPC sdk if drm enabled
 * v4.0x9.b-rc1
 * - AWB/LSC: Support OTP feature
 * - TB/IQ: Modify CAC path for sc200ai
 * - TB/IQ: Fix greenish of first frame for gc2093
 * - DEHAZE: Fix null stats issue
 * - DEHAZE: Deal with no ynr sigma case which causes maze artifact
 * - HWI: Handle exceptions caused by no stats

 * v5.0x1.0
 * - merge with isp3x, contains v3.0x8.7 - v3.0x9.4
 *
 * v5.0x1.1
 * - Support ISP d-gain for rv1106
 * - Support Android build system
 * - Support rkstream and media_enquiry for rv1106
 * - Support compact API for rk3588
 * - Fix init param match issues for thunder boot product
 * - Fix libc check for buildroot build system
 * - Fix effected param match for all products
 * - Fix issues caused by branch merge
 *
 * v5.0x1.2-rc1
 * - AWB: fix Access Violation in awbV32_GetAttrib
 * - isp32_lite: iqfiles: add gc8034/ov13855 iqfiles
 * - dpcc: fix bug in memcpy Expert_mode params
 * - LSC: fix stack smashing detect when using api
 * - isp32_lite: adapts to isp32 lite
 * - aiq: hwi: Update kernel driver header of isp32_lite
 *   - until update to date kernel header commit:
 *   - https://10.10.10.29/c/rk/kernel/+/177679
 *
 * v5.0x1.2-rc2
 * - AF: change default value for FineSearchStep in M4 comment
 * - iqfiles: sync iqfiles to aiq_v5.0x1.2-rc1
 * - SmartIr: rename get_env_value function to avoid conflict
 * - hwi: Fix thunder boot for dual camera
 *
 * v5.0x1.2-rc3
 * - hwi: Adapt to new dvp driver model
 * - uAPI: support completely offline mode for Android
 * - FakeSensorHw: change the id if the enqueue frame id isn't contiguous
 * - rkvi_demo: fix build error in buildroot
 * - rkisp_demo: fix build error in buildroot
 *
 * v5.0x1.2-rc4
 * - afd: add afd algo
 * - awb: modify apis
 *
 * v5.0x1.2-rc5
 * - optimize cpu usage
 * - modify blc and sharp api attribs
 * 
 * v5.0x1.3
 * - optimize cpu usage extreamly 
 * - kernel depandency: MUST contain this patch:
 *   https://10.10.10.29/c/rk/kernel/+/191593
 *   media: rockchip: isp: distinguish buf done or subscribed event for param
 *   poll
 *
 *   Change-Id: I9b366a9f47ce24783651c93512125dca7c49917f
 * 
 * v5.0x1.3-rc1
 * - multi isp: isp32 support 4k
 * - ldch: support multi isp mode
 * - output ERROR and KEY logs as default
 * - fix lots of CppCheck errors
 * - kernel depandency: isp32 use 4k need this patch:
 *   https://10.10.10.29/c/rk/kernel/+/184947
 *   media: rockchip: isp: support unite mode for isp32
 * 
 *   Change-Id: Iae10911ee0f199210c7913d3e99dd85ec4642f81
 * 
 * v5.0x1.3-rc2
 * - optimize ae stats usage
 * - ccm/lsc/dehaze: fix interpolation bugs 
 * - support 2 cam's online tuning for Android
 * - add default manual settings for algo's API getAtrrib
 * - fix tool online tuning bug that params in page sysctl may be overwitten by
 *   main page.
 * v5.0x3.0
 * - update kernel headers to match the ISP version v2.3.0
 * v5.0x4.0
 * - update kernel headers to match the ISP version v2.4.0
 * - support single frame mode 
 * - optimize TB procedure
 * - support j2s4b on device
 * - support AI nr
 * - fix bugs:
 *   - fix ae is not running for 3562
 *   - fix drc longframeMode error
 * v5.0x4.0-rk3576-rc0
 * - AE & Adebayer: adapt to bnr20bit
 * - cac: bring up for 3576
 * - rkisp_demo: don't check if hdr mode is supported in offline mode
 * - aiq_core: send bay3d stat to the tnr
 * - hwi: support raw16 format for fake camera
 * - Asharp: support rk3576
 * - Adehaze: support rk3576
 * - YNR: support rk3576
 * - Adrc: support expander function
 * - CamHwIsp20: get isp info from drv
 * - Adrc: supprot rk3576
 * - aiq: hwi: Update kernel driver header of isp39
 * - NR: adapt to isp39
 * - aiq: hwi: Update kernel driver header of isp39
 * - iqfiles: update sc4336_OT01_40IRC_F16.json
 * - Debayer: adapt to isp39
 * - AF: adapt to isp39
 * - aiq: hwi: Update kernel driver header of isp39
 * - aiq: hwi: Update kernel driver header of isp39
 * - CCM: adapt to isp39
 * - AE: adapt to isp39
 * - TEST: add '-DENABLE_PARTIAL_ALOGS' to only enable partial algos
 * - [TEST ONLY] display in intel fpga
 * - aiq: hwi: Update kernel driver header of isp39
 * - scripts: Jenkinsfile: support rk3576
 * - rkaiq: Initial support for ISP39
 * - aiq: hwi: Update kernel driver header of isp39
 * v5.0x4.1
 * - update kernel headers: rkisp_thunderboot_resmem_head
 * - awb:
 *   - support user awb OTP
 *   - support awb gain mapped from main sensor
 * - support fastboot aiq started one or two times
 * - fix bugs:
 *   - fix some bugs of rk3562 8k
 *   - fix ae may be not running on all platform introduced in v5.0x3.0
 *   - fix aiq_core state error when aiq stop
 * v5.0x5.0
 * - on isp32 branch
 * - update kernel headers
 *   - https://10.10.10.29/c/rk/kernel/+/198712/2
 * - split to 2 or 4 grids according to resolution on rk3562
 * - API rk_aiq_uapi2_sysctl_preInit_tb_info is no longer needed for AOV or TB
 * - add some members for awb in iq json, rtt should be updated along with it
 * - fix some AIQ crash bugs when tuning
 * - update IQBIN Version v1.0.0
 * v6.0x6.1
 * - isp39 branch update to 29 first version
 * - support rk3576
 * - support aiisp
 * - isp driver version: 2.6.0
 * - update IQBIN Version v1.0.1
 */

#define RK_AIQ_VERSION_REAL_V "v6.0x6.1"
#define RK_AIQ_RELEASE_DATE "2024-03-03"

#define RK_AIQ_IQ_HEAD_VERSION_REAL_V "v1.0.1"

/******* DO NOT EDIT THE FOLLOWINGS ***********/

#define RK_AIQ_VERSION_HEAD "AIQ "
#define RK_AIQ_VERSION \
    RK_AIQ_VERSION_HEAD\
    RK_AIQ_VERSION_REAL_V

#define RK_AIQ_IQ_HEAD_VERSION_PREFIX "IQBIN Version: "
#define RK_AIQ_IQ_HEAD_VERSION \
    RK_AIQ_IQ_HEAD_VERSION_PREFIX\
    RK_AIQ_IQ_HEAD_VERSION_REAL_V

#endif
