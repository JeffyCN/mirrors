#ifndef _CAM_THREAD_H_
#define _CAM_THREAD_H_

using namespace std;

#include "oslayer/oslayer.h"
#include <memory>
#ifdef ANDROID_SHARED_PTR
#include "shared_ptr.h"

# ifndef UTIL_GTL_USE_STD_SHARED_PTR
using google::protobuf::internal::shared_ptr;
using google::protobuf::internal::enable_shared_from_this;
using google::protobuf::internal::weak_ptr;
# endif

#endif
class CamThread : public enable_shared_from_this<CamThread> {
 public:
  // Create a Thread object, but doesn't create or start the associated
  // thread. See the run() method.
  CamThread();
  virtual             ~CamThread();

  // Start the thread in threadLoop() which needs to be implemented.
  virtual int    run(const char* name = 0,
                     int32_t priority = OSLAYER_THREAD_PRIO_NORMAL
                    );
  virtual int readyToRun();
  virtual bool        threadLoop() = 0;

  // Call requestExit() and wait until this object's thread exits.
  // BE VERY CAREFUL of deadlocks. In particular, it would be silly to call
  // this function from this object's thread. Will return WOULD_BLOCK in
  // that case.
  int    requestExitAndWait();
  bool   isRunning() {
    bool ret;
    osMutexLock(&mMutex);
    ret = mRunning;
    osMutexUnlock(&mMutex);
    return ret;
  };
 private:
  CamThread& operator=(const CamThread&);
  static  int             _threadLoop(void* user);
  // note that all accesses of mExitPending and mRunning need to hold mLock
  volatile bool           mExitPending;
  volatile bool           mRunning;
  shared_ptr<CamThread> mHoldSelf;
  osThread mThread;
  osMutex  mMutex;
};
#endif

