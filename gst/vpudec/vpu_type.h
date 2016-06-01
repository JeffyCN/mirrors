/*
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
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

#ifndef _VPU_TYPE_H
#define _VPU_TYPE_H

#ifdef WIN32
typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned __int64        RK_U64;

typedef signed char             RK_S8;
typedef signed short            RK_S16;
typedef signed int              RK_S32;
typedef signed __int64          RK_S64;

typedef void                    VOID;

#else

typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned long long int  RK_U64;


typedef signed char             RK_S8;
typedef signed short            RK_S16;
typedef signed int              RK_S32;
typedef signed long long int    RK_S64;

typedef void                    VOID;

#endif

#endif /*_VPU_TYPE_H*/
