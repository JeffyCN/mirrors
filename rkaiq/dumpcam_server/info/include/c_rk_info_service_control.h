/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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

#ifndef INCLUDE_RT_MPI_RK_INFO_SERVICE_CONTROL_H__
#define INCLUDE_RT_MPI_RK_INFO_SERVICE_CONTROL_H__

#include "c_RTJsonParser.h"
#include "c_rk_ipcs_request.h"
#include "c_rk_ipcs_response.h"
#include "rk_info_utils.h"

typedef struct _AIQInfoServiceControl AIQInfoServiceControl;

typedef void (*fptr_AIQInfoServiceControl_onReceived)(AIQInfoServiceControl *const me,\
                                                    AIQIPCRequest *request, AIQIPCResponse *response);

struct _AIQInfoServiceControl {
    c_AIQJsonParserCtx *parser;

    fptr_AIQInfoServiceControl_onReceived onReceived;
};

/* Constructor and Destructor */
void AIQInfoServiceControl_Constructor(AIQInfoServiceControl* const me,
                                       fptr_AIQInfoServiceControl_onReceived onReceived_function);
void AIQInfoServiceControl_Destructor(AIQInfoServiceControl *const me);

void AIQInfoServiceControl_onReceived(AIQInfoServiceControl *const me,\
                                     AIQIPCRequest *request, AIQIPCResponse *response);

void AIQInfoServiceControl_dumpAllModule(AIQInfoServiceControl *const me,\
                                            st_string *dumpInfo,\
                                            int argc, void *argv[]);

void AIQInfoServiceControl_dumpMultiModule(AIQInfoServiceControl *const me,\
                                            st_string *dumpInfo, char *moduleName,\
                                            int argc, void *argv[]);

string_list *AIQInfoServiceControl_getModules(AIQInfoServiceControl *const me,\
                                            const char *moduleName, const char delim);

AIQInfoServiceControl* AIQInfoServiceControlCreate();
void AIQInfoServiceControlDestroy(AIQInfoServiceControl *const me);

#endif /* INCLUDE_RT_MPI_RK_INFO_SERVICE_CONTROL_H__ */
