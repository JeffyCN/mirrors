#ifndef __WL_DEBUG_H__
#define __WL_DEBUG_H__
#include <pthread.h>

extern pthread_mutex_t wl_dbg_mutex;

#define	WL_LOG_NONE 	0
#define	WL_LOG_ERROR 	1
#define	Wl_LOG_INFO 	2
#define	WL_LOG_DEBUG 	3
#define	WL_LOG_DUMP 	4

#ifndef WL_LOG_LEVEL
#define WL_LOG_LEVEL	WL_LOG_ERROR	
#endif

#if WL_LOG_LEVEL >= WL_LOG_ERROR
#define WL_ERR(fmt, ...) \
			do { \
				pthread_mutex_lock(&wl_dbg_mutex); \
				printf("[WL][ERROR] %s, " fmt, __FUNCTION__, ##__VA_ARGS__); \
				pthread_mutex_unlock(&wl_dbg_mutex); \
			} while(0)
#else
#define WL_ERR(fmt, ...)
#endif

#if WL_LOG_LEVEL >= Wl_LOG_INFO
#define WL_INFO(fmt, ...) \
			do { \
				pthread_mutex_lock(&wl_dbg_mutex); \
				printf("[WL][INFO] %s, " fmt, __FUNCTION__, ##__VA_ARGS__); \
				pthread_mutex_unlock(&wl_dbg_mutex); \
			} while(0)
#else
#define WL_INFO(fmt, ...)
#endif

#if WL_LOG_LEVEL >= WL_LOG_DEBUG
#define WL_DBG(fmt, ...) \
			do { \
				pthread_mutex_lock(&wl_dbg_mutex); \
				printf("[WL][DEBUG] %s, " fmt, __FUNCTION__, ##__VA_ARGS__); \
				pthread_mutex_unlock(&wl_dbg_mutex); \
			} while(0)
#else
#define WL_DBG(fmt, ...)
#endif

#endif /* __WL_DEBUG_H__ */
