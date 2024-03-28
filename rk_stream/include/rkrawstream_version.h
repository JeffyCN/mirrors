/*
 *  Copyright (c) 2022 Rockchip Corporation
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

/*!
 * ==================== NOTE ===================================
 *
 *  编译适配说明
 *  rkrawstream库在不同代码仓库平台间使用完全相同的目录结构和文件
 *  编译需要aiq仓库包含相关提交，以下是各个平台适配情况
 *
 *  内部仓库-linux-isp3x-3588       make-Makefiles-aarch64.bash             通过
 *  内部仓库-linux-isp32-3588       make-Makefiles-aarch64-isp32.bash       通过
 *  内部仓库-linux-isp32-1106       make-Makefiles-uclibc.bash              通过
 *  外部仓库-linux-rk3588-3588      make-Makefiles-aarch64.bash             报错，aiq部分修改还未合并
 *  外部仓库-linux-rv1106-1106      由 ipc sdk Makefile 支持                通过，需要 release v5.0x1.1
 *  外部仓库-android-deveop-3588    由 rkaiq Android.bp 支持                通过
 *
 */

/*!
 * ==================== TODO LIST ==============================
 *
 *  2023/2/15
 *  - 完善 rkvi_demo 用参数传递 sensor name --done
 *  - 完全离线帧支持	--done
 *
 *  2023/3/13
 *  - API参数检查,判断返回值
 *  - 文档添加流程图
 */

/*!
 * ==================== RKRAWSTREAM VERSION HISTORY ============
 *
 * v0.0.9
 *  - initial version, merage with Android.
 *
 * v0.1.0
 *  - fix stop poll bug.
 *  - 添加 API rkrawstream_readback_set_rkraw2
 *  - buf_mem_type 在prepare阶段配置
 *  - buffer 数量 可以在prepare阶段配置
 *  - 添加isp32分支编译脚本
 *
 * v0.1.1
 *  - 重构rkrawstream_demo,支持更多参数传递,支持编译命令开关aiq
 *  - 改变vicap和readback回调函数参数为 rkrawstream_xxxx_cb_param_t
 *  - 添加HDR, buftype, memtype的枚举类型
 *  - 添加isp32分支编译脚本
 */
