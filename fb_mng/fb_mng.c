#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdarg.h>
#include <errno.h>
#include "fb_mng.h"
#include "gc320_wrapper.h"

#if GC320_POOL_USER & !DRM_CREATE_MUTI_FB
#define ADDRESS_ALIGN  (0x100)
#endif

/*************************************************/
static void *pFbBuffers = NULL;  // define for free memery
static stFbNode *pFbList = NULL; // define for free memery
static stFbListMng s_fb_list_mng;

static Int32 CreateTmpFile(char* fileName, uInt32 size)
{
	Int32 fd = mkstemp(fileName); // create temp file, fileName must be: .....XXXXXX
	if(fd >= 0){
		unlink(fileName);
		if(ftruncate(fd, size) < 0){ // set file size
			close(fd);
			FBMNG_LOG_ERR("ftruncate() Fail!");
			return -1;
		}
	}
	else{
		FBMNG_LOG_ERR("mkstemp()  Fail!");
		return -1;
	}
	return fd;
}

enBOOL CreateFbBufferList(void *pDrmFbList)
{
	if(pFbBuffers) return TRUE;
	#if DRM_CREATE_MUTI_FB
	pFbBuffers = (void*)pDrmFbList;
	#else
		#if GC320_POOL_USER
		uInt32 addrAlignSize = Gc320GetAddrAlignment(gcvSURF_A8R8G8B8);
		/* 需要将地址按addrAlignSize对齐 */
		pFbBuffers = (void*)malloc(COMM_ALIGN(FB_MEM_SIZE, addrAlignSize) * FB_BUFFER_NUM + (addrAlignSize - 1));
		#else
		pFbBuffers = (void*)malloc(FB_MEM_SIZE * FB_BUFFER_NUM);
		#endif
		
		if(!pFbBuffers) return FALSE;
	#endif
	
	pFbList = (stFbNode*)malloc(sizeof(stFbNode) * FB_BUFFER_NUM);
	if(!pFbList){
		#if DRM_CREATE_MUTI_FB
		pFbBuffers = NULL;
		#else
		free(pFbBuffers);
		pFbBuffers = NULL;
		#endif
		return FALSE;
	}

	uInt32 offset = 0;
	#if GC320_POOL_USER && !DRM_CREATE_MUTI_FB
	uInt8 *pAddr = (uInt8*)pFbBuffers;
	pAddr = (uInt8*)COMM_ALIGN((Int32)pAddr, addrAlignSize);
	offset = COMM_ALIGN(FB_MEM_SIZE, addrAlignSize);
	FBMNG_LOG_WARNING("Before align: %p, after align %p, alignsize = 0x%x, memsize = 0x%x, offset = 0x%x",
		pFbBuffers, pAddr, addrAlignSize, FB_MEM_SIZE, offset);
	#else
	uInt8 *pAddr = (uInt8*)pFbBuffers;
	offset = FB_MEM_SIZE;
	#endif
	stFbNode *pNode = pFbList;
	
	s_fb_list_mng.freeCnt = FB_BUFFER_NUM;
	s_fb_list_mng.pFreeList = pNode;
	s_fb_list_mng.pFreeList->pAddr = pAddr;
	s_fb_list_mng.pFreeList->pNext = NULL;
	FBMNG_LOG_WARNING("FB mem address[0] = %p", pAddr);
	pAddr += offset;
	pNode++;
	stFbNode* pCur = s_fb_list_mng.pFreeList;
	for(int i = 0; i < FB_BUFFER_NUM - 1; i++){
		stFbNode* pNew = pNode;
		pNew->pAddr = pAddr;
		pNew->pNext = NULL;
		FBMNG_LOG_WARNING("FB mem address[%d] = %p", i+1, pAddr);
		pAddr += offset;
		pNode++;
		pCur->pNext = pNew;
		pCur = pNew;
	}
	s_fb_list_mng.pUsedList = NULL;
	s_fb_list_mng.pFreeListTail = pCur;
	
	pthread_mutex_init(&s_fb_list_mng.mutex, NULL);
	return TRUE;
}


void DestoryFbBufferList()
{
	#if DRM_CREATE_MUTI_FB
	pFbBuffers = NULL;
	#else
	if(pFbBuffers){
		free(pFbBuffers);
		pFbBuffers = NULL;
	}
	#endif

	if(pFbList){
		free(pFbList);
		pFbList = NULL;
	}
	return;
}

enBOOL GetFbBuffer(void **ppFbBuff)
{
	enBOOL ret = TRUE;
	SAFETY_MUTEX_LOCK(s_fb_list_mng.mutex);
	CHECK_POINTER_NULL(ppFbBuff, FALSE);
	FBMNG_LOG_NOTIFY("s_fb_list_mng.freeCnt = %d", s_fb_list_mng.freeCnt);
	if(s_fb_list_mng.freeCnt <= 0){
		FBMNG_LOG_NONE("All the fb buffer has been used!!!");
		ret = FALSE;
	}
	else{
		*ppFbBuff = s_fb_list_mng.pFreeList->pAddr;
		stFbNode *pUsed = s_fb_list_mng.pFreeList;
		s_fb_list_mng.pFreeList = s_fb_list_mng.pFreeList->pNext;
		s_fb_list_mng.freeCnt--;
		if(!s_fb_list_mng.pUsedList){
			pUsed->pNext = NULL;
			pUsed->pAddr = NULL;
			s_fb_list_mng.pUsedList = pUsed;
		}
		else{
			pUsed->pAddr = NULL;
			pUsed->pNext = s_fb_list_mng.pUsedList->pNext;
			s_fb_list_mng.pUsedList->pNext = pUsed;
		}
	}
	SAFETY_MUTEX_UNLOCK(s_fb_list_mng.mutex);
	FBMNG_LOG_NOTIFY("Get Buffer : 0x%x", *ppFbBuff);
	return ret;
}

enBOOL FreeFbBuffer(void *pFbBuff)
{
	CHECK_POINTER_NULL(pFbBuff, FALSE);
	SAFETY_MUTEX_LOCK(s_fb_list_mng.mutex);
	stFbNode *pFree = s_fb_list_mng.pUsedList;
	s_fb_list_mng.pUsedList = s_fb_list_mng.pUsedList->pNext;
	pFree->pAddr = pFbBuff;
	pFree->pNext = NULL;
	if(!s_fb_list_mng.pFreeList){
		s_fb_list_mng.pFreeList = pFree;
		s_fb_list_mng.pFreeListTail = s_fb_list_mng.pFreeList;
		s_fb_list_mng.pFreeListTail->pNext = NULL;
	}
	else{
		#if 1
		/* add to the free list tail */
		s_fb_list_mng.pFreeListTail->pNext = pFree;
		s_fb_list_mng.pFreeListTail = pFree;
		#else
		/* add to the free list head */
		pFree->pNext = s_fb_list_mng.pFreeList->pNext;
		s_fb_list_mng.pFreeList->pNext = pFree;
		#endif
	}
	s_fb_list_mng.freeCnt++;
	SAFETY_MUTEX_UNLOCK(s_fb_list_mng.mutex);
	FBMNG_LOG_NOTIFY("Free Buffer : 0x%x", pFbBuff);
	return TRUE;
}

static stLoopQueue* CreateLoopQueue(Int32 queueMaxCnt)
{
	if(queueMaxCnt <= 0) return NULL;
	stLoopQueue *pLoopQueue = (stLoopQueue*)malloc(sizeof(stLoopQueue));
	if(pLoopQueue){
		pLoopQueue->readIdx = 0;
		pLoopQueue->writeIdx = 0;
		pLoopQueue->ppQueue = (void**)malloc(sizeof(void*) * (queueMaxCnt + 1));
		pLoopQueue->maxCnt = queueMaxCnt + 1;
	}
	return pLoopQueue;
}

static void DeleteLoopQueue(stLoopQueue *pLoopQueue)
{
	if(pLoopQueue){
		free(pLoopQueue->ppQueue);
		free(pLoopQueue);
		pLoopQueue = NULL;
	}
	return;
}

static void calcLoopQueueNextIdx(volatile int *pIndex, int maxCnt)
{
	CHECK_POINTER_VALID(pIndex);
	int newIndex = __sync_add_and_fetch(pIndex, 1);
    if (newIndex >= maxCnt){
		/*bool __sync_bool_compare_and_swap (type *ptr, type oldval type newval, ...)
		   if *ptr == oldval, use newval to update *ptr value */
        __sync_bool_compare_and_swap(pIndex, newIndex, newIndex % maxCnt);
    }
	return;
}

static enBOOL checkLoopQueueEmpty(stLoopQueue *pLoopQueue)
{
	CHECK_POINTER_NULL(pLoopQueue, FALSE);
	return (pLoopQueue->readIdx == pLoopQueue->writeIdx) ? TRUE : FALSE;
}

static enBOOL checkLoopQueueFull(stLoopQueue *pLoopQueue)
{
	CHECK_POINTER_NULL(pLoopQueue, FALSE);
	return ((pLoopQueue->writeIdx+1)%pLoopQueue->maxCnt== pLoopQueue->readIdx) ? TRUE : FALSE;
}

static uInt32 getLoopQueueNums(stLoopQueue *pLoopQueue)
{
	CHECK_POINTER_NULL(pLoopQueue, 0);
	return (pLoopQueue->writeIdx - pLoopQueue->readIdx + pLoopQueue->maxCnt) % pLoopQueue->maxCnt;
}

static enBOOL WriteLoopQueue(stLoopQueue *pLoopQueue, void **ppIn)
{
	CHECK_POINTER_NULL(pLoopQueue, FALSE);
	CHECK_POINTER_NULL(ppIn, FALSE);
	if(checkLoopQueueFull(pLoopQueue)){
		FBMNG_LOG_NONE("loop queue have not enough space to write, readIdx=%d,writeIdx=%d",
			pLoopQueue->readIdx,pLoopQueue->writeIdx);
		return FALSE;
	}

	pLoopQueue->ppQueue[pLoopQueue->writeIdx] = *ppIn;
	FBMNG_LOG_NOTIFY("write[%d]: 0x%x", pLoopQueue->writeIdx, *ppIn);
	calcLoopQueueNextIdx(&pLoopQueue->writeIdx, pLoopQueue->maxCnt);
	return TRUE;
}

static enBOOL ReadLoopQueue(stLoopQueue *pLoopQueue, void **ppOut)
{
	CHECK_POINTER_NULL(pLoopQueue, FALSE);
	CHECK_POINTER_NULL(ppOut, FALSE);
	if(checkLoopQueueEmpty(pLoopQueue)){
		FBMNG_LOG_WARNING("Loop queue is empty!!!");
		return FALSE;
	}
	*ppOut = pLoopQueue->ppQueue[pLoopQueue->readIdx];
	FBMNG_LOG_NOTIFY("read[%d]: 0x%x", pLoopQueue->readIdx, *ppOut);
	calcLoopQueueNextIdx(&pLoopQueue->readIdx, pLoopQueue->maxCnt);
	return TRUE;
}

static stDuFbQueueMng s_du_fb_queue_mngs[DU_MNG_NUM];
static pthread_mutex_t s_du_fb_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void InitDuFbQueues()
{
	for(int i = 0; i < DU_MNG_NUM; i++){
		s_du_fb_queue_mngs[i].state = AVAIL_STATUS_FREE;
		s_du_fb_queue_mngs[i].pLoopQueue = NULL;
	}
	return;
}

enBOOL CreateDuFbQueue(Int32 duId, Int32 queueMaxCnt)
{
	enBOOL ret = TRUE;
	SAFETY_MUTEX_LOCK(s_du_fb_queue_mutex);
	pthread_condattr_t attrCond;
	if(duId < 0 || duId >= DU_MNG_NUM || 
		s_du_fb_queue_mngs[duId].state != AVAIL_STATUS_FREE){
		ret = FALSE;
	}
	else{
		s_du_fb_queue_mngs[duId].state = AVAIL_STATUS_USED;
		pthread_mutex_init(&s_du_fb_queue_mngs[duId].mutex, NULL);
		pthread_condattr_init(&attrCond);
		pthread_condattr_setclock(&attrCond, CLOCK_MONOTONIC);
		pthread_cond_init(&s_du_fb_queue_mngs[duId].pushCond, &attrCond);
		pthread_cond_init(&s_du_fb_queue_mngs[duId].popCond, &attrCond);
		s_du_fb_queue_mngs[duId].pLoopQueue = CreateLoopQueue(queueMaxCnt);
	}
	SAFETY_MUTEX_UNLOCK(s_du_fb_queue_mutex);
	return (ret && s_du_fb_queue_mngs[duId].pLoopQueue) ? TRUE : FALSE;
}

enBOOL DestoryDuFbQueue(Int32 duId)
{
	enBOOL ret = TRUE;
	SAFETY_MUTEX_LOCK(s_du_fb_queue_mutex);
	if(duId < 0 || duId >= DU_MNG_NUM || 
		s_du_fb_queue_mngs[duId].state != AVAIL_STATUS_USED){
		ret = FALSE;
	}
	else{
		s_du_fb_queue_mngs[duId].state = AVAIL_STATUS_FREE;
		DeleteLoopQueue(s_du_fb_queue_mngs[duId].pLoopQueue);
		pthread_cond_destroy(&s_du_fb_queue_mngs[duId].pushCond);
		pthread_cond_destroy(&s_du_fb_queue_mngs[duId].popCond);
	}
	SAFETY_MUTEX_UNLOCK(s_du_fb_queue_mutex);
	return ret;
}

uInt32 GetDuFbQueueNum(Int32 duId)
{
	if(duId < 0 || duId >= DU_MNG_NUM || 
		s_du_fb_queue_mngs[duId].state != AVAIL_STATUS_USED){
		return 0;
	}
	return getLoopQueueNums(s_du_fb_queue_mngs[duId].pLoopQueue);
}

/* 	function: put the fb data into display cache
	timeOutMs :   -1  waitforever
			     >0  wait timeOutMs ms
*/
enBOOL SendToDuFbQueue(Int32 duId, void *pFbMem, Int32 timeOutMs)
{
	CHECK_POINTER_NULL(pFbMem, FALSE);
	if(duId < 0 || duId >= DU_MNG_NUM || 
		s_du_fb_queue_mngs[duId].state != AVAIL_STATUS_USED ||
		timeOutMs < -1){
		return FALSE;
	}

	enBOOL ret = TRUE;
	int os_result = 0;
	SAFETY_MUTEX_LOCK(s_du_fb_queue_mngs[duId].mutex);
	while(WriteLoopQueue(s_du_fb_queue_mngs[duId].pLoopQueue, &pFbMem) == FALSE){
		/* if loop queue is full */
		if(timeOutMs == -1){
			os_result = pthread_cond_wait(&s_du_fb_queue_mngs[duId].pushCond, &s_du_fb_queue_mngs[duId].mutex);
		}
		else{
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			tv.tv_nsec += (timeOutMs%1000)*1000*1000;
			tv.tv_sec += tv.tv_nsec/(1000*1000*1000) + timeOutMs/1000;
			tv.tv_nsec = tv.tv_nsec%(1000*1000*1000);
			os_result = pthread_cond_timedwait(&s_du_fb_queue_mngs[duId].pushCond, &s_du_fb_queue_mngs[duId].mutex, &tv);
		}

		if(os_result == ETIMEDOUT){
			FBMNG_LOG_ERR("SendToDispCache time out!");
			ret = FALSE;
			break;
		}
	}
	SAFETY_MUTEX_UNLOCK(s_du_fb_queue_mngs[duId].mutex);
	pthread_cond_signal(&s_du_fb_queue_mngs[duId].popCond);
	return ret;
}

/* timeOutMs :   -1  waitforever
			     >0  wait timeOutMs ms
*/
enBOOL ReceiveFromDuFbQueue(Int32 duId, void **ppFbMem, Int32 timeOutMs)
{
	if(duId < 0 || duId >= DU_MNG_NUM || 
		s_du_fb_queue_mngs[duId].state != AVAIL_STATUS_USED ||
		timeOutMs < -1){
		return FALSE;
	}
	enBOOL ret = TRUE;
	int os_result = 0;
	SAFETY_MUTEX_LOCK(s_du_fb_queue_mngs[duId].mutex);
	while(ReadLoopQueue(s_du_fb_queue_mngs[duId].pLoopQueue, ppFbMem) == FALSE){
		/* if loop queue is empty */
		//FBMNG_LOG_WARNING("Receive from Du FB Queue waiting.....");
		if(timeOutMs == -1){
			os_result = pthread_cond_wait(&s_du_fb_queue_mngs[duId].popCond, &s_du_fb_queue_mngs[duId].mutex);
		}
		else{
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			tv.tv_nsec += (timeOutMs%1000)*1000*1000;
			tv.tv_sec += tv.tv_nsec/(1000*1000*1000) + timeOutMs/1000;
			tv.tv_nsec = tv.tv_nsec%(1000*1000*1000);
			os_result = pthread_cond_timedwait(&s_du_fb_queue_mngs[duId].popCond, &s_du_fb_queue_mngs[duId].mutex, &tv);
		}
		//FBMNG_LOG_WARNING("Receive from Du FB Queue wait Over.....");

		if(os_result == ETIMEDOUT){
			FBMNG_LOG_ERR("ReceiveFromDispCache time out!");
			ret = FALSE;
			break;
		}
	}
	SAFETY_MUTEX_UNLOCK(s_du_fb_queue_mngs[duId].mutex);
	pthread_cond_signal(&s_du_fb_queue_mngs[duId].pushCond);
	return ret;
}
