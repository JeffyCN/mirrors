#include "cam_thread.h"
#include "../../include/common/return_codes.h"
#include <sys/prctl.h>

int CamThread::_threadLoop(void* user) {
  CamThread* const self = static_cast<CamThread*>(user);

  shared_ptr<CamThread> strong(self->mHoldSelf);
  weak_ptr<CamThread> weak(strong);
  //self->mHoldSelf = NULL;
  self->mHoldSelf.reset();
  if (self->mThread.name)
    prctl(PR_SET_NAME, (unsigned long)(self->mThread.name), 0, 0, 0);
  bool first = true;
  do {
    bool result = false;
    if (first) {
      first = false;
      result = self->readyToRun();
      osMutexLock(&self->mMutex);
      if (!result && !self->mExitPending) {
        osMutexUnlock(&self->mMutex);
        result = self->threadLoop();
      } else
        osMutexUnlock(&self->mMutex);
    } else {
      result = self->threadLoop();
    }

    osMutexLock(&self->mMutex);
    if (result == false || self->mExitPending) {
      self->mExitPending = true;
      self->mRunning = false;
      osMutexUnlock(&self->mMutex);
      break;
    } else
      osMutexUnlock(&self->mMutex);

    // Release our strong reference, to let a chance to the thread
    // to die a peaceful death.
    //strong = 0;
    strong.reset();
    // And immediately, re-acquire a strong reference for the next loop
    if (!weak.expired())
      strong = weak.lock();

  } while (strong.get() != NULL);

  return RET_SUCCESS;
}

CamThread::CamThread()
  : mExitPending(false),
    mRunning(false),
    mHoldSelf(NULL) {
  osMutexInit(&mMutex);
}
CamThread::~CamThread() {
  osMutexDestroy(&mMutex);
}


int CamThread::run(const char* name, int32_t priority) {
  bool res;
  osMutexLock(&mMutex);
  if (mRunning) {
    // thread already started
    return -RET_BUSY;
  }

  // reset status and exitPending to their default value, so we can
  // try again after an error happened (either below, or in readyToRun())
  mExitPending = false;
  mRunning = true;
  mHoldSelf = shared_from_this();
  //set name
  mThread.name = name;
  res = osThreadCreate(&mThread, _threadLoop, this);
  if (res) {
    mRunning = false;
    //mHoldSelf = NULL;  // "this" may have gone away after this.
    mHoldSelf.reset();
    osMutexUnlock(&mMutex);
    osThreadClose(&mThread);
    return -RET_FAILURE;
  }
  //set priority
  res = osThreadSetPriority(&mThread, (OSLAYER_THREAD_PRIO)priority);
  osMutexUnlock(&mMutex);
  return RET_SUCCESS;
}
int CamThread::readyToRun() {
  return RET_SUCCESS;
}


int CamThread::requestExitAndWait() {
  int ret;
  osMutexLock(&mMutex);
  mExitPending = true;
  osMutexUnlock(&mMutex);
  ret = osThreadClose(&mThread);
  // This next line is probably not needed any more, but is being left for
  // historical reference. Note that each interested party will clear flag.
  mExitPending = false;

  return ret;

}


