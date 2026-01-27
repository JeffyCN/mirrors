/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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

#ifndef DUMP_LOG_H_
#define DUMP_LOG_H_

#include <cstdio>
#include <stdio.h>

#if 0
#define DEBUG_MSG  printf
#else
#define DEBUG_MSG(...)
#endif

#define DEBUG_MSG_ERROR printf

#endif /* DUMP_LOG_H_ */
