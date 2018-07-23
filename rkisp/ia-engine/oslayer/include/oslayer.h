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
 * @file    oslayer.h
 *
 * @brief   Operating System Abstraction Layer
 *
 *          Encapsulates and abstracts services from different operating
 *          system, including user-mode as well as kernel-mode services.
 ******************************************************************************/
#ifndef OSLAYER_H
#define OSLAYER_H

/**
 * @defgroup OS_LAYER_CONFIG OS layer configuration
 * @{
 * @brief   Enables/disables the different services provided by the OS layer.
 *****************************************************************************/

/******************************************************************************/
/** @brief  Include time-stamp handling in OS Abstraction Layer library */
#define OSLAYER_TIMESTAMP

/******************************************************************************/
/** @brief  Include event handling in OS Abstraction Layer library */
#define OSLAYER_EVENT

/******************************************************************************/
/** @brief  Include mutex handling in OS Abstraction Layer library */
#define OSLAYER_MUTEX

/******************************************************************************/
/** @brief  Include semaphore handling in OS Abstraction Layer library */
#define OSLAYER_SEMAPHORE

/******************************************************************************/
/** @brief  Include queue handling in OS Abstraction Layer library */
#if defined OSLAYER_SEMAPHORE && defined OSLAYER_MUTEX
#define OSLAYER_QUEUE
#endif

/******************************************************************************/
/** @brief  Include atomic operations in OS Abstraction Layer library */
#define OSLAYER_ATOMIC

/******************************************************************************/
/** @brief  Include thread handling in OS Abstraction Layer library */
#if defined OSLAYER_EVENT && defined OSLAYER_MUTEX
#define OSLAYER_THREAD
#endif

/******************************************************************************/
/** @brief  Include IRQ service functionality in OS Abstraction Layer library */
#if defined OSLAYER_EVENT && defined OSLAYER_MUTEX && defined OSLAYER_THREAD
#define OSLAYER_IRQ
#endif

/******************************************************************************/
/** @brief  Include misc functionality in OS Abstraction Layer library */
#define OSLAYER_MISC

//!@} defgroup OS_LAYER_CONFIG

/**
 * @defgroup OS_LAYER OS layer interface
 * @{
 * @brief   Encapsulates and abstracts services from different operating
 *          system, including user-mode as well as kernel-mode services,
 *          in a operating system independent way.
 *****************************************************************************/

#ifdef WIN32
#include "oslayer_win32.h"
#endif
#ifdef LINUX
#ifdef MSVD_COSIM
#include "oslayer_systemc.h"
#else
#include "oslayer_linux.h"
#endif
#endif
#ifdef UCOSII
#include "oslayer_ucosii.h"
#endif
#ifdef MFS
#include "oslayer_mfs.h"
#endif


#ifdef __cplusplus
extern "C"
{
#endif



/* in debug case, each OS layer implementation
   shall define the following macros as needed */
#ifndef OSLAYER_ASSERT
#define OSLAYER_ASSERT(x)
#endif /* OSLAYER_DEBUG */


/******************************************************************************/
/** @brief  Status codes of OS Abstraction Layer operation */
typedef enum _OSLAYER_STATUS {
  OSLAYER_OK                    = 0,   /*< success */
  OSLAYER_ERROR                 = -1,  /*< general error */
  OSLAYER_INVALID_PARAM         = -2,  /*< invalid parameter supplied */
  OSLAYER_OPERATION_FAILED      = -3,  /*< operation failed (i.e. current operation is interrupted) */
  OSLAYER_NOT_INITIALIZED       = -4,  /*< resource object is not initialized */
  OSLAYER_TIMEOUT               = -5,  /*< operation failed due to elapsed timeout */
  OSLAYER_SIGNAL_PENDING        = -6   /*< operation interrupted due to pending signal for waiting thread/process */
} OSLAYER_STATUS;



/******************************************************************************/
/** @brief  Priority of thread created by OS Abstraction Layer */
typedef enum _OSLAYER_THREAD_PRIO {
  OSLAYER_THREAD_PRIO_HIGHEST,
  OSLAYER_THREAD_PRIO_HIGH,
  OSLAYER_THREAD_PRIO_NORMAL,
  OSLAYER_THREAD_PRIO_LOW,
  OSLAYER_THREAD_PRIO_LOWEST
} OSLAYER_THREAD_PRIO;


/*****************************************************************************/
/** @brief  Interrupt object of OS Abstraction Layer */
typedef struct _osInterrupt {
#ifdef OSLAYER_KERNEL
#else
  osThread    isr_thread;
  osEvent     isr_event;
  osEvent     isr_exit_event;
  osMutex     isr_access_lock;
#endif
  uint32_t    irq_num;
  osIsrFunc   IsrFunc;
  osDpcFunc   DpcFunc;
  void*       p_context;
} osInterrupt;



#ifdef OSLAYER_TIMESTAMP
/******************************************************************************
 *  osTimeStampUs()
 *****************************************************************************/
/**
 *  @brief  Returns a 64-bit timestamp [us]
 *
 *  @param  pEvent         Reference of the timestamp object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Event successfully created
 *
 ******************************************************************************/
extern int32_t osTimeStampUs(int64_t* pTimeStamp);

/******************************************************************************
 *  osTimeStampNs()
 *****************************************************************************/
/**
 *  @brief  Returns a 64-bit timestamp [ns]
 *
 *  @param  pEvent         Reference of the timestamp object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Event successfully created
 *
 ******************************************************************************/
extern int32_t osTimeStampNs(int64_t* pTimeStamp);
#endif /* OSLAYER_TIMESTAMP */



#ifdef OSLAYER_EVENT
/******************************************************************************
 *  osEventInit()
 *****************************************************************************/
/**
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
extern int32_t osEventInit(osEvent* pEvent, int32_t Automatic, int32_t InitState);

/******************************************************************************
 *  osEventSignal()
 *****************************************************************************/
/**
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
extern int32_t osEventSignal(osEvent* pEvent);

/******************************************************************************
 *  osEventReset()
 *****************************************************************************/
/**
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
extern int32_t osEventReset(osEvent* pEvent);

/******************************************************************************
 *  osEventPulse()
 *****************************************************************************/
/**
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
extern int32_t osEventPulse(osEvent* pEvent);

/******************************************************************************
 *  osEventWait()
 *****************************************************************************/
/**
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
 +  @retval OSLAYER_ERROR             Tried to wait for a not initialized event
 *  @retval OSLAYER_OPERATION_FAILED  Wait for event failed
 *
 ******************************************************************************/
extern int32_t osEventWait(osEvent* pEvent);

/******************************************************************************
 *  osEventTimedWait()
 *****************************************************************************/
/**
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
extern int32_t osEventTimedWait(osEvent* pEvent, uint32_t msec);

/******************************************************************************
 *  osEventDestroy()
 *****************************************************************************/
/**
 *  @brief  Destroy the event.
 *
 *  Destroy the event and free resources associated with event object.
 *
 *  @param  pEvent      Reference of the event object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
extern int32_t osEventDestroy(osEvent* pEvent);
#endif /* OSLAYER_EVENT */



#ifdef OSLAYER_MUTEX
/******************************************************************************
 *  osMutextInit()
 *****************************************************************************/
/**
 *  @brief  Initialize a mutex object.
 *
 *  @param  pMutex         Reference of the mutex object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Mutex successfully created
 *  @retval OSLAYER_ERROR  Mutex is not created
 *
 ******************************************************************************/
extern int32_t osMutexInit(osMutex* pMutex);

/******************************************************************************
 *  osMutexLock()
 *****************************************************************************/
/**
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
extern int32_t osMutexLock(osMutex* pMutex);

/******************************************************************************
 *   osMutexUnlock()
 *****************************************************************************/
/**
 *  @brief  Unlock a mutex object.
 *
 *  @param  pMutex         Reference of the mutex object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Mutex successfully unlocked
 *
 ******************************************************************************/
extern int32_t osMutexUnlock(osMutex* pMutex);

/******************************************************************************
 *  osMutexTryLock()
 *****************************************************************************/
/**
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
extern int32_t osMutexTryLock(osMutex* pMutex);

/******************************************************************************
 *  osMutexDestroy()
 *****************************************************************************/
/**
 *  @brief  Destroy a mutex object.
 *
 *  @param  pMutex      Reference of the mutex object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
extern int32_t osMutexDestroy(osMutex* pMutex);
#endif /* OSLAYER_MUTEX */



#ifdef OSLAYER_SEMAPHORE
/******************************************************************************
 *  osSemaphoreInit()
 *****************************************************************************/
/**
 *  @brief  Init a semaphore with init count.
 *
 *  @param  pSem           Reference of the semaphore object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Semaphore successfully created
 *  @retval OSLAYER_ERROR  Semaphore is not created
 *
 ******************************************************************************/
extern int32_t osSemaphoreInit(osSemaphore* pSem, int32_t init_count);

/******************************************************************************
 *  osSemaphoreTimedWait()
 *****************************************************************************/
/**
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
extern int32_t osSemaphoreTimedWait(osSemaphore* pSem, uint32_t msec);

/******************************************************************************
 *  osSemaphoreWait()
 *****************************************************************************/
/**
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
extern int32_t osSemaphoreWait(osSemaphore* pSem);

/******************************************************************************
 *  osSemaphoreTryWait()
 *****************************************************************************/
/**
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
extern int32_t osSemaphoreTryWait(osSemaphore* pSem);

/******************************************************************************
 *  osSemaphorePost()
 *****************************************************************************/
/**
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
extern int32_t osSemaphorePost(osSemaphore* pSem);

/******************************************************************************
 *   osSemaphoreDestroy()
 *****************************************************************************/
/**
 *  @brief  Destroy the semaphore object.
 *
 *  @param  pSem        Reference of the semaphore object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
extern int32_t osSemaphoreDestroy(osSemaphore* pSem);
#endif /* OSLAYER_SEMAPHORE */



#ifdef OSLAYER_QUEUE
/*****************************************************************************/
/** @brief  Queue object (generic Version) of OS Abstraction Layer */
typedef struct _osQueue {
  void*    p_next;            //!< for storing into a list (multiplexer)

  int32_t ItemSize;           //!< Size of queue item.
  int32_t ItemNum;            //!< Max number of queue items (just for debugging).
  int32_t ItemCount;          //!< Current number of queue items (just for debugging).

  char*    pItemBuffer;       //!< Pointer to buffer holding queued items.
  char*    pItemBufferEnd;    //!< Pointer to end of that buffer, used for wrapping.
  char*    pItemBufferRead;   //!< Pointer to next item to read.
  char*    pItemBufferWrite;  //!< Pointer to next item to write.

  osSemaphore ItemsFreeSema;  //!< Counting semaphore increased/decreased for every free/used item in the queue. Used to synchronize concurrent write accesses.
  osSemaphore ItemsUsedSema;  //!< Counting semaphore increased/decreased for every used/free item in the queue. Used to synchronize concurrent read accesses.

  osMutex AccessMutex;        //!< Must be held to access/modify any of the fields in this struct, but not to get/put the semaphores.
} osQueue;

/******************************************************************************
 *  osQueueInit()
 *****************************************************************************/
/**
 *  @brief  Initialize queue object.
 *
 *  Init a queue. Item size and item number can be set.
 *
 *  @param  pQueue         Reference of the queue object.
 *
 *  @param  ItemNum        Number of items this queue can hold.
 *
 *  @param  ItemSize       Size of a single queue item.
 *
 *  @return                Status of operation.
 *  @retval OSLAYER_OK     Queue successfully created.
 *  @retval OSLAYER_ERROR  Queue is not created.
 *
 ******************************************************************************/
extern int32_t osQueueInit(osQueue* pQueue, int32_t ItemNum, int32_t ItemSize);

/******************************************************************************
 *  osQueueRead()
 *****************************************************************************/
/**
 *  @brief  Blocking read from the queue.
 *
 *  Wait for item being available in the queue and block calling thread, then copy item
 *  from queue. The function call returns immediatly, if an item is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to read from a not initialized queue.
 *  @retval OSLAYER_OPERATION_FAILED  Reading from queue failed.
 *
 ******************************************************************************/
extern int32_t osQueueRead(osQueue* pQueue, void* pvItem);

/******************************************************************************
 *  osQueueTimedRead()
 *****************************************************************************/
/**
 *  @brief  Blocking read from the queue with timeout.
 *
 *  Wait for item being available in the queue and block calling thread, then copy item
 *  from queue. The function call returns immediatly, if an item is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to read from a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           Timeout occurred while waiting for item being
 *                                    available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Reading from queue failed.
 *
 ******************************************************************************/
extern int32_t osQueueTimedRead(osQueue* pQueue, void* pvItem, uint32_t msec);

/******************************************************************************
 *  osQueueTryRead()
 *****************************************************************************/
/**
 *  @brief  Non-blocking read from the queue.
 *
 *  If an item is available in the queue, then copy item from queue. If no
 *  item is available, then return with error.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to read from a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           No item was available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Reading from queue failed.
 *
 ******************************************************************************/
extern int32_t osQueueTryRead(osQueue* pQueue, void* pvItem);

/******************************************************************************
 *  osQueueWrite()
 *****************************************************************************/
/**
 *  @brief  Blocking write into the queue.
 *
 *  Wait for space in queue being available and block calling thread, then copy item
 *  into queue. The function call returns immediatly, if space is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to write into queue.
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Writing into queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to write into a not initialized queue.
 *  @retval OSLAYER_OPERATION_FAILED  Writing into queue failed.
 *
 ******************************************************************************/
extern int32_t osQueueWrite(osQueue* pQueue, void* pvItem);

/******************************************************************************
 *  osQueueTimedWrite()
 *****************************************************************************/
/**
 *  @brief  Blocking write into the queue with timeout.
 *
 *  Wait for space in queue being available and block calling thread, then copy item
 *  into queue. The function call returns immediatly, if space is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to write into queue.
 *
 *  @param  msec                      Timeout value in milliseconds.
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Writing into queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to write into a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           Timeout occurred while waiting for space being
 *                                    available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Writing into queue failed.
 *
 ******************************************************************************/
extern int32_t osQueueTimedWrite(osQueue* pQueue, void* pvItem, uint32_t msec);

/******************************************************************************
 *  osQueueTryWrite()
 *****************************************************************************/
/**
 *  @brief  Non-blocking write into the queue.
 *
 *  If a free item is available in the queue, then copy item into queue. If no
 *  free item is available, then return with error.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Writing into queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to write into a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           Writing into queue failed, as no free item
 *                                    was available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Writing into queue failed.
 *
 ******************************************************************************/
extern int32_t osQueueTryWrite(osQueue* pQueue, void* pvItem);

/******************************************************************************
 *  osQueueDestroy()
 *****************************************************************************/
/**
 *  @brief  Destroy the queue.
 *
 *  Destroy the queue and free resources associated with queue object.
 *
 *  @param  pQueue      Reference of the queue object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
extern int32_t osQueueDestroy(osQueue* pQueue);
#endif /* OSLAYER_QUEUE */



#ifdef OSLAYER_ATOMIC
/******************************************************************************
 *  osAtomicInit()
 *****************************************************************************/
/**
 *  @brief  Initialize atomic operation functionality.
 *
 *  This function must be called before any other osAtomicXXX call.
 *
 ******************************************************************************/
extern int32_t osAtomicInit();

/******************************************************************************
 *  osAtomicShutdown()
 *****************************************************************************/
/**
 *  @brief  Shutdown atomic operation functionality.
 *
 *  This function must be called before process is terminated
 *  when osAtomicInit has been called before.
 *
 ******************************************************************************/
extern int32_t osAtomicShutdown();

/******************************************************************************
 *  osAtomicTestAndClearBit()
 *****************************************************************************/
/**
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
extern uint32_t osAtomicTestAndClearBit(uint32_t* pVar, uint32_t bitpos);

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
extern uint32_t osAtomicIncrement(uint32_t* pVar);

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
extern uint32_t osAtomicDecrement(uint32_t* pVar);

/******************************************************************************
*   osAtomicSetBit()
 *****************************************************************************/
/**
 *  @brief  Set a bit position atomically.
 *
 *  Set a bit position inside a variable.
 *  The operation is atomic (includes IRQ safety).
 *
 *  @param  pVar           32-bit unsigned variable to be modified
 *  @param  bitpos         Bit to be set
 *
 *  @return                always OSLAYER_OK
 ******************************************************************************/
extern int32_t osAtomicSetBit(uint32_t* pVar, uint32_t bitpos);

/******************************************************************************
 * osAtomicSet()
 *****************************************************************************/
/**
 *  @brief  Set value atomically.
 *
 *  Set variable to value. The operation is atomic (includes IRQ safety).
 *
 *  @param  pVar           32-bit unsigned variable to be modified
 *  @param  value          Value to be set
 *
 *  @return                always OSLAYER_OK
 ******************************************************************************/
extern int32_t osAtomicSet(uint32_t* pVar, uint32_t value);

//add by zyc
extern uint32_t osAtomicCompareAndSwap(uint32_t* pVar, uint32_t oldVal, uint32_t newVal);

#endif /* OSLAYER_ATOMIC */



#ifdef OSLAYER_THREAD
/******************************************************************************
 *  osThreadCreate()
 *****************************************************************************/
/**
 *  @brief  Create a thread.
 *
 *  @param  pThread                   Reference of the semaphore object
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Thread object created successfully
 *  @retval OSLAYER_OPERATION_FAILED  Creation of thread object failed
 +
 ******************************************************************************/
extern int32_t osThreadCreate(osThread* pThread, osThreadFunc thread_func, void* arg);

/******************************************************************************
 *  osThreadSetPriority()
 *****************************************************************************/
/**
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
extern int32_t osThreadSetPriority(osThread* pThread, OSLAYER_THREAD_PRIO priority);

/******************************************************************************
 *   osThreadWait()
 *****************************************************************************/
/**
 *  @brief  Wait until thread exits.
 *
 *          To avoid memory leak, always call @ref osThreadWait and wait
 *          for the child thread to terminate in the calling thread.
 *
 *  @param  pThread        Reference of the semaphore object
 *
 *  @return                always OSLAYER_OK
 ******************************************************************************/
extern int32_t osThreadWait(osThread* pThread);

/******************************************************************************
 *  osThreadClose()
 *****************************************************************************/
/**
 *  @brief  Destroy thread object.
 *
 *  @param  pThread       Reference of the semaphore object
 *
 *  @return               always OSLAYER_OK
 ******************************************************************************/
extern int32_t osThreadClose(osThread* pThread);
#endif /* OSLAYER_THREAD */



#ifdef OSLAYER_MISC
/******************************************************************************
 *  osSleep()
 *****************************************************************************/
/**
 *  @brief  Yield the execution of current thread for msec miliseconds.
 *
 *  @param  msec       Wait time in millisecobds
 *
 *  @return            always OSLAYER_OK
 ******************************************************************************/
extern int32_t osSleep(uint32_t msec);

/******************************************************************************
 *   osGetTick()
 *****************************************************************************/
/**
 *  @brief  Obtain the clock tick.
 *
 *  @return            Current clock tick. The resoultion of clock tick can
 *                     be requested with @ref osGetFrequency
 *
 ******************************************************************************/
extern uint64_t osGetTick(void);

/******************************************************************************
 *  osGetFrequency()
 *****************************************************************************/
/**
 *  @brief  Obtain the clock resolution.
 *
 *  @return            The resoultion of the clock tick.
 *
 ******************************************************************************/
extern uint64_t osGetFrequency(void);

#if defined LINUX && OSLAYER_KERNEL
/******************************************************************************
 *  osMallocEx()
 *****************************************************************************/
/**
 *  @brief  Allocate a continuous block of memory.
 *
 *  @param  size       Size of memory block to be allocated
 *
 *  @param  type       Type of memory block to be allocated
 *
 ******************************************************************************/
void* osMallocEx(uint32_t size, uint32_t type);
#endif

/******************************************************************************
 *  osMalloc()
 *****************************************************************************/
/**
 *  @brief  Allocate a continuous block of memory.
 *
 *  @param  size       Size of memory block to be allocated
 *
 ******************************************************************************/
extern void* osMalloc(uint32_t size);

/******************************************************************************
 *  osFree()
 *****************************************************************************/
/**
 *  @brief  Free a continuous block of memory.
 *
 *  @param  p       Pointer to previously allocated memory block
 *
 *  @return         always OSLAYER_OK
 ******************************************************************************/
extern int32_t osFree(void* p);


#ifdef OSLAYER_KERNEL
/******************************************************************************
 *  osSpinLockInit()
 *****************************************************************************/
/**
 *  @brief  Init a spin lock object.
 *
 *  @param  p_spin_lock      Reference of the spin lock object
 *
 *  @return                  always OSLAYER_OK
 ******************************************************************************/
extern int32_t osSpinLockInit(osSpinLock* p_spin_lock);

/******************************************************************************
 *  osSpinLockAcquire()
 *****************************************************************************/
/**
 *  @brief  Acquire a spin lock object.
 *
 *  @param  p_spin_lock      Reference of the spin lock object
 *
 *  @param  flags            Interrupt context flags which need to be safed
 *                           in a local variable
 *
 *  @return                  always OSLAYER_OK
 ******************************************************************************/
extern int32_t osSpinLockAcquire(osSpinLock* p_spin_lock, uint32_t flags);

/******************************************************************************
 *   osSpinLockRelease()
 *****************************************************************************/
/**
 *  @brief  Release a spin lock object.
 *
 *  @param  p_spin_lock      Reference of the spin lock object
 *
 *  @param  flags            Interrupt context flags which need to be safed
 *                           in a local variable
 *
 *  @return                  always OSLAYER_OK
 ******************************************************************************/
extern int32_t osSpinLockRelease(osSpinLock* p_spin_lock, uint32_t flags);
#endif /* OSLAYER_KERNEL */
#endif /* OSLAYER_MISC */


#ifdef __cplusplus
}
#endif


//!@} defgroup OS_LAYER

#endif /* MSVD_KAL_H */
