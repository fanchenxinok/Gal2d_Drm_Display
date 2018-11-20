/************************************************
*   Author:  Fanchenxin
*   Date:  2018/05/17
*************************************************/
#ifndef __FB_QUEUE_MNG_H__
#define __FB_QUEUE_MNG_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdarg.h>
#include <errno.h>
#include "common.h"
#include "log.h"
#if DRM_CREATE_MUTI_FB
#include "drm_wrapper.h"
#endif

/*************************************************/
#define FBMNG_LOG_ERR(msg, ...)     LOG_ERR(LOG_COL_RED_YLW "[FBMNG_ERR]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define FBMNG_LOG_WARNING(msg, ...) LOG_WARNING(LOG_COL_RED_WHI "[FBMNG_WARNING]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define FBMNG_LOG_NOTIFY(msg, ...)   LOG_NOTIFY(LOG_COL_NONE "[FBMNG_NOTIFY]"msg LOG_COL_NONE"\n", ##__VA_ARGS__)
#define FBMNG_LOG_NONE

#define CHECK_POINTER_NULL(pointer, ret) \
    do{\
        if(pointer == NULL){ \
            FBMNG_LOG_ERR("Check '%s' is NULL at:%u\n", #pointer, __LINE__);\
            return ret; \
        }\
    }while(0)

		
#define DU_MNG_NUM   		 (4)
#define FB_BUFFER_MAX_SIZE   (1920 * 1080 * 4)

typedef struct
{
	uInt8 imageData[FB_BUFFER_MAX_SIZE];  //放在第一位置是为了GC320使用POOL_USER模式时，地址对齐。
	stRect dispArea;
	uInt32 width;
	uInt32 height;
	uInt8 bytesPix;  /* bytes per pix */
}stFbMem;

/* if create muti frame buffer on drm size */
#if DRM_CREATE_MUTI_FB
#define FB_MEM_SIZE     (sizeof(stFrameBufferInfo))
#define FB_BUFFER_NUM   (DRM_FRAME_BUFFER_NUM) /* in this case, the drm buffer can used by user. */

#else
#define FB_MEM_SIZE     (sizeof(stFbMem))
#define FB_BUFFER_NUM   (8)    /* in this case, i create 8 frame buffer for user use */
#endif

typedef struct
{
	void *pAddr;
	struct stFbNode *pNext;
}stFbNode;

typedef struct
{
	stFbNode *pFreeList;
	stFbNode *pUsedList;
	stFbNode *pFreeListTail; // the tail pointer of free list
	volatile Int32 freeCnt;
	pthread_mutex_t mutex;
}stFbListMng;

typedef struct
{
	volatile Int32 readIdx;
	volatile Int32 writeIdx;
	void** ppQueue;
	Int32 maxCnt;
}stLoopQueue;

typedef enum
{
	AVAIL_STATUS_FREE,
	AVAIL_STATUS_USED
}enAvailableStatus;

typedef struct{
	enAvailableStatus state;
	stLoopQueue *pLoopQueue;
	pthread_mutex_t mutex;
	pthread_cond_t  pushCond;
	pthread_cond_t  popCond;
}stDuFbQueueMng;

enBOOL CreateFbBufferList(void *pDrmFbList);
void DestoryFbBufferList();
enBOOL GetFbBuffer(void **ppFbBuff);
enBOOL FreeFbBuffer(void *pFbBuff);

void InitDuFbQueues();
enBOOL CreateDuFbQueue(Int32 duId, Int32 queueMaxCnt);
enBOOL DestoryDuFbQueue(Int32 duId);
uInt32 GetDuFbQueueNum(Int32 duId);
enBOOL SendToDuFbQueue(Int32 duId, void *pFbMem, Int32 timeOutMs);
enBOOL ReceiveFromDuFbQueue(Int32 duId, void **ppFbMem, Int32 timeOutMs);


#endif
