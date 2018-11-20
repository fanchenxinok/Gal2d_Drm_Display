/************************************************
*   Author:  Fanchenxin
*   Date:  2018/05/17
*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <linux/sched.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <getopt.h>
#include <poll.h>
#include <sys/resource.h>

#include "drm_wrapper.h"
#include "gc320_wrapper.h"
#include "log.h"
#include "fb_mng.h"
#include "image_wrapper.h"

#define TEST_LOG_ERR(msg, ...)	    LOG_ERR(LOG_COL_RED_YLW "[TEST_ERR]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define TEST_LOG_WARNING(msg, ...)	LOG_WARNING(LOG_COL_RED_WHI "[TEST_WARNING]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define TEST_LOG_NOTIFY(msg, ...)	LOG_NOTIFY(LOG_COL_NONE "[TEST_NOTIFY]"msg LOG_COL_NONE"\n", ##__VA_ARGS__)
#define TEST_LOG_TIME(msg, ...)		LOG_WARNING(LOG_COL_YLW_PUR "[TEST_NOTIFY]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define TEST_LOG_FPS(msg, ...)		LOG_ERR(LOG_COL_YLW_GRN "[TEST_NOTIFY]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define TEST_LOG_NONE

#define DESIRE_FPS  (30)

static uInt16 s_test_case = 0;
static enBOOL s_use_epoll = TRUE;
static uInt32 s_render_interval = 1; //1ms
static Int32 s_run_time = -1; // second
static Int32 s_run_frames = -1; // render frames
static enBOOL s_run_pause = FALSE;

#if USE_SOFTWARE_VSYNC
static pthread_mutex_t sw_vsync_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sw_vsync_cond;
static enBOOL sw_vsync_loop_active = FALSE;
static Int64 sw_vsync_rate;
#endif

typedef gctBOOL (*pfTestCase)(gctUINT32, void**);

static void getRenderTestCase(uInt32 frameNo, pfTestCase *ppfTestCase)
{
	switch(s_test_case){
		case 1:
			*ppfTestCase = Gc320RenderTest1;
			break;
		case 2:
			Gc320SetTest2(frameNo);
			*ppfTestCase = Gc320RenderTest2;
			break;
		case 3:
			//Gc320SetTest3(frameNo);
			*ppfTestCase = Gc320RenderTest3;
			break;
		case 4:
			Gc320SetTest4(frameNo);
			*ppfTestCase = Gc320RenderTest4;
			break;
		case 5:
			*ppfTestCase = Gc320RenderTest5;
			break;
		case 6:
			*ppfTestCase = Gc320RenderTest6;
			break;
		case 7:
			*ppfTestCase = Gc320RenderTest7;
			break;
		case 8:
			*ppfTestCase = Gc320RenderTest8;
			break;
		case 9:
			*ppfTestCase = Gc320RenderTest9;
			break;
		default:
			*ppfTestCase = NULL;
			break;
	}
	return;
}

#if 0
static char *memcpy_improve(char *to, char *from, size_t n)
{
    long esi, edi;
    int ecx;
    esi = (long)from;
    edi = (long)to;
    asm volatile("rep ; movsl"
        : "=&c" (ecx), "=&D" (edi), "=&S" (esi)
        : "0" (n / 4), "1" (edi), "2" (esi)
        : "memory"
        );
    return to;
}
#endif

void readUserCommand()
{
	Int32 ret = -1;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds); //标准输入的fd = 0
	Int8 msg[256] = {'\0'};
	while(1){
		memset(msg, '\0', 256 * sizeof(Int8));
		ret = select(1, &fds, NULL, NULL, NULL);
	    if (ret <= 0) {
	        TEST_LOG_WARNING( "select error (ret %d %d)", ret, errno);
	        continue;
	    } else if (FD_ISSET(0, &fds)) { // if user input something
	    	fgets(msg, 256, stdin);
			printf("User msg: %s\n", msg);
			if(strncmp(msg, "pause", 1) == 0){
				s_run_pause = TRUE;
			}
			else if(strncmp(msg, "resume", 1) == 0){
				s_run_pause = FALSE;
			}
			else if(strncmp(msg, "quit", 4) == 0){
				break;
			}
			else{
				//do noting
			}
	    }
		else{
			TEST_LOG_WARNING("poll no events");
		}
	}
	return;
}

static void fillDataToRect(
	uInt8 *pDstAddr, 
	uInt32 dstWidth, 
	uInt32 dstHeight,
	uInt8 *pSrcAddr, 
	stRect *pRect, 
	uInt8 pixBytes)
{
	CHECK_POINTER_VALID(pDstAddr);
	CHECK_POINTER_VALID(pSrcAddr);
	CHECK_POINTER_VALID(pRect);
	uInt8 *pDstStartPos = NULL, *pSrcStartPos = NULL;
	uInt32 i = 0, j = 0, srcStride = 0, dstStride = 0, cpyStride = 0, cpyHeight;
	srcStride = pRect->width * pixBytes;
	dstStride = dstWidth * pixBytes;
	Int32 ms = 0;
	ms = calcProcessTime(NULL);
	/* safty check */
	cpyStride = ((pRect->x + pRect->width) > dstWidth) ?  (dstWidth - pRect->x) * pixBytes: srcStride;
	cpyHeight = ((pRect->y + pRect->height) > dstHeight) ? (dstHeight - pRect->y): pRect->height;
	
	pDstStartPos = pDstAddr+ pRect->y * dstStride + pRect->x * pixBytes;
	pSrcStartPos = pSrcAddr;
	TEST_LOG_NONE("cpyHeight = %d, cpyStride = %d", cpyHeight, cpyStride);
	for (i = 0; i < cpyHeight; ++i) {
	    memcpy(pDstStartPos, pSrcStartPos, cpyStride);
	    pDstStartPos += dstStride;
		pSrcStartPos += srcStride;
	}
	ms = calcProcessTime(&ms);
	TEST_LOG_TIME("FillDataToRect spend time: %d ms", ms);
	return;
}

static void epollAddEvent(Int32 epollFd, Int32 addFd, Int32 state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = addFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, addFd, &ev);
	return;
}

static void drawDuMem2Drm(Int32 drmFd, stDisplayUnit *pDu)
{
	CHECK_POINTER_VALID(pDu);
	Int32 ret = -1;
	#if DRM_CREATE_MUTI_FB
	/* in this case, pDu->curFbIdx is not use */
	stFrameBufferInfo *pfb = NULL;
	if(ReceiveFromDuFbQueue(CUR_DU_IDX, (void**)&pfb, -1)){
		TEST_LOG_NONE("ReceiveFromDuFbQueue: 0x%x", pfb);
		if(pfb == NULL) return;

		#if 0
		static Int32 cnt = 0;
		char fileName[255] = {'\0'};
		if(cnt++ < 60){
			sprintf(fileName, "./dumpImage/bmp_%03d.bmp", cnt);
			GalDumpDataToBmp(pfb->pMapAddr, fileName, gcvSURF_A8R8G8B8, pfb->virtualWidth, pfb->virtualHeight);
		}
		#endif
		//drmSetCursor(drmFd, pDu->crtcId, pfb->handle);
		//drmMoveCursor(drmFd, pDu->crtcId, 100, 100);
		
		TEST_LOG_WARNING("Rec(FrameBufferInfo): virtualWidth = %d, virtualHeight = %d, stride = %d," \
					"size = %d, handle = %d, pMapAddr = 0x%x, fbId = %d", pfb->virtualWidth,
					pfb->virtualHeight, pfb->stride, pfb->size, pfb->handle,
					pfb->pMapAddr, pfb->fbId);
		ret = drmModePageFlip(drmFd, pDu->crtcId, pfb->fbId, DRM_MODE_PAGE_FLIP_EVENT, pDu);
		if (ret) {
			TEST_LOG_ERR("cannot flip CRTC for connector %u : %d", pDu->pDrmConnector->connector_id, errno);
		}
		TEST_LOG_NOTIFY("------- FB no = %d", pfb->fbNo);
		FreeFbBuffer((void*)pfb);
	}
	#else
	stFrameBufferInfo *pfb = &pDu->frameBuffers[pDu->curFbIdx];
	if(pfb->pMapAddr == NULL){
		TEST_LOG_ERR("pfb->pMapAddr is NULL!");
		return;
	}

	TEST_LOG_NONE("Rec(FrameBufferInfo): virtualWidth = %d, virtualHeight = %d, stride = %d," \
					"size = %d, handle = %d, pMapAddr = 0x%x, fbId = %d", pfb->virtualWidth,
					pfb->virtualHeight, pfb->stride, pfb->size, pfb->handle,
					pfb->pMapAddr, pfb->fbId);
	memset(pfb->pMapAddr, 0, pfb->size);

	stFbMem *pFbMem = NULL;
	/* read image data from cache queue */
	if(ReceiveFromDuFbQueue(CUR_DU_IDX, (void**)&pFbMem, -1)){
		TEST_LOG_NONE("ReceiveFromDispCache: 0x%x", pFbMem);
		if(pFbMem == NULL) return;
		fillDataToRect(pfb->pMapAddr, pDu->width, pDu->height, pFbMem->imageData, &pFbMem->dispArea, pFbMem->bytesPix);
		ret = drmModePageFlip(drmFd, pDu->crtcId, pfb->fbId, DRM_MODE_PAGE_FLIP_EVENT, pDu);
		if (ret) {
			TEST_LOG_ERR("cannot flip CRTC for connector %u : %d", pDu->pDrmConnector->connector_id, errno);
		}
		else{
			pDu->curFbIdx ^= 1;
		}
		FreeFbBuffer((void*)pFbMem);
		TEST_LOG_NONE("Free: 0x%x", pFbMem);
	}
	#endif
	return;
}

/* Show frames per second */
static void showFps(void)
{
    static Int32 framecount = 0;
    static Int32 lastframecount = 0;
    static Int32 lastfpstime = 0;
    static float fps = 0;

	struct timespec endTime;
	clock_gettime(CLOCK_REALTIME, &endTime);

    framecount++;
    if (!(framecount & 0x7)) {
        Int32 now = endTime.tv_sec * 1000000000 + endTime.tv_nsec;
        Int32 diff = now - lastfpstime;
        fps = ((framecount - lastframecount) * (float)(1000000000)) / diff;
        lastfpstime = now;
        lastframecount = framecount;
        TEST_LOG_FPS("%d Frames, %f FPS", framecount, fps);
    }
	return;
}

static void showTestDefine()
{
	#if GC320_POOL_USER
	printf("GC320_POOL_USER = 1\n");
	#else
	printf("GC320_POOL_USER = 0\n");
	#endif

	#if DRM_CREATE_MUTI_FB
	printf("DRM_CREATE_MUTI_FB = 1\n");
	#else
	printf("DRM_CREATE_MUTI_FB = 0\n");
	#endif
	return;
}

static void pageFlipEvent(
	Int32 drmFd, 
	uInt32 frame,
	uInt32 sec,
	uInt32 usec,
	void *data)
{
	stDisplayUnit *pDu = (stDisplayUnit*)data;
	CHECK_POINTER_VALID(pDu);
	drawDuMem2Drm(drmFd, pDu);
	TEST_LOG_WARNING("Flip page: current du is %d", pDu->duNo);
	showFps();
	TEST_LOG_NOTIFY("Du Fb queue have %d number valid.", GetDuFbQueueNum(pDu->duNo));
	return;
}

void closeEpoll(void * arg)
{
	close(*(Int32*)arg);
	printf("closeEpoll() was called, ID = %d!\n", *(Int32*)arg);
	return;
}

/* wait vsync and get image data from cache queue, then draw to drm */
static void* epollWaitVsyncThread(void* pUserData)
{
	stDrmInfo *pDrmInfo = (stDrmInfo*)pUserData;
	CHECK_POINTER_VALID_RET(pDrmInfo, NULL);
	Int32 epollId = epoll_create(1);
	epollAddEvent(epollId, pDrmInfo->drmFd, EPOLLIN);
	struct epoll_event events[1] = {0};
	Int32 i = 0, cnt = 0, ret = -1;

	/* if thread was terminated abnormal, should close epoll fd */
	pthread_cleanup_push(closeEpoll, &epollId);

	drmEventContext ev;
	/* Set this to only the latest version you support. Version 2
	 * introduced the page_flip_handler, so we use that. */
	ev.version = 2;
	ev.page_flip_handler = pageFlipEvent;

	#if 0
	for(i = 0; i < DU_NUM; i++){
		if(pDrmInfo->dispUnits[i].duValid){
			drawDuMem2Drm(pDrmInfo->drmFd ,&pDrmInfo->dispUnits[i]);
		}
	}
	#endif
	drawDuMem2Drm(pDrmInfo->drmFd ,&pDrmInfo->dispUnits[CUR_DU_IDX]);
	TEST_LOG_WARNING("ePoll Wait: pDrmInfo->drmFd = %d", pDrmInfo->drmFd);
	while(1)
	{
		Int32 ms = 0;
		ms = calcProcessTime(NULL);
		Int32 fireEvents = epoll_wait(epollId, events, 1, -1);
		ms = calcProcessTime(&ms);
		if(fireEvents > 0){
			TEST_LOG_NONE("VSync %d: %d ms", cnt++, ms);
			drmHandleEvent(pDrmInfo->drmFd, &ev);
		}
		else{
			TEST_LOG_WARNING("fireEvents = %d", fireEvents);
		}
	}

	close(epollId);
	printf("Epoll was closed!\n");
	pthread_cleanup_pop(0);
	return (void*)0;
}

static void *pollWaitVsyncThread(void *pUserData)
{
    fd_set fds;
    Int32 ret = -1;
	stDrmInfo *pDrmInfo = (stDrmInfo*)pUserData;
	CHECK_POINTER_VALID_RET(pDrmInfo, NULL);
	drmEventContext evctx = {DRM_EVENT_CONTEXT_VERSION, NULL, pageFlipEvent};
	drawDuMem2Drm(pDrmInfo->drmFd ,&pDrmInfo->dispUnits[CUR_DU_IDX]);
    FD_ZERO(&fds);
    FD_SET(pDrmInfo->drmFd, &fds);
	TEST_LOG_WARNING("Poll Wait: pDrmInfo->drmFd = %d", pDrmInfo->drmFd);
	while(1)
	{
        ret = select(pDrmInfo->drmFd + 1, &fds, NULL, NULL, NULL);
        if (ret <= 0) {
            TEST_LOG_WARNING( "select error (ret %d %d)", ret, errno);
            continue;
        } else if (FD_ISSET(pDrmInfo->drmFd, &fds)) { //  one flip end
            drmHandleEvent(pDrmInfo->drmFd, &evctx);
        }
		else{
			TEST_LOG_WARNING("poll no events");
		}
    }

    return NULL;
}


#if USE_SOFTWARE_VSYNC
void startSwVsync()
{
    pthread_mutex_lock(&sw_vsync_mutex);
    sw_vsync_rate = 1000000000 / DESIRE_FPS;
	TEST_LOG_TIME("sw_vsync_rate = %ld ns", sw_vsync_rate);
    if (sw_vsync_loop_active) {
        pthread_mutex_unlock(&sw_vsync_mutex);
        return;
    }
    sw_vsync_loop_active = TRUE;
    pthread_mutex_unlock(&sw_vsync_mutex);
    pthread_cond_signal(&sw_vsync_cond);
	return;
}

void stopSwVsync()
{
    pthread_mutex_lock(&sw_vsync_mutex);
    if (!sw_vsync_loop_active) {
        pthread_mutex_unlock(&sw_vsync_mutex);
        return;
    }
    sw_vsync_loop_active = FALSE;
    pthread_mutex_unlock(&sw_vsync_mutex);
    pthread_cond_signal(&sw_vsync_cond);
	return;
}

static struct timespec timespecDiff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec-start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

static void *waitSwVsyncThread(void *pUserData)
{
    Int32 ret = -1;
	struct timespec tp, tp_next, tp_sleep;
    Int64 now = 0, period = sw_vsync_rate, next_vsync = 0, next_fake_vsync = 0, sleep = 0;
    tp_sleep.tv_sec = tp_sleep.tv_nsec = 0;
	
	stDrmInfo *pDrmInfo = (stDrmInfo*)pUserData;
	CHECK_POINTER_VALID_RET(pDrmInfo, NULL);
	drawDuMem2Drm(pDrmInfo->drmFd ,&pDrmInfo->dispUnits[CUR_DU_IDX]);
	startSwVsync();
	
	TEST_LOG_WARNING("Sw Wait: pDrmInfo->drmFd = %d", pDrmInfo->drmFd);
	while(1)
	{
        pthread_mutex_lock(&sw_vsync_mutex);
        while (!sw_vsync_loop_active) {
            pthread_cond_wait(&sw_vsync_cond, &sw_vsync_mutex);
        }
        /* the vsync_rate should be re-read after
        * user sets the vsync_rate by calling start_sw_vsync
        * explicitly. This is guaranteed by re-reading it
        * after the vsync_cond is signalled.
        */
        period = sw_vsync_rate; /* re-read rate */
		TEST_LOG_TIME("Period = %ld", period);
        pthread_mutex_unlock(&sw_vsync_mutex);

#if 1
		#if 1
        clock_gettime(CLOCK_MONOTONIC, &tp);
        now = (Int64)(tp.tv_sec * 1000000000) + tp.tv_nsec;
        next_vsync = next_fake_vsync;
        sleep = next_vsync - now;
		TEST_LOG_TIME("(1) now = %llu, next_vsync = %llu, sleep = %llu", now, next_vsync, sleep);
        if (sleep < 0) {
            // we missed, find where the next vsync should be
            sleep = (period - ((now - next_vsync) % period));
            next_vsync = now + sleep;
        }
		TEST_LOG_TIME("(2) now = %llu, next_vsync = %llu, sleep = %llu", now, next_vsync, sleep);
        next_fake_vsync = next_vsync + period;
        tp_next.tv_sec = (next_vsync / 1000000000);
        tp_next.tv_nsec = (next_vsync % 1000000000);
        tp_sleep = timespecDiff(tp, tp_next);
		TEST_LOG_TIME("tp.tv_sec = %ld, tp.tv_nsec = %ld, tp_next.tv_sec = %ld, tp_next.tv_nsec = %ld", 
			tp.tv_sec, tp.tv_nsec, tp_next.tv_sec, tp_next.tv_nsec);
		#else
		tp_sleep.tv_nsec = period % 1000000000;
		tp_sleep.tv_sec = period / 1000000000;
		#endif

		Int32 ms = 0;
		ms = calcProcessTime(NULL);
        if(nanosleep(&tp_sleep, NULL) != 0){
			TEST_LOG_ERR("nanosleep fail, errno = %d, tp_sleep.tv_sec = %ld, tp_sleep.tv_nsec = %ld, sleep = %d ms", 
				errno, tp_sleep.tv_sec, tp_sleep.tv_nsec, ms);
			//continue;
		}
		ms = calcProcessTime(&ms);
		TEST_LOG_TIME("tp_sleep.tv_sec = %lu, tp_sleep.tv_nsec = %lu, sleep = %d ms", 
			tp_sleep.tv_sec, tp_sleep.tv_nsec, ms);
#else
		struct timeval tv;
	    tv.tv_sec = sw_vsync_rate / 1000000000;
	    tv.tv_usec= sw_vsync_rate / 1000 % 1000000;
	    Int32 err, ms;
		ms = calcProcessTime(NULL);
	    do{
	        err = select(0, NULL, NULL, NULL, &tv);
	    }while(err < 0 && errno == EINTR);
		ms = calcProcessTime(&ms);
		TEST_LOG_TIME("ms = %d, tv.tv_sec = %lu, tv.tv_usec = %lu, period = %d", ms, tv.tv_sec, tv.tv_usec, sw_vsync_rate);
#endif			
        drawDuMem2Drm(pDrmInfo->drmFd ,&pDrmInfo->dispUnits[CUR_DU_IDX]);
		TEST_LOG_WARNING("Flip page: current du is %d", pDrmInfo->dispUnits[CUR_DU_IDX].duNo);
		showFps();
		TEST_LOG_NOTIFY("Du Fb queue have %d number valid.", GetDuFbQueueNum(pDrmInfo->dispUnits[CUR_DU_IDX].duNo));
    }

    return NULL;
}
#endif


#if GC320_POOL_USER
/* 直接将drm的bo的虚拟地址空间用作目标surface的输出，就不用memcpy */
/* render image data to cache queue */
static void* renderBufferThread(void* pUserData)
{
	stGalRuntime *pRunTime = (stGalRuntime*)pUserData;
	Int32 frameNo = 0;
	uInt32 i = 0, j = 0;
	
    while(1)
	{
		/* if pause rendering */
		if(s_run_pause){
			usleep(100);
			continue;
		}
		
		Int32 ms = 0;
		ms = calcProcessTime(NULL);
		
		void *pFrameData = NULL;
        TEST_LOG_NOTIFY("Begin to render frame %d ...", frameNo);
		#if DRM_CREATE_MUTI_FB
		stFrameBufferInfo *pFbMem = NULL;
		GetFbBuffer((void**)&pFbMem);
		if(!pFbMem || !pFbMem->pMapAddr) continue;
		
		if(!Gc320SetTargetSurfaceBuffer((void*)pFbMem->pMapAddr)){
			TEST_LOG_ERR("Gc320SetTargetSurfaceBuffer() Fail!!!");
			break;
		}
		TEST_LOG_NOTIFY("======= FB no = %d", pFbMem->fbNo);
		Gc320CheckAlignment(gcvSURF_A8R8G8B8, (void*)pFbMem->pMapAddr);
		#else
		stFbMem *pFbMem = NULL;
		GetFbBuffer((void**)&pFbMem);
		if(!pFbMem || !pFbMem->imageData) continue;
		if(!Gc320SetTargetSurfaceBuffer((void*)pFbMem->imageData)){
			TEST_LOG_ERR("Gc320SetTargetSurfaceBuffer() Fail!!!");
			break;
		}

		Gc320CheckAlignment(gcvSURF_A8R8G8B8, (void*)pFbMem->imageData);
		#endif

        /* clear target surface to black. */
        if(!Gc320CleanDstSurface(0xFF000000)){
			TEST_LOG_ERR("Gc320CleanSuffer Fail!!!");
			break;
        }

		pfTestCase pTestCase = NULL;
		getRenderTestCase(frameNo, &pTestCase);
		
        if (pTestCase && pTestCase(frameNo, &pFrameData)){
            /* Rendering succeed. */
			if(pFrameData){
				Int32 c_ms = 0;
				TEST_LOG_TIME("Test case spend time: %d ms", calcProcessTime(&ms));
				#if DRM_CREATE_MUTI_FB
				TEST_LOG_NONE("FrameBufferInfo: virtualWidth = %d, virtualHeight = %d, stride = %d," \
					"size = %d, handle = %d, pMapAddr = 0x%x, fbId = %d", pFbMem->virtualWidth,
					pFbMem->virtualHeight, pFbMem->stride, pFbMem->size, pFbMem->handle,
					pFbMem->pMapAddr, pFbMem->fbId);
				
				Int32 stride = 0;
				uInt32 surfWidth = 0, surfHeight = 0;
				uInt8 surfBytesPix = 0;
				Gc320GetSurfaceInfo(&surfWidth,
									&surfHeight,
									&stride,
									&surfBytesPix);
				TEST_LOG_NONE("aligned W = %d, H = %d, S = %d, BP = %d",
								surfWidth,
								surfHeight,
								stride,
								surfBytesPix);

				#if 0
				static Int32 cnt = 0;
				char fileName[255] = {'\0'};
				if(cnt++ < 60){
					sprintf(fileName, "./dumpImage/bmp_%03d.bmp", cnt);
					GalDumpDataToBmp(pFrameData, fileName, gcvSURF_A8R8G8B8, surfWidth, surfHeight);
				}
				#endif
				
				#else //#if DRM_CREATE_MUTI_FB

				Int32 stride = 0;
				Gc320GetSurfaceInfo(&pFbMem->dispArea.width,
									&pFbMem->dispArea.height,
									&stride,
									&pFbMem->bytesPix);
				TEST_LOG_NOTIFY("aligned W = %d, H = %d, S = %d, BP = %d",
								pFbMem->dispArea.width,
								pFbMem->dispArea.height,
								stride,
								pFbMem->bytesPix);
				#endif

				SendToDuFbQueue(CUR_DU_IDX, pFbMem, -1);
				TEST_LOG_WARNING( "Rendering frame %d ... succeed", frameNo);
			}
			else{
				TEST_LOG_WARNING("Frame data pointer is NULL");
			}
        }
		else{
            /* Un expect case test. */
			uInt32 i =0, j = 0;
			uInt32 color = ImageRandGetRGB();
      		#if DRM_CREATE_MUTI_FB
			uInt32 *p = (uInt32*)pFbMem->pMapAddr;
			for(i = 0; i < pFbMem->virtualHeight; i++){
				for(j = 0; j < pFbMem->virtualWidth; j++){
					*p = color;
					p++;
				}
			}
			#else
			//TEST_LOG_WARNING("color = %d", color);
			pFbMem->width = 1920;
			pFbMem->height = 720;
			pFbMem->dispArea.x = 0;
			pFbMem->dispArea.y = 0;
			pFbMem->dispArea.width = 1920;
			pFbMem->dispArea.height = 720;
			pFbMem->bytesPix = 4;
			uInt32 *p = (uInt32*)pFbMem->imageData;
			for(i = 0; i < 720; i++){
				for(j = 0; j < 1920; j ++){
					*p = color;
					p++;
				}
			}
			#endif
			
			SendToDuFbQueue(CUR_DU_IDX, pFbMem, -1);
			TEST_LOG_WARNING( "Rendering frame %d ... succeed", frameNo);
        }

		#if DRM_CREATE_MUTI_FB
		uInt32 msPerFrame = 1000 / DESIRE_FPS;
		Int32 diff = msPerFrame - calcProcessTime(&ms);
		if(diff > 0){
			s_render_interval = diff - 1; 
		}
		#endif
		
		usleep(s_render_interval * 1000); // s_render_interval ms
		ms = calcProcessTime(&ms);
		TEST_LOG_TIME("Rendering %d frame spend time: %d ms", frameNo, ms);
        /* next frame */
        frameNo++;
		if(s_run_frames != -1 && frameNo > s_run_frames){
			break;
		}
    }

	return (void*)0;
}

#else
/* 多用了memcpy  时间花费比较多 */
/* render image data to cache queue */
static void* renderBufferThread(void* pUserData)
{
	stGalRuntime *pRunTime = (stGalRuntime*)pUserData;
	Int32 frameNo = 0;
	uInt32 i = 0, j = 0;
	
    while(1)
	{
		/* if pause rendering */
		if(s_run_pause){
			usleep(100);
			continue;
		}
	
		Int32 ms = 0;
		ms = calcProcessTime(NULL);
		void *pFrameData = NULL;
        TEST_LOG_NOTIFY("Begin to render frame %d ...", frameNo);
			
        /* clear target surface to black. */
        if(!Gc320CleanDstSurface(0xFF000000)){
			TEST_LOG_ERR("Gc320CleanSuffer Fail!!!");
			break;
        }

		pfTestCase pTestCase = NULL;
		getRenderTestCase(frameNo, &pTestCase);

        if (pTestCase && pTestCase(frameNo, &pFrameData)){
            /* Rendering succeed. */
			if(pFrameData){
				Int32 c_ms = 0;
				TEST_LOG_TIME("Test case spend time: %d ms", calcProcessTime(&ms));
				#if DRM_CREATE_MUTI_FB
				stFrameBufferInfo *pFbMem = NULL;
				GetFbBuffer((void**)&pFbMem);

				if(!pFbMem) continue;
				TEST_LOG_NONE("FrameBufferInfo: virtualWidth = %d, virtualHeight = %d, stride = %d," \
					"size = %d, handle = %d, pMapAddr = 0x%x, fbId = %d", pFbMem->virtualWidth,
					pFbMem->virtualHeight, pFbMem->stride, pFbMem->size, pFbMem->handle,
					pFbMem->pMapAddr, pFbMem->fbId);
				
				Int32 stride = 0;
				uInt32 surfWidth = 0, surfHeight = 0;
				uInt8 surfBytesPix = 0;
				Gc320GetSurfaceInfo(&surfWidth,
									&surfHeight,
									&stride,
									&surfBytesPix);
				TEST_LOG_NONE("aligned W = %d, H = %d, S = %d, BP = %d",
								surfWidth,
								surfHeight,
								stride,
								surfBytesPix);

				stRect dispArea = {0};
				/* test dispArea change */
				//dispArea.x = frameNo % surfWidth;
				//dispArea.y = frameNo % surfHeight;
				dispArea.width = surfWidth;
				dispArea.height = surfHeight;

				if(pFbMem->pMapAddr == NULL){
					TEST_LOG_WARNING("pFbMem->pMapAddr is NULL!");
					continue;
				}

				memset(pFbMem->pMapAddr, 0, pFbMem->size);

				if((dispArea.x == 0) && (dispArea.y == 0) && (stride == pFbMem->stride)){
					/* map地址拷贝到map地址速度很慢 ， 大概80ms */
					c_ms = calcProcessTime(NULL);
					memcpy(pFbMem->pMapAddr, pFrameData, stride * surfHeight);
					c_ms = calcProcessTime(&c_ms);
					TEST_LOG_TIME("memcpy(pFrameData[0x%x] To pFbMem->pMapAddr[0x%x]) spend time: %d ms",
									pFrameData, pFbMem->pMapAddr, c_ms);
				}
				else{
					/* just support ARGB8888 */
					fillDataToRect(pFbMem->pMapAddr, pFbMem->virtualWidth, pFbMem->virtualHeight, pFrameData, &dispArea, surfBytesPix);				
				}
				
				#else
				
				stFbMem *pFbMem = NULL;
				GetFbBuffer((void**)&pFbMem);
				
				if(!pFbMem) continue;
				memset(pFbMem, 0, sizeof(stFbMem));

				Int32 stride = 0;
				Gc320GetSurfaceInfo(&pFbMem->dispArea.width,
									&pFbMem->dispArea.height,
									&stride,
									&pFbMem->bytesPix);
				TEST_LOG_NOTIFY("aligned W = %d, H = %d, S = %d, BP = %d",
								pFbMem->dispArea.width,
								pFbMem->dispArea.height,
								stride,
								pFbMem->bytesPix);
				/* test dispArea change */
				//pFbMem->dispArea.x = frameNo % pFbMem->dispArea.width;
				//pFbMem->dispArea.y = frameNo % pFbMem->dispArea.height;

				/* map地址拷贝到用户地址，也很慢大概35ms */
				c_ms = calcProcessTime(NULL);
				memcpy(pFbMem->imageData, pFrameData, sizeof(uInt8) * stride * pFbMem->dispArea.height);
				//memcpy_improve(pFbMem->imageData, pFrameData, sizeof(uInt8) * stride * pFbMem->dispArea.height);
				c_ms = calcProcessTime(&c_ms);
				TEST_LOG_TIME("memcpy(pFrameData[0x%x] To pFbMem->imageData[0x%x]) spend time: %d ms",
									pFrameData, pFbMem->imageData, c_ms);
				#endif

				SendToDuFbQueue(CUR_DU_IDX, pFbMem, -1);
				TEST_LOG_NOTIFY( "Rendering frame %d ... succeed", frameNo);
			}
			else{
				TEST_LOG_WARNING("Frame data pointer is NULL");
			}
        }
		else{
            /* Un expect case test. */
			uInt32 i =0, j = 0;
			uInt32 color = ImageRandGetRGB();
      		#if DRM_CREATE_MUTI_FB
			stFrameBufferInfo *pFbMem = NULL;
			GetFbBuffer((void**)&pFbMem);
			if(!pFbMem) continue;
			uInt32 *p = (uInt32*)pFbMem->pMapAddr;
			for(i = 0; i < pFbMem->virtualHeight; i++){
				for(j = 0; j < pFbMem->virtualWidth; j++){
					*p = color;
					p++;
				}
			}
			#else
			stFbMem *pFbMem = NULL;
			GetFbBuffer((void**)&pFbMem);
			if(!pFbMem) continue;

			//TEST_LOG_WARNING("color = %d", color);
			pFbMem->width = 1920;
			pFbMem->height = 720;
			pFbMem->dispArea.x = 0;
			pFbMem->dispArea.y = 0;
			pFbMem->dispArea.width = 1920;
			pFbMem->dispArea.height = 720;
			pFbMem->bytesPix = 4;
			uInt32 *p = (uInt32*)pFbMem->imageData;
			for(i = 0; i < 720; i++){
				for(j = 0; j < 1920; j ++){
					*p = color;
					p++;
				}
			}
			#endif
			SendToDuFbQueue(CUR_DU_IDX, pFbMem, -1);
			TEST_LOG_NOTIFY( "Rendering frame %d ... succeed", frameNo);
        }

		#if DRM_CREATE_MUTI_FB
		uInt32 msPerFrame = 1000 / DESIRE_FPS;
		Int32 diff = msPerFrame - calcProcessTime(&ms);
		if(diff > 0){
			s_render_interval = diff - 1; 
		}
		#endif
		
		usleep(s_render_interval * 1000); // s_render_interval ms
		ms = calcProcessTime(&ms);
		TEST_LOG_TIME("Rendering %d frame spend time: %d ms", frameNo, ms);
        /* next frame */
        frameNo++;
		if(s_run_frames != -1 && frameNo > s_run_frames){
			break;
		}
    }

	return (void*)0;
}
#endif


typedef void* (*pThreadFunc)(void*);
static pthread_t threadCreate(pThreadFunc thread_func)
{	
	pthread_attr_t attr;
	pthread_t thread_id;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create (&thread_id, &attr, &thread_func, NULL);
	pthread_attr_destroy (&attr);
	return thread_id;
}

static void showUsage()
{
	printf("Usage: ./test_main  [params]\n" );
	printf("	(1) -c [--case] test_case (1 ~ 7).\n");
	printf("	(2) -l [--log] log_level (0 ~ 3) [0: only printf work,  1: ERR work,  2: WARNING work, 3: NOTIFY work]\n");
	printf("	(3) -p [--poll] use_poll or epoll (0 or 1).\n");
	printf("	(4) -i [--interval] render interval time (ms).\n");
	printf("	(5) -t [--time] test running time (seconds).\n");
	printf("	(6) -f [--frame] render frames");
	printf("	(7) -h [--help] show the usage of this test.\n");
	printf("	Example: ./test_main -c 1 -l 2 -p 0 -i 10 -t 100\n");
	return;
}

static enBOOL parseArgs(Int32 argc, char* argv[])
{
	if(argc != 1 && argc != 2 &&
		argc != 3 && argc != 5 &&
		argc != 7 && argc != 9 && 
		argc != 11 && argc != 13){
		showUsage();
		return FALSE;
	}
	
	Int8* short_options = "c:l:p:i:t:f:h"; 
    struct option long_options[] = {  
       //{"reqarg", required_argument, NULL, 'r'},  
       //{"noarg",  no_argument,       NULL, 'n'},  
       //{"optarg", optional_argument, NULL, 'o'}, 
       {"case", required_argument, NULL, 'c'+'l'},
       {"log", required_argument, NULL, 'l' + 'l'},
	   {"poll", required_argument, NULL, 'p' + 'l'},
	   {"interval", required_argument, NULL, 'i' + 'l'},
	   {"time", required_argument, NULL, 't' + 'l'},
	   {"frame", required_argument, NULL, 'f' + 'l'},
	   {"help", no_argument, NULL, 'h' + 'l'},
       {0, 0, 0, 0}};  

    Int32 opt = 0;
    while ( (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1){
        switch(opt){
            case 'c':
            case 'c'+'l':
            {
                if(optarg){
                    char* case_n = optarg;
                    s_test_case = atoi(case_n);
                    printf("Test case = %d\n", s_test_case);
                }
                else{
                    printf("case optarg is null\n");
                }
                break;
            }

			case 'l':
            case 'l'+'l':
            {
                if(optarg){
                    char* log_level = optarg;
                    log_show_level_set = atoi(log_level);
                    printf("log_show_level_set = %d\n", log_show_level_set);
                }
                else{
                    printf("log level optarg is null\n");
                }
                break;
            }

			case 'p':
            case 'p'+'l':
            {
                if(optarg){
                    char* use_poll = optarg;
                    s_use_epoll = (atoi(use_poll) == 1) ? FALSE : TRUE;
                    printf("use epoll = %d\n", s_use_epoll);
                }
                else{
                    printf("use epoll optarg is null\n");
                }
                break;
            }

			case 'i':
            case 'i'+'l':
            {
                if(optarg){
                    char* time = optarg;
                    s_render_interval = atoi(time);
                    printf("Render interval time = %d ms\n", s_render_interval);
                }
                else{
                    printf("Render interval time optarg is null\n");
                }
                break;
            }
			case 't':
            case 't'+'l':
            {
                if(optarg){
                    char* time = optarg;
                    s_run_time = atoi(time);
                    printf("Test run time = %d ms\n", s_run_time);
                }
                else{
                    printf("Test run time optarg is null\n");
                }
                break;
            }
			case 'f':
            case 'f'+'l':
            {
                if(optarg){
                    char* frames = optarg;
                    s_run_frames = atoi(frames);
                    printf("Test run frames = %d ms\n", s_run_frames);
                }
                else{
                    printf("Test run frames optarg is null\n");
                }
                break;
            }
			case 'h':
            case 'h'+'l':
			{
				showUsage();
				return FALSE;
            }
            default:
                break;
        }
    }
	return TRUE;
}

Int32 main(Int32 argc, char** argv)
{
	pthread_t renderThreadId = -1, waitVsyThreadId = -1, swVsyThreadId = -1;
	stDrmInfo drmInfo = {0};
	Int32 ret = -1;
	printf("sizeof(Int64) = %d, sizeof(Int32) = %d, sizeof(long int) = %d\n", sizeof(Int64), sizeof(Int32), sizeof(long int));
	
	/* prepare log thread */
	#if LOG_THREAD_ON
	logTaskInit();
	#endif

	if(!parseArgs(argc, argv)){
		return -1;
	}

	showTestDefine();
	
	/* prepare drm */
	ret = drmInit(&drmInfo);
	if(ret < 0){
		TEST_LOG_ERR("DRM Init Fail!!!");
		return -1;
	}
	TEST_LOG_NOTIFY("Prepare Drm success!");

	/* prepare fb buffer cache */
	CreateFbBufferList(drmInfo.dispUnits[CUR_DU_IDX].frameBuffers);
	InitDuFbQueues();
	if(!CreateDuFbQueue(CUR_DU_IDX, FB_BUFFER_NUM)){
		TEST_LOG_ERR("CreateDispUnitMng Fail!!!");
		goto DRM_FIN;
	}
	TEST_LOG_NOTIFY("Prepare Frame buffer list success!");

	/* prepare gc320 */
	if(!Gc320Initialize()){
		TEST_LOG_ERR("Gc320Initialize Fail!!!");
		goto FB_FIN;
	}
	
	Gc320DumpFeatureAvailableInfo();
	if(!Gc320CreateDstSufface()){
		TEST_LOG_ERR("Gc320CreateSufface Fail!!!");
		goto GC320_FIN;
	}

	switch(s_test_case)
	{
		case 1:
			Gc320PrepareTestCase1();
			break;
		case 2:
			Gc320PrepareTestCase2();
			break;
		case 3:
			Gc320PrepareTestCase3();
			break;
		case 4:
			Gc320PrepareTestCase4();
			break;
		case 5:
			Gc320PrepareTestCase5();
			break;
		case 6:
			Gc320PrepareTestCase6();
			break;
		case 7:
			Gc320PrepareTestCase7();
			break;
		case 8:
			Gc320PrepareTestCase8();
			break;
		case 9:
			Gc320PrepareTestCase9();
			break;
		default:
			break;
	}
	
	TEST_LOG_NOTIFY("Prepare Gc320 success!");

	const stGalRuntime *pHandle = Gc320GetHandle(); 

	#if USE_SOFTWARE_VSYNC
	/* use software vsync */
	pthread_cond_init(&sw_vsync_cond, NULL);
    pthread_create(&swVsyThreadId, NULL, waitSwVsyncThread, (void*)&drmInfo);
	pthread_setname_np(swVsyThreadId, "swVsync");
	#else
	/* create wait vsync signal thread */
	if(s_use_epoll){
		pthread_create(&waitVsyThreadId, NULL, epollWaitVsyncThread, (void*)&drmInfo);
	}
	else{
		pthread_create(&waitVsyThreadId, NULL, pollWaitVsyncThread, (void*)&drmInfo);
	}
	pthread_setname_np(waitVsyThreadId, "waitVsync");
	#endif

	/* create render buffer thread */
	pthread_create(&renderThreadId, NULL, renderBufferThread, (void*)&pHandle);
	pthread_setname_np(renderThreadId, "renderBuffer");

	//usleep(300000);  //sleep 300 ms
	//drawDuMem2Drm(drmInfo.drmFd ,&drmInfo.dispUnits[CUR_DU_IDX]);
	if(s_run_time == -1){
		#if 0
		while(1){
			usleep(1000000);
		}
		#else
		readUserCommand();
		#endif
	}
	else{
		uInt32 cnt = 0;
		while(cnt++ < s_run_time){
			usleep(1000000);
		}
	}

	/* end thread */
	if (0 == pthread_cancel(renderThreadId)){
		pthread_join(renderThreadId, NULL);
        printf("render thread finish success\n");
    } else {
        printf("render thread finish fail\n");
    }

	#if USE_SOFTWARE_VSYNC
	stopSwVsync();
	if (0 == pthread_cancel(swVsyThreadId)){
		pthread_join(swVsyThreadId, NULL);
        printf("sw vsync thread finish success\n");
    } else {
        printf("sw vsync thread finish fail\n");
    }
	#else
	if (0 == pthread_cancel(waitVsyThreadId)){
		pthread_join(waitVsyThreadId, NULL);
        printf("wait vsync thread finish success\n");
    } else {
        printf("wait vsync thread finish fail\n");
    }
	#endif
	
	/* free resources */
	Gc320DestroyAllSurfaces();
	Gc320Finalize();
	DestoryDuFbQueue(CUR_DU_IDX);
	DestoryFbBufferList();
	drmFin(&drmInfo);
	logTaskFin();
	return 0;

GC320_FIN:
	Gc320DestroyAllSurfaces();
	Gc320Finalize();
FB_FIN:
	DestoryDuFbQueue(CUR_DU_IDX);
	DestoryFbBufferList();
DRM_FIN:
	drmFin(&drmInfo);
	logTaskFin();
	return -1;
}
