/************************************************
*   Author:  Fanchenxin
*   Date:  2018/05/17
*************************************************/
#ifndef __GC320_WRAPPER_H__
#define __GC320_WRAPPER_H__

#include <gc_hal.h>
#include <gc_hal_raster.h>
#include <gc_hal_base.h>
#include <limits.h>
#include <linux/sched.h>
#include <stdarg.h>
#include "log.h"

#define GC320_LOG_ERR(msg, ...)     LOG_ERR(LOG_COL_RED_YLW "[GC320_ERR]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define GC320_LOG_WARNING(msg, ...) LOG_WARNING(LOG_COL_RED_WHI "[GC320_WARNING]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define GC320_LOG_NOTIFY(msg, ...)  LOG_NOTIFY(LOG_COL_NONE "[GC320_NOTIFY]"msg LOG_COL_NONE"\n", ##__VA_ARGS__)
#define GC320_LOG_NONE

#define GC320_CHECK(func) \
	do{ \
		gceSTATUS status = func; \
		if(gcmIS_ERROR(status)){ \
			GC320_LOG_ERR("%s() call %s Fail at Line: %d, status = %d (%s)", __FUNCTION__, #func, \
				__LINE__, status, gcoOS_DebugStatus2Name(status)); \
			return; \
		} \
	}while(0)

#define GC320_CHECK_RETURN(func, ret) \
	do{ \
		gceSTATUS status = func; \
		if(gcmIS_ERROR(status)){ \
			GC320_LOG_ERR("%s() call %s Fail at Line: %d, status = %d (%s)", __FUNCTION__, #func, \
				__LINE__, status, gcoOS_DebugStatus2Name(status)); \
			return ret; \
		} \
	}while(0)


#define MAX_MINORFEATURE_NUMBER (10)
#define SRC_SURF_NUM	(8)
#define FULL_TRANSPARENT_COLOR   (0x00000000)

typedef enum
{
	GC320_PLANE0 = 1,
	GC320_PLANE1 = 2,
	GC320_PLANE2 = 4,
	GC320_PLANE3 = 8
}enPlaneNo;

typedef struct
{
    const char      *name;
    gceSURF_FORMAT  format;
} stFomateInfo;

typedef struct
{
    const char* name;
    const char* msg;
    gceFEATURE  feature;
    gctBOOL     status;
}stFeatureInfo;

typedef struct
{
    gcoOS           os;
    gcoHAL          hal;
    gco2D           engine2d;

    gceCHIPMODEL    ChipModel;
    gctUINT32       ChipRevision;
    gctUINT32       PatchRevision;
    gctUINT32       ChipFeatures;
    gctUINT32       ChipMinorFeaturesNumber;
    gctUINT32       ChipMinorFeatures[MAX_MINORFEATURE_NUMBER];

	gcoSURF         target;
    gctUINT         width;
    gctUINT         height;
    gceSURF_FORMAT  format;
    gcePOOL         pool;
    gctBOOL         pe20;
    gctBOOL         fullDFB;

    gctBOOL         saveTarget;
    gctBOOL         cleanTarget;
    gctSTRING       saveFullName;

    gctBOOL         noSaveTargetNew;
    gctBOOL         createTarget;
    gctBOOL         notSupport;

    gctUINT32       startFrame;
    gctUINT32       endFrame;
} stGalRuntime;

typedef struct
{
	gcoSURF			srcSurf;
	gceSURF_FORMAT	srcFormat;
	gctUINT			srcWidth;
	gctUINT			srcHeight;
	gctINT			srcStrides[3];
	gctINT			srcStrideNum;
	gctUINT32		srcPhyAddrs[3];
	gctINT			srcAddrNum;
	gctPOINTER		srcLgcAddrs[3];
	gctBOOL			srcAlphaBlend;
}stSrcSurfaceInfo;

typedef struct
{
	// dst
    gcoSURF			dstSurf;
	gceSURF_FORMAT	dstFormat;
	gctUINT			dstWidth;
	gctUINT			dstHeight;
	gctINT			dstStride;
	gctUINT32		dstPhyAddr;
	gctPOINTER		dstLgcAddr;
	gctSHBUF		dstShareBuff;

	//source surfaces
    stSrcSurfaceInfo  srcSurfs[SRC_SURF_NUM];

	//tmp surface for scaling surface
	gcoSURF			tmpSurf;
	gceSURF_FORMAT	tmpFormat;
	gctUINT			tmpWidth;
	gctUINT			tmpHeight;
	gctINT			tmpStride;
	gctUINT32		tmpPhyAddr;
	gctPOINTER		tmpLgcAddr;
} stTest2D;

typedef struct _STATUS_MAP {
    gceSTATUS status;
    const char*string;
} STATUS_MAP;

static STATUS_MAP sStatusMap[] = {
    {gcvSTATUS_OK                    , "No Error" },
    {gcvSTATUS_NO_MORE_DATA         , "No More Data" },
    {gcvSTATUS_CACHED                , "Cached" },
    {gcvSTATUS_MIPMAP_TOO_LARGE        , "Mipmap Too Large" },
    {gcvSTATUS_NAME_NOT_FOUND        , "Name Not Found" },
    {gcvSTATUS_NOT_OUR_INTERRUPT    , "Not Our Interrupt" },
    {gcvSTATUS_MISMATCH                , "Mismatch" },
    {gcvSTATUS_TRUE                    , "True" },
    {gcvSTATUS_INVALID_ARGUMENT        , "Invalid Argument" },
    {gcvSTATUS_INVALID_OBJECT         , "Invalid Object" },
    {gcvSTATUS_OUT_OF_MEMORY         , "Out Of Memory" },
    {gcvSTATUS_MEMORY_LOCKED        , "Memory Locked" },
    {gcvSTATUS_MEMORY_UNLOCKED        , "Memory Unlocked" },
    {gcvSTATUS_HEAP_CORRUPTED        , "Heap Corrupted" },
    {gcvSTATUS_GENERIC_IO            , "Generic IO" },
    {gcvSTATUS_INVALID_ADDRESS        , "Invalid Address" },
    {gcvSTATUS_CONTEXT_LOSSED        , "Context Lossed" },
    {gcvSTATUS_TOO_COMPLEX            , "Too Complex" },
    {gcvSTATUS_BUFFER_TOO_SMALL        , "Buffer Too Small" },
    {gcvSTATUS_INTERFACE_ERROR        , "Interface Error" },
    {gcvSTATUS_NOT_SUPPORTED        , "Not Supported" },
    {gcvSTATUS_MORE_DATA            , "More Data" },
    {gcvSTATUS_TIMEOUT                , "Timeout" },
    {gcvSTATUS_OUT_OF_RESOURCES        , "Out Of Resources" },
    {gcvSTATUS_INVALID_DATA            , "Invalid Data" },
    {gcvSTATUS_INVALID_MIPMAP        , "Invalid Mipmap" },
    {gcvSTATUS_CHIP_NOT_READY        , "Chip Not Ready" },
    /* Linker errors. */
    {gcvSTATUS_GLOBAL_TYPE_MISMATCH    , "Global Type Mismatch" },
    {gcvSTATUS_TOO_MANY_ATTRIBUTES    , "Too Many Attributes" },
    {gcvSTATUS_TOO_MANY_UNIFORMS    , "Too Many Uniforms" },
    {gcvSTATUS_TOO_MANY_VARYINGS    , "Too Many Varyings" },
    {gcvSTATUS_UNDECLARED_VARYING    , "Undeclared Varying" },
    {gcvSTATUS_VARYING_TYPE_MISMATCH, "Varying Type Mismatch" },
    {gcvSTATUS_MISSING_MAIN            , "Missing Main" },
    {gcvSTATUS_NAME_MISMATCH        , "Name Mismatch" },
    {gcvSTATUS_INVALID_INDEX        , "Invalid Index" },
};

gctINT Gc320ChipCheck(stGalRuntime *rt);
gctBOOL Gc320Initialize();
gctBOOL Gc320CreateDstSufface();
gctBOOL Gc320SetTargetSurfaceBuffer(gctPOINTER lgcAddr);
void Gc320PrepareTestCase1();
gctBOOL Gc320RenderTest1(gctUINT32 frameNo, void** ppFrameData);
void Gc320PrepareTestCase2();
gctBOOL Gc320RenderTest2(gctUINT32 frameNo, void** ppFrameData);
void Gc320PrepareTestCase3();
void Gc320SetTest2(gctUINT32 frameNo);
gctBOOL Gc320RenderTest3(gctUINT32 frameNo, void** ppFrameData);
void Gc320SetTest3(gctUINT32 frameNo);
void Gc320PrepareTestCase4();
gctBOOL Gc320RenderTest4(gctUINT32 frameNo, void** ppFrameData);
void Gc320SetTest4(gctUINT32 frameNo);
void Gc320PrepareTestCase5();
gctBOOL Gc320RenderTest5(gctUINT32 frameNo, void** ppFrameData);
void Gc320PrepareTestCase6();
gctBOOL Gc320RenderTest6(gctUINT32 frameNo, void** ppFrameData);
void Gc320PrepareTestCase7();
gctBOOL Gc320RenderTest7(gctUINT32 frameNo, void** ppFrameData);
void Gc320PrepareTestCase8();
gctBOOL Gc320RenderTest8(gctUINT32 frameNo, void** ppFrameData);
void Gc320PrepareTestCase9();
gctBOOL Gc320RenderTest9(gctUINT32 frameNo, void** ppFrameData);

void Gc320DestroyAllSurfaces();
gctBOOL Gc320CleanDstSurface(gctUINT32 color);
void Gc320Finalize();
void Gc320DumpFeatureAvailableInfo();
const stGalRuntime* Gc320GetHandle();
gctBOOL Gc320GetSurfaceInfo(
	gctUINT *pAlignWidth, 
	gctUINT *pAlignHeight,
	gctINT *pAlignStride,
	gctUINT8 *bytesPix);

gctBOOL Gc320CheckAlignment(gceSURF_FORMAT surfFormat, void *pBuffer);
gctUINT32 Gc320GetAddrAlignment(gceSURF_FORMAT surfFormat);
gctBOOL GalDumpDataToBmp(
	void *pData,
	const char* bmpFileName, 
	gceSURF_FORMAT format,
	gctUINT width,
	gctUINT height);

gctBOOL Gc320ClearSurfaceRegions(
	gctUINT8 surfIdx,
	gcsRECT_PTR fillRects,
	gctUINT8 rectNum,
	gctUINT32 color);

gctBOOL Gc320HollowSurfaceRegions(
	gctUINT8 surfIdx,
	gcsRECT_PTR fillRects,
	gctUINT8 rectNum);

#endif
