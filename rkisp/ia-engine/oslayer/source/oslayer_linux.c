/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * Module    : Linux Kernel Abstraction Layer
 *
 * Hierarchy :
 *
 * Purpose   : Encapsulates and abstracts services from different operating
 *             system, including user-mode as well as kernel-mode services.
 ******************************************************************************/
#ifdef LINUX

#include "oslayer.h"

#ifdef OSLAYER_TIMESTAMP

/* Parameters used to convert the timespec values: */
#define OS_MSEC_PER_SEC    1000L
#define OS_USEC_PER_MSEC   1000L
#define OS_NSEC_PER_USEC   1000L
#define OS_NSEC_PER_MSEC   1000000L
#define OS_USEC_PER_SEC    1000000L
#define OS_NSEC_PER_SEC    1000000000L
#define OS_FSEC_PER_SEC    1000000000000000LL


/**
 * timespec_to_ns - Convert timespec to nanoseconds
 * @ts:         pointer to the timespec variable to be converted
 *
 * Returns the scalar nanosecond representation of the timespec
 * parameter.
 */
static inline int64_t timespec_to_ns(const struct timespec* ts) {
  return (((int64_t)ts->tv_sec) * OS_NSEC_PER_SEC) + ts->tv_nsec;
}

static inline int64_t timespec_to_us(const struct timeval* ts) {
  return (((int64_t)ts->tv_sec) * OS_USEC_PER_SEC) + ts->tv_usec;
}


/******************************************************************************
 *  osTimeStampUs()
 ******************************************************************************
 *  @brief  Returns a 64-bit timestamp [us]
 *
 *  @param  pTimeStamp     Reference of the 64-bit timestamp
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Timestamp successfully returned
 *
 ******************************************************************************/
int32_t osTimeStampUs(int64_t* pTimeStamp) {
#ifndef OSLAYER_KERNEL
  struct timeval tval;

  OSLAYER_ASSERT(pTimeStamp == NULL);

  gettimeofday(&tval, NULL);
  *pTimeStamp = timespec_to_us(&tval);
#endif

  return (OSLAYER_OK);
}



/******************************************************************************
 *  osTimeStampNs()
 ******************************************************************************
 *  @brief  Returns a 64-bit timestamp [ns]
 *
 *  @param  pTimeStamp     Reference of the 64-bit timestamp
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Timestamp successfully returned
 *
 ******************************************************************************/
int32_t osTimeStampNs(int64_t* pTimeStamp) {
#ifndef OSLAYER_KERNEL
  // struct timespec tspec;

  OSLAYER_ASSERT(pTimeStamp == NULL);

  // clock_gettime(CLOCK_MONOTONIC_RAW, &tspec);
  // clock_gettime(CLOCK_MONOTONIC, &tspec);
  // *pTimeStamp = timespec_to_ns( &tspec );
#endif

  return (OSLAYER_OK);
}
#endif /* OSLAYER_TIMESTAMP */





#ifdef OSLAYER_EVENT
/******************************************************************************
 *  osEventInit()
 ******************************************************************************
 *  @brief  Initialize an event object.
 *
 *  Init an event. Automatic reset flag as well as initial state could be set,
 *  but event is not signaled. Automatic reset means osEventReset() must
 *  not be called to set event state to zero. This also implies that all
 *  waiting threads, otherwise only one thread, will be restarted.
 *
 *  @param  pEvent         Reference of the event object
 *
 *  @param  Automatic      Automatic Reset flag
 *
 *  @param  InitState      Initial state of the event object (true/false)
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Event successfully created
 *  @retval OSLAYER_ERROR  Event is not created
 *
 ******************************************************************************/
int32_t osEventInit(osEvent* pEvent, int32_t Automatic, int32_t InitState) {
  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pEvent->automatic = Automatic;
  pEvent->state = InitState;
  pthread_cond_init(&pEvent->cond, 0);
  pthread_mutex_init(&pEvent->mutex, 0);
#else
  /* Automatic reset is always true and initial state is not applicable since
   * state is not of type bool and more than one thread can wait for
   * completion (signal).
   */
  init_completion(&pEvent->x);
#endif

  return OSLAYER_OK;
}


/******************************************************************************
 *  osEventSignal()
 ******************************************************************************
 *  @brief  Set the event state to true.
 *
 *  Set the state of the event object to true and signal waiting thread(s).
 *
 *  @param  pEvent                    Reference of the event object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Signal send successfully
 *  @retval OSLAYER_ERROR             Tried to signal a not initialized event
 *  @retval OSLAYER_OPERATION_FAILED  Signal not send
 *
 ******************************************************************************/
int32_t osEventSignal(osEvent* pEvent) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;

  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pEvent->mutex);
  if (pEvent->state == false) {
    pEvent->state = true;
    if (pEvent->automatic)
      pthread_cond_broadcast(&pEvent->cond);
    else
      pthread_cond_signal(&pEvent->cond);
  }

  Ret = OSLAYER_OK;
  pthread_mutex_unlock(&pEvent->mutex);
#else
  Ret = OSLAYER_OK;
  complete(&pEvent->x);
  /* complete_all(&pEvent->x); */
#endif

  return Ret;
}


/******************************************************************************
 *  osEventReset()
 ******************************************************************************
 *  @brief  Reset the event state to true.
 *
 *  Reset the state of the event object to false.
 *
 *  @param  pEvent                    Reference of the event object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Event reset successfully
 *  @retval OSLAYER_ERROR             Tried to reset a not initialized event
 *  @retval OSLAYER_OPERATION_FAILED  Event not reset
 *
 ******************************************************************************/
int32_t osEventReset(osEvent* pEvent) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;

  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pEvent->mutex);

  pEvent->state = false;

  Ret = OSLAYER_OK;
  pthread_mutex_unlock(&pEvent->mutex);
#else
  Ret = OSLAYER_OK;

  if (pEvent->x.done)
    pEvent->x.done--;
#endif

  return Ret;
}


/******************************************************************************
 *  osEventPulse()
 ******************************************************************************
 *  @brief  Pulse the event false -> true -> false.
 *
 *  Pulse the state of the event object with the following sequence:
 *  false -> true -> false and signal waiting thread(s).
 *
 *  @param  pEvent                    Reference of the event object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Event pulsed successfully
 *  @retval OSLAYER_ERROR             Tried to pulse a not initialized event
 *  @retval OSLAYER_OPERATION_FAILED  Event not pulsed
 *
******************************************************************************/
int32_t osEventPulse(osEvent* pEvent) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;

  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pEvent->mutex);

  if (pEvent->automatic)
    pthread_cond_broadcast(&pEvent->cond);
  else
    pthread_cond_signal(&pEvent->cond);

  pEvent->state = false;
  Ret = OSLAYER_OK;
  pthread_mutex_unlock(&pEvent->mutex);
#else
  Ret = OSLAYER_OK;

  complete(&pEvent->x);
  /*complete_all(&pEvent->x);*/
#endif

  return Ret;
}


/******************************************************************************
 *  osEventWait()
 ******************************************************************************
 *  @brief  Blocking wait for event to be true.
 *
 *  Wait for the state of the event object becoming true and block calling
 *  thread. The function call returns immediatly, if event is already signaled.
 *
 *  @param  pEvent                    Reference of the event object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Wait for event succeeded and function
 *                                    returned due to signal sent
 *  @retval OSLAYER_ERROR             Tried to wait for a not initialized event
 *  @retval OSLAYER_OPERATION_FAILED  Wait for event failed
 *
 ******************************************************************************/
int32_t osEventWait(osEvent* pEvent) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;

  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pEvent->mutex);

  if (!pEvent->state)
    pthread_cond_wait(&pEvent->cond, &pEvent->mutex);

  if (pEvent->automatic)
    pEvent->state = 0;

  Ret = OSLAYER_OK;
  pthread_mutex_unlock(&pEvent->mutex);
#else
  if (wait_for_completion_interruptible(&pEvent->x) == -ERESTARTSYS)
    Ret = OSLAYER_SIGNAL_PENDING;
  else
    Ret = OSLAYER_OK;
#endif

  return Ret;
}


/******************************************************************************
 *  osEventTimedWait()
 ******************************************************************************
 *  @brief  Blocking wait with timeout for event to be true.
 *
 *  Wait for the state of the event object becoming true and block calling
 *  thread. The function call returns immediatly, if event is already signaled.
 *  If specified timeout elapses, the function returns with apropriate timeout
 *  error code.
 *
 *  @param  pEvent      reference of the event object
 *
 *  @param  msec        timeout value in milliseconds
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Wait for event succeeded and function
 *                                    returned due to signal sent
 *  @retval OSLAYER_TIMEOUT           Wait for event succeeded and function
 *                                    returned due to timeout and no signal
 *                                    was sent
 *  @retval OSLAYER_OPERATION_FAILED  Wait for event failed
 *
 ******************************************************************************/
int32_t osEventTimedWait(osEvent* pEvent, uint32_t msec) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;

  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pEvent->mutex);

  if (!pEvent->state) {
    struct timeval tval;
    struct timespec tspec;
    int32_t i_res;

    gettimeofday(&tval, NULL);
    msec = 1000 * msec + tval.tv_usec;
    tspec.tv_sec = tval.tv_sec + msec / 1000000;
    tspec.tv_nsec = (msec % 1000000) * 1000;
    i_res = pthread_cond_timedwait(&pEvent->cond,
                                   &pEvent->mutex,
                                   &tspec);
    if (i_res == 0)
      Ret = OSLAYER_OK;
    else if (i_res == ETIMEDOUT)
      /* Needed because it seems that the return value of
       * pthread_cond_timedwait is determined before the mutex
       * is locked again in that function */
      Ret =  pEvent->state ? OSLAYER_OK : OSLAYER_TIMEOUT;
    else
      Ret = OSLAYER_OPERATION_FAILED;
  } else
    Ret = OSLAYER_OK;

  if (pEvent->automatic)
    pEvent->state = false;

  pthread_mutex_unlock(&pEvent->mutex);
#else
  if (wait_for_completion_interruptible_timeout(&pEvent->x, msecs_to_jiffies(msec)) == -ERESTARTSYS)
    Ret = OSLAYER_SIGNAL_PENDING;
  else
    Ret = OSLAYER_OK;
#endif

  return Ret;
}


/******************************************************************************
 *  osEventDestroy()
 ******************************************************************************
 *  @brief  Destroy the event.
 *
 *  Destroy the event and free resources associated with event object.
 *
 *  @param  pEvent      Reference of the event object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
int32_t osEventDestroy(osEvent* pEvent) {
  /* check pointer */
  OSLAYER_ASSERT(pEvent == NULL);

#ifndef OSLAYER_KERNEL
  pthread_cond_destroy(&pEvent->cond);
  pthread_mutex_destroy(&pEvent->mutex);
#endif

  return OSLAYER_OK;
}
#endif /* OSLAYER_EVENT */







#ifdef OSLAYER_MUTEX
/******************************************************************************
 *  osMutextInit()
 ******************************************************************************
 *  @brief  Initialize a mutex object.
 *
 *  @param  pMutex         Reference of the mutex object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Mutex successfully created
 *  @retval OSLAYER_ERROR  Mutex is not created
 *
 ******************************************************************************/
int32_t osMutexInit(osMutex* pMutex) {
#ifndef OSLAYER_KERNEL
  pthread_mutexattr_t mutex_attr;


  /* check pointer */
  OSLAYER_ASSERT(pMutex == NULL);

  pthread_mutexattr_init(&mutex_attr);
  /* The following code line leads to a warning if __USE_UNIX98 is not defined;
   * since it is not vital it has been commented out
   * pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK_NP);
   */
  pthread_mutex_init(&pMutex->handle, &mutex_attr);
  pthread_mutexattr_destroy(&mutex_attr);
#else
  sema_init(&pMutex->sem, 1);
#endif

  return OSLAYER_OK;
}


/******************************************************************************
 *  osMutexLock()
 ******************************************************************************
 *  @brief  Lock a mutex object.
 *
 *          Lock the mutex. Thread will be blocked if mutex already locked.
 *
 *  @param  pMutex         Reference of the mutex object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Mutex successfully locked
 *
 ******************************************************************************/
int32_t osMutexLock(osMutex* pMutex) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;


  /* check pointer */
  OSLAYER_ASSERT(pMutex == NULL);

#ifndef OSLAYER_KERNEL
  if (!pthread_mutex_lock(&pMutex->handle))
#else
  if (!down_interruptible(&pMutex->sem))
#endif
    Ret = OSLAYER_OK;
  else
    Ret = OSLAYER_SIGNAL_PENDING;

  return Ret;
}


/******************************************************************************
 *  osMutexUnlock()
 ******************************************************************************
 *  @brief  Unlock a mutex object.
 *
 *  @param  pMutex         Reference of the mutex object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Mutex successfully unlocked
 *
 ******************************************************************************/
int32_t osMutexUnlock(osMutex* pMutex) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;


  /* check pointer */
  OSLAYER_ASSERT(pMutex == NULL);

#ifndef OSLAYER_KERNEL
  if (!pthread_mutex_unlock(&pMutex->handle))
    Ret = OSLAYER_OK;
  else
    Ret = OSLAYER_OPERATION_FAILED;

  return Ret;
#else
  up(&pMutex->sem);
  return OSLAYER_OK;
#endif

}


/******************************************************************************
 *  osMutexTryLock()
 ******************************************************************************
 *  @brief  Try to lock a mutex object.
 *
 *  Try to lock mutex in Non-Blocking mode. Returns OSLAYER_OK if successful.
 *
 *  @param  pMutex                    Reference of the mutex object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Mutex successfully locked
 *  @retval OSLAYER_OPERATION_FAILED  Mutex not locked (already locked by
 *                                    someone else)
 *
 ******************************************************************************/
int32_t osMutexTryLock(osMutex* pMutex) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;
  int32_t res;


  /* check pointer */
  OSLAYER_ASSERT(pMutex == NULL);

#ifndef OSLAYER_KERNEL
  res = pthread_mutex_trylock(&pMutex->handle);
  switch (res) {
    case 0:
      Ret = OSLAYER_OK;
      break;
    case EBUSY:
      Ret = OSLAYER_TIMEOUT;
      break;
    default:
      Ret = OSLAYER_OPERATION_FAILED;
      break;
  }
#else
  if (!down_trylock(&pMutex->sem))
    Ret = OSLAYER_OK;
  else
    Ret = OSLAYER_TIMEOUT;
#endif

  return Ret;
}


/******************************************************************************
 *  osMutexDestroy()
 ******************************************************************************
 *  @brief  Destroy a mutex object.
 *
 *  @param  pMutex      Reference of the mutex object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
int32_t osMutexDestroy(osMutex* pMutex) {
  /* check pointer */
  OSLAYER_ASSERT(pMutex == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_destroy(&pMutex->handle);
#endif

  return OSLAYER_OK;
}
#endif /* OSLAYER_MUTEX */









#ifdef OSLAYER_SEMAPHORE
/******************************************************************************
 *  osSemaphoreInit()
 ******************************************************************************
 *  @brief  Init a semaphore with init count.
 *
 *  @param  pSem           Reference of the semaphore object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Semaphore successfully created
 *  @retval OSLAYER_ERROR  Semaphore is not created
 *
 ******************************************************************************/
int32_t osSemaphoreInit(osSemaphore* pSem, int32_t init_count) {
  /* check pointer */
  OSLAYER_ASSERT(pSem == NULL);

#ifndef OSLAYER_KERNEL
  pSem->count = init_count;
  pthread_cond_init(&pSem->cond, 0);
  pthread_mutex_init(&pSem->mutex, 0);
#else
  sema_init(&pSem->sem, init_count);
#endif

  return OSLAYER_OK;
}


/******************************************************************************
 *  osSemaphoreTimedWait()
 ******************************************************************************
 *  @brief  Decrease the semaphore value in blocking mode, but with timeout.
 *
 *  @param  pSem                      Reference of the semaphore object
 *
 *  @param  msec                      Timeout value in milliseconds
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Wait for semaphore succeeded and
 *                                    function returned due to signal sent
 *  @retval OSLAYER_TIMEOUT           Wait for semaphore succeeded and
 *                                    function returned due to timeout and
 *                                    no signal was sent
 *  @retval OSLAYER_OPERATION_FAILED  Wait for semaphore failed
 *  @retval OSLAYER_ERROR             Tried to wait for a not initialized
 *                                    semaphore
 *
 ******************************************************************************/
int32_t osSemaphoreTimedWait(osSemaphore* pSem, uint32_t msec) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;


  /* check pointer */
  OSLAYER_ASSERT(pSem == NULL);

#ifndef OSLAYER_KERNEL
  Ret = OSLAYER_OK;
  pthread_mutex_lock(&pSem->mutex);

  if (pSem->count == 0) {
    struct timeval tval;
    struct timespec tspec;
    int32_t res;

    gettimeofday(&tval, NULL);
    msec = 1000 * msec + tval.tv_usec;
    tspec.tv_sec = tval.tv_sec + msec / 1000000;
    tspec.tv_nsec = (msec % 1000000) * 1000;
    res = pthread_cond_timedwait(&pSem->cond, &pSem->mutex, &tspec);

    if (res == ETIMEDOUT)
      /* Needed because it seems that the return value of
       * pthread_cond_timedwait is determined before the mutex
       * is locked again in that function */
      Ret = pSem->count ? OSLAYER_OK : OSLAYER_TIMEOUT;
    else if (res != 0)
      Ret = OSLAYER_OPERATION_FAILED;
  }

  if (Ret == OSLAYER_OK)
    pSem->count--;

  pthread_mutex_unlock(&pSem->mutex);
#endif

  return Ret;
}


/******************************************************************************
 *  osSemaphoreWait()
 ******************************************************************************
 *  @brief  Decrease the semaphore value in blocking mode.
 *
 *  @param  pSem                      Reference of the semaphore object
 *
 *  @param  msec                      Timeout value in milliseconds
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Wait for semaphore succeeded and
 *                                    function returned due to signal sent
 *  @retval OSLAYER_OPERATION_FAILED  Wait for semaphore failed
 *  @retval OSLAYER_ERROR             Tried to wait for a not initialized
 *                                    semaphore
 *
 ******************************************************************************/
int32_t osSemaphoreWait(osSemaphore* pSem) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;


  /* check pointer */
  OSLAYER_ASSERT(pSem == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pSem->mutex);

  if ((pSem->count == 0) && (pthread_cond_wait(&pSem->cond, &pSem->mutex) != 0))
    Ret = OSLAYER_OPERATION_FAILED;
  else {
    Ret = OSLAYER_OK;
    pSem->count--;
  }

  pthread_mutex_unlock(&pSem->mutex);
#else
  if (!down_interruptible(&pSem->sem))
    Ret = OSLAYER_OK;
  else
    Ret = OSLAYER_SIGNAL_PENDING;
#endif

  return Ret;
}


/******************************************************************************
 *  osSemaphoreTryWait()
 ******************************************************************************
 *  @brief  Try to wait for a semaphore object.
 *
 *          Try to decrease the semaphore value in non-blocking mode. This
 *          functionality is realized internally with a timed wait called
 *          with a timeout value of zero.
 *
 *  @param  pSem                      Reference of the mutex object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Wait for semaphore succeeded and
 *                                    function returned due to signal sent
 *  @retval OSLAYER_TIMEOUT           Wait for semaphore succeeded and
 *                                    function returned due to timeout and
 *                                    no signal was sent
 *  @retval OSLAYER_OPERATION_FAILED  Wait for semaphore failed
 *  @retval OSLAYER_ERROR             Tried to wait for a not initialized
 *                                    semaphore
 *
 ******************************************************************************/
int32_t osSemaphoreTryWait(osSemaphore* pSem) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;


  /* check pointer */
  OSLAYER_ASSERT(pSem == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pSem->mutex);
  if (pSem->count == 0)
    Ret = OSLAYER_TIMEOUT;
  else {
    pSem->count--;
    Ret = OSLAYER_OK;
  }

  pthread_mutex_unlock(&pSem->mutex);
#else
  if (!down_trylock(&pSem->sem))
    ret = OSLAYER_OK;
  else
    ret = OSLAYER_TIMEOUT;
#endif

  return Ret;
}


/******************************************************************************
 *  osSemaphorePost()
 ******************************************************************************
 *  @brief  Increase the semaphore value.
 *
 *  @param  pSem                      Reference of the semaphore object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Semaphore value successfully increased
 *  @retval OSLAYER_ERROR             Tried to increase the value of a not
 *                                    initialized semaphore object
 *  @retval OSLAYER_OPERATION_FAILED  Increase of semaphore value failed
 *
 ******************************************************************************/
int32_t osSemaphorePost(osSemaphore* pSem) {
  OSLAYER_STATUS Ret = OSLAYER_ERROR;


  /* check pointer */
  OSLAYER_ASSERT(pSem == NULL);

#ifndef OSLAYER_KERNEL
  pthread_mutex_lock(&pSem->mutex);

  if (pSem->count == 0x7fffffffL) {
    Ret = OSLAYER_OPERATION_FAILED;
  } else {
    /* The following behaviour is simulated:
     * The application can create a semaphore with an initial count of zero.
     * This sets the semaphore's state to nonsignaled and blocks all threads
     * from accessing the protected resource. When the application finishes its
     * initialization, it uses osSemaphorePost() to increase the count to
     * its maximum value, to permit normal access to the protected resource.
       */
    if (pSem->count++ == 0)
      pthread_cond_signal(&pSem->cond);

    Ret = OSLAYER_OK;
  }
  pthread_mutex_unlock(&pSem->mutex);
#else
  up(&pMutex->sem);
  Ret = OSLAYER_OK;
#endif

  return Ret;
}


/******************************************************************************
 *  osSemaphoreDestroy()
 ******************************************************************************
 *  @brief  Destroy the semaphore object.
 *
 *  @param  pSem        Reference of the semaphore object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
int32_t osSemaphoreDestroy(osSemaphore* pSem) {
  /* check pointer */
  OSLAYER_ASSERT(pSem == NULL);

#ifndef OSLAYER_KERNEL
  pthread_cond_destroy(&pSem->cond);
  pthread_mutex_destroy(&pSem->mutex);
#endif

  return OSLAYER_OK;
}
#endif /* OSLAYER_SEMAPHORE */








#ifdef OSLAYER_ATOMIC

#ifndef OSLAYER_KERNEL
static osMutex gAtomicMutex; /* variable to enable "atomic operations" in user mode */
static int gAtomicInitCnt = 0; /* conuter of Atomic init and destroy */
#endif /* OSLAYER_KERNEL */

/******************************************************************************
 *  osAtomicInit()
 ******************************************************************************
 *  @brief  Initialize atomic operation functionality.
 *
 *  This function must be called before any other osAtomicXXX call.
 *
 ******************************************************************************/
int32_t osAtomicInit() {
#ifndef OSLAYER_KERNEL
  /* initialize atomic mutex */
  if (gAtomicInitCnt++ == 0) /* TODO: multithread will be wrong */
    osMutexInit(&gAtomicMutex);
#endif /* OSLAYER_KERNEL */
  return OSLAYER_OK;
}

/******************************************************************************
 *  osAtomicShutdown()
 ******************************************************************************
 *  @brief  Shutdown atomic operation functionality.
 *
 *  This function must be called before process is terminated
 *  when osAtomicInit has been called before.
 *
 ******************************************************************************/
int32_t osAtomicShutdown() {
#ifndef OSLAYER_KERNEL
  /* destroy atomic mutex */
  /* TODO: multithread will be wrong */
  if ((gAtomicInitCnt > 0) && (--gAtomicInitCnt == 0))
    osMutexDestroy(&gAtomicMutex);
#endif /* OSLAYER_KERNEL */
  return OSLAYER_OK;
}

/******************************************************************************
 *  osAtomicTestAndClearBit()
 ******************************************************************************
 *  @brief  Test and set a bit position atomically.
 *
 *  Test if a bit position inside a variable is set and clears the bit
 *  afterwards. The complete operation is atomic (includes IRQ safety).
 *
 *  @param  pVar           32-bit unsigned variable to be modified
 *  @param  bitpos         Bit to be tested and cleared
 *
 *  @return                *pVar & (1 << bitpos)
 *
 ******************************************************************************/
uint32_t osAtomicTestAndClearBit(uint32_t* pVar, uint32_t bitpos) {
#ifndef OSLAYER_KERNEL
  uint32_t ret;

  OSLAYER_ASSERT(bitpos < 32);

  /* Lock the atomic mutex */
  (void)osMutexLock(&gAtomicMutex);

  /* Test bit */
  ret = *pVar & (1 << bitpos);
  /* Clear bit */
  *pVar &= ~(1 << bitpos);

  /* Unlock the atomic mutex */
  (void)osMutexUnlock(&gAtomicMutex);

  return ret;
#else  /* OSLAYER_KERNEL */
  /* TODO: implement it */
  OSLAYER_ASSERT(0);
#endif /* OSLAYER_KERNEL */
}

/******************************************************************************
 *  osAtomicIncrement()
 ******************************************************************************
 *  @brief  Increments a 32-bit unsigned variable atomically.
 *
 *  Increments a 32-bit unsigned variable atomically and returns the value of
 *  the variable after the increment operation. The complete operation is
 *  atomic (includes IRQ safety).
 *
 *  @param  pVar           32-bit unsigned variable to be modified
 *
 *  @return                ++(*pVar)
 *
 ******************************************************************************/
uint32_t osAtomicIncrement(uint32_t* pVar) {
#ifndef OSLAYER_KERNEL
  uint32_t ret;

  /* Lock the atomic mutex */
  (void)osMutexLock(&gAtomicMutex);

  /* increment */
  ret = ++(*pVar);

  /* Unlock the atomic mutex */
  (void)osMutexUnlock(&gAtomicMutex);

  return ret;
#else  /* OSLAYER_KERNEL */
  /* TODO: implement it */
  OSLAYER_ASSERT(0);
#endif /* OSLAYER_KERNEL */
}

/******************************************************************************
 *  osAtomicDecrement()
 ******************************************************************************
 *  @brief  Decrements a 32-bit unsigned variable atomically.
 *
 *  Decrements a 32-bit unsigned variable atomically and returns the value of
 *  the variable after the decrement operation. The complete operation is
 *  atomic (includes IRQ safety).
 *
 *  @param  pVar           32-bit unsigned variable to be modified
 *
 *  @return                --(*pVar)
 *
 ******************************************************************************/
uint32_t osAtomicDecrement(uint32_t* pVar) {
#ifndef OSLAYER_KERNEL
  uint32_t ret;

  /* Lock the atomic mutex */
  (void)osMutexLock(&gAtomicMutex);

  /* increment */
  ret = --(*pVar);

  /* Unlock the atomic mutex */
  (void)osMutexUnlock(&gAtomicMutex);

  return ret;
#else  /* OSLAYER_KERNEL */
  /* TODO: implement it */
  OSLAYER_ASSERT(0);
#endif /* OSLAYER_KERNEL */
}

/******************************************************************************
 * osAtomicSetBit()
 ******************************************************************************
 * @brief  Set a bit position atomically.
 *
 * Set a bit position inside a variable.
 * The operation is atomic (includes IRQ safety).
 *
 * @param  pVar           32-bit unsigned variable to be modified
 + @param  bitpos         Bit to be set
 *
 + @return                always OSLAYER_OK
 ******************************************************************************/
int32_t osAtomicSetBit(uint32_t* pVar, uint32_t bitpos) {
#ifndef OSLAYER_KERNEL
  OSLAYER_ASSERT(bitpos < 32);

  /* Lock the atomic mutex */
  (void)osMutexLock(&gAtomicMutex);

  /* Set bit */
  *pVar |= (1 << bitpos);

  /* Unlock the atomic mutex */
  (void)osMutexUnlock(&gAtomicMutex);
#else /* OSLAYER_KERNEL */
  /* TODO: implement it */
  OSLAYER_ASSERT(0);
#endif /* OSLAYER_KERNEL */
  return OSLAYER_OK;
}

/******************************************************************************
 *  osAtomicSet()
 ******************************************************************************
 * @brief  Set value atomically.
 *
 * Set variable to value. The operation is atomic (includes IRQ safety).
 *
 * @param  pVar           32-bit unsigned variable to be modified
 * @param  value          Value to be set
 *
 * @return                always OSLAYER_OK
 ******************************************************************************/
int32_t osAtomicSet(uint32_t* pVar, uint32_t value) {
#ifndef OSLAYER_KERNEL
  /* Lock the atomic mutex */
  (void)osMutexLock(&gAtomicMutex);

  /* Set variable */
  *pVar = value;

  /* Unlock the atomic mutex */
  (void)osMutexUnlock(&gAtomicMutex);
#else /* OSLAYER_KERNEL */
  /* TODO: implement it */
  OSLAYER_ASSERT(0);
#endif /* OSLAYER_KERNEL */
  return OSLAYER_OK;
}

uint32_t osAtomicCompareAndSwap(uint32_t* pVar, uint32_t oldVal, uint32_t newVal) {
  uint32_t result = 0;
#ifndef OSLAYER_KERNEL
  /* Lock the atomic mutex */
  (void)osMutexLock(&gAtomicMutex);
  result = *pVar;
  if (*pVar == oldVal)
    *pVar = newVal;
  /* Unlock the atomic mutex */
  (void)osMutexUnlock(&gAtomicMutex);
#else /* OSLAYER_KERNEL */
  /* TODO: implement it */
  OSLAYER_ASSERT(0);
#endif /* OSLAYER_KERNEL */
  return result;
}
#endif /* OSLAYER_ATOMIC */





#ifdef OSLAYER_THREAD
/******************************************************************************
 *  osThreadProc()
 ******************************************************************************
 *
 *  Wrapper for the thread function.
 *
 ******************************************************************************/
#ifndef OSLAYER_KERNEL
static void* osThreadProc(void* pParams) {
  osThread* pThread = (osThread*)pParams;

  /* check error(s) */
  if (pParams == NULL)
    return ((void*) - 1);

  pThread->pThreadFunc(pThread->p_arg);
  osEventSignal(&pThread->exit_event);

  return ((void*) 1);
}

#else

static int32_t* osThreadProc(void* pParams) {
  int32_t ret = 0;
  osThread* pThread = (osThread*)pParams;

  /* check error(s) */
  if (pParams == NULL)
    return ((void*) - 1);

  /* max 16 characters */
  daemonize("OS-Thread");
  allow_signal(SIGTERM);
  ret = pThread->pThreadFunc(pThread->p_arg);

  /*osEventSignal(&pThread->exit_event);*/
  complete_and_exit(&pThread->exit_event->x, ret);

  return ret;
}
#endif


/******************************************************************************
 *  osThreadCreate()
 ******************************************************************************
 *  @brief  Create a thread.
 *
 *  @param  pThread                   Reference of the semaphore object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Thread object created successfully
 *  @retval OSLAYER_OPERATION_FAILED  Creation of thread object failed
 *
 ******************************************************************************/
int32_t osThreadCreate(osThread* pThread, osThreadFunc thread_func/*int32_t (*osThreadFunc)(void *)*/, void* arg) {
  OSLAYER_STATUS res = OSLAYER_OPERATION_FAILED;

  /* check error(s) */
  OSLAYER_ASSERT(pThread == NULL);
  OSLAYER_ASSERT(osThreadFunc == NULL);

  if (osEventInit(&pThread->exit_event, 1, 0) != OSLAYER_OK) {
    return OSLAYER_OPERATION_FAILED;
  }

#ifndef OSLAYER_KERNEL
  if (osMutexInit(&pThread->access_mut) != OSLAYER_OK) {
    return OSLAYER_OPERATION_FAILED;
  } else {
    int result = -1;
    pthread_attr_t attr;
    int policy;
    struct sched_param param;

    osMutexLock(&pThread->access_mut);

    // prepare thread attributes...
    pthread_attr_init(&attr);

    // ...don't inherit scheduler settings as we want to...
    //Android has no this func,just comment,zyc
    //result = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // ...always start new threads with normal priority,
    policy = SCHED_OTHER;
    param.sched_priority = sched_get_priority_min(policy);
    result = pthread_attr_setschedpolicy(&attr, policy);
    result = pthread_attr_setschedparam(&attr, &param);

    // ...affinity set to single CPU (hack to avoid libpthread bug in Ubuntu linux)
    cpu_set_t cpuMask;
    CPU_ZERO(&cpuMask);
    CPU_SET(0, &cpuMask);
    //Android has no this func,just comment,zyc
    //if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuMask) != 0)
    // {
    //     printf("%s: Couldn't set affinity!\n", __FUNCTION__);
    // }
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // create thread
    pThread->pThreadFunc = thread_func;
    pThread->p_arg = arg;
    result = pthread_create(&pThread->handle, &attr, osThreadProc, (void*)pThread);
    if (result) {
      //set pThread->handle  to 0 to make  osThreadClose called when osThreadCreate failed happy.
      pThread->handle = 0;
      res = OSLAYER_OPERATION_FAILED;
    } else {
      res = OSLAYER_OK;
    }
    // cleanup attributes
    pthread_attr_destroy(&attr);
    //set name
//  if ( pThread->name )
//    pthread_setname_np(pThread->handle,pThread->name);
    osMutexUnlock(&pThread->access_mut);
  }

#else
  pThread->p_thread_func = osThreadFunc;
  pThread->p_arg = arg;
  /* CLONE_KERNEL: Don't copy filesystem info, file descriptors and signal handler
  *               from parent process
  */
  pThread->handle = kernel_thread(osThreadProc, (void*)pThread, CLONE_KERNEL);

  if (pThread->handle != NULL)
    res = OSLAYER_OK;
#endif

  return res;
}


/******************************************************************************
 *  osThreadSetPriority()
 ******************************************************************************
 *  @brief  Set thread priority.
 *
 *  @param  pThread                   Reference of the semaphore object
 *
 *  @param  priority                  New thread priority to be set
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Thread priority changed successfully
 *  @retval OSLAYER_ERROR             Invalid thread priority value passed
 *  @retval OSLAYER_OPERATION_FAILED  Thread priority change failed
 *
 ******************************************************************************/
int32_t osThreadSetPriority(osThread* pThread, OSLAYER_THREAD_PRIO priority) {
  OSLAYER_STATUS res = OSLAYER_OK;
#ifndef OSLAYER_KERNEL
  int result;
  int policy, pmin, pmax;
  struct sched_param param;

  /* check error(s) */
  OSLAYER_ASSERT(pThread == NULL);
  OSLAYER_ASSERT(pThread->handle == NULL);

  osMutexLock(&pThread->access_mut);

  switch (priority) {
    case OSLAYER_THREAD_PRIO_HIGHEST:   // e.g. for deferred IRQ handlers
      policy = SCHED_FIFO;            // only root may select a realtime scheduler policy...
      break;

    case OSLAYER_THREAD_PRIO_HIGH:      // e.g. for communication protocol handlers
      policy = SCHED_RR;              // only root may select a realtime scheduler policy...
      // NOTE: the timeslices are quite small with SCHED_RR: 150us only!
      break;

    case OSLAYER_THREAD_PRIO_NORMAL:    // e.g. default interactive threads
      policy = SCHED_OTHER;
      break;
    case OSLAYER_THREAD_PRIO_LOW:       // e.g. house keeping threads
    case OSLAYER_THREAD_PRIO_LOWEST:    // we treat the lower priotity ones all the same
      policy = SCHED_OTHER;           // TODO: use SCHED_BATCH and/or SCHED_IDLE instead
      break;

    default:
      res = OSLAYER_OPERATION_FAILED;
      break;
  }

  if (res == OSLAYER_OK) {
    int plowest, plow, pnormal, phight, phighest;

    pmin = sched_get_priority_min(policy);
    pmax = sched_get_priority_max(policy);

    if (pmax >= (pmin + 4)) {
      phighest = pmin + 4;
      phight   = pmin + 3;
      pnormal  = pmin + 2;
      plowest  = pmin + 1;
      plow     = pmin + 0;
    } else {
      int32_t pmean = (pmin + pmax) / 2;

      phighest = pmax;             // 4/4
      phight   = (pmax + pmean) / 2; // 3/4
      pnormal  = pmean;            // 2/4
      plow     = (pmin + pmean) / 2; // 1/4
      plowest  = pmin;             // 0/4
    }

    switch (priority) {
      case OSLAYER_THREAD_PRIO_HIGHEST:
        param.sched_priority = phighest;
        break;

      case OSLAYER_THREAD_PRIO_HIGH:
        param.sched_priority = phight;
        break;

      case OSLAYER_THREAD_PRIO_NORMAL:
        param.sched_priority = pnormal;
        break;

      case OSLAYER_THREAD_PRIO_LOW:
        param.sched_priority = plow;
        break;

      case OSLAYER_THREAD_PRIO_LOWEST:
        param.sched_priority = plowest;
        break;

      default:
        res = OSLAYER_OPERATION_FAILED;
        break;
    }

    if (res == OSLAYER_OK) {
      result = pthread_setschedparam(pThread->handle, policy, &param); // only root may select a realtime scheduler policy...
      if (!result) {
        res = OSLAYER_OK;
      } else {
        res = OSLAYER_OPERATION_FAILED;
      }
    }
  }

  osMutexUnlock(&pThread->access_mut);
#endif

  return res;
}


/******************************************************************************
 *  osThreadWait()
 ******************************************************************************
 *  @brief  Wait until thread exits.
 *
 *          To avoid memory leak, always call osThreadWait() and wait for the
 *          child thread to terminate in the calling thread. Otherwise you
 *          have to cope with ZOMBIE threads under linux (memory resources
 *          still blocked and not reusable).
 *
 *  @param  pThread       Reference of the semaphore object
 *
 * @return                always OSLAYER_OK
 ******************************************************************************/
int32_t osThreadWait(osThread* pThread) {
  /* check error(s) */
  OSLAYER_ASSERT(pThread == NULL);

#ifndef OSLAYER_KERNEL
  osMutexLock(&pThread->access_mut);
  if (pThread->handle) {
    pThread->wait_count++;
    osMutexUnlock(&pThread->access_mut);

    osEventWait(&pThread->exit_event);

    osMutexLock(&pThread->access_mut);
    pThread->wait_count--;
    if (pThread->wait_count == 0) {
      //needn't do pthread_join if thread is detached.
      //pthread_join(pThread->handle, NULL);
      pThread->handle = 0;
    }
  }
  osMutexUnlock(&pThread->access_mut);
#else
  /* maybe we have to wait here non-interruptible? */
  osEventWait(&pThread->exit_event);
#endif
  return OSLAYER_OK;
}


/******************************************************************************
 *  osThreadClose()
 ******************************************************************************
 *  @brief  Destroy thread object.
 *
 *  @param  pThread       Reference of the semaphore object
 *
 *  @return               always OSLAYER_OK
 ******************************************************************************/
int32_t osThreadClose(osThread* pThread) {
  /* check error(s) */
  OSLAYER_ASSERT(pThread == NULL);

  osThreadWait(pThread);
  osEventDestroy(&pThread->exit_event);
#ifndef OSLAYER_KERNEL
  osMutexDestroy(&pThread->access_mut);
#endif
  return OSLAYER_OK;
}
#endif /* OSLAYER_THREAD */











#ifdef OSLAYER_MISC
/******************************************************************************
 *  osSleep()
 ******************************************************************************
 *  @brief  Yield the execution of current thread for msec miliseconds.
 *
 *  @param  msec       Wait time in millisecobds
 *
 *  @return            always OSLAYER_OK
 ******************************************************************************/
int32_t osSleep(uint32_t msec) {
#ifndef OSLAYER_KERNEL
  if (msec) {
    struct timeval timeout;
    timeout.tv_sec  = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;
    select(0, NULL, NULL, NULL, &timeout); // won't get interrupted by signals if no FD_SETs are given (says libc's manual...)
  } else {
    sched_yield();
  }
#else
  if (msec)
    schedule_timeout(msecs_to_jiffies(msec));
  else
    schedule();
#endif
  return OSLAYER_OK;
}


/******************************************************************************
 *  osGetTick()
 ******************************************************************************
 *  @brief  Obtain the clock tick.
 *
 *  @return            Current clock tick. The resoultion of clock tick can
 *                     be requested with @ref osGetFrequency
 *
 ******************************************************************************/
uint64_t osGetTick(void) {
  struct timeval tv;

#ifndef OSLAYER_KERNEL
  gettimeofday(&tv, NULL);
#else
  do_gettimeofday(&tv);
#endif
  return (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec;
}


/******************************************************************************
 *  osGetFrequency()
 ******************************************************************************
 *  @brief  Obtain the clock resolution.
 *
 *  @return            The resoultion of the clock tick.
 *
 ******************************************************************************/
uint64_t osGetFrequency(void) {
  return (uint64_t)1000000;
}




/******************************************************************************
 *  osMalloc()
 ******************************************************************************
 *  @brief  Allocate a continuous block of memory.
 *
 *  @param  size       Size of memory block to be allocated
 *
 ******************************************************************************/
void* osMalloc(uint32_t size) {
#ifndef OSLAYER_KERNEL
  return malloc(size);
#else
  return kmalloc(size, GFP_USER);
#endif
}


/******************************************************************************
 *  osMallocEx()
 ******************************************************************************
 *  @brief  Allocate a continuous block of memory.
 *
 *  @param  size       Size of memory block to be allocated
 *
 *  @param  type       Type of memory block to be allocated
 *
 ******************************************************************************/
#ifdef OSLAYER_KERNEL
void* osMallocEx(uint32_t size, uint32_t type) {
  if ((uiType != OSLAYER_KERNEL) || (uiType != OSLAYER_USER) || (uiType != OSLAYER_ATOMIC))
    return NULL;

  return kmalloc(size, type);
}
#endif


/******************************************************************************
 *  osFree()
 ******************************************************************************
 *  @brief  Free a continuous block of memory.
 *
 *  @param  p       Pointer to previously allocated memory block
 *
 *  @return         always OSLAYER_OK
 ******************************************************************************/
int32_t osFree(void* p) {
#ifndef OSLAYER_KERNEL
  free(p);
#else
  kfree(p);
#endif
  return OSLAYER_OK;
}
#endif /* OSLAYER_MISC */








#ifdef OSLAYER_MISC
#ifdef OSLAYER_KERNEL
/******************************************************************************
 *  osSpinLockInit()
 ******************************************************************************
 *  @brief  Init a spin lock object.
 *
 *  @param  p_spin_lock      Reference of the spin lock object
 *
 *  @return                  always OSLAYER_OK
 ******************************************************************************/
int32_t osSpinLockInit(osSpinLock* pSpinLock) {
  spin_lock_init(&pSpinLock->lock);
  return OSLAYER_OK;
}


/******************************************************************************
 *  osSpinLockAcquire()
 ******************************************************************************
 *  @brief  Acquire a spin lock object.
 *
 *  @param  p_spin_lock      Reference of the spin lock object
 *
 *  @param  flags            Interrupt context flags which need to be safed
 *                           in a local variable
 *
 *  @return                  always OSLAYER_OK
 ******************************************************************************/
int32_t osSpinLockAcquire(osSpinLock* pSpinLock, uint32_t Flags) {
  spin_lock_irqsave(&pSpinLock->lock, Flags);
  return OSLAYER_OK;
}


/******************************************************************************
 *  osSpinLockRelease()
 ******************************************************************************
 *  @brief  Release a spin lock object.
 *
 +  @param  p_spin_lock      Reference of the spin lock object
 *
 *  @param  flags            Interrupt context flags which need to be safed
 *                           in a local variable
 *
 *  @return                  always OSLAYER_OK
 ******************************************************************************/
int32_t osSpinLockRelease(osSpinLock* pSpinLock, uint32_t Flags) {
  spin_unlock_irqrestore(&pSpinLock->lock, Flags);
  return OSLAYER_OK;
}
#endif /* OSLAYER_KERNEL */
#endif /* OSLAYER_MISC */


#endif /* LINUX */
