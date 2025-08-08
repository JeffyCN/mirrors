/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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

#include "RkAiqCamGroupHandleInt.h"

namespace RkCam {
#if (USE_NEWSTRUCT == 0)
XCamReturn RkAiqCamGroupAdpccHandleInt::updateConfig(bool needSync) {
  ENTER_ANALYZER_FUNCTION();

  XCamReturn ret = XCAM_RETURN_NO_ERROR;
  if (needSync)
    mCfgMutex.lock();
  // if something changed
  if (updateAtt) {
    mCurAtt = mNewAtt;
    rk_aiq_uapi_adpcc_SetAttrib(mAlgoCtx, &mCurAtt, false);
    updateAtt = false;
    sendSignal(mCurAtt.sync.sync_mode);
  }

  if (needSync)
    mCfgMutex.unlock();

  EXIT_ANALYZER_FUNCTION();
  return ret;
}

XCamReturn
RkAiqCamGroupAdpccHandleInt::setAttrib(rk_aiq_dpcc_attrib_V20_t att) {
  ENTER_ANALYZER_FUNCTION();

  XCamReturn ret = XCAM_RETURN_NO_ERROR;
  mCfgMutex.lock();

  // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
  // if something changed, set att to mNewAtt, and
  // the new params will be effective later when updateConfig
  // called by RkAiqCore
  bool isChanged = false;
  if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
          memcmp(&mNewAtt, &att, sizeof(att)))
      isChanged = true;
  else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
          memcmp(&mCurAtt, &att, sizeof(att)))
      isChanged = true;

  // if something changed
  if (isChanged) {
      mNewAtt = att;
      updateAtt = true;
      waitSignal(att.sync.sync_mode);
  }

  mCfgMutex.unlock();

  EXIT_ANALYZER_FUNCTION();
  return ret;
}

XCamReturn
RkAiqCamGroupAdpccHandleInt::getAttrib(rk_aiq_dpcc_attrib_V20_t *att) {
  ENTER_ANALYZER_FUNCTION();

  XCamReturn ret = XCAM_RETURN_NO_ERROR;

  if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
    mCfgMutex.lock();
    rk_aiq_uapi_adpcc_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
  } else {
    if (updateAtt) {
      memcpy(att, &mNewAtt, sizeof(mNewAtt));
      att->sync.done = false;
    } else {
      rk_aiq_uapi_adpcc_GetAttrib(mAlgoCtx, att);
      att->sync.sync_mode = mNewAtt.sync.sync_mode;
      att->sync.done = true;
    }
  }

  EXIT_ANALYZER_FUNCTION();
  return ret;
}
#endif
} // namespace RkCam
