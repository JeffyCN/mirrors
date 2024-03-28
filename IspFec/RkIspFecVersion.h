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

#ifndef _RK_ISP_FEC_VERSION_H_
/*!
 * ==================== RK ISP FEC VERSION HISTORY ====================
 *
 * v1.0.0
 * - FEC: supports different resolution of input and output
 * - add IspFec lib
 *
 * v1.0.1
 * - add the function that generating mesh online
 * - support that prepare and process can be called by different threads
 *
 */

#define RK_ISP_FEC_VERSION_REAL_V "v1.0.1"
#define RK_ISP_FEC_RELEASE_DATE "2023-06-01"

/******* DO NOT EDIT THE FOLLOWINGS ***********/

#define RK_ISP_FEC_VERSION_HEAD "ISP_FEC "
#define RK_ISP_FEC_VERSION \
    RK_ISP_FEC_VERSION_HEAD\
    RK_ISP_FEC_VERSION_REAL_V

#endif
