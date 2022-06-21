/*
 * Copyright (C) 2022 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Cerf Yu <cerf.yu@rock-chips.com>
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
 */
#ifndef _im2d_symbol_h_
#define _im2d_symbol_h_

#define IM_API /* define API export as needed */

#ifdef __cplusplus
#define IM_C_API extern "C"
#define IM_EXPORT_API extern "C"
#else
#define IM_C_API
#define IM_EXPORT_API
#endif

#endif /* #ifndef _im2d_symbol_h_ */
