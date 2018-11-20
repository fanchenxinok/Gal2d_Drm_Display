/************************************************
*   Author:  Fanchenxin
*   Date:  2018/05/17
*************************************************/

#ifndef __DRM_WRAPPER_H__
#define __DRM_WRAPPER_H__
/* drm headers */
#include <omap/omap_drm.h>
#include <libdrm/omap_drmif.h>
#include <xf86drm.h>
#include <drm_fourcc.h>
#include <xf86drmMode.h>
#include "log.h"
#include "common.h"

#define DRM_LOG_ERR(msg, ...)	    LOG_ERR(LOG_COL_RED_YLW "[DRM_ERR]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define DRM_LOG_WARNING(msg, ...)	LOG_WARNING(LOG_COL_RED_WHI "[DRM_WARNING]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define DRM_LOG_NOTIFY(msg, ...)	LOG_NOTIFY(LOG_COL_NONE "[DRM_NOTIFY]"msg LOG_COL_NONE"\n", ##__VA_ARGS__)
#define DRM_LOG_NONE

#define DU_NUM  		  (4)
#define CUR_DU_IDX 		  (2)
#define USE_BO_MAP		  (1)


/* drm frame buffer num */
#if DRM_CREATE_MUTI_FB
#define DRM_FRAME_BUFFER_NUM (8)
#else
#define DRM_FRAME_BUFFER_NUM (2)
#endif

typedef struct
{
	uInt32 virtualWidth;
	uInt32 virtualHeight;
	uInt32 stride;
	uInt32 size;
	uInt32 handle;
	uInt8 *pMapAddr;
	uInt32 fbId;
	uInt32 fbNo;
}stFrameBufferInfo;

typedef struct
{
	uInt32 width;
	uInt32 height;
	Int32 crtcId;
	Int32 fourcc;
	uInt32 curFbIdx;
	enBOOL duValid;
	uInt32 duNo;
	stFrameBufferInfo frameBuffers[DRM_FRAME_BUFFER_NUM];
	drmModeModeInfoPtr pDrmMode;
	drmModeConnector *pDrmConnector;
}stDisplayUnit;

typedef struct
{
	enBOOL init;
	Int32 drmFd;
	drmModeResPtr pDrmResources;
	stDisplayUnit dispUnits[DU_NUM];
}stDrmInfo;

#define MODULE_NAME ((const char *)"omapdrm")

Int32 calcProcessTime(Int32 *pStartMs);
Int32 drmInit(stDrmInfo *pDrmInfo);
void drmFin(stDrmInfo *pDrmInfo);
#endif
