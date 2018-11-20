#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <linux/sched.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include "drm_wrapper.h"
#include "image_wrapper.h"

static Int32 drmFormat2Bpp(Int32 fourcc)
{
    Int32 bpp;
    switch (fourcc) {
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV16:
    case DRM_FORMAT_NV61:
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
        bpp = 8;
        break;

    case DRM_FORMAT_ARGB4444:
    case DRM_FORMAT_XRGB4444:
    case DRM_FORMAT_ABGR4444:
    case DRM_FORMAT_XBGR4444:
    case DRM_FORMAT_RGBA4444:
    case DRM_FORMAT_RGBX4444:
    case DRM_FORMAT_BGRA4444:
    case DRM_FORMAT_BGRX4444:
    case DRM_FORMAT_ARGB1555:
    case DRM_FORMAT_XRGB1555:
    case DRM_FORMAT_ABGR1555:
    case DRM_FORMAT_XBGR1555:
    case DRM_FORMAT_RGBA5551:
    case DRM_FORMAT_RGBX5551:
    case DRM_FORMAT_BGRA5551:
    case DRM_FORMAT_BGRX5551:
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_VYUY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_YVYU:
        bpp = 16;
        break;

    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
        bpp = 24;
        break;

    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_RGBX8888:
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_ABGR2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_RGBA1010102:
    case DRM_FORMAT_RGBX1010102:
    case DRM_FORMAT_BGRA1010102:
    case DRM_FORMAT_BGRX1010102:
        bpp = 32;
        break;

    default:
        DRM_LOG_WARNING( "unsupported format 0x%08x", fourcc);
        return -1;
    }
    return bpp;
}

static uInt32 drmFormat2height(Int32 fourcc, uInt32 height)
{
    uInt32 virtual_height;
    switch (fourcc) {
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
        virtual_height = height * 3 / 2;
        break;

    case DRM_FORMAT_NV16:
    case DRM_FORMAT_NV61:
        virtual_height = height * 2;
        break;

    default:
        virtual_height = height;
        break;
    }
    return virtual_height;
}

static Int32 drmFormat2Handle(
	Int32 fourcc,
	uint32_t *handles,
	uint32_t *pitches,
	uint32_t *offsets,
	uint32_t handle,
	uint32_t pitch,
	uint32_t height,
	uint32_t offset_base)
{
    switch (fourcc) {
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_VYUY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_YVYU:
        offsets[0] = offset_base;
        handles[0] = handle;
        pitches[0] = pitch;
        break;

    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV16:
    case DRM_FORMAT_NV61:
        offsets[0] = offset_base;
        handles[0] = handle;
        pitches[0] = pitch;
        pitches[1] = pitches[0];
        offsets[1] = offsets[0] + pitches[0] * height;
        handles[1] = handle;
        break;

    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
        offsets[0] = offset_base;
        handles[0] = handle;
        pitches[0] = pitch;
        pitches[1] = pitches[0] / 2;
        offsets[1] = offsets[0] + pitches[0] * height;
        handles[1] = handle;
        pitches[2] = pitches[1];
        offsets[2] = offsets[1] + pitches[1] * height / 2;
        handles[2] = handle;
        break;

    case DRM_FORMAT_ARGB4444:
    case DRM_FORMAT_XRGB4444:
    case DRM_FORMAT_ABGR4444:
    case DRM_FORMAT_XBGR4444:
    case DRM_FORMAT_RGBA4444:
    case DRM_FORMAT_RGBX4444:
    case DRM_FORMAT_BGRA4444:
    case DRM_FORMAT_BGRX4444:
    case DRM_FORMAT_ARGB1555:
    case DRM_FORMAT_XRGB1555:
    case DRM_FORMAT_ABGR1555:
    case DRM_FORMAT_XBGR1555:
    case DRM_FORMAT_RGBA5551:
    case DRM_FORMAT_RGBX5551:
    case DRM_FORMAT_BGRA5551:
    case DRM_FORMAT_BGRX5551:
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_RGBX8888:
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_ABGR2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_RGBA1010102:
    case DRM_FORMAT_RGBX1010102:
    case DRM_FORMAT_BGRA1010102:
    case DRM_FORMAT_BGRX1010102:
        offsets[0] = offset_base;
        handles[0] = handle;
        pitches[0] = pitch;
        break;
	default:
		DRM_LOG_WARNING("Invalid format!!!!");
		break;
    }
    return 0;
}

static Int32 drmCreateOneFrameBoMap(
	Int32 drmFd,
	Int32 fourcc,
	uInt32 width,
	uInt32 height,
	stFrameBufferInfo *pFBInfo)
{
	CHECK_POINTER_VALID_RET(pFBInfo, -1);
	struct omap_bo *bo = NULL;
    struct omap_device *dev = NULL;
	uint32_t bpp = 0;
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	
	dev = omap_device_new(drmFd);
	if(!dev){
		DRM_LOG_ERR("%s()->omap_device_new() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		return -1;
	}

	pFBInfo->virtualWidth = width;
	pFBInfo->virtualHeight = drmFormat2height(fourcc, height);
	bpp = drmFormat2Bpp(fourcc);
	pFBInfo->stride = pFBInfo->virtualWidth * bpp / 8;
	pFBInfo->size = pFBInfo->stride * pFBInfo->virtualHeight;
	bo = omap_bo_new(dev, pFBInfo->size, OMAP_BO_WC);
	if(!bo){
		DRM_LOG_ERR("%s()->omap_bo_new() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		goto FAIL;
	}

	void *vaddr = omap_bo_map(bo);
	if(vaddr == NULL){
		DRM_LOG_ERR("%s()->omap_bo_map() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		goto FAIL;
	}
    memset(vaddr, 0x0, pFBInfo->size);
	pFBInfo->pMapAddr = (uInt8*)vaddr;

	pFBInfo->handle = omap_bo_handle(bo);
	/* create framebuffer object for the dumb-buffer */
	drmFormat2Handle(fourcc, handles, pitches, offsets, pFBInfo->handle, pFBInfo->stride, height, 0);
	if(drmModeAddFB2(drmFd, width, height, fourcc, handles, pitches, offsets, &pFBInfo->fbId, 0) < 0){
		DRM_LOG_ERR("%s()->drmIoctl() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		goto FAIL;
	}
	
    omap_device_del(dev);

	DRM_LOG_NOTIFY("FBInfo: fourcc: %d; virtual w , h = %d, %d; bpp = %d, handle = %d, fbId = %d",
		fourcc, pFBInfo->virtualWidth, pFBInfo->virtualHeight, bpp, pFBInfo->handle, pFBInfo->fbId);
	
	DRM_LOG_WARNING("mapAddr = 0x%x, size = %d", pFBInfo->pMapAddr, pFBInfo->size);
	return 0;
FAIL:
	if(bo){
		omap_bo_del(bo);
	}
	if(dev){
    	omap_device_del(dev);
	}
	return -1;
}

static Int32 drmCreateOneFrameBuffer(
	Int32 drmFd,
	Int32 fourcc,
	uInt32 width,
	uInt32 height,
	stFrameBufferInfo *pFBInfo)
{
	CHECK_POINTER_VALID_RET(pFBInfo, -1);
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	/*create dumb buffer */
	memset(&creq, 0, sizeof(creq));
	pFBInfo->virtualWidth = creq.width = width;
	pFBInfo->virtualHeight = creq.height = drmFormat2height(fourcc, height);
	creq.bpp = drmFormat2Bpp(fourcc);
	if(drmIoctl(drmFd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0){
		DRM_LOG_ERR("%s()->drmIoctl() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		return -1;
	}
	pFBInfo->stride = creq.pitch;
	pFBInfo->size = creq.size;
	pFBInfo->handle = creq.handle;

	/* create framebuffer object for the dumb-buffer */
	drmFormat2Handle(fourcc, handles, pitches, offsets, pFBInfo->handle, pFBInfo->stride, height, 0);
	if(drmModeAddFB2(drmFd, width, height, fourcc, handles, pitches, offsets, &pFBInfo->fbId, 0) < 0){
		DRM_LOG_ERR("%s()->drmIoctl() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		goto DESTORY_CREATE_DUMB;
	}

	/* create buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = pFBInfo->handle;
	if(drmIoctl(drmFd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0){
		DRM_LOG_ERR("%s()->drmIoctl() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		goto REMOVE_FB;
	}

	pFBInfo->pMapAddr = mmap(0, pFBInfo->size, PROT_READ | PROT_WRITE, MAP_SHARED, drmFd, mreq.offset);
	if(pFBInfo->pMapAddr == MAP_FAILED){
		DRM_LOG_ERR("%s()->mmap() Fail!!, line: %d, errorNo = %d", __FUNCTION__, __LINE__, errno);
		goto REMOVE_FB;
	}

	/* clear map buffer */
	memset(pFBInfo->pMapAddr, 0, pFBInfo->size);
	DRM_LOG_NOTIFY("FBInfo: fourcc: %d; virtual w , h = %d, %d; bpp = %d, handle = %d, fbId = %d",
		fourcc, pFBInfo->virtualWidth, pFBInfo->virtualHeight, creq.bpp, pFBInfo->handle, pFBInfo->fbId);
	DRM_LOG_WARNING("Offset = 0x%llx, mapAddr = 0x%x, size = %d", mreq.offset, pFBInfo->pMapAddr, pFBInfo->size);
	return 0;
	
REMOVE_FB:
	drmModeRmFB(drmFd, pFBInfo->fbId);
DESTORY_CREATE_DUMB:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = pFBInfo->handle;
	drmIoctl(drmFd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	return -1;
}

static void drmDestoryOneFrameBoMap(Int32 drmFd, stFrameBufferInfo *pFBInfo)
{
	struct drm_gem_close close_args;
	/* delete frame buffer */
	drmModeRmFB(drmFd, pFBInfo->fbId);
	memset(&close_args, 0, sizeof(struct drm_gem_close));
 
    close_args.handle = pFBInfo->handle;
    drmIoctl(drmFd, DRM_IOCTL_GEM_CLOSE, &close_args);
	return;
}

static void drmDestoryOneFrameBuffer(Int32 drmFd, stFrameBufferInfo *pFBInfo)
{
	CHECK_POINTER_VALID(pFBInfo);
	struct drm_mode_destroy_dumb dreq;
	/* unmap buffer */
	munmap(pFBInfo->pMapAddr, pFBInfo->size);

	/* delete frame buffer */
	drmModeRmFB(drmFd, pFBInfo->fbId);

	/* delete dump buffer */
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = pFBInfo->handle;
	drmIoctl(drmFd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	return;
}

static void drmInfoClear(stDrmInfo *pDrmInfo)
{
	CHECK_POINTER_VALID(pDrmInfo);
	pDrmInfo->drmFd = -1;
	pDrmInfo->pDrmResources = NULL;
	uInt32 i = 0;
	for(i = 0; i < DU_NUM; i++){
		pDrmInfo->dispUnits[i].width = 0;
		pDrmInfo->dispUnits[i].height = 0;
		pDrmInfo->dispUnits[i].crtcId = -1;
		pDrmInfo->dispUnits[i].fourcc = DRM_FORMAT_ARGB8888;
		pDrmInfo->dispUnits[i].curFbIdx = 0;
		pDrmInfo->dispUnits[i].duValid = FALSE;
		pDrmInfo->dispUnits[i].pDrmConnector = NULL;
		pDrmInfo->dispUnits[i].pDrmMode = NULL;
		pDrmInfo->dispUnits[i].duNo = -1;
		memset(pDrmInfo->dispUnits[i].frameBuffers, 0, sizeof(stFrameBufferInfo) * DRM_FRAME_BUFFER_NUM);
	}
	return;
}

static enBOOL drmCheckCrtcHasUsed(stDrmInfo *pDrmInfo, uInt32 curDuIdx, Int32 crtcId)
{
	uInt32 i = 0;
	for(i = 0; i < curDuIdx; i++){
		if(pDrmInfo->dispUnits[i].crtcId == crtcId){
			return TRUE;
		}
	}
	return FALSE;
}

static Int32 drmFindCrtc(stDrmInfo *pDrmInfo, uInt32 curDuIdx)
{
	CHECK_POINTER_VALID_RET(pDrmInfo, -1);
	stDisplayUnit *pCurDu = &pDrmInfo->dispUnits[curDuIdx];
	drmModeEncoder *pDrmEnc = NULL;
	uInt32 i = 0, j = 0;
	/* First try the currently connector */
	if(pCurDu->pDrmConnector){
		pDrmEnc = drmModeGetEncoder(pDrmInfo->drmFd, pCurDu->pDrmConnector->encoder_id);
	}
	else{
		pDrmEnc = NULL;
	}

	if(pDrmEnc){
		if(pDrmEnc->crtc_id){
			/*Current crtc id is not used by other display unit*/
			if(!drmCheckCrtcHasUsed(pDrmInfo, curDuIdx, pDrmEnc->crtc_id)){
				pCurDu->crtcId = pDrmEnc->crtc_id;
				DRM_LOG_NOTIFY("Find Crtc success, crtc id = %d", pDrmEnc->crtc_id);
				drmModeFreeEncoder(pDrmEnc);
				pDrmEnc = NULL;
				return 0;
			}
		}
		/* current encoder has no valid crtc or crtc has been used by other display unit */
		drmModeFreeEncoder(pDrmEnc);
		pDrmEnc = NULL;
	}

	/* iterate all encoders of a connector */
	for(i = 0; i < pCurDu->pDrmConnector->count_encoders; i++){
		pDrmEnc = drmModeGetEncoder(pDrmInfo->drmFd, pCurDu->pDrmConnector->encoders[i]);
		if(!pDrmEnc){
			DRM_LOG_WARNING("[%d] cannot get encoder id = %d", pCurDu->pDrmConnector->encoders[i]);
			continue;
		}
		
		/* iterate all crtcs of an encoder */
		for(j = 0; j < pDrmInfo->pDrmResources->count_crtcs; j++){
			if(pDrmEnc->possible_crtcs & (1 << j)){
				Int32 crtcId = pDrmInfo->pDrmResources->crtcs[j];
				if(!drmCheckCrtcHasUsed(pDrmInfo, curDuIdx, crtcId)){
					pCurDu->crtcId = crtcId;
					DRM_LOG_NOTIFY("Find Crtc success, crtc id = %d", pDrmEnc->crtc_id);
					drmModeFreeEncoder(pDrmEnc);
					pDrmEnc = NULL;
					return 0;
				}
			}
		}
		drmModeFreeEncoder(pDrmEnc);
		pDrmEnc = NULL;
	}
	DRM_LOG_WARNING("Cannot find suitable crtc for connector!!!");
	return -1;
}

static Int32 drmSetCrtc(Int32 drmFd, stDisplayUnit *pDispUnit)
{
	CHECK_POINTER_VALID_RET(pDispUnit, -1);
	uInt32 i = 0;
	Int32 ret = -1;
	ret = drmModeSetCrtc(drmFd,
					  pDispUnit->crtcId, 
					  pDispUnit->frameBuffers[0].fbId,
					  0,
					  0,
					  &pDispUnit->pDrmConnector->connector_id,
					  1,
					  pDispUnit->pDrmMode
					  );
	if(ret < 0){
		DRM_LOG_ERR("Cannot set crtc for connector: %d, ret = %d!", i, ret);
		return -1;
	}
	DRM_LOG_NOTIFY("Set CRTC Fb ID: %d", pDispUnit->frameBuffers[0].fbId);
	return 0;
}

static uint8_t getNextColor(enBOOL *up, uint8_t cur, uInt32 mod)
{
	uint8_t next;

	next = cur + (*up ? 1 : -1) * (rand() % mod);
	if ((*up && next < cur) || (!*up && next > cur)) {
		*up = !*up;
		next = cur;
	}

	return next;
}

static void drmTestDrawRGB(stDrmInfo *pDrmInfo)
{
	uint8_t r, g, b;
	enBOOL r_up, g_up, b_up;
	uInt32 i, h, w, off;
	uInt32 bufferIdx = 0, cnt = 0;

	srand(time(NULL));
	r = rand() % 0xff;
	g = rand() % 0xff;
	b = rand() % 0xff;
	r_up = g_up = b_up = TRUE;

	while(1){
		r = getNextColor(&r_up, r, 20);
		g = getNextColor(&g_up, g, 10);
		b = getNextColor(&b_up, b, 5);

		for (i = 0; i < pDrmInfo->pDrmResources->count_connectors; i++) {
			stDisplayUnit *pdu = &pDrmInfo->dispUnits[i];
			//if(pdu->pDrmConnector->connector_type != DRM_MODE_CONNECTOR_Unknown){
				stFrameBufferInfo *pfb = &pdu->frameBuffers[bufferIdx ^ 1];
				for (h = 0; h < pdu->height; ++h) {
					for (w = 0; w < pdu->width; ++w) {
						off = pfb->stride * h + w * 4;
						*(uint32_t*)&pfb->pMapAddr[off] = (r << 16) | (g << 8) | b;
					}
				}
				bufferIdx ^= 1;
			//}
		}

		usleep(100000);
		if(++cnt > 1000){
			break;
		}
	}
	return;
}

/* return ms */
Int32 calcProcessTime(Int32 *pStartMs)
{
	struct timespec endTime;
	clock_gettime(CLOCK_REALTIME, &endTime);
	if(pStartMs){
		return endTime.tv_sec * 1000 + endTime.tv_nsec / 1000000 - *pStartMs;
	}
	return endTime.tv_sec * 1000 + endTime.tv_nsec / 1000000;
}

static void epollAddEvent(Int32 epollFd, Int32 addFd, Int32 state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = addFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, addFd, &ev);
	return;
}

Int32 drmInit(stDrmInfo *pDrmInfo)
{
	CHECK_POINTER_VALID_RET(pDrmInfo, -1);
	uInt32 i = 0, j = 0;
	uint64_t hasDumb;
	drmModeConnectorPtr drmConn = NULL;
	if(pDrmInfo->init) return 0;
	drmInfoClear(pDrmInfo);
	/* open drm device */
	pDrmInfo->drmFd = drmOpen(MODULE_NAME, NULL);
	if (pDrmInfo->drmFd < 0) {
    	DRM_LOG_ERR("Failed to open omapdrm device");
    	return -1;
	}

	if (drmGetCap(pDrmInfo->drmFd, DRM_CAP_DUMB_BUFFER, &hasDumb) < 0 ||!hasDumb) {
		DRM_LOG_ERR("drm device '%s' does not support dumb buffers", MODULE_NAME);
		goto FAIL;
	}

	DRM_LOG_NOTIFY("Open Drm %s success; fd: %d", MODULE_NAME, pDrmInfo->drmFd);

	/* get drm resources */
	pDrmInfo->pDrmResources = drmModeGetResources(pDrmInfo->drmFd);
	if (!pDrmInfo->pDrmResources) {
		DRM_LOG_ERR("Failed to get drm resources: %s", strerror(errno));
		goto FAIL;
	}

	/* get connectors */
	DRM_LOG_NOTIFY("pDrmInfo->pDrmResources->count_connectors = %d", pDrmInfo->pDrmResources->count_connectors);
	for (i = 0; i < pDrmInfo->pDrmResources->count_connectors; i++) {
        drmConn = drmModeGetConnector(pDrmInfo->drmFd, pDrmInfo->pDrmResources->connectors[i]);
		if(!drmConn) continue;
		/* check if a monitor is connected Or if there is at least one valid mode*/
		if((drmConn->connection != DRM_MODE_CONNECTED) || (drmConn->count_modes == 0)){
			DRM_LOG_WARNING("[%d]Connector ID: %d is unused connector[%d] or has no valid mode[%d].",
				drmConn->connector_id, drmConn->connection, drmConn->count_modes);
			drmModeFreeConnector(drmConn);
			drmConn = NULL;
			continue;
		}

		pDrmInfo->dispUnits[i].width = drmConn->modes[0].hdisplay;
		pDrmInfo->dispUnits[i].height = drmConn->modes[0].vdisplay;
		pDrmInfo->dispUnits[i].pDrmConnector = drmConn;
		pDrmInfo->dispUnits[i].pDrmMode = &drmConn->modes[0];
        
		DRM_LOG_NOTIFY("[%d]Connector ID: %d, ConnectType = %d, [w, h] = [%d, %d]",
			i, drmConn->connector_id, drmConn->connector_type, 
			drmConn->modes[0].hdisplay, drmConn->modes[0].vdisplay);

		if(drmFindCrtc(pDrmInfo, i) < 0){
			DRM_LOG_WARNING("drmFindCrtc Fail!!");
			drmModeFreeConnector(pDrmInfo->dispUnits[i].pDrmConnector);
			pDrmInfo->dispUnits[i].pDrmConnector = NULL;
			pDrmInfo->dispUnits[i].pDrmMode = NULL;
			continue;
		}
		DRM_LOG_NOTIFY("drmFindCrtc() success!!");

		Int32 (*pDrmCreateOneFB)(Int32, Int32, uInt32, uInt32, stFrameBufferInfo*) = NULL;
		void (*pDrmDestoryOneFB)(Int32, stFrameBufferInfo*) = NULL;
		#if USE_BO_MAP
		pDrmCreateOneFB = drmCreateOneFrameBoMap;
		pDrmDestoryOneFB = drmDestoryOneFrameBoMap;
		#else
		pDrmCreateOneFB = drmCreateOneFrameBuffer;
		pDrmDestoryOneFB = drmDestoryOneFrameBuffer;
		#endif

		for(j = 0; j < DRM_FRAME_BUFFER_NUM; j++){
			if(pDrmCreateOneFB(pDrmInfo->drmFd, 
									  pDrmInfo->dispUnits[i].fourcc, 
									  pDrmInfo->dispUnits[i].width,
									  pDrmInfo->dispUnits[i].height,
									  &pDrmInfo->dispUnits[i].frameBuffers[j]) < 0){
				DRM_LOG_ERR("Drm create frame buffer %d fail!!!", j);
				for(; j >= 0; j--){
					pDrmDestoryOneFB(pDrmInfo->drmFd, &pDrmInfo->dispUnits[i].frameBuffers[j]);
				}
				goto FAIL;
			}
			pDrmInfo->dispUnits[i].frameBuffers[j].fbNo = j;
		}

		DRM_LOG_NOTIFY("drm create frame buffers success!!");

		if(drmSetCrtc(pDrmInfo->drmFd, &pDrmInfo->dispUnits[i]) < 0){
			DRM_LOG_ERR("Failed to set crtc for connector: %d", i);
			for(j = 0; j < DRM_FRAME_BUFFER_NUM; j++){
				pDrmDestoryOneFB(pDrmInfo->drmFd, &pDrmInfo->dispUnits[i].frameBuffers[j]);
			}
			goto FAIL;
		}

		pDrmInfo->dispUnits[i].duValid = TRUE;
		pDrmInfo->dispUnits[i].duNo = i;
		drmConn = NULL;
    }
	pDrmInfo->init = TRUE;
	return 0;
FAIL:

	for (i = 0; i < pDrmInfo->pDrmResources->count_connectors; i++) {
		if(pDrmInfo->dispUnits[i].pDrmConnector){
			drmModeFreeConnector(pDrmInfo->dispUnits[i].pDrmConnector);
			pDrmInfo->dispUnits[i].pDrmConnector = NULL;
		}
	}

	if(pDrmInfo->pDrmResources){
		drmModeFreeResources(pDrmInfo->pDrmResources);
		pDrmInfo->pDrmResources = NULL;
	}

	if(pDrmInfo->drmFd != -1){
		drmClose(pDrmInfo->drmFd);
		pDrmInfo->drmFd = -1;
	}
	
	return -1;
}

void drmFin(stDrmInfo *pDrmInfo)
{
	uInt32 i = 0, j = 0;
	void (*pDrmDestoryOneFB)(Int32, stFrameBufferInfo*) = NULL;
	#if USE_BO_MAP
	pDrmDestoryOneFB = drmDestoryOneFrameBoMap;
	#else
	pDrmDestoryOneFB = drmDestoryOneFrameBuffer;
	#endif
	
	for(i = 0; i < pDrmInfo->pDrmResources->count_connectors; i++){
		for(j = 0; j < DRM_FRAME_BUFFER_NUM; j++){
			pDrmDestoryOneFB(pDrmInfo->drmFd, &pDrmInfo->dispUnits[i].frameBuffers[j]);
		}
		drmModeFreeConnector(pDrmInfo->dispUnits[i].pDrmConnector);
		pDrmInfo->dispUnits[i].pDrmConnector = NULL;
	}
	if(pDrmInfo->pDrmResources){
		drmModeFreeResources(pDrmInfo->pDrmResources);
		pDrmInfo->pDrmResources = NULL;
	}

	if(pDrmInfo->drmFd != -1){
		drmClose(pDrmInfo->drmFd);
		pDrmInfo->drmFd = -1;
	}
	pDrmInfo->init = FALSE;
	printf("DRM Finalize success!\n");
	return;
}


// not work of cursor
Int32 drmSetCursor(Int32 drmFd, Int32 crtcId, uInt32 boHandle)
{
	Int32 ret = -1;
	ret = drmModeSetCursor(drmFd, crtcId, boHandle, 16, 16);
	if(ret < 0){
		DRM_LOG_ERR("Drm Set Cursor fail, ret = %d, errno =  %d", ret, errno);
	}
	return ret;
}

Int32 drmMoveCursor(Int32 drmFd, Int32 crtcId, uInt32 x, uInt32 y)
{
	Int32 ret = -1;
	ret = drmModeMoveCursor(drmFd, crtcId, x, y);
	if(ret < 0){
		DRM_LOG_ERR("Drm  Move Cursor fail, ret = %d, errno =  %d", ret, errno);
	}
	return ret;
}


