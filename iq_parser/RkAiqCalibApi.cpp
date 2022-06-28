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
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "RkAiqCalibApi.h"
#include <fstream>
#include <iostream>



bool CamCalibDbCreate
(
    CamCalibDbContext_t*  pCamCalibDbCtx
)
{
    if(pCamCalibDbCtx == NULL)
        return false;

#if 1
    list_head *list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_bayernr_v2));
    if (list)
        INIT_LIST_HEAD(list);

    list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_ynr_v2));
    if (list)
        INIT_LIST_HEAD(list);

    list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_cnr_v1));
    if (list)
        INIT_LIST_HEAD(list);

    list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_sharp_v3));
    if (list)
        INIT_LIST_HEAD(list);

#endif

    return true;
}


static void ClearBayernrV2Setting3DList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        CalibDb_Bayernr_3DParams_V2_t *pParam = container_of(pListItem, CalibDb_Bayernr_3DParams_V2_t, listItem);
        struct list_head* pTemp = pListItem->next;
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(CalibDb_Bayernr_3DParams_V2_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearBayernrV2Setting2DList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Bayernr_2Dparams_V2_t *pParam = container_of(pListItem, Calibdb_Bayernr_2Dparams_V2_t, listItem);
        struct list_head* pTemp = pListItem->next;
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Bayernr_2Dparams_V2_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearBayernrV2ProfileList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        CalibDb_Bayernr_V2_t *pParam = container_of(pListItem, CalibDb_Bayernr_V2_t, listItem);
        struct list_head* pTemp = pListItem->next;
        ClearBayernrV2Setting2DList(&pParam->st2DParams.listHead);
        ClearBayernrV2Setting3DList(&pParam->st3DParams.listHead);
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(CalibDb_Bayernr_V2_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearYnrV2SettingList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Ynr_params_V2_t *pParam = container_of(pListItem, Calibdb_Ynr_params_V2_t, listItem);
        struct list_head* pTemp = pListItem->next;
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Ynr_params_V2_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearYnrV2ProfileList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Ynr_V2_t *pParam = container_of(pListItem, Calibdb_Ynr_V2_t, listItem);
        struct list_head* pTemp = pListItem->next;
        ClearYnrV2SettingList(&pParam->listHead);
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Ynr_V2_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearCnrV1SettingList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Cnr_params_V1_t *pParam = container_of(pListItem, Calibdb_Cnr_params_V1_t, listItem);
        struct list_head* pTemp = pListItem->next;
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Cnr_params_V1_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearCnrV1ProfileList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Cnr_V1_t *pParam = container_of(pListItem, Calibdb_Cnr_V1_t, listItem);
        struct list_head* pTemp = pListItem->next;
        ClearCnrV1SettingList(&pParam->listHead);
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Cnr_V1_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}



static void ClearSharpV3SettingList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Sharp_params_V3_t *pParam = container_of(pListItem, Calibdb_Sharp_params_V3_t, listItem);
        struct list_head* pTemp = pListItem->next;
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Sharp_params_V3_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


static void ClearSharpV3ProfileList(struct list_head *profile_list) {

    struct list_head* pListItem = profile_list->next;

    while (pListItem != profile_list) {
        Calibdb_Sharp_V3_t *pParam = container_of(pListItem, Calibdb_Sharp_V3_t, listItem);
        struct list_head* pTemp = pListItem->next;
        ClearSharpV3SettingList(&pParam->listHead);
        list_del(pListItem);
        memset(pParam, 0x00, sizeof(Calibdb_Sharp_V3_t));
        free(pParam);
        pListItem = pTemp->next;
    }

    INIT_LIST_HEAD(profile_list);
}


bool CamCalibDbRelease
(
    CamCalibDbContext_t*  pCamCalibDbCtx
)
{
    LOGE_ASHARP("%s:%d  !!!!!!!!!!!  resleae  list !!!!!!!!!!!!!\n",  __FUNCTION__, __LINE__);
    if(pCamCalibDbCtx == NULL)
        return false;

#if 1
    list_head *list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_bayernr_v2));
    if (list)
        ClearBayernrV2ProfileList(list);

    list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_ynr_v2));
    if (list)
        ClearYnrV2ProfileList(list);

    list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_cnr_v1));
    if (list)
        ClearCnrV1ProfileList(list);

    list = (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCamCalibDbCtx, list_sharp_v3));
    if (list)
        ClearSharpV3ProfileList(list);
#endif
    return true;

}
/******************************************************************************
 * AddAecCalibProfile2AecCalibList
 *****************************************************************************/
bool AddAecCalibProfile2AecCalibList
(
    list_head *profile_list,
    CalibDb_Aec_CalibPara_t* pAddAec
) {
    CalibDb_Aec_CalibPara_t* pNewAec = NULL;
    // initial check
    if(pAddAec == NULL ) {
        LOGE("pNewAec is invalid");
        return(false);
    }
    // check if scene already exists
    CalibDb_Aec_CalibPara_t* pAecFind = NULL;
    GetAecProfileFromAecCalibListBySceneName(profile_list, pAddAec->scene, &pAecFind);

    if(pAecFind == NULL) {
        pNewAec = (CalibDb_Aec_CalibPara_t*)malloc(sizeof(CalibDb_Aec_CalibPara_t));
        //printf("%p\n", pNewAec);
        memcpy(pNewAec, pAddAec, sizeof(CalibDb_Aec_CalibPara_t));
        list_prepare_item(pNewAec);
        list_add_tail((list_head*)pNewAec, profile_list);
        return (true);
    } else {
        LOGE("aec scene (%s) is repeated", pAecFind->scene);
        return (false);
    }
}

/******************************************************************************
 * GetAecProfileFromAecCalibListBySceneName
 *****************************************************************************/
bool GetAecProfileFromAecCalibListBySceneName
(
    const list_head* profile_list,
    const char* scene,
    CalibDb_Aec_CalibPara_t** pAecProfile,
    int *scene_index
) {
    list_head* p;
    p = profile_list->next;
    if(scene_index != NULL) {
        *scene_index = 0;
    }

    while (p != profile_list)
    {
        CalibDb_Aec_CalibPara_t* pProfile = container_of(p, CalibDb_Aec_CalibPara_t, listHead);
        //printf("%s  %p ", pProfile->scene, p);
        if (!strncmp(pProfile->scene, scene, sizeof(pProfile->scene))) {
            *pAecProfile = pProfile;
            break;
        }
        p = p->next;
        if(scene_index != NULL) {
            *scene_index = *scene_index + 1;
        }
    }
    return(true);
}


/******************************************************************************
 * GetAecProfileFromAecCalibListByIdx
 *****************************************************************************/
bool GetAecProfileFromAecCalibListByIdx
(
    const list_head* profile_list,
    int idx,
    const CalibDb_Aec_CalibPara_t** pAecProfile
) {
    list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Aec_CalibPara_t* pProfile = container_of(p, CalibDb_Aec_CalibPara_t, listHead);
            //printf("%p ", pProfile);
            *pAecProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

/******************************************************************************
 * AddAecTuneProfile2AecTuneList
 *****************************************************************************/
bool AddAecTuneProfile2AecTuneList
(
    list_head *profile_list,
    CalibDb_Aec_TunePara_t* pAddAec
) {
    CalibDb_Aec_TunePara_t* pNewAec = NULL;

    // initial check
    if(pAddAec == NULL ) {
        LOGE("pNewAec is invalid");
        return(false);
    }
    // check if scene already exists
    CalibDb_Aec_TunePara_t* pAecFind = NULL;
    GetAecProfileFromAecTuneListBySceneName(profile_list, pAddAec->scene, &pAecFind);
    if(pAecFind == NULL) {
        pNewAec = (CalibDb_Aec_TunePara_t*)malloc(sizeof(CalibDb_Aec_TunePara_t));
        //printf("%p\n", pNewAec);
        memcpy(pNewAec, pAddAec, sizeof(CalibDb_Aec_TunePara_t));
        list_prepare_item(pNewAec);
        list_add_tail((list_head*)pNewAec, profile_list);
        return (true);
    } else {
        LOGE("aec scene (%s) is repeated", pAecFind->scene);
        return (false);
    }
}


/******************************************************************************
 * GetAecProfileFromAecCalibListBySceneName
 *****************************************************************************/
bool GetAecProfileFromAecTuneListBySceneName
(
    const list_head* profile_list,
    const char* scene,
    CalibDb_Aec_TunePara_t** pAecProfile
) {
    list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        CalibDb_Aec_TunePara_t* pProfile = container_of(p, CalibDb_Aec_TunePara_t, listHead);
        //printf("%s  %p ", pProfile->scene, p);
        if (!strncmp(pProfile->scene, scene, sizeof(pProfile->scene))) {
            *pAecProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAecProfileFromAecCalibListByIdx
 *****************************************************************************/
bool GetAecProfileFromAecTuneListByIdx
(
    const list_head* profile_list,
    int idx,
    const CalibDb_Aec_TunePara_t** pAecProfile
) {
    list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Aec_TunePara_t* pProfile = container_of(p, CalibDb_Aec_TunePara_t, listHead);
            //printf("%p ", pProfile);
            *pAecProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

/******************************************************************************
 * AddAwbCalibV200Profile2AwbCalibV200List
 *****************************************************************************/
bool AddAwbCalibV200Profile2AwbCalibV200List
(
    list_head *profile_list,
    CalibDb_Awb_Calib_Para_V200_t* pAddAwb
) {
    CalibDb_Awb_Calib_Para_V200_t* pNewAwb = NULL;

    // check if pAddAwb  is valid
    if(pAddAwb == NULL ) {
        LOGE("pNewAwb is invalid");
        return(false);
    }
    // check if scene already exists
    CalibDb_Awb_Calib_Para_V200_t* pAwbFind = NULL;
    GetAwbProfileFromAwbCalibV200ListBySceneName(profile_list, pAddAwb->scene, &pAwbFind);
    if(pAwbFind == NULL) {
        pNewAwb = (CalibDb_Awb_Calib_Para_V200_t*)malloc(sizeof(CalibDb_Awb_Calib_Para_V200_t));
        //printf("%p\n", pNewAwb);
        memcpy(pNewAwb, pAddAwb, sizeof(CalibDb_Awb_Calib_Para_V200_t));
        list_prepare_item(pNewAwb);
        list_add_tail((list_head*)pNewAwb, profile_list);
        return (true);
    } else {
        LOGE("awb scene (%s) is repeated", pAwbFind->scene);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromAwbCalibV200ListByName
 *****************************************************************************/
bool GetAwbProfileFromAwbCalibV200ListBySceneName
(
    const list_head* profile_list,
    const char* scene,
    CalibDb_Awb_Calib_Para_V200_t** pAwbProfile,
    int *scene_index
) {
    list_head* p;
    p = profile_list->next;
    if(scene_index != NULL) {
        *scene_index = 0;
    }
    while (p != profile_list)
    {
        CalibDb_Awb_Calib_Para_V200_t* pProfile = container_of(p, CalibDb_Awb_Calib_Para_V200_t, listHead);
        //printf("%s  %p ", pProfile->scene, p);
        if (!strncmp(pProfile->scene, scene, sizeof(pProfile->scene))) {
            *pAwbProfile = pProfile;
            break;
        }
        p = p->next;
        if(scene_index != NULL) {
            *scene_index = *scene_index + 1;
        }
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromAwbCalibV200ListByIdx
 *****************************************************************************/
bool GetAwbProfileFromAwbCalibV200ListByIdx
(
    const list_head* profile_list,
    int idx,
    const CalibDb_Awb_Calib_Para_V200_t** pAwbProfile
) {
    list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Awb_Calib_Para_V200_t* pProfile = container_of(p, CalibDb_Awb_Calib_Para_V200_t, listHead);
            //printf("%p ", pProfile);
            *pAwbProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

/******************************************************************************
 * AddAwbCalibV200Profile2AwbCalibV201List
 *****************************************************************************/
bool AddAwbCalibV201Profile2AwbCalibV201List
(
    list_head *profile_list,
    CalibDb_Awb_Calib_Para_V201_t* pAddAwb
) {
    CalibDb_Awb_Calib_Para_V201_t* pNewAwb = NULL;

    // check if pAddAwb  is valid
    if(pAddAwb == NULL ) {
        LOGE("pNewAwb is invalid");
        return(false);
    }
    // check if scene already exists
    CalibDb_Awb_Calib_Para_V201_t* pAwbFind = NULL;
    GetAwbProfileFromAwbCalibV201ListBySceneName(profile_list, pAddAwb->scene, &pAwbFind);
    if(pAwbFind == NULL) {
        pNewAwb = (CalibDb_Awb_Calib_Para_V201_t*)malloc(sizeof(CalibDb_Awb_Calib_Para_V201_t));
        //printf("%p\n", pNewAwb);
        memcpy(pNewAwb, pAddAwb, sizeof(CalibDb_Awb_Calib_Para_V201_t));
        list_prepare_item(pNewAwb);
        list_add_tail((list_head*)pNewAwb, profile_list);
        return (true);
    } else {
        LOGE("awb scene (%s) is repeated", pAwbFind->scene);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromAwbCalibV201ListByName
 *****************************************************************************/
bool GetAwbProfileFromAwbCalibV201ListBySceneName
(
    const list_head* profile_list,
    const char* scene,
    CalibDb_Awb_Calib_Para_V201_t** pAwbProfile,
    int *scene_index
) {
    list_head* p;
    p = profile_list->next;
    if(scene_index != NULL) {
        *scene_index = 0;
    }
    while (p != profile_list)
    {
        CalibDb_Awb_Calib_Para_V201_t* pProfile = container_of(p, CalibDb_Awb_Calib_Para_V201_t, listHead);
        //printf("%s  %p ", pProfile->scene, p);
        if (!strncmp(pProfile->scene, scene, sizeof(pProfile->scene))) {
            *pAwbProfile = pProfile;
            break;
        }
        p = p->next;
        if(scene_index != NULL) {
            *scene_index = *scene_index + 1;
        }
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromAwbCalibV201ListByIdx
 *****************************************************************************/
bool GetAwbProfileFromAwbCalibV201ListByIdx
(
    const list_head* profile_list,
    int idx,
    const CalibDb_Awb_Calib_Para_V201_t** pAwbProfile
) {
    list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Awb_Calib_Para_V201_t* pProfile = container_of(p, CalibDb_Awb_Calib_Para_V201_t, listHead);
            //printf("%p ", pProfile);
            *pAwbProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

/******************************************************************************
 * AddAwbAdjustProfile2AwbAdjustList
 *****************************************************************************/
bool AddAwbAdjustProfile2AwbAdjustList
(
    list_head *profile_list,
    CalibDb_Awb_Adjust_Para_t* pAddAwb
) {
    CalibDb_Awb_Adjust_Para_t* pNewAwb = NULL;

    // check if pAddAwb  is valid
    if(pAddAwb == NULL ) {
        LOGE("pNewAwb is invalid");
        return(false);
    }
    // check if scene already exists
    CalibDb_Awb_Adjust_Para_t* pAwbFind = NULL;
    GetAwbProfileFromAwbAdjustListBySceneName(profile_list, pAddAwb->scene, &pAwbFind);
    if(pAwbFind == NULL) {
        pNewAwb = (CalibDb_Awb_Adjust_Para_t*)malloc(sizeof(CalibDb_Awb_Adjust_Para_t));
        //printf("%p\n", pNewAwb);
        memcpy(pNewAwb, pAddAwb, sizeof(CalibDb_Awb_Adjust_Para_t));
        list_prepare_item(pNewAwb);
        list_add_tail((list_head*)pNewAwb, profile_list);
        return (true);
    } else {
        LOGE("awb scene (%s) is repeated", pAwbFind->scene);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromAwbAdjustListByName
 *****************************************************************************/
bool GetAwbProfileFromAwbAdjustListBySceneName
(
    const list_head* profile_list,
    const char* scene,
    CalibDb_Awb_Adjust_Para_t** pAwbProfile
) {
    list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        CalibDb_Awb_Adjust_Para_t* pProfile = container_of(p, CalibDb_Awb_Adjust_Para_t, listHead);
        //printf("%s  %p ", pProfile->scene, p);
        if (!strncmp(pProfile->scene, scene, sizeof(pProfile->scene))) {
            *pAwbProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromAwbAdjustListByIdx
 *****************************************************************************/
bool GetAwbProfileFromAwbAdjustListByIdx
(
    const list_head* profile_list,
    int idx,
    const CalibDb_Awb_Adjust_Para_t** pAwbProfile
) {
    list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Awb_Adjust_Para_t* pProfile = container_of(p, CalibDb_Awb_Adjust_Para_t, listHead);
            //printf("%p ", pProfile);
            *pAwbProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

bool CamCalibdbAddBayernrV2Setting2DProfile
(
    struct list_head *profile_list,
    Calibdb_Bayernr_2Dparams_V2_t* pAdd

) {
    Calibdb_Bayernr_2Dparams_V2_t* pNew = NULL;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid\n");
        return(false);
    }
    // check if scene already exists
    Calibdb_Bayernr_2Dparams_V2_t* pFind = NULL;
    CamCalibdbGetBayernrV2Setting2DByName(profile_list, pAdd->snr_mode, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Bayernr_2Dparams_V2_t*)malloc(sizeof(Calibdb_Bayernr_2Dparams_V2_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Bayernr_2Dparams_V2_t));
        list_prepare_item(pNew);
        list_add_tail(&pNew->listItem, profile_list);
        return (true);
    } else {
        LOGD_ASHARP("bayernrV2 2dSetting snr_mode(%s) is repeated\n", pFind->snr_mode);
        return (false);
    }
}

bool CamCalibdbGetBayernrV2Setting2DByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Bayernr_2Dparams_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Bayernr_2Dparams_V2_t* pProfile = container_of(p, Calibdb_Bayernr_2Dparams_V2_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->snr_mode, p);
        if (!strncmp(pProfile->snr_mode, name, sizeof(pProfile->snr_mode))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetBayernrV2Setting2DByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Bayernr_2Dparams_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Bayernr_2Dparams_V2_t* pProfile = container_of(p, Calibdb_Bayernr_2Dparams_V2_t, listItem);
            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}


bool CamCalibdbAddBayernrV2Setting3DProfile
(
    struct list_head *profile_list,
    CalibDb_Bayernr_3DParams_V2_t* pAdd
) {
    CalibDb_Bayernr_3DParams_V2_t* pNew = NULL;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid\n");
        return(false);
    }
    // check if scene already exists
    CalibDb_Bayernr_3DParams_V2_t* pFind = NULL;
    CamCalibdbGetBayernrV2Setting3DByName(profile_list, pAdd->snr_mode, &pFind);
    if(pFind == NULL) {
        pNew = (CalibDb_Bayernr_3DParams_V2_t*)malloc(sizeof(CalibDb_Bayernr_3DParams_V2_t));
        memcpy(pNew, pAdd, sizeof(CalibDb_Bayernr_3DParams_V2_t));
        list_prepare_item(pNew);
        list_add_tail(&pNew->listItem, profile_list);
        return (true);
    } else {
        LOGD_ASHARP("bayernrV2 2dSetting snr_mode(%s) is repeated\n", pFind->snr_mode);
        return (false);
    }
}


bool CamCalibdbGetBayernrV2Setting3DByName
(
    const struct list_head* profile_list,
    char* name,
    CalibDb_Bayernr_3DParams_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        CalibDb_Bayernr_3DParams_V2_t* pProfile = container_of(p, CalibDb_Bayernr_3DParams_V2_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->snr_mode, p);
        if (!strncmp(pProfile->snr_mode, name, sizeof(pProfile->snr_mode))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetBayernrV2Setting3DByIdx
(
    const struct list_head* profile_list,
    int idx,
    CalibDb_Bayernr_3DParams_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Bayernr_3DParams_V2_t* pProfile = container_of(p, CalibDb_Bayernr_3DParams_V2_t, listItem);
            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

/******************************************************************************
 * AddAwbProfile2AwbList
 *****************************************************************************/
bool CamCalibdbAddBayernrV2Profile
(
    struct list_head *profile_list,
    CalibDb_Bayernr_V2_t* pAdd
) {
    CalibDb_Bayernr_V2_t* pNew = NULL;
    bool result = false;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid");
        return(false);
    }
    // check if scene already exists
    CalibDb_Bayernr_V2_t* pFind = NULL;
    CamCalibdbGetBayernrV2ProfileByName(profile_list, pAdd->modeName, &pFind);
    if(pFind == NULL) {
        pNew = (CalibDb_Bayernr_V2_t*)malloc(sizeof(CalibDb_Bayernr_V2_t));
        memcpy(pNew, pAdd, sizeof(CalibDb_Bayernr_V2_t));
        INIT_LIST_HEAD(&pNew->st2DParams.listHead);
        INIT_LIST_HEAD(&pNew->st3DParams.listHead);

        list_prepare_item(pNew);
        list_add_tail((struct list_head*)pNew, profile_list);

        /* add already linked 3dnr as well */
        struct list_head* p2DList = pAdd->st2DParams.listHead.next;
        while (p2DList != &pAdd->st2DParams.listHead) {
            Calibdb_Bayernr_2Dparams_V2_t *p2DParam = container_of(p2DList, Calibdb_Bayernr_2Dparams_V2_t, listItem);
            result = CamCalibdbAddBayernrV2Setting2DProfile(&pNew->st2DParams.listHead, p2DParam);
            if (result != true) {
                return (result);
            }
            p2DList = p2DList->next;
        }

        struct list_head* p3DList = pAdd->st3DParams.listHead.next;
        while (p3DList != &pAdd->st3DParams.listHead) {
            CalibDb_Bayernr_3DParams_V2_t* p3DParam = container_of(p3DList, CalibDb_Bayernr_3DParams_V2_t, listItem);
            result = CamCalibdbAddBayernrV2Setting3DProfile(&pNew->st3DParams.listHead, p3DParam);
            if (result != true) {
                return (result);
            }
            p3DList = p3DList->next;
        }

        return (true);
    } else {
        LOGD_ASHARP("bayernrV2 scene (%s) is repeated\n", pFind->modeName);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromListByName
 *****************************************************************************/
bool CamCalibdbGetBayernrV2ProfileByName
(
    const struct list_head* profile_list,
    char* name,
    CalibDb_Bayernr_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        CalibDb_Bayernr_V2_t* pProfile = container_of(p, CalibDb_Bayernr_V2_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}

/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetBayernrV2ProfileByIdx
(
    const struct list_head* profile_list,
    int idx,
    CalibDb_Bayernr_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            CalibDb_Bayernr_V2_t* pProfile = container_of(p, CalibDb_Bayernr_V2_t, listItem);
            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}


bool CamCalibdbAddYnrV2SettingProfile
(
    struct list_head *profile_list,
    Calibdb_Ynr_params_V2_t* pAdd

) {
    Calibdb_Ynr_params_V2_t* pNew = NULL;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid\n");
        return(false);
    }
    // check if scene already exists
    Calibdb_Ynr_params_V2_t* pFind = NULL;
    CamCalibdbGetYnrV2SettingByName(profile_list, pAdd->snr_mode, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Ynr_params_V2_t*)malloc(sizeof(Calibdb_Ynr_params_V2_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Ynr_params_V2_t));
        list_prepare_item(pNew);
        list_add_tail(&pNew->listItem, profile_list);
        return (true);
    } else {
        LOGD_ASHARP("bayernrV2 2dSetting snr_mode(%s) is repeated\n", pFind->snr_mode);
        return (false);
    }
}

bool CamCalibdbGetYnrV2SettingByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Ynr_params_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Ynr_params_V2_t* pProfile = container_of(p, Calibdb_Ynr_params_V2_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->snr_mode, p);
        if (!strncmp(pProfile->snr_mode, name, sizeof(pProfile->snr_mode))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetYnrV2SettingByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Ynr_params_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Ynr_params_V2_t* pProfile = container_of(p, Calibdb_Ynr_params_V2_t, listItem);
            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}



/******************************************************************************
 * AddAwbProfile2AwbList
 *****************************************************************************/
bool CamCalibdbAddYnrV2Profile
(
    struct list_head *profile_list,
    Calibdb_Ynr_V2_t* pAdd
) {
    Calibdb_Ynr_V2_t* pNew = NULL;
    bool result = false;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid");
        return(false);
    }
    // check if scene already exists
    Calibdb_Ynr_V2_t* pFind = NULL;
    CamCalibdbGetYnrV2ProfileByName(profile_list, pAdd->modeName, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Ynr_V2_t*)malloc(sizeof(Calibdb_Ynr_V2_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Ynr_V2_t));
        INIT_LIST_HEAD(&pNew->listHead);

        list_prepare_item(pNew);
        list_add_tail((struct list_head*)pNew, profile_list);

        /* add already linked 3dnr as well */
        struct list_head* pList = pAdd->listHead.next;
        while (pList != &pAdd->listHead) {
            Calibdb_Ynr_params_V2_t *pParam = container_of(pList, Calibdb_Ynr_params_V2_t, listItem);
            result = CamCalibdbAddYnrV2SettingProfile(&pNew->listHead, pParam);
            if (result != true) {
                return (result);
            }
            pList = pList->next;
        }
        return (true);
    } else {
        LOGD_ASHARP("bayernrV2 scene (%s) is repeated\n", pFind->modeName);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromListByName
 *****************************************************************************/
bool CamCalibdbGetYnrV2ProfileByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Ynr_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Ynr_V2_t* pProfile = container_of(p, Calibdb_Ynr_V2_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetYnrV2ProfileByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Ynr_V2_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Ynr_V2_t* pProfile = container_of(p, Calibdb_Ynr_V2_t, listItem);

            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

bool CamCalibdbAddCnrV1SettingProfile
(
    struct list_head *profile_list,
    Calibdb_Cnr_params_V1_t* pAdd

) {
    Calibdb_Cnr_params_V1_t* pNew = NULL;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid\n");
        return(false);
    }
    // check if scene already exists
    Calibdb_Cnr_params_V1_t* pFind = NULL;
    CamCalibdbGetCnrV1SettingByName(profile_list, pAdd->snr_mode, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Cnr_params_V1_t*)malloc(sizeof(Calibdb_Cnr_params_V1_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Cnr_params_V1_t));
        list_prepare_item(pNew);
        list_add_tail(&pNew->listItem, profile_list);
        return (true);
    } else {
        LOGD_ASHARP("cnrV1  snr_mode(%s) is repeated\n", pFind->snr_mode);
        return (false);
    }
}

bool CamCalibdbGetCnrV1SettingByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Cnr_params_V1_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Cnr_params_V1_t* pProfile = container_of(p, Calibdb_Cnr_params_V1_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->snr_mode, p);
        if (!strncmp(pProfile->snr_mode, name, sizeof(pProfile->snr_mode))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetCnrV1SettingByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Cnr_params_V1_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Cnr_params_V1_t* pProfile = container_of(p, Calibdb_Cnr_params_V1_t, listItem);
            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * AddAwbProfile2AwbList
 *****************************************************************************/
bool CamCalibdbAddCnrV1Profile
(
    struct list_head *profile_list,
    Calibdb_Cnr_V1_t* pAdd
) {
    Calibdb_Cnr_V1_t* pNew = NULL;
    bool result = false;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid");
        return(false);
    }
    // check if scene already exists
    Calibdb_Cnr_V1_t* pFind = NULL;
    CamCalibdbGetCnrV1ProfileByName(profile_list, pAdd->modeName, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Cnr_V1_t*)malloc(sizeof(Calibdb_Cnr_V1_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Cnr_V1_t));
        INIT_LIST_HEAD(&pNew->listHead);

        list_prepare_item(pNew);
        list_add_tail((struct list_head*)pNew, profile_list);

        /* add already linked 3dnr as well */
        struct list_head* pList = pAdd->listHead.next;
        while (pList != &pAdd->listHead) {
            Calibdb_Cnr_params_V1_t *pParam = container_of(pList, Calibdb_Cnr_params_V1_t, listItem);
            result = CamCalibdbAddCnrV1SettingProfile(&pNew->listHead, pParam);
            if (result != true) {
                return (result);
            }
            pList = pList->next;
        }
        return (true);
    } else {
        LOGD_ASHARP("cnrV1 scene (%s) is repeated\n", pFind->modeName);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromListByName
 *****************************************************************************/
bool CamCalibdbGetCnrV1ProfileByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Cnr_V1_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Cnr_V1_t* pProfile = container_of(p, Calibdb_Cnr_V1_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetCnrV1ProfileByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Cnr_V1_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Cnr_V1_t* pProfile = container_of(p, Calibdb_Cnr_V1_t, listItem);

            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}

bool CamCalibdbAddSharpV3SettingProfile
(
    struct list_head *profile_list,
    Calibdb_Sharp_params_V3_t* pAdd

) {
    Calibdb_Sharp_params_V3_t* pNew = NULL;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid\n");
        return(false);
    }
    // check if scene already exists
    Calibdb_Sharp_params_V3_t* pFind = NULL;
    CamCalibdbGetSharpV3SettingByName(profile_list, pAdd->snr_mode, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Sharp_params_V3_t*)malloc(sizeof(Calibdb_Sharp_params_V3_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Sharp_params_V3_t));
        list_prepare_item(pNew);
        list_add_tail(&pNew->listItem, profile_list);

        int num = get_list_num(profile_list);

        LOGD_ASHARP("%s:%d name:%s list:%p num:%d\n", __FUNCTION__, __LINE__, pNew->snr_mode, profile_list, num);
        return (true);
    } else {
        LOGD_ASHARP("sharpV3 snr_mode(%s) is repeated\n", pFind->snr_mode);
        return (false);
    }
}

bool CamCalibdbGetSharpV3SettingByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Sharp_params_V3_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Sharp_params_V3_t* pProfile = container_of(p, Calibdb_Sharp_params_V3_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->snr_mode, p);
        if (!strncmp(pProfile->snr_mode, name, sizeof(pProfile->snr_mode))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetSharpV3SettingByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Sharp_params_V3_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Sharp_params_V3_t* pProfile = container_of(p, Calibdb_Sharp_params_V3_t, listItem);
            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * AddAwbProfile2AwbList
 *****************************************************************************/
bool CamCalibdbAddSharpV3Profile
(
    struct list_head *profile_list,
    Calibdb_Sharp_V3_t* pAdd
) {
    Calibdb_Sharp_V3_t* pNew = NULL;
    bool result = false;

    // check if pAddAwb  is valid
    if(pAdd == NULL ) {
        LOGE_ASHARP("pAdd is invalid");
        return(false);
    }
    // check if scene already exists
    Calibdb_Sharp_V3_t* pFind = NULL;
    CamCalibdbGetSharpV3ProfileByName(profile_list, pAdd->modeName, &pFind);
    if(pFind == NULL) {
        pNew = (Calibdb_Sharp_V3_t*)malloc(sizeof(Calibdb_Sharp_V3_t));
        memcpy(pNew, pAdd, sizeof(Calibdb_Sharp_V3_t));
        INIT_LIST_HEAD(&pNew->listHead);

        list_prepare_item(&pNew->listItem);
        list_add_tail(&pNew->listItem, profile_list);
        int num = get_list_num(profile_list);

        LOGD_ASHARP("%s:%d add name:%s  list:%p num:%d \n", __FUNCTION__, __LINE__, pNew->modeName, profile_list, num);
        /* add already linked 3dnr as well */
        struct list_head* pList = pAdd->listHead.next;
        while (pList != &pAdd->listHead) {
            Calibdb_Sharp_params_V3_t *pParam = container_of(pList, Calibdb_Sharp_params_V3_t, listItem);
            result = CamCalibdbAddSharpV3SettingProfile(&pNew->listHead, pParam);
            if (result != true) {
                return (result);
            }
            pList = pList->next;
        }
        return (true);
    } else {
        LOGD_ASHARP("sharpV3 scene (%s) is repeated\n", pFind->modeName);
        return (false);
    }
}


/******************************************************************************
 * GetAwbProfileFromListByName
 *****************************************************************************/
bool CamCalibdbGetSharpV3ProfileByName
(
    const struct list_head* profile_list,
    char* name,
    Calibdb_Sharp_V3_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    while (p != profile_list)
    {
        Calibdb_Sharp_V3_t* pProfile = container_of(p, Calibdb_Sharp_V3_t, listItem);
        LOGD_ASHARP("%s:%d name:%s   now:%s  %p \n",
                    __FUNCTION__, __LINE__,
                    name, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            break;
        }
        p = p->next;
    }
    return(true);
}


/******************************************************************************
 * GetAwbProfileFromListByIdx
 *****************************************************************************/
bool CamCalibdbGetSharpV3ProfileByIdx
(
    const struct list_head* profile_list,
    int idx,
    Calibdb_Sharp_V3_t** ppProfile
) {
    struct list_head* p;
    p = profile_list->next;
    int cnt = 0;
    while (p != profile_list)
    {
        if (cnt == idx) {
            Calibdb_Sharp_V3_t* pProfile = container_of(p, Calibdb_Sharp_V3_t, listItem);

            //LOGD_ASHARP("%p ", pProfile);
            *ppProfile = pProfile;
            break;
        }
        cnt++;
        p = p->next;
    }
    return(true);
}


