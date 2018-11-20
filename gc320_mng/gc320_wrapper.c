#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc320_wrapper.h"
#include "image_wrapper.h"
#include "common.h"

static stGalRuntime s_Runtime = {0};
static stTest2D s_Test2D = {0};

/* Video memory mapping. */
static gctPHYS_ADDR  g_InternalPhysical,  g_ExternalPhysical,  g_ContiguousPhysical;
static gctSIZE_T    g_InternalSize,     g_ExternalSize,     g_ContiguousSize;
static gctPOINTER   g_Internal,         g_External,         g_Contiguous;

stFomateInfo c_formatInfos[] =
{
    {"X4R4G4B4" , gcvSURF_X4R4G4B4},

    {"A4R4G4B4" , gcvSURF_A4R4G4B4},

    {"R4G4B4X4" , gcvSURF_R4G4B4X4},

    {"R4G4B4A4" , gcvSURF_R4G4B4A4},

    {"X4B4G4R4" , gcvSURF_X4B4G4R4},

    {"A4B4G4R4" , gcvSURF_A4B4G4R4},

    {"B4G4R4X4" , gcvSURF_B4G4R4X4},

    {"B4G4R4A4" , gcvSURF_B4G4R4A4},



    {"X1R5G5B5" , gcvSURF_X1R5G5B5},

    {"A1R5G5B5" , gcvSURF_A1R5G5B5},

    {"R5G5B5X1" , gcvSURF_R5G5B5X1},

    {"R5G5B5A1" , gcvSURF_R5G5B5A1},

    {"B5G5R5X1" , gcvSURF_B5G5R5X1},

    {"B5G5R5A1" , gcvSURF_B5G5R5A1},

    {"X1B5G5R5" , gcvSURF_X1B5G5R5},

    {"A1B5G5R5" , gcvSURF_A1B5G5R5},



    {"X8R8G8B8" , gcvSURF_X8R8G8B8},

    {"A8R8G8B8" , gcvSURF_A8R8G8B8},

    {"R8G8B8X8" , gcvSURF_R8G8B8X8},

    {"R8G8B8A8" , gcvSURF_R8G8B8A8},

    {"B8G8R8X8" , gcvSURF_B8G8R8X8},

    {"B8G8R8A8" , gcvSURF_B8G8R8A8},

    {"X8B8G8R8" , gcvSURF_X8B8G8R8},

    {"A8B8G8R8" , gcvSURF_A8B8G8R8},



    {"R5G6B5"   , gcvSURF_R5G6B5  },

    {"B5G6R5"   , gcvSURF_B5G6R5  },

    {"YUY2",    gcvSURF_YUY2},
    {"UYVY",    gcvSURF_UYVY},
    {"YVYU",    gcvSURF_YVYU},
    {"VYUY",    gcvSURF_VYUY},
    {"YV12",    gcvSURF_YV12},
    {"I420",    gcvSURF_I420},
    {"NV12",    gcvSURF_NV12},
    {"NV21",    gcvSURF_NV21},
    {"NV16",    gcvSURF_NV16},
    {"NV61",    gcvSURF_NV61},
};

stFeatureInfo c_featureInfos[] =
{
	{"Unknown feature" , "Unknown feature", gcvFEATURE_PIPE_2D, gcvFALSE},
	{"gcvFEATURE_2D_NO_COLORBRUSH_INDEX8 " , "ColorBursh or index8", gcvFEATURE_2D_NO_COLORBRUSH_INDEX8, gcvTRUE},
	{"gcvFEATURE_ANDROID_ONLY " , "Android only feature", gcvFEATURE_ANDROID_ONLY, gcvTRUE},
	{"gcvFEATURE_2DPE20 " , "PE2.0 and related function", gcvFEATURE_2DPE20, gcvFALSE},
	{"gcvFEATURE_2D_BITBLIT_FULLROTATION " , "Flip_Y or Full rotation", gcvFEATURE_2D_BITBLIT_FULLROTATION, gcvFALSE},
	{"gcvFEATURE_2D_DITHER " , "Dither", gcvFEATURE_2D_DITHER, gcvFALSE},
	{"gcvFEATURE_2D_ALL_QUAD " , "One pass filterblit", gcvFEATURE_2D_ALL_QUAD, gcvFALSE},
	{"gcvFEATURE_YUV420_SCALER " , "YUV420 scaler", gcvFEATURE_YUV420_SCALER, gcvFALSE},
	{"gcvFEATURE_2D_OPF_YUV_OUTPUT " , "YUV output of opf", gcvFEATURE_2D_OPF_YUV_OUTPUT, gcvFALSE},
	{"gcvFEATURE_2D_TILING " , "Tilling", gcvFEATURE_2D_TILING, gcvFALSE},
	{"gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND " , "Filter with alphablend", gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND, gcvFALSE},
	{"gcvFEATURE_2D_ONE_PASS_FILTER_TAP " , "One pass filter with 7/9 tap", gcvFEATURE_2D_ONE_PASS_FILTER_TAP, gcvFALSE},
	{"gcvFEATURE_2D_MULTI_SOURCE_BLT " , "MultisrcBlit or some YUV output", gcvFEATURE_2D_MULTI_SOURCE_BLT, gcvFALSE},
	{"gcvFEATURE_2D_MULTI_SOURCE_BLT_EX " , "MultisrcBlit 8 source", gcvFEATURE_2D_MULTI_SOURCE_BLT_EX, gcvFALSE},
	{"gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2 " , "MultisrcBlitV2", gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2, gcvFALSE},
	{"gcvFEATURE_2D_YUV_BLIT " , "YUV blit", gcvFEATURE_2D_YUV_BLIT, gcvFALSE},
	{"gcvFEATURE_2D_A8_TARGET " , "Format A8 output", gcvFEATURE_2D_A8_TARGET, gcvFALSE},
	{"gcvFEATURE_SEPARATE_SRC_DST " , "Separate src and dst address", gcvFEATURE_SEPARATE_SRC_DST, gcvTRUE},
	{"gcvFEATURE_2D_COMPRESSION " , "2D COMPRESSION", gcvFEATURE_2D_COMPRESSION, gcvFALSE},
	{"gcvFEATURE_2D_YUV_SEPARATE_STRIDE " , "Seperate U/V stride", gcvFEATURE_2D_YUV_SEPARATE_STRIDE, gcvFALSE},
	{"gcvFEATURE_2D_COLOR_SPACE_CONVERSION " , "Color space conversion", gcvFEATURE_2D_COLOR_SPACE_CONVERSION, gcvFALSE},
	{"gcvFEATURE_NO_USER_CSC " , "User defined color space conversion", gcvFEATURE_NO_USER_CSC, gcvTRUE},
	{"gcvFEATURE_2D_TILING " , "2D tiling", gcvFEATURE_2D_TILING, gcvFALSE},
	{"gcvFEATURE_2D_SUPER_TILE_V2 " , "2D super tiling v2", gcvFEATURE_2D_TILING, gcvFALSE},
	{"gcvFEATURE_2D_SUPER_TILE_V3 " , "2D super tiling v3", gcvFEATURE_2D_TILING, gcvFALSE},
	{"gcvFEATURE_2D_MINOR_TILING " , "2D minor tiling", gcvFEATURE_2D_MINOR_TILING, gcvFALSE},
	{"gcvFEATURE_2D_SUPER_TILE_VERSION " , "2D super tile version", gcvFEATURE_2D_SUPER_TILE_VERSION, gcvFALSE},
	{"gcvFEATURE_2D_GAMMA " , "Gamma", gcvFEATURE_2D_GAMMA, gcvFALSE},
	{"gcvFEATURE_2D_A8_NO_ALPHA " , "A8 alphablend", gcvFEATURE_2D_A8_NO_ALPHA, gcvTRUE},
	{"gcvFEATURE_2D_FILTERBLIT_A8_ALPHA " , "Filterblit with A8 alphablend", gcvFEATURE_2D_FILTERBLIT_A8_ALPHA, gcvFALSE},
	{"gcvFEATURE_2D_FC_SOURCE " , "FC surface", gcvFEATURE_2D_FC_SOURCE, gcvFALSE},
	{"gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT " , "2D MultiSourceBlit v1.5 (Dest Rect)", gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT, gcvFALSE},
	{"gcvFEATURE_TPC_COMPRESSION " , "TPC compression", gcvFEATURE_TPC_COMPRESSION, gcvFALSE},
	{"gcvFEATURE_DEC_COMPRESSION " , "DEC compression", gcvFEATURE_DEC_COMPRESSION, gcvFALSE},
	{"gcvFEATURE_DEC_TPC_COMPRESSION " , "DEC TPC compression", gcvFEATURE_DEC_TPC_COMPRESSION, gcvFALSE},
	{"gcvFEATURE_FULL_DIRECTFB " , "DFB dest color key mode", gcvFEATURE_FULL_DIRECTFB, gcvFALSE},
	{"gcvFEATURE_2D_MIRROR_EXTENSION " , "2D mirror extension", gcvFEATURE_2D_MIRROR_EXTENSION, gcvFALSE},
	{"gcvFEATURE_2D_FILTERBLIT_FULLROTATION " , "Filterblit full rotation", gcvFEATURE_2D_FILTERBLIT_FULLROTATION, gcvFALSE},
	{"gcvFEATURE_SCALER " , "Filterblit", gcvFEATURE_SCALER, gcvFALSE},

};

const gceFEATURE FeatureList[]=
{
	gcvFEATURE_PIPE_2D,
    gcvFEATURE_PIPE_3D,
    gcvFEATURE_PIPE_VG,
    gcvFEATURE_DC,
    gcvFEATURE_HIGH_DYNAMIC_RANGE,
    gcvFEATURE_MODULE_CG,
    gcvFEATURE_MIN_AREA,
    gcvFEATURE_BUFFER_INTERLEAVING,
    gcvFEATURE_BYTE_WRITE_2D,
    gcvFEATURE_ENDIANNESS_CONFIG,
    gcvFEATURE_DUAL_RETURN_BUS,
    gcvFEATURE_DEBUG_MODE,
    gcvFEATURE_YUY2_RENDER_TARGET,
    gcvFEATURE_FRAGMENT_PROCESSOR,
    gcvFEATURE_2DPE20,
    gcvFEATURE_FAST_CLEAR,
    gcvFEATURE_YUV420_TILER,
    gcvFEATURE_YUY2_AVERAGING,
    gcvFEATURE_FLIP_Y,
    gcvFEATURE_EARLY_Z,
    gcvFEATURE_COMPRESSION,
    gcvFEATURE_MSAA,
    gcvFEATURE_SPECIAL_ANTI_ALIASING,
    gcvFEATURE_SPECIAL_MSAA_LOD,
    gcvFEATURE_422_TEXTURE_COMPRESSION,
    gcvFEATURE_DXT_TEXTURE_COMPRESSION,
    gcvFEATURE_ETC1_TEXTURE_COMPRESSION,
    gcvFEATURE_CORRECT_TEXTURE_CONVERTER,
    gcvFEATURE_TEXTURE_8K,
    gcvFEATURE_SCALER,
    gcvFEATURE_YUV420_SCALER,
    gcvFEATURE_SHADER_HAS_W,
    gcvFEATURE_SHADER_HAS_SIGN,
    gcvFEATURE_SHADER_HAS_FLOOR,
    gcvFEATURE_SHADER_HAS_CEIL,
    gcvFEATURE_SHADER_HAS_SQRT,
    gcvFEATURE_SHADER_HAS_TRIG,
    gcvFEATURE_VAA,
    gcvFEATURE_HZ,
    gcvFEATURE_CORRECT_STENCIL,
    gcvFEATURE_VG20,
    gcvFEATURE_VG_FILTER,
    gcvFEATURE_VG21,
    gcvFEATURE_VG_DOUBLE_BUFFER,
    gcvFEATURE_MC20,
    gcvFEATURE_SUPER_TILED,
    gcvFEATURE_FAST_CLEAR_FLUSH,
    gcvFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND,
    gcvFEATURE_2D_DITHER,
    gcvFEATURE_2D_A8_TARGET,
    gcvFEATURE_2D_A8_NO_ALPHA,
    gcvFEATURE_2D_FILTERBLIT_FULLROTATION,
    gcvFEATURE_2D_BITBLIT_FULLROTATION,
    gcvFEATURE_WIDE_LINE,
    gcvFEATURE_FC_FLUSH_STALL,
    gcvFEATURE_FULL_DIRECTFB,
    gcvFEATURE_HALF_FLOAT_PIPE,
    gcvFEATURE_LINE_LOOP,
    gcvFEATURE_2D_YUV_BLIT,
    gcvFEATURE_2D_TILING,
    gcvFEATURE_NON_POWER_OF_TWO,
    gcvFEATURE_3D_TEXTURE,
    gcvFEATURE_TEXTURE_ARRAY,
    gcvFEATURE_TILE_FILLER,
    gcvFEATURE_LOGIC_OP,
    gcvFEATURE_COMPOSITION,
    gcvFEATURE_MIXED_STREAMS,
    gcvFEATURE_2D_MULTI_SOURCE_BLT,
    gcvFEATURE_END_EVENT,
    gcvFEATURE_VERTEX_10_10_10_2,
    gcvFEATURE_TEXTURE_10_10_10_2,
    gcvFEATURE_TEXTURE_ANISOTROPIC_FILTERING,
    gcvFEATURE_TEXTURE_FLOAT_HALF_FLOAT,
    gcvFEATURE_2D_ROTATION_STALL_FIX,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX,
    gcvFEATURE_BUG_FIXES10,
    gcvFEATURE_2D_MINOR_TILING,
    /* Supertiled compressed textures are supported. */
    gcvFEATURE_TEX_COMPRRESSION_SUPERTILED,
    gcvFEATURE_FAST_MSAA,
    gcvFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP,
    gcvFEATURE_INDEX_FETCH_FIX,
    gcvFEATURE_TEXTURE_TILE_STATUS_READ,
    gcvFEATURE_DEPTH_BIAS_FIX,
    gcvFEATURE_RECT_PRIMITIVE,
    gcvFEATURE_BUG_FIXES11,
    gcvFEATURE_SUPERTILED_TEXTURE,
    gcvFEATURE_2D_NO_COLORBRUSH_INDEX8,
    gcvFEATURE_RS_YUV_TARGET,
    gcvFEATURE_2D_FC_SOURCE,
    gcvFEATURE_2D_CC_NOAA_SOURCE,
    gcvFEATURE_PE_DITHER_FIX,
    gcvFEATURE_2D_YUV_SEPARATE_STRIDE,
    gcvFEATURE_FRUSTUM_CLIP_FIX,
    gcvFEATURE_TEXTURE_SWIZZLE,
    gcvFEATURE_PRIMITIVE_RESTART,
    gcvFEATURE_TEXTURE_LINEAR,
    gcvFEATURE_TEXTURE_YUV_ASSEMBLER,
    gcvFEATURE_LINEAR_RENDER_TARGET,
    gcvFEATURE_SHADER_HAS_ATOMIC,
    gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE,
    gcvFEATURE_SHADER_ENHANCEMENTS2,
    gcvFEATURE_BUG_FIXES7,
    gcvFEATURE_SHADER_HAS_RTNE,
    gcvFEATURE_SHADER_HAS_EXTRA_INSTRUCTIONS2,
    gcvFEATURE_SHADER_ENHANCEMENTS3,
    gcvFEATURE_DYNAMIC_FREQUENCY_SCALING,
    gcvFEATURE_SINGLE_BUFFER,
    gcvFEATURE_OCCLUSION_QUERY,
    gcvFEATURE_2D_GAMMA,
    gcvFEATURE_2D_COLOR_SPACE_CONVERSION,
    gcvFEATURE_2D_SUPER_TILE_VERSION,
    gcvFEATURE_HALTI0,
    gcvFEATURE_HALTI1,
    gcvFEATURE_HALTI2,
    gcvFEATURE_2D_MIRROR_EXTENSION,
    gcvFEATURE_TEXTURE_ASTC,
    gcvFEATURE_TEXTURE_ASTC_FIX,
    gcvFEATURE_2D_SUPER_TILE_V1,
    gcvFEATURE_2D_SUPER_TILE_V2,
    gcvFEATURE_2D_SUPER_TILE_V3,
    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2,
    gcvFEATURE_NEW_RA,
    gcvFEATURE_BUG_FIXED_IMPLICIT_PRIMITIVE_RESTART,
    gcvFEATURE_PE_MULTI_RT_BLEND_ENABLE_CONTROL,
    gcvFEATURE_SMALL_MSAA, /* An upgraded version of Fast MSAA */
    gcvFEATURE_VERTEX_INST_ID_AS_ATTRIBUTE,
    gcvFEATURE_DUAL_16,
    gcvFEATURE_BRANCH_ON_IMMEDIATE_REG,
    gcvFEATURE_2D_COMPRESSION,
    gcvFEATURE_TPC_COMPRESSION,
    gcvFEATURE_DEC_COMPRESSION,
    gcvFEATURE_DEC_TPC_COMPRESSION,
    gcvFEATURE_DEC_COMPRESSION_TILE_NV12_8BIT,
    gcvFEATURE_DEC_COMPRESSION_TILE_NV12_10BIT,
    gcvFEATURE_2D_OPF_YUV_OUTPUT,
    gcvFEATURE_2D_FILTERBLIT_A8_ALPHA,
    gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT,
    gcvFEATURE_V2_COMPRESSION_Z16_FIX,

    gcvFEATURE_VERTEX_INST_ID_AS_INTEGER,
    gcvFEATURE_2D_YUV_MODE,
    gcvFEATURE_ACE,
    gcvFEATURE_COLOR_COMPRESSION,

    gcvFEATURE_32BPP_COMPONENT_TEXTURE_CHANNEL_SWIZZLE,
    gcvFEATURE_64BPP_HW_CLEAR_SUPPORT,
    gcvFEATURE_TX_LERP_PRECISION_FIX,
    gcvFEATURE_COMPRESSION_V2,
    gcvFEATURE_MMU,
    gcvFEATURE_COMPRESSION_V3,
    gcvFEATURE_TX_DECOMPRESSOR,
    gcvFEATURE_MRT_TILE_STATUS_BUFFER,
    gcvFEATURE_COMPRESSION_V1,
    gcvFEATURE_V1_COMPRESSION_Z16_DECOMPRESS_FIX,
    gcvFEATURE_RTT,
    gcvFEATURE_GENERICS,
    gcvFEATURE_2D_ONE_PASS_FILTER,
    gcvFEATURE_2D_ONE_PASS_FILTER_TAP,
    gcvFEATURE_2D_POST_FLIP,
    gcvFEATURE_2D_PIXEL_ALIGNMENT,
    gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT,
    gcvFEATURE_CORRECT_AUTO_DISABLE_COUNT_WIDTH,

    gcvFEATURE_HALTI3,
    gcvFEATURE_EEZ,
    gcvFEATURE_INTEGER_SIGNEXT_FIX,
    gcvFEATURE_INTEGER_PIPE_FIX,
    gcvFEATURE_PSOUTPUT_MAPPING,
    gcvFEATURE_8K_RT_FIX,
    gcvFEATURE_TX_TILE_STATUS_MAPPING,
    gcvFEATURE_SRGB_RT_SUPPORT,
    gcvFEATURE_UNIFORM_APERTURE,
    gcvFEATURE_TEXTURE_16K,
    gcvFEATURE_PA_FARZCLIPPING_FIX,
    gcvFEATURE_PE_DITHER_COLORMASK_FIX,
    gcvFEATURE_ZSCALE_FIX,

    gcvFEATURE_MULTI_PIXELPIPES,
    gcvFEATURE_PIPE_CL,

    gcvFEATURE_BUG_FIXES18,

    gcvFEATURE_UNIFIED_SAMPLERS,
    gcvFEATURE_CL_PS_WALKER,
    gcvFEATURE_NEW_HZ,

    gcvFEATURE_TX_FRAC_PRECISION_6BIT,
    gcvFEATURE_SH_INSTRUCTION_PREFETCH,
    gcvFEATURE_PROBE,

    gcvFEATURE_BUG_FIXES8,
    gcvFEATURE_2D_ALL_QUAD,

    gcvFEATURE_SINGLE_PIPE_HALTI1,

    gcvFEATURE_BLOCK_SIZE_16x16,

    gcvFEATURE_NO_USER_CSC,
    gcvFEATURE_ANDROID_ONLY,
    gcvFEATURE_HAS_PRODUCTID,

    gcvFEATURE_V2_MSAA_COMP_FIX,

    gcvFEATURE_S8_ONLY_RENDERING,

    gcvFEATURE_SEPARATE_SRC_DST,

    gcvFEATURE_FE_START_VERTEX_SUPPORT,
    gcvFEATURE_FE_RESET_VERTEX_ID,
    gcvFEATURE_RS_DEPTHSTENCIL_NATIVE_SUPPORT,

    gcvFEATURE_HALTI4,
    gcvFEATURE_MSAA_FRAGMENT_OPERATION,
    gcvFEATURE_ZERO_ATTRIB_SUPPORT,
    gcvFEATURE_TEX_CACHE_FLUSH_FIX,
};

/* ROP: Raster operation pipelines ? */
gctUINT8 sRopList[] = {
		0xF0, // brush
		0xCC, // src
		0xAA, // dst
 		0x0F, // brush converse
		0x33, // src converse
		0x55, // dst converse
		0xFF, // white
		0x00, // black

		0xA0, // brush AND dst
		0xFA, // brush OR dst
		0x5A, // brush XOR dst
		0xC0, // brush AND src
		0xFC, // brush OR src
		0x3C, // brush XOR src
		0x88, // dst AND src
		0xEE, // dst OR src
		0x66, // dst XOR src

		0xFE, // brush  OR src  OR dst
		0xF8, // brush  OR src AND dst
		0x56, // brush  OR src XOR dst
		0xEA, // brush AND src  OR dst
		0x80, // brush AND src AND dst
		0x6A, // brush AND src XOR dst
		0xBE, // brush XOR src  OR dst
		0x28, // brush XOR src AND dst
		0x96, // brush XOR src XOR dst

		//0xFE, // brush  OR dst  OR src
		0xC8, // brush  OR dst AND src
		0x36, // brush  OR dst XOR src
		0xEC, // brush AND dst  OR src
		//0x80, // brush AND dst AND src
		0x0C, // brush AND dst XOR src
		0xDE, // brush XOR dst  OR src
		0x48, // brush XOR dst AND src
		//0x96, // brush XOR dst XOR src

		//0xFE, // src  OR dst  OR brush
		0xE0, // src  OR dst AND brush
		0x1E, // src  OR dst XOR brush
		//0xF8, // src AND dst  OR brush
		//0x80, // src AND dst AND brush
		0x78, // src AND dst XOR brush
		0xF6, // src XOR dst  OR brush
		0x60, // src XOR dst AND brush
		//0x96, // src XOR dst XOR brush
};

static gceSURF_ROTATION rotationList [] =
{
	gcvSURF_0_DEGREE,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,
};

const char* GalStatusString(gceSTATUS status)
{
    int i;

    for (i = 0; i < sizeof(sStatusMap)/sizeof(sStatusMap[0]); i++)
    {
        if (sStatusMap[i].status == status)
            return sStatusMap[i].string;
    }

    return "Unknown Status";
}

static gceSTATUS Gal2DCleanSurface(gcoHAL hal, gcoSURF surface, gctUINT32 color)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER bits[3] = {0, 0, 0};

    do {
        gctUINT width, height;
        gctINT stride;
        gcsRECT dstRect;
        gceSURF_TYPE type;
        gceSURF_FORMAT fmt;
        gctUINT32 addr[3];
        gco2D egn2D = NULL;

        gcmERR_BREAK(gcoHAL_Get2DEngine(hal, &egn2D));

        gcmERR_BREAK(gcoSURF_GetFormat(surface, &type, &fmt));
        if (type != gcvSURF_BITMAP){
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }

        gcmERR_BREAK(gcoSURF_GetAlignedSize(surface, &width, &height, &stride));

        gcmERR_BREAK(gcoSURF_Lock(surface, addr, bits));

        dstRect.left = 0;
        dstRect.top = 0;
        dstRect.right = width;
        dstRect.bottom = height;

        gcmERR_BREAK(gco2D_SetClipping(egn2D, &dstRect));

        gcmERR_BREAK(gco2D_SetTarget(egn2D, addr[0], stride, 0, width));

        gcmERR_BREAK(gco2D_Clear(egn2D, 1, &dstRect, color, 0xCC, 0xCC, fmt));

        gcmERR_BREAK(gcoHAL_Commit(hal, gcvTRUE));

        gcmERR_BREAK(gcoSURF_Unlock(surface, bits[0]));
        bits[0] = gcvNULL;

    } while (gcvFALSE);

    if (bits[0]){
        gcmVERIFY_OK(gcoSURF_Unlock(surface,bits));
    }

    return status;
}

gctBOOL Gal2DRectangle(gcoHAL hal, gcoSURF surface, gcoBRUSH brush, gcsRECT rect)
{
    gctUINT width, height;
    gctINT stride;
    gcsRECT dstRect;
    gceSURF_TYPE type;
    gceSURF_FORMAT fmt;
    gctPOINTER bits;
    gctUINT32 addr;
    gco2D egn2D = NULL;

    gcmVERIFY_OK(gcoSURF_GetFormat(surface, &type, &fmt));
    if (type != gcvSURF_BITMAP)
        return gcvFALSE;

    gcmVERIFY_OK(gcoSURF_GetAlignedSize(surface, &width, &height, &stride));

    dstRect.left = MAX(0, rect.left);
    dstRect.top = MAX(0, rect.top);
    dstRect.right = MIN(width, (gctUINT32)rect.right);
    dstRect.bottom = MIN(height, (gctUINT32)rect.bottom);

    if (dstRect.left >= dstRect.right || dstRect.top >= dstRect.bottom){
        // invalid rectangle
        return gcvFALSE;
    }

    gcmVERIFY_OK(gcoSURF_Lock(surface, &addr, &bits));

    gcmVERIFY_OK(gcoHAL_Get2DEngine(hal, &egn2D));
    gcmASSERT(egn2D);

    gcmVERIFY_OK(gco2D_FlushBrush(egn2D, brush, fmt));

    gcmVERIFY_OK(gco2D_SetTarget(egn2D, addr, stride, 0, width));

    gcmVERIFY_OK(gco2D_SetClipping(egn2D, &dstRect));

    gcmVERIFY_OK(gco2D_Blit(egn2D, 1, &dstRect, 0xF0, 0xF0, fmt));

    gcmVERIFY_OK(gcoHAL_Commit(hal, gcvTRUE));

    gcmVERIFY_OK(gcoSURF_Unlock(surface,bits));

    return gcvTRUE;
}

static gctBOOL GalSetAlphaBlend()
{
	#if 0
	if(gcoHAL_IsFeatureAvailable(s_Runtime.hal, gcvFEATURE_2DPE20)){
		//GC320_CHECK_RETURN(gco2D_SetSourceGlobalColorAdvanced(s_Runtime.engine2d, srcAlpha << 24), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetTargetGlobalColorAdvanced(s_Runtime.engine2d, 0xFF << 24), gcvFALSE);

		/* gcvSURF_PIXEL_ALPHA_INVERSED: 0x00 为全透
		   gcvSURF_PIXEL_ALPHA_STRAIGHT: 0xFF为不透  
		*/
		GC320_CHECK_RETURN(gco2D_EnableAlphaBlendAdvanced(s_Runtime.engine2d,
					gcvSURF_PIXEL_ALPHA_INVERSED, gcvSURF_PIXEL_ALPHA_INVERSED,
					gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_ON,
					gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT), gcvFALSE);
	}
	else{
		GC320_CHECK_RETURN(gco2D_EnableAlphaBlend(s_Runtime.engine2d,
					0xFF, 0xFF,
					gcvSURF_PIXEL_ALPHA_INVERSED, gcvSURF_PIXEL_ALPHA_INVERSED,
					gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_ON,
					gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
					gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT), gcvFALSE);
	}
	#else
	GC320_CHECK_RETURN(gco2D_SetPorterDuffBlending(s_Runtime.engine2d, gcvPD_SRC_OVER), gcvFALSE);
	#endif
	return gcvTRUE;
}

gctINT Gc320ChipCheck(stGalRuntime *rt)
{
    gceSTATUS status;
    gctUINT32 i;
    gctINT ret = 0;

    status = gcoHAL_QueryChipIdentity(rt->hal, &rt->ChipModel, &rt->ChipRevision, &rt->ChipFeatures, gcvNULL);
    if (status < 0){
        GC320_LOG_ERR("*ERROR* Failed to query chip info (status = 0x%x)", status);
        ret = -1;
        goto EXIT;
    }

    status = gcoOS_ReadRegister(rt->os, 0x98, &rt->PatchRevision);
    if (status < 0){
        GC320_LOG_ERR("*ERROR* Failed to read patch version (status = 0x%x)", status);
        ret = -2;
        goto EXIT;
    }

    GC320_LOG_NOTIFY( "=================== Chip Information ==================");
    GC320_LOG_NOTIFY( "Chip : GC%x", rt->ChipModel);
    GC320_LOG_NOTIFY( "Chip revision: 0x%08x", rt->ChipRevision);
    GC320_LOG_NOTIFY( "Patch revision: 0x%08x", rt->PatchRevision);
    GC320_LOG_NOTIFY( "Chip Features: 0x%08x", rt->ChipFeatures);

    status = gcoHAL_QueryChipMinorFeatures(rt->hal, &rt->ChipMinorFeaturesNumber, gcvNULL);
    if (status < 0){
        GC320_LOG_ERR("*ERROR* Failed to query minor feature count (status = 0x%x)", status);
        ret = -2;
        goto EXIT;
    }

    if (MAX_MINORFEATURE_NUMBER < rt->ChipMinorFeaturesNumber){
        GC320_LOG_ERR("*ERROR* no enough space for minor features");
        ret = -3;
        goto EXIT;
    }

    status = gcoHAL_QueryChipMinorFeatures(rt->hal, &rt->ChipMinorFeaturesNumber, rt->ChipMinorFeatures);
    if (status < 0){
        GC320_LOG_ERR( "*ERROR* Failed to query minor features (status = 0x%x)", status);
        ret = -4;
        goto EXIT;
    }

    for (i = 0; i < rt->ChipMinorFeaturesNumber; i++){
        GC320_LOG_NOTIFY("Chip MinorFeatures%d: 0x%08x", i, rt->ChipMinorFeatures[i]);
    }

    GC320_LOG_NOTIFY("=======================================================");

EXIT:

    return ret;
}


gctBOOL Gc320Initialize()
{
	gceSTATUS status;
	memset(&s_Runtime, 0, sizeof(stGalRuntime));
	/* target surface. */
    s_Runtime.width    	 	   = 1920;
    s_Runtime.height    	   = 720;
    s_Runtime.format    	   = gcvSURF_A8R8G8B8;
	#if GC320_POOL_USER
	s_Runtime.pool		   	   = gcvPOOL_USER;
	#else
    s_Runtime.pool      	   = gcvPOOL_DEFAULT;
	#endif
    s_Runtime.saveTarget       = gcvTRUE;
    s_Runtime.noSaveTargetNew  = gcvFALSE;
    s_Runtime.cleanTarget 	   = gcvTRUE;
    s_Runtime.createTarget 	   = gcvTRUE;
    s_Runtime.notSupport 	   = gcvFALSE;
    s_Runtime.startFrame 	   = 0;
    s_Runtime.endFrame 		   = 0;

	/* Construct the gcoOS object. */
    status = gcoOS_Construct(gcvNULL, &s_Runtime.os);
    if (status < 0){
        GC320_LOG_ERR("*ERROR* Failed to construct OS object (status = %d)", status);
        return gcvFALSE;
    }

	/* Construct the gcoHAL object. */
    status = gcoHAL_Construct(gcvNULL, s_Runtime.os, &s_Runtime.hal);
    if (status < 0){
        GC320_LOG_ERR("*ERROR* Failed to construct GAL object (status = %d)", status);
        return gcvFALSE;
    }

#if defined(LINUX) || defined(ANDROID)
	/* Query the amount of video memory. */
    status = gcoHAL_QueryVideoMemory(s_Runtime.hal,
                                 &g_InternalPhysical, &g_InternalSize,
                                 &g_ExternalPhysical, &g_ExternalSize,
                                 &g_ContiguousPhysical, &g_ContiguousSize);
    if (gcmIS_ERROR(status)){
        GC320_LOG_ERR("gcoHAL_QueryVideoMemory failed %d.", status);
        return gcvFALSE;
    }

    /* Map the local internal memory. */
    if (g_InternalSize > 0){
        status = gcoHAL_MapMemory(s_Runtime.hal,
                              g_InternalPhysical, g_InternalSize,
                              &g_Internal);
        if (gcmIS_ERROR(status)){
            GC320_LOG_ERR("gcoHAL_MapMemory failed %d.", status);
            return gcvFALSE;
        }
    }
	GC320_LOG_WARNING("g_InternalSize = %d", g_InternalSize);

    /* Map the local external memory. */
    if (g_ExternalSize > 0){
        status = gcoHAL_MapMemory(s_Runtime.hal,
                              g_ExternalPhysical, g_ExternalSize,
                              &g_External);
        if (gcmIS_ERROR(status)){
            GC320_LOG_ERR("gcoHAL_MapMemory failed %d.", status);
            return gcvFALSE;
        }
    }
	GC320_LOG_WARNING("g_ExternalSize = %d", g_ExternalSize);

    /* Map the contiguous memory. */
    if (g_ContiguousSize > 0){
        status = gcoHAL_MapMemory(s_Runtime.hal,
                              g_ContiguousPhysical, g_ContiguousSize,
                              &g_Contiguous);
        if (gcmIS_ERROR(status)){
            GC320_LOG_ERR("gcoHAL_MapMemory failed %d.", status);
            return gcvFALSE;
        }
    }
	GC320_LOG_WARNING("g_ContiguousSize = %d", g_ContiguousSize);
#endif

	status = gcoHAL_Get2DEngine(s_Runtime.hal, &s_Runtime.engine2d);
    if (status < 0){
        GC320_LOG_ERR("*ERROR* Failed to get 2D engine object (status = %d)", status);
        return gcvFALSE;
    }

	s_Runtime.pe20        = gcoHAL_IsFeatureAvailable(s_Runtime.hal, gcvFEATURE_2DPE20);
#if gcvVERSION_MAJOR >= 4
    s_Runtime.fullDFB     = gcoHAL_IsFeatureAvailable(s_Runtime.hal, gcvFEATURE_FULL_DIRECTFB);
#else
    s_Runtime.fullDFB     = gcvFALSE;
#endif
	return gcvTRUE;
}

gctBOOL Gc320CreateDstSufface()
{
	stTest2D *t2d = &s_Test2D;
	/* create dest surface */
	GC320_CHECK_RETURN(gcoSURF_Construct(s_Runtime.hal,
                          s_Runtime.width,
                          s_Runtime.height,
                          1,
                          gcvSURF_BITMAP,
                          s_Runtime.format,
                          s_Runtime.pool,
                          &s_Runtime.target), gcvFALSE);

	t2d->dstSurf = s_Runtime.target;
	t2d->dstFormat = s_Runtime.format;
	t2d->dstWidth = 0;
	t2d->dstHeight = 0;
	t2d->dstStride = 0;
	t2d->dstPhyAddr = 0;
	t2d->dstLgcAddr = 0;

	/* get dest surface aligned size */
	GC320_CHECK_RETURN(gcoSURF_GetAlignedSize(t2d->dstSurf,
									&t2d->dstWidth,
									&t2d->dstHeight,
									&t2d->dstStride), gcvFALSE);
	GC320_LOG_WARNING("Dest surface aligned width: %d, hight: %d, stride: %d",
		t2d->dstWidth, t2d->dstHeight, t2d->dstStride);

	#if !GC320_POOL_USER
	/* clear source surface with green */
	GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, t2d->dstSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)), gcvFALSE);
	/* lock dest surface */
	GC320_CHECK_RETURN(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr), gcvFALSE);
	GC320_LOG_WARNING("Dest surface physic address: 0x%x, logic address: 0x%x", t2d->dstPhyAddr, t2d->dstLgcAddr);

	//GC320_CHECK_RETURN(gcoSURF_AllocShBuffer(t2d->dstSurf, &t2d->dstShareBuff), gcvFALSE);
	//GC320_LOG_WARNING("Dest surface Share buffer: %p", t2d->dstShareBuff);
	#endif

	return gcvTRUE;
}

gctBOOL Gc320SetTargetSurfaceBuffer(gctPOINTER lgcAddr)
{
	stTest2D *t2d = &s_Test2D;
	#if 1
	GC320_CHECK_RETURN(gcoSURF_SetBuffer(t2d->dstSurf,
						                    gcvSURF_BITMAP,
						                    t2d->dstFormat,
						                    t2d->dstStride,
						                    lgcAddr,
						                    ~0U), gcvFALSE);
											
	GC320_CHECK_RETURN(gcoSURF_SetWindow(t2d->dstSurf, 0, 0, t2d->dstWidth, t2d->dstHeight), gcvFALSE);
	GC320_CHECK_RETURN(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr), gcvFALSE);
	GC320_LOG_WARNING("====Dest surface physic address: 0x%x, logic address: 0x%x, setLogicAddress: 0x%x",
					t2d->dstPhyAddr, t2d->dstLgcAddr, lgcAddr);

	#else
	GC320_LOG_WARNING("Set target surface lgcAddr = 0x%x", lgcAddr);
	GC320_CHECK_RETURN(gcoSURF_MapUserSurface(t2d->dstSurf, 0, lgcAddr, ~0U), gcvFALSE);
	GC320_CHECK_RETURN(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr), gcvFALSE);
	GC320_LOG_WARNING("====Dest surface physic address: 0x%x, logic address: 0x%x, setLogicAddress: 0x%x",
					t2d->dstPhyAddr, t2d->dstLgcAddr, lgcAddr);
	#endif
	return gcvTRUE;
}

gctBOOL Gc320ClearSurfaceRegions(
	gctUINT8 surfIdx,
	gcsRECT_PTR fillRects,
	gctUINT8 rectNum,
	gctUINT32 color)
{
	if(surfIdx < 0 || surfIdx >= SRC_SURF_NUM){
		return gcvFALSE;
	}
	gctUINT8 i = 0;
	for(i = 0; i < rectNum; i++){
		GC320_CHECK_RETURN(gco2D_SetTarget(s_Runtime.engine2d, s_Test2D.srcSurfs[surfIdx].srcPhyAddrs[0], s_Test2D.srcSurfs[surfIdx].srcStrides[0], gcvSURF_0_DEGREE, s_Test2D.srcSurfs[surfIdx].srcWidth), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetSource(s_Runtime.engine2d, &fillRects[i]), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetClipping(s_Runtime.engine2d, &fillRects[i]), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_Clear(s_Runtime.engine2d, 1, &fillRects[i], color, 0xCC, 0xCC, s_Test2D.srcSurfs[surfIdx].srcFormat), gcvFALSE);
		GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);
	}
	s_Test2D.srcSurfs[surfIdx].srcAlphaBlend = gcvFALSE;
	return gcvTRUE;
}

gctBOOL Gc320HollowSurfaceRegions(
	gctUINT8 surfIdx,
	gcsRECT_PTR fillRects,
	gctUINT8 rectNum)
{
	if(!Gc320ClearSurfaceRegions(surfIdx, fillRects, rectNum, FULL_TRANSPARENT_COLOR)){
		return gcvFALSE;
	}
	s_Test2D.srcSurfs[surfIdx].srcAlphaBlend = gcvTRUE;
	return gcvTRUE;
}

static void GalPrepareSurfaceForBmp(const Int8* bmpName, gctINT surIdx)
{
	stBmpInfo bmpInfo = {0};
	uInt8 *pData = ImageLoadBmp(bmpName, &bmpInfo);
	gctUINT32 width = COMM_ALIGN(abs(bmpInfo.bmiHeader.biWidth), 4);
	gctUINT32 height = abs(bmpInfo.bmiHeader.biHeight);
	gctUINT32 alignedWidth, alignedHeight;
	gctINT32 alignedStride;
	gceSURF_FORMAT format;
	gctUINT surfDepth, biDepth;
	GC320_LOG_WARNING("[BMP: %s] biCompression = %d, biBitCount = %d, width = %d, hight = %d", bmpName,
		bmpInfo.bmiHeader.biCompression,
		bmpInfo.bmiHeader.biBitCount,
		width,
		height);
	
	if(bmpInfo.bmiHeader.biCompression == 0){
		switch(bmpInfo.bmiHeader.biBitCount){
			case 32:
				format = gcvSURF_A8R8G8B8;
				surfDepth = 4;
				biDepth = 4;
				break;
			case 24:
				format = gcvSURF_A8R8G8B8;
				surfDepth = 4;
				biDepth = 3;
				break;
			case 16:
				format = gcvSURF_R5G6B5;
            	surfDepth = 2;
            	biDepth = 2;
				break;
			case 8:
				format = gcvSURF_INDEX8;
				surfDepth = 1;
				biDepth = 1;
				if(gcmIS_ERROR(gco2D_LoadPalette(s_Runtime.engine2d, 0, 256, bmpInfo.bmiColors, gcvTRUE))){
					GC320_LOG_ERR("%s call gco2D_LoadPalette() Fail!!!", __FUNCTION__);
					free(pData);
					pData = NULL;
					return;
				}
				break;
			default:
				break;
		}
	}
	else if(bmpInfo.bmiHeader.biCompression == 3){
		GC320_LOG_WARNING("mask[0] = 0x%x, mask[1] = 0x%x, mask[2] = 0x%x",
			bmpInfo.mask[0], bmpInfo.mask[1], bmpInfo.mask[2]);
		switch (bmpInfo.bmiHeader.biBitCount){
        case 32:
            if (bmpInfo.mask[0] == 0x00FF0000
                && bmpInfo.mask[1] == 0x0000FF00
                && bmpInfo.mask[2] == 0x000000FF){
                format = gcvSURF_A8R8G8B8;
                surfDepth = 4;
                biDepth = 4;
            }
            else if (bmpInfo.mask[0] == 0xFF000000
                && bmpInfo.mask[1] == 0x00FF0000
                && bmpInfo.mask[2] == 0x0000FF00){
                format = gcvSURF_R8G8B8A8;
                surfDepth = 4;
                biDepth = 4;
            }
            else if (bmpInfo.mask[0] == 0x000000FF
                && bmpInfo.mask[1] == 0x0000FF00
                && bmpInfo.mask[2] == 0x00FF0000){
                format = gcvSURF_A8B8G8R8;
                surfDepth = 4;
                biDepth = 4;
            }
            else if (bmpInfo.mask[0] == 0x0000FF00
                && bmpInfo.mask[1] == 0x00FF0000
                && bmpInfo.mask[2] == 0xFF000000){
                format = gcvSURF_B8G8R8A8;
                surfDepth = 4;
                biDepth = 4;
            }
            else{
                GC320_LOG_ERR("*ERROR* unsupported format");
                free(pData);
				pData = NULL;
				return;
            }

            break;

        case 16:
            if (bmpInfo.mask[0] == 0x00000F00
                && bmpInfo.mask[1] == 0x000000F0
                && bmpInfo.mask[2] == 0x0000000F){
                format = gcvSURF_A4R4G4B4;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000F000
                && bmpInfo.mask[1] == 0x00000F00
                && bmpInfo.mask[2] == 0x000000F0){
                format = gcvSURF_R4G4B4A4;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000000F
                && bmpInfo.mask[1] == 0x000000F0
                && bmpInfo.mask[2] == 0x00000F00){
                format = gcvSURF_A4B4G4R4;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x000000F0
                && bmpInfo.mask[1] == 0x00000F00
                && bmpInfo.mask[2] == 0x0000F000){
                format = gcvSURF_B4G4R4A4;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000F800
                && bmpInfo.mask[1] == 0x000007E0
                && bmpInfo.mask[2] == 0x0000001F){
                format = gcvSURF_R5G6B5;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000001F
                && bmpInfo.mask[1] == 0x000007E0
                && bmpInfo.mask[2] == 0x0000F800){
                format = gcvSURF_B5G6R5;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x00007C00
                && bmpInfo.mask[1] == 0x000003E0
                && bmpInfo.mask[2] == 0x0000001F){
                format = gcvSURF_A1R5G5B5;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000F800
                && bmpInfo.mask[1] == 0x000007C0
                && bmpInfo.mask[2] == 0x0000003E){
                format = gcvSURF_R5G5B5A1;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000003E
                && bmpInfo.mask[1] == 0x000007C0
                && bmpInfo.mask[2] == 0x0000F800){
                format = gcvSURF_B5G5R5A1;
                surfDepth = 2;
                biDepth = 2;
            }
            else if (bmpInfo.mask[0] == 0x0000001F
                && bmpInfo.mask[1] == 0x000003E0
                && bmpInfo.mask[2] == 0x00007C00){
                format = gcvSURF_A1B5G5R5;
                surfDepth = 2;
                biDepth = 2;
            }
            else{
                GC320_LOG_ERR("*ERROR* unsupported format");
                free(pData);
				pData = NULL;
				return;
            }

            break;

        default:
            GC320_LOG_ERR("*ERROR* unsupported bpp:%d",
                bmpInfo.bmiHeader.biBitCount);
            free(pData);
			pData = NULL;
			return;
        }
	}
	else{
		GC320_LOG_ERR("*ERROR* unsupported compression:%d",
                bmpInfo.bmiHeader.biCompression);
        free(pData);
		pData = NULL;
		return;
	}

	if(gcmIS_ERROR(gcoSURF_Construct(s_Runtime.hal, 
									width,
									height,
									1,
									gcvSURF_BITMAP,
									format,
									gcvPOOL_DEFAULT,
									&s_Test2D.srcSurfs[surIdx].srcSurf))){
		GC320_LOG_ERR("%s call gcoSURF_Construct() Fail!!!", __FUNCTION__);
		free(pData);
		pData = NULL;
		return;
	}
	if(gcmIS_ERROR(gcoSURF_GetAlignedSize(s_Test2D.srcSurfs[surIdx].srcSurf, &alignedWidth, &alignedHeight, &alignedStride))){
		GC320_LOG_ERR("%s call gcoSURF_GetAlignedSize() Fail!!!", __FUNCTION__);
		free(pData);
		pData = NULL;
		return;
	}

	if(gcmIS_ERROR(gcoSURF_Lock(s_Test2D.srcSurfs[surIdx].srcSurf,
								s_Test2D.srcSurfs[surIdx].srcPhyAddrs,
								s_Test2D.srcSurfs[surIdx].srcLgcAddrs))){
		GC320_LOG_ERR("%s call gcoSURF_Lock() Fail!!!", __FUNCTION__);
		free(pData);
		pData = NULL;
		return;
	}

	uInt8 *pAddr = (uInt8*)s_Test2D.srcSurfs[surIdx].srcLgcAddrs[0];
	uInt32 h, w;

	/* 将bmp数据的alpha值设置为0xff, 不透明 */
	if(strncmp("./argb8_800x600.bmp", bmpName, 19) == 0){
		uInt32 *pT = pData;
		for(h = 0; h < height; h++){
			for(w = 0; w < width; w++){
				*(pT + h * width + w) |= 0xFF000000;
			}
		}
	}

	#if 0
	static uInt32 cnt = 0;
	Int8 file[128] = {'\0'};
	sprintf(file, "./bmp_%03d.raw", cnt++);
	FILE* fp = fopen(file, "wb+");
	fwrite(pData, width * height * 4, 1, fp);
	#endif
	
	for(h = 0; h < height; h++){
		for(w = 0; w < width; w++){
			gctUINT dstOffset = h * alignedWidth + w;
			gctUINT srcOffset = bmpInfo.bmiHeader.biHeight >= 0 ? (height - 1 - h) * width + w : h * width + w;
			if(isBigEndian() && (biDepth == 3)){
				memcpy(pAddr + dstOffset * surfDepth + 1, pData + srcOffset * biDepth, biDepth);
			}
			else{
				memcpy(pAddr + dstOffset * surfDepth, pData + srcOffset * biDepth, biDepth);
			}
		}
	}

	free(pData);
	pData = NULL;

	gcoSURF_GetFormat(s_Test2D.srcSurfs[surIdx].srcSurf, gcvNULL, &s_Test2D.srcSurfs[surIdx].srcFormat);
	s_Test2D.srcSurfs[surIdx].srcWidth = alignedWidth;
	s_Test2D.srcSurfs[surIdx].srcHeight = alignedHeight;
	s_Test2D.srcSurfs[surIdx].srcStrides[0] = alignedStride;
	s_Test2D.srcSurfs[surIdx].srcStrideNum = 1;
	s_Test2D.srcSurfs[surIdx].srcAddrNum = 1;
	return;
}

gctBOOL GalDumpDataToBmp(
	void *pData,
	const char* bmpFileName, 
	gceSURF_FORMAT format,
	gctUINT width,
	gctUINT height)
{
    stBmpInfo bitmap;
    gctINT bitCount, i;
    stRGB tmp;
    gctBOOL ret = gcvFALSE;

    if (bmpFileName && bmpFileName[0]){
        switch (format)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            bitmap.mask[0] = 0x00FF0000;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x000000FF;
            bitCount = 32;
            break;

        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
            bitmap.mask[0] = 0xFF000000;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0x0000FF00;
            bitCount = 32;
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            bitmap.mask[0] = 0x000000FF;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x00FF0000;
            bitCount = 32;
            break;

        case gcvSURF_B8G8R8A8:
        case gcvSURF_B8G8R8X8:
            bitmap.mask[0] = 0x0000FF00;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0xFF000000;
            bitCount = 32;
            break;

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
            bitmap.mask[0] = 0x00000F00;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x0000000F;
            bitCount = 16;
            break;

        case gcvSURF_R4G4B4X4:
        case gcvSURF_R4G4B4A4:
            bitmap.mask[0] = 0x0000F000;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x000000F0;
            bitCount = 16;
            break;

        case gcvSURF_X4B4G4R4:
        case gcvSURF_A4B4G4R4:
            bitmap.mask[0] = 0x0000000F;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x00000F00;
            bitCount = 16;
            break;

        case gcvSURF_B4G4R4X4:
        case gcvSURF_B4G4R4A4:
            bitmap.mask[0] = 0x000000F0;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x0000F000;
            bitCount = 16;
            break;

        case gcvSURF_R5G6B5:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_B5G6R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            bitmap.mask[0] = 0x00007C00;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_R5G5B5X1:
        case gcvSURF_R5G5B5A1:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000003E;
            bitCount = 16;
            break;

        case gcvSURF_B5G5R5X1:
        case gcvSURF_B5G5R5A1:
            bitmap.mask[0] = 0x0000003E;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_X1B5G5R5:
        case gcvSURF_A1B5G5R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x00007C00;
            bitCount = 16;
            break;

        case gcvSURF_A8:
            bitCount = 8;
            break;

        default:
            // can not save and display
            return gcvFALSE;
        }

        /* Fill in the BITMAPINFOHEADER information. */
        bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
        bitmap.bmiHeader.biWidth = width;
        bitmap.bmiHeader.biHeight = -(gctINT)height;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = bitCount;
        if (format == gcvSURF_A8)
            bitmap.bmiHeader.biCompression = BIT_RGB;
        else
            bitmap.bmiHeader.biCompression = BIT_BITFIELDS;
        bitmap.bmiHeader.biSizeImage = 0;
        bitmap.bmiHeader.biXPelsPerMeter = 0;
        bitmap.bmiHeader.biYPelsPerMeter = 0;
        bitmap.bmiHeader.biClrUsed = 0;
        bitmap.bmiHeader.biClrImportant = 0;

        if (format == gcvSURF_A8){
            for (i = 0; i < 256; i++){
                tmp.rgbRed = tmp.rgbGreen = tmp.rgbReserved = 0;
                tmp.rgbBlue = i;
                bitmap.bmiColors[i] = tmp;
            }
        }

        /* Lock the resolve buffer. */
        if (ImageSaveBmp(bmpFileName, &bitmap, (unsigned char *)pData) == 0){
            ret = gcvTRUE;
        }
    }

	GC320_LOG_NOTIFY("Dump Bmp %s Success!", bmpFileName);
    return ret;
}

gctBOOL GalDumpSurfaceToBmp(gcoSURF surface, const char* bmpFileName)
{
	gctUINT alignedWidth, alignedHeight, width, height;
    gctINT bitsStride;
    stBmpInfo bitmap;
    gctUINT32 resolveAddress;
    gceSURF_FORMAT  format;
    gctPOINTER bits;
    gctINT bitCount, i;
    stRGB tmp;
    gctBOOL ret = gcvFALSE;

    if (surface && bmpFileName && bmpFileName[0]){
        GC320_CHECK_RETURN(gcoSURF_GetAlignedSize(surface, &alignedWidth, &alignedHeight, &bitsStride), gcvFALSE);
        GC320_CHECK_RETURN(gcoSURF_GetSize(surface, &width, &height, NULL), gcvFALSE);

        GC320_CHECK_RETURN(gcoSURF_GetFormat(surface, NULL, &format), gcvFALSE);
        switch (format)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            bitmap.mask[0] = 0x00FF0000;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x000000FF;
            bitCount = 32;
            break;

        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
            bitmap.mask[0] = 0xFF000000;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0x0000FF00;
            bitCount = 32;
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            bitmap.mask[0] = 0x000000FF;
            bitmap.mask[1] = 0x0000FF00;
            bitmap.mask[2] = 0x00FF0000;
            bitCount = 32;
            break;

        case gcvSURF_B8G8R8A8:
        case gcvSURF_B8G8R8X8:
            bitmap.mask[0] = 0x0000FF00;
            bitmap.mask[1] = 0x00FF0000;
            bitmap.mask[2] = 0xFF000000;
            bitCount = 32;
            break;

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
            bitmap.mask[0] = 0x00000F00;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x0000000F;
            bitCount = 16;
            break;

        case gcvSURF_R4G4B4X4:
        case gcvSURF_R4G4B4A4:
            bitmap.mask[0] = 0x0000F000;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x000000F0;
            bitCount = 16;
            break;

        case gcvSURF_X4B4G4R4:
        case gcvSURF_A4B4G4R4:
            bitmap.mask[0] = 0x0000000F;
            bitmap.mask[1] = 0x000000F0;
            bitmap.mask[2] = 0x00000F00;
            bitCount = 16;
            break;

        case gcvSURF_B4G4R4X4:
        case gcvSURF_B4G4R4A4:
            bitmap.mask[0] = 0x000000F0;
            bitmap.mask[1] = 0x00000F00;
            bitmap.mask[2] = 0x0000F000;
            bitCount = 16;
            break;

        case gcvSURF_R5G6B5:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_B5G6R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000007E0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            bitmap.mask[0] = 0x00007C00;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x0000001F;
            bitCount = 16;
            break;

        case gcvSURF_R5G5B5X1:
        case gcvSURF_R5G5B5A1:
            bitmap.mask[0] = 0x0000F800;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000003E;
            bitCount = 16;
            break;

        case gcvSURF_B5G5R5X1:
        case gcvSURF_B5G5R5A1:
            bitmap.mask[0] = 0x0000003E;
            bitmap.mask[1] = 0x000007C0;
            bitmap.mask[2] = 0x0000F800;
            bitCount = 16;
            break;

        case gcvSURF_X1B5G5R5:
        case gcvSURF_A1B5G5R5:
            bitmap.mask[0] = 0x0000001F;
            bitmap.mask[1] = 0x000003E0;
            bitmap.mask[2] = 0x00007C00;
            bitCount = 16;
            break;

        case gcvSURF_A8:
            bitCount = 8;
            break;

        default:
            // can not save and display
            return gcvFALSE;
        }

        /* Fill in the BITMAPINFOHEADER information. */
        bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
        bitmap.bmiHeader.biWidth = width;
        bitmap.bmiHeader.biHeight = -(gctINT)height;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = bitCount;
        if (format == gcvSURF_A8)
            bitmap.bmiHeader.biCompression = BIT_RGB;
        else
            bitmap.bmiHeader.biCompression = BIT_BITFIELDS;
        bitmap.bmiHeader.biSizeImage = 0;
        bitmap.bmiHeader.biXPelsPerMeter = 0;
        bitmap.bmiHeader.biYPelsPerMeter = 0;
        bitmap.bmiHeader.biClrUsed = 0;
        bitmap.bmiHeader.biClrImportant = 0;

        if (format == gcvSURF_A8){
            for (i = 0; i < 256; i++){
                tmp.rgbRed = tmp.rgbGreen = tmp.rgbReserved = 0;
                tmp.rgbBlue = i;
                bitmap.bmiColors[i] = tmp;
            }
        }

        /* Lock the resolve buffer. */
        GC320_CHECK_RETURN(gcoSURF_Lock(surface, &resolveAddress, &bits), gcvFALSE);

        if (ImageSaveBmp(bmpFileName, &bitmap, (unsigned char *)bits) == 0){
            ret = gcvTRUE;
        }

        GC320_CHECK_RETURN(gcoSURF_Unlock(surface, bits), gcvFALSE);
        GC320_CHECK_RETURN(gcoSURF_CPUCacheOperation(surface, gcvCACHE_FLUSH), gcvFALSE);
    }

	GC320_LOG_NOTIFY("Dump Bmp %s Success!", bmpFileName);
    return ret;
}

static gctUINT32 GalConvertTiling(gctUINT32 tiling)
{
	gctUINT32 convert_tiling = gcvLINEAR;
	gctUINT32 vsupertile = 0;
	switch(tiling & 0x0f)
	{
		case 0:
            convert_tiling = gcvLINEAR;
            break;

        case 1:
            convert_tiling = gcvTILED;
            break;

        case 2:
            convert_tiling = gcvSUPERTILED;
            break;

        case 3:
            convert_tiling = gcvMINORTILED;
            break;

        default:
            GC320_LOG_WARNING("Tiling is not support!");
			return gcvLINEAR;
	}

	
	if (convert_tiling == gcvSUPERTILED){
		switch ((tiling >> 28) & 0x0f)
		{
			case 0:
				vsupertile = gcv2D_SUPER_TILE_VERSION_V1;
				break;

			case 1:
				vsupertile = gcv2D_SUPER_TILE_VERSION_V2;
				break;

			case 2:
				vsupertile = gcv2D_SUPER_TILE_VERSION_V3;
				break;

			default:
				GC320_LOG_WARNING("supertitle is not support!");
				return gcvLINEAR;
		}
	}

	if ((tiling >> 27) & 0x01){
#if gcvVERSION_MAJOR >= 5
        convert_tiling |= gcvTILING_SPLIT_BUFFER;
#else
        if (convert_tiling == gcvTILED){
            convert_tiling = gcvMULTI_TILED;
        }
        else if (convert_tiling == gcvSUPERTILED){
            convert_tiling = gcvMULTI_SUPERTILED;
        }
        else{
            GC320_LOG_WARNING("tiling muti is not support!");
			return gcvLINEAR;
        }
#endif
    }
	return convert_tiling;
}

gctBOOL GalIsYUVFormat(gceSURF_FORMAT Format)
{
    switch (Format)
    {
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
    case gcvSURF_I420:
    case gcvSURF_YV12:
    case gcvSURF_NV16:
    case gcvSURF_NV12:
    case gcvSURF_NV61:
    case gcvSURF_NV21:

        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}


static void GalPrepareSurfaceForVimg(const Int8* vimgName, gctINT surIdx)
{
	gctUINT32 address[3] = {0, 0, 0};
    gctUINT32 width[3] = {0, 0, 0};
    gctUINT32 height[3] = {0, 0, 0};
    gctUINT32 bpp[3] = {0, 0, 0};
    gctUINT nPlane, n, i;
    gceSURF_FORMAT format;
	gctBOOL swap = gcvFALSE;
	gctUINT alignedWidth, alignedHeight;
    gctINT alignedStride;

	uInt8 *pData = NULL;
	stVimgInfo vimgInfo;
	pData = ImageLoadVimg(vimgName, &vimgInfo);
	if(!pData) return;
	uInt8 *pTmpData = pData;
	vimgInfo.tiling = GalConvertTiling(vimgInfo.tiling);
	GC320_LOG_WARNING("[VIMG: %s] format = %d, tiling = %d, width = %d, hight = %d, stride = %d", vimgName,
		vimgInfo.format,
		vimgInfo.tiling,
		vimgInfo.imageWidth,
		vimgInfo.imageHeight,
		vimgInfo.imageStride);
	/* Check the type. */
	switch (vimgInfo.format)
	{
	case gcvSURF_A8:
		format = gcvSURF_A8;
		nPlane = 1;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		break;

	case gcvSURF_YUY2:
		format = gcvSURF_YUY2;
		nPlane = 1;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 2;
		break;

	case gcvSURF_YVYU:
		format = gcvSURF_YVYU;
		nPlane = 1;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 2;
		break;

	case gcvSURF_UYVY:
		format = gcvSURF_UYVY;
		nPlane = 1;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 2;
		break;

	case gcvSURF_VYUY:
		format = gcvSURF_VYUY;
		nPlane = 1;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 2;
		break;

	case gcvSURF_I420:
		format = gcvSURF_I420;
		nPlane = 3;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		width[1] = width[2] =vimgInfo.imageWidth / 2;
		height[1] = height[2] = vimgInfo.imageHeight / 2;
		bpp[1] = bpp[2] = 1;
		break;

	case gcvSURF_YV12:
		format = gcvSURF_YV12;
		nPlane = 3;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		width[1] = width[2] = vimgInfo.imageWidth / 2;
		height[1] = height[2] = vimgInfo.imageHeight / 2;
		bpp[1] = bpp[2] = 1;
		swap = gcvTRUE;
		break;

	case gcvSURF_NV16:
		format = gcvSURF_NV16;
		nPlane = 2;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		width[1] = vimgInfo.imageWidth / 2;
		height[1] = vimgInfo.imageHeight;
		bpp[1] = 2;
		break;

	case gcvSURF_NV12:
		format = gcvSURF_NV12;
		nPlane = 2;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		width[1] = vimgInfo.imageWidth / 2;
		height[1] = vimgInfo.imageHeight / 2;
		bpp[1] = 2;
		break;

	case gcvSURF_NV21:
		format = gcvSURF_NV21;
		nPlane = 2;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		width[1] = vimgInfo.imageWidth / 2;
		height[1] = vimgInfo.imageHeight / 2;
		bpp[1] = 2;
		break;

	case gcvSURF_NV61:
		format = gcvSURF_NV61;
		nPlane = 2;
		width[0] = vimgInfo.imageWidth;
		height[0] = vimgInfo.imageHeight;
		bpp[0] = 1;
		width[1] = vimgInfo.imageWidth / 2;
		height[1] = vimgInfo.imageHeight;
		bpp[1] = 2;
		break;

	default:
		break;
	}

	if(gcmIS_ERROR(gcoSURF_Construct(s_Runtime.hal,
									vimgInfo.imageWidth, 
									vimgInfo.imageHeight,
									1,
									gcvSURF_BITMAP,
									format, 
									gcvPOOL_DEFAULT, 
									&s_Test2D.srcSurfs[surIdx].srcSurf))){
		GC320_LOG_ERR("%s call gcoSURF_Construct() Fail!", __FUNCTION__);
		free(pData);
		return;
	}

	if(gcmIS_ERROR(gcoSURF_GetAlignedSize(s_Test2D.srcSurfs[surIdx].srcSurf, &alignedWidth, &alignedHeight, &alignedStride))){
		GC320_LOG_ERR("%s call gcoSURF_GetAlignedSize() Fail!", __FUNCTION__);
		free(pData);
		return;
	}

	GC320_LOG_NOTIFY("Vimg: alignedWidth = %d, alignedHeight = %d, alignedStride = %d",
						alignedWidth, alignedHeight, alignedStride);

	if(gcmIS_ERROR(gcoSURF_Lock(s_Test2D.srcSurfs[surIdx].srcSurf,
							s_Test2D.srcSurfs[surIdx].srcPhyAddrs, 
							s_Test2D.srcSurfs[surIdx].srcLgcAddrs))){
		GC320_LOG_ERR("%s call gcoSURF_Lock() Fail!!!", __FUNCTION__);
		free(pData);
		pData = NULL;
		return;
	}

	gctUINT32 lineSize, aStride;
	aStride = alignedWidth * bpp[0];
	lineSize = width[0] * bpp[0];

	for (n = 0; n < height[0]; n++){
		/* Fill plane 1. */
		gctUINT8_PTR p = (gctUINT8_PTR)s_Test2D.srcSurfs[surIdx].srcLgcAddrs[0] + n * aStride;
		memcpy(p, pTmpData, lineSize);
		pTmpData += lineSize;
	}

	if (nPlane > 1){
		gctUINT aStride1 = (alignedWidth/2) * bpp[1];
		gctUINT lineSize1 = width[1] * bpp[1];

		if (s_Test2D.srcSurfs[surIdx].srcLgcAddrs[1] == gcvNULL){
			GC320_LOG_ERR("s_Test2D.srcSurfs[surIdx].srcLgcAddrs[1] is NULL!!!");
			free(pData);
			pData = NULL;
			return;
		}

		for (n = 0; n < height[1]; n++){
			/* Fill plane 2. */
			gctUINT8_PTR p = (gctUINT8_PTR)s_Test2D.srcSurfs[surIdx].srcLgcAddrs[swap ? 2 : 1] + n * aStride1;
			memcpy(p, pTmpData, lineSize1);
			pTmpData += lineSize1;
		}

		if (nPlane > 2){
			gctUINT aStride2;
			gctUINT lineSize2;

			if (nPlane != 3 || s_Test2D.srcSurfs[surIdx].srcLgcAddrs[2] == gcvNULL){
				GC320_LOG_ERR("s_Test2D.srcSurfs[surIdx].srcLgcAddrs[1] is NULL!!!");
				free(pData);
				pData = NULL;
				return;
			}

			aStride2 = (alignedWidth/2) * bpp[2];
			lineSize2 = width[2] * bpp[2];
			for (n = 0; n < height[2]; n++){
				/* Fill plane 3. */
				gctUINT8_PTR p = (gctUINT8_PTR)s_Test2D.srcSurfs[surIdx].srcLgcAddrs[swap ? 1 : 2] + n * aStride2;
				memcpy(p, pTmpData, lineSize2);
				pTmpData += lineSize2;
			}
		}
	}

	free(pData);
	pData = NULL;

	gcoSURF_GetFormat(s_Test2D.srcSurfs[surIdx].srcSurf, gcvNULL, &s_Test2D.srcSurfs[surIdx].srcFormat);
	s_Test2D.srcSurfs[surIdx].srcWidth = alignedWidth;
	s_Test2D.srcSurfs[surIdx].srcHeight = alignedHeight;
	s_Test2D.srcSurfs[surIdx].srcStrides[0] = alignedStride;
	s_Test2D.srcSurfs[surIdx].srcStrideNum = 1;
	s_Test2D.srcSurfs[surIdx].srcAddrNum = 1;
	if(GalIsYUVFormat(s_Test2D.srcSurfs[surIdx].srcFormat)){
		switch(s_Test2D.srcSurfs[surIdx].srcFormat){
			case gcvSURF_YUY2:
		    case gcvSURF_UYVY:
		    case gcvSURF_YVYU:
		    case gcvSURF_VYUY:
		        s_Test2D.srcSurfs[surIdx].srcStrides[1] = 0;
				s_Test2D.srcSurfs[surIdx].srcStrides[2] = 0;
				s_Test2D.srcSurfs[surIdx].srcStrideNum = 1;
				s_Test2D.srcSurfs[surIdx].srcAddrNum = 1;
		        break;

		    case gcvSURF_I420:
		    case gcvSURF_YV12:
				s_Test2D.srcSurfs[surIdx].srcStrides[1] = s_Test2D.srcSurfs[surIdx].srcStrides[0] / 2;
				s_Test2D.srcSurfs[surIdx].srcStrides[2] = s_Test2D.srcSurfs[surIdx].srcStrides[0] / 2;
				s_Test2D.srcSurfs[surIdx].srcStrideNum = 3;
				s_Test2D.srcSurfs[surIdx].srcAddrNum = 3;
		        break;

		    case gcvSURF_NV16:
		    case gcvSURF_NV12:
		    case gcvSURF_NV61:
		    case gcvSURF_NV21:
		        s_Test2D.srcSurfs[surIdx].srcStrides[1] = s_Test2D.srcSurfs[surIdx].srcStrides[0];
		        s_Test2D.srcSurfs[surIdx].srcStrides[2] = 0;
				s_Test2D.srcSurfs[surIdx].srcStrideNum = 2;
				s_Test2D.srcSurfs[surIdx].srcAddrNum = 2;
		        break;

		    default:
		        break;
		}
	}
	return;
}

void Gc320PrepareTestCase1()
{
	return;
}

/*
* 简单测试： 在目标surface上clear一块颜色为color的矩形区域，这个区域及颜色不断变化
*/
gctBOOL Gc320RenderTest1(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctINT deltaX = t2d->dstWidth >> 4;
	gctINT deltaY = t2d->dstHeight >> 4;
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth), gcvFALSE);

	gcsRECT fillRect = {0, 0, 400, 400};
	fillRect.left = frameNo;
	fillRect.top = frameNo;
	fillRect.left %= (t2d->dstWidth - 400);
	fillRect.top %= (t2d->dstHeight - 400);
	fillRect.right = fillRect.left + 400;
	fillRect.bottom = fillRect.top + 400;

	gctUINT32 color = ImageRandGetRGB();
	GC320_LOG_NOTIFY("FillRect: [%d, %d , 400, 400], destSize: [%d, %d]",
			fillRect.left, fillRect.top, t2d->dstWidth, t2d->dstHeight);
	GC320_CHECK_RETURN(gco2D_Clear(egn2D, 1, &fillRect, color, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);

	fillRect.left += 100;
	fillRect.top += 100;
	fillRect.right -= 100;
	fillRect.bottom -= 100;
	GC320_CHECK_RETURN(gco2D_Clear(egn2D, 1, &fillRect, 0x00, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;

    return gcvTRUE;
}

/*
* 测试四个平面的融合
*/
void Gc320PrepareTestCase2()
{
	const Int8* imageNames[] = {"./flyHouse_1920x720.bmp", "./NV12_1280x720.vimg", "./argb8_800x600.bmp", "./argb8_640x480.bmp"};
	uInt32 i = 0;
	for(i = 0; i < 4; i++){
		gctSTRING pSubStr = gcvNULL;
		pSubStr = strstr(imageNames[i], ".bmp");
		if(pSubStr){
			/* bmp */
			GalPrepareSurfaceForBmp(imageNames[i], i);
		}
		else{
			/* vimg */
			GalPrepareSurfaceForVimg(imageNames[i], i);
		}
	}

	/* hollow regions in surface */
	gcsRECT fillRects[1] = {{100, 200, 300, 400}};
	Gc320HollowSurfaceRegions(3, &fillRects[0], 1);
	return;
}

void Gc320SetTest2(gctUINT32 frameNo)
{
	if(frameNo % 10 != 0) return;
	gctUINT32 color = 0x00;
	color = ImageRandGetRGB();
	gcsRECT fillRect = {100, 200, 300, 400}; 
	Gc320ClearSurfaceRegions(3, &fillRect, 1, color);

	/* hollow regions in surface */
	if(frameNo == 40){
		gcsRECT fillRects[1] = {{400, 200, 600, 400}};
		Gc320HollowSurfaceRegions(3, &fillRects[0], 1);
	}
	if(frameNo == 80){
		gcsRECT fillRects[1] = {{400, 200, 600, 400}};
		Gc320HollowSurfaceRegions(2, &fillRects[0], 1);
	}
	s_Test2D.srcSurfs[2].srcAlphaBlend = gcvTRUE;
	s_Test2D.srcSurfs[3].srcAlphaBlend = gcvTRUE;
	return;
}

gctBOOL Gc320RenderTest2(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	for(i = 0; i < 4; i++){
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;

		/* vail area check */
		srcRect.left = (srcRect.left < 0) ? 0 : srcRect.left;
		srcRect.top = (srcRect.top < 0) ? 0 : srcRect.top;
		srcRect.right = (srcRect.right > pCurSrc->srcWidth) ? pCurSrc->srcWidth : srcRect.right;
		srcRect.bottom = (srcRect.bottom > pCurSrc->srcHeight) ? pCurSrc->srcHeight : srcRect.bottom;
		GC320_LOG_NOTIFY("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		if(pCurSrc->srcAlphaBlend == gcvTRUE){
			GalSetAlphaBlend();
		}
	}
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, 0x0f, &dstRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
    return gcvTRUE;
}

/*
* 测试alpha融合，挖洞透过的功能
*/

void Gc320PrepareTestCase3()
{
	const Int8* imageNames[] = {"./flyHouse_1920x720.bmp", "./NV12_1280x720.vimg", "./argb8_800x600.bmp", "./argb8_640x480.bmp"};
	uInt32 i = 0;
	for(i = 0; i < 4; i++){
		gctSTRING pSubStr = gcvNULL;
		pSubStr = strstr(imageNames[i], ".bmp");
		if(pSubStr){
			/* bmp */
			GalPrepareSurfaceForBmp(imageNames[i], i);
		}
		else{
			/* vimg */
			GalPrepareSurfaceForVimg(imageNames[i], i);
		}
	}

	/* 第二个和第三个平面，挖个洞透到下一个平面 */
	/* hollow regions in surface */
	gcsRECT fillRects[2] = {{100, 100, 300, 300},{400, 100, 600, 300}};
	Gc320HollowSurfaceRegions(2, &fillRects[1], 1);
	Gc320HollowSurfaceRegions(3, fillRects, 2);
	return;
}

void Gc320SetTest3(gctUINT32 frameNo)
{
	static enBOOL flag = TRUE;
	if(frameNo % 50 != 0) return;
	if(!flag){
		/* set not transparent */
		s_Test2D.srcSurfs[2].srcAlphaBlend = gcvFALSE;
		s_Test2D.srcSurfs[3].srcAlphaBlend = gcvFALSE;
		GC320_LOG_ERR("Gc320SetTest3 Set AlphaBlend False");
	}
	else{
		/* set full transparent */
		uInt32 i = 2;
		if(i == 2){
			gcsRECT fillRect = {400, 100, 600, 300}; 
			GC320_CHECK(gco2D_SetTarget(s_Runtime.engine2d, s_Test2D.srcSurfs[i].srcPhyAddrs[0], s_Test2D.srcSurfs[i].srcStrides[0], gcvSURF_0_DEGREE, s_Test2D.srcSurfs[i].srcWidth));
			GC320_CHECK(gco2D_Clear(s_Runtime.engine2d, 1, &fillRect, 0xFF000000, 0xCC, 0xCC, s_Test2D.srcSurfs[i].srcFormat));
			GC320_CHECK(gcoHAL_Commit(s_Runtime.hal, gcvTRUE));
			s_Test2D.srcSurfs[i].srcAlphaBlend = gcvTRUE;
		}

		i = 3;
		if(i == 3){
			gcsRECT fillRect[2] = {{100, 100, 300, 300}, {400, 100, 600, 300}}; 
			GC320_CHECK(gco2D_SetTarget(s_Runtime.engine2d, s_Test2D.srcSurfs[i].srcPhyAddrs[0], s_Test2D.srcSurfs[i].srcStrides[0], gcvSURF_0_DEGREE, s_Test2D.srcSurfs[i].srcWidth));
			GC320_CHECK(gco2D_Clear(s_Runtime.engine2d, 2, fillRect, 0xFF000000, 0xCC, 0xCC, s_Test2D.srcSurfs[i].srcFormat));
			GC320_CHECK(gcoHAL_Commit(s_Runtime.hal, gcvTRUE));
			s_Test2D.srcSurfs[i].srcAlphaBlend = gcvTRUE;
		}

		GC320_LOG_ERR("Gc320SetTest3 Set AlphaBlend True");
	}

	flag = !flag;
	return;
}

gctBOOL Gc320RenderTest3(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	for(i = 0; i < 4; i++){
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		if(i == 1 || i == 2){
			/* 第二个和第三个平面，显示区域变化 */
			srcRect.left = frameNo * i;
			srcRect.top = frameNo * i;
			srcRect.left %= pCurSrc->srcWidth;
			srcRect.top %= pCurSrc->srcHeight;
			srcRect.right = pCurSrc->srcWidth;
			srcRect.bottom = pCurSrc->srcHeight;

			/* vail area check */
			srcRect.left = (srcRect.left < 0) ? 0 : srcRect.left;
			srcRect.top = (srcRect.top < 0) ? 0 : srcRect.top;
			srcRect.right = (srcRect.right > pCurSrc->srcWidth) ? pCurSrc->srcWidth : srcRect.right;
			srcRect.bottom = (srcRect.bottom > pCurSrc->srcHeight) ? pCurSrc->srcHeight : srcRect.bottom;
		}
		else{
			srcRect.left = 0;
			srcRect.top = 0;
			srcRect.right = pCurSrc->srcWidth;
			srcRect.bottom = pCurSrc->srcHeight;
		}
		GC320_LOG_NOTIFY("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		if(pCurSrc->srcAlphaBlend == gcvTRUE){
			GalSetAlphaBlend();
		}
	}
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	gctUINT8 horKernel = 5, verKernel = 5;
	// set kernel size
	//GC320_CHECK_RETURN(gco2D_SetKernelSize(egn2D, horKernel, verKernel), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, 0x0f, &dstRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);
	
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
    return gcvTRUE;
}

/*
测试透过和收缩拉伸的功能
*/
void Gc320PrepareTestCase4()
{
	/* 目标： 将第三副图片融合后刷到一个temp surface（960x540）上， 再将temp surface拉伸后刷到目标surface(1920x720)上 */
	const Int8* imageNames[] = {"./flyHouse_960x540.bmp", "./argb8_800x600.bmp", "./argb8_640x480.bmp"};
	uInt32 i = 0;
	for(i = 0; i < 3; i++){
		gctSTRING pSubStr = gcvNULL;
		pSubStr = strstr(imageNames[i], ".bmp");
		if(pSubStr){
			/* bmp */
			GalPrepareSurfaceForBmp(imageNames[i], i);
		}
		else{
			/* vimg */
			GalPrepareSurfaceForVimg(imageNames[i], i);
		}
	}

	/* hollow regions in surface */
	gcsRECT fillRects[2] = {{100, 100, 300, 300},{400, 100, 600, 300}};
	Gc320HollowSurfaceRegions(1, &fillRects[1], 1);
	Gc320HollowSurfaceRegions(2, fillRects, 2);

	/* create tmp surface for scaling */
	GC320_CHECK(gcoSURF_Construct(s_Runtime.hal,
                          960,
                          540,
                          1,
                          gcvSURF_BITMAP,
                          s_Runtime.format,
                          gcvPOOL_DEFAULT,
                          &s_Test2D.tmpSurf));

	s_Test2D.tmpFormat = s_Runtime.format;
	s_Test2D.tmpWidth = 0;
	s_Test2D.tmpHeight = 0;
	s_Test2D.tmpStride = 0;
	s_Test2D.tmpPhyAddr = 0;
	s_Test2D.tmpLgcAddr = 0;

	/* get dest surface aligned size */
	GC320_CHECK(gcoSURF_GetAlignedSize(s_Test2D.tmpSurf,
									&s_Test2D.tmpWidth,
									&s_Test2D.tmpHeight,
									&s_Test2D.tmpStride));

	/* clear surface with green */
	GC320_CHECK(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.tmpSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

	/* lock tmp surface */
	GC320_CHECK(gcoSURF_Lock(s_Test2D.tmpSurf, &s_Test2D.tmpPhyAddr, &s_Test2D.tmpLgcAddr));

	return;
}

void Gc320SetTest4(gctUINT32 frameNo)
{
	static enBOOL flag = TRUE;
	if(frameNo % 50 != 0) return;
	if(!flag){
		/* set not transparent */
		gcsRECT fillRect = {200, 350, 500, 450}; 
		Gc320ClearSurfaceRegions(2, &fillRect, 1, 0xFFF88024);
		s_Test2D.srcSurfs[2].srcAlphaBlend = gcvTRUE;
	}
	else{
		/* set full transparent */
		gcsRECT fillRect = {200, 350, 500, 450};
		Gc320HollowSurfaceRegions(2, &fillRect, 1);
	}

	flag = !flag;
	return;
}

gctBOOL Gc320RenderTest4(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT tmpRect = {0, 0, t2d->tmpWidth, t2d->tmpHeight};
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	for(i = 0; i < 3; i++){
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;
		GC320_LOG_NOTIFY("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		if(pCurSrc->srcAlphaBlend == gcvTRUE){
			/* test alpha blending  */
			GalSetAlphaBlend();
		}
	}


	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &tmpRect), gcvFALSE);
	gceSURF_ROTATION dstRot;
	dstRot = rotationList[frameNo % gcmCOUNTOF(rotationList)];
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->tmpPhyAddr, 1,
		&t2d->tmpStride, 1,
		gcvLINEAR,
		t2d->tmpFormat,
		gcvSURF_0_DEGREE, //dstRot
		t2d->tmpWidth,
		t2d->tmpHeight), gcvFALSE);

	gctUINT8 horKernel = 5, verKernel = 5;
	// set kernel size
	//GC320_CHECK_RETURN(gco2D_SetKernelSize(egn2D, horKernel, verKernel), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	static gctUINT cnt = 0;
	static gctBOOL flag = gcvTRUE;
	gctUINT32 srcMask = 0x07;
	if(cnt++ >= 10){
		cnt = 0;
		flag = !flag;
	}
	/* 让GC320_PLANE1闪烁显示 */
	srcMask = (flag) ? (GC320_PLANE0 | GC320_PLANE1 | GC320_PLANE2) : (GC320_PLANE0 | GC320_PLANE2);
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, srcMask, &tmpRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	
	/* after merge 3 surface into tmp surface */
	GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            &t2d->tmpPhyAddr, 1,
            &t2d->tmpStride, 1,
            gcvLINEAR,
            t2d->tmpFormat,
            gcvSURF_0_DEGREE,
            t2d->tmpWidth,
            t2d->tmpHeight), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &tmpRect), gcvFALSE);
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	/* 设置缩放的比例 */
	GC320_CHECK_RETURN(gco2D_SetStretchRectFactors(egn2D, &tmpRect, &dstRect), gcvFALSE);

	/* 不缩放，直接blit上去 */
	//GC320_CHECK_RETURN(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
	/* 缩放处理 */
	GC320_CHECK_RETURN(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
	
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);
	
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
    return gcvTRUE;
}


/* 超过4个平面 */
void Gc320PrepareTestCase5()
{
	const Int8* imageNames[] = {"./flyHouse_960x540.bmp", "./argb8_800x600.bmp", "./argb8_640x480.bmp"};
	uInt32 i = 0;
	for(i = 0; i < 7; i++){
		if(i < 3){
			gctSTRING pSubStr = gcvNULL;
			pSubStr = strstr(imageNames[i], ".bmp");
			if(pSubStr){
				/* bmp */
				GalPrepareSurfaceForBmp(imageNames[i], i);
			}
			else{
				/* vimg */
				GalPrepareSurfaceForVimg(imageNames[i], i);
			}
		}
		else{
			GC320_CHECK(gcoSURF_Construct(s_Runtime.hal, 
									960,
									540,
									1,
									gcvSURF_BITMAP,
									s_Runtime.format,
									gcvPOOL_DEFAULT,
									&s_Test2D.srcSurfs[i].srcSurf));
			GC320_CHECK(gcoSURF_GetAlignedSize(s_Test2D.srcSurfs[i].srcSurf, 
												&s_Test2D.srcSurfs[i].srcWidth,
												&s_Test2D.srcSurfs[i].srcHeight, 
												&s_Test2D.srcSurfs[i].srcStrides[0]));

			GC320_CHECK(gcoSURF_Lock(s_Test2D.srcSurfs[i].srcSurf,
										s_Test2D.srcSurfs[i].srcPhyAddrs,
										s_Test2D.srcSurfs[i].srcLgcAddrs));

			GC320_CHECK(gcoSURF_GetFormat(s_Test2D.srcSurfs[i].srcSurf, gcvNULL, &s_Test2D.srcSurfs[i].srcFormat));
			s_Test2D.srcSurfs[i].srcStrideNum = 1;
			s_Test2D.srcSurfs[i].srcAddrNum = 1;
			GC320_CHECK(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[i].srcSurf, 0xFF000000));
		}
	}

	/* hollow regions in surface */
	gcsRECT fillRects[2] = {{100, 100, 300, 300},{400, 100, 600, 300}};
	Gc320HollowSurfaceRegions(1, &fillRects[1], 1);
	Gc320HollowSurfaceRegions(2, fillRects, 2);
				
	/* create tmp surface for scaling */
	GC320_CHECK(gcoSURF_Construct(s_Runtime.hal,
                          960,
                          540,
                          1,
                          gcvSURF_BITMAP,
                          s_Runtime.format,
                          gcvPOOL_DEFAULT,
                          &s_Test2D.tmpSurf));

	s_Test2D.tmpFormat = s_Runtime.format;
	s_Test2D.tmpWidth = 0;
	s_Test2D.tmpHeight = 0;
	s_Test2D.tmpStride = 0;
	s_Test2D.tmpPhyAddr = 0;
	s_Test2D.tmpLgcAddr = 0;

	/* get dest surface aligned size */
	GC320_CHECK(gcoSURF_GetAlignedSize(s_Test2D.tmpSurf,
									&s_Test2D.tmpWidth,
									&s_Test2D.tmpHeight,
									&s_Test2D.tmpStride));

	/* clear surface with green */
	GC320_CHECK(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.tmpSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

	/* lock tmp surface */
	GC320_CHECK(gcoSURF_Lock(s_Test2D.tmpSurf, &s_Test2D.tmpPhyAddr, &s_Test2D.tmpLgcAddr));

	return;
}

gctBOOL Gc320RenderTest5(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT tmpRect = {0, 0, t2d->tmpWidth, t2d->tmpHeight};
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	/* 前三个平面 */
	for(i = 0; i < 3; i++){
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;
		GC320_LOG_NOTIFY("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		if(pCurSrc->srcAlphaBlend == gcvTRUE){
			/* test alpha blending  */
			GalSetAlphaBlend();
		}
	}

	/* 将第四个平面作为临时平面 */
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &tmpRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		t2d->srcSurfs[3].srcPhyAddrs, 1,
		t2d->srcSurfs[3].srcStrides, 1,
		gcvLINEAR,
		t2d->srcSurfs[3].srcFormat,
		gcvSURF_0_DEGREE,
		t2d->srcSurfs[3].srcWidth,
		t2d->srcSurfs[3].srcHeight), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	static gctUINT cnt = 0;
	static gctBOOL flag = gcvTRUE;
	gctUINT32 srcMask = 0x07;
	if(cnt++ >= 10){
		cnt = 0;
		flag = !flag;
	}
	/* 让GC320_PLANE1闪烁显示 */
	srcMask = (flag) ? (GC320_PLANE0 | GC320_PLANE1 | GC320_PLANE2) : (GC320_PLANE0 | GC320_PLANE2);
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, srcMask, &tmpRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);

	/* 将前面三个平面融合到第四个平面再作为第一平面和后三个平面融合 */
	for(i = 3; i < 7; i++){
		if(i != 3){
			gcsRECT fillRect = {0, 0, 400, 400};
			fillRect.left = frameNo + (i - 3) * 40;
			fillRect.top = frameNo + (i - 3) * 40;
			fillRect.left %= (t2d->dstWidth - 400);
			fillRect.top %= (t2d->dstHeight - 400);
			fillRect.right = fillRect.left + 400;
			fillRect.bottom = fillRect.top + 400;

			gctUINT32 color = 0x00;
			color = ImageRandGetRGB() + i * 0x123456;
			color |= 0xFF000000;
			GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[i].srcSurf, FULL_TRANSPARENT_COLOR), gcvFALSE);
			gcoSURF_Clear2D(s_Test2D.srcSurfs[i].srcSurf, 1, &fillRect, color, color);
			fillRect.left += 100;
			fillRect.top += 100;
			fillRect.right -= 100;
			fillRect.bottom -= 100;
			gcoSURF_Clear2D(s_Test2D.srcSurfs[i].srcSurf, 1, &fillRect, FULL_TRANSPARENT_COLOR, FULL_TRANSPARENT_COLOR);
			GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);
		}
	
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i - 3), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		/* test alpha blending  */
		if(i != 3){
			GalSetAlphaBlend();
		}
	}

	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &tmpRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->tmpPhyAddr, 1,
		&t2d->tmpStride, 1,
		gcvLINEAR,
		t2d->tmpFormat,
		gcvSURF_0_DEGREE,
		t2d->tmpWidth,
		t2d->tmpHeight), gcvFALSE);

	//srcMask = (!flag) ? (GC320_PLANE0 | GC320_PLANE1 | GC320_PLANE2 | GC320_PLANE3) : (GC320_PLANE0 | GC320_PLANE1 | GC320_PLANE3);
	//GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, srcMask, &tmpRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, 0x0F, &tmpRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	
	/* after merge 3 surface into tmp surface */
	GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            &t2d->tmpPhyAddr, 1,
            &t2d->tmpStride, 1,
            gcvLINEAR,
            t2d->tmpFormat,
            gcvSURF_0_DEGREE,
            t2d->tmpWidth,
            t2d->tmpHeight), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &tmpRect), gcvFALSE);
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	/* 设置缩放的比例 */
	GC320_CHECK_RETURN(gco2D_SetStretchRectFactors(egn2D, &tmpRect, &dstRect), gcvFALSE);

	/* 不缩放，直接blit上去 */
	//GC320_CHECK_RETURN(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
	/* 缩放处理 */
	GC320_CHECK_RETURN(gco2D_StretchBlit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
	
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);
	
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
	#if 0
	if(frameNo < 20){
		char fileName[255] = {'\0'};
		sprintf(fileName, "./dumpImage/bmp_%03d.bmp", frameNo);
		GalDumpSurfaceToBmp(t2d->dstSurf, fileName);
	}
	#endif
	return gcvTRUE;
}

void Gc320PrepareTestCase6()
{
	uInt32 i = 0;
	for(i = 0; i < 4; i++){
		GC320_CHECK(gcoSURF_Construct(s_Runtime.hal, 
								1920,
								720,
								1,
								gcvSURF_BITMAP,
								s_Runtime.format,
								gcvPOOL_DEFAULT,
								&s_Test2D.srcSurfs[i].srcSurf));
		GC320_CHECK(gcoSURF_GetAlignedSize(s_Test2D.srcSurfs[i].srcSurf, 
											&s_Test2D.srcSurfs[i].srcWidth,
											&s_Test2D.srcSurfs[i].srcHeight, 
											&s_Test2D.srcSurfs[i].srcStrides[0]));

		GC320_CHECK(gcoSURF_Lock(s_Test2D.srcSurfs[i].srcSurf,
									s_Test2D.srcSurfs[i].srcPhyAddrs,
									s_Test2D.srcSurfs[i].srcLgcAddrs));

		gcoSURF_GetFormat(s_Test2D.srcSurfs[i].srcSurf, gcvNULL, &s_Test2D.srcSurfs[i].srcFormat);
		s_Test2D.srcSurfs[i].srcStrideNum = 1;
		s_Test2D.srcSurfs[i].srcAddrNum = 1;
		GC320_CHECK(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[i].srcSurf, 0xFF000000));
	}

	/* create tmp surface for scaling */
	GC320_CHECK(gcoSURF_Construct(s_Runtime.hal,
                          1920,
                          720,
                          1,
                          gcvSURF_BITMAP,
                          s_Runtime.format,
                          gcvPOOL_DEFAULT,
                          &s_Test2D.tmpSurf));

	s_Test2D.tmpFormat = s_Runtime.format;
	s_Test2D.tmpWidth = 0;
	s_Test2D.tmpHeight = 0;
	s_Test2D.tmpStride = 0;
	s_Test2D.tmpPhyAddr = 0;
	s_Test2D.tmpLgcAddr = 0;

	/* get dest surface aligned size */
	GC320_CHECK(gcoSURF_GetAlignedSize(s_Test2D.tmpSurf,
									&s_Test2D.tmpWidth,
									&s_Test2D.tmpHeight,
									&s_Test2D.tmpStride));

	/* clear surface with green */
	GC320_CHECK(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.tmpSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)));

	/* lock tmp surface */
	GC320_CHECK(gcoSURF_Lock(s_Test2D.tmpSurf, &s_Test2D.tmpPhyAddr, &s_Test2D.tmpLgcAddr));

	return;
}

gctBOOL Gc320RenderTest6(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT tmpRect = {0, 0, t2d->tmpWidth, t2d->tmpHeight};
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	for(i = 0; i < 4; i++){
		if(i != 0){
			gcsRECT fillRect = {0, 0, 400, 400};
			#if 0
			fillRect.left = frameNo + (i - 3) * 20;
			fillRect.top = frameNo + (i - 3) * 20;
			fillRect.left %= (t2d->dstWidth - 400);
			fillRect.top %= (t2d->dstHeight - 400);
			fillRect.right = fillRect.left + 400;
			fillRect.bottom = fillRect.top + 400;
			#else
			fillRect.left = (i - 1) * 200;
			fillRect.top = (i - 1) * 200;
			fillRect.right = fillRect.left + 200;
			fillRect.bottom = fillRect.top + 200;
			#endif

			gctUINT32 color = 0x00;
			color = ImageRandGetRGB() + i * 0x123456;
			color |= 0xFF000000;
			GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[i].srcSurf, FULL_TRANSPARENT_COLOR), gcvFALSE);
			#if 0
			gco2D_SetTarget(s_Runtime.engine2d, s_Test2D.srcSurfs[i].srcPhyAddrs[0], s_Test2D.srcSurfs[i].srcStrides[0], gcvSURF_0_DEGREE, s_Test2D.srcSurfs[i].srcWidth);
			GC320_CHECK_RETURN(gco2D_Clear(egn2D, 1, &fillRect, color, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
			fillRect.left += 50;
			fillRect.top += 50;
			fillRect.right -= 50;
			fillRect.bottom -= 50;
			GC320_CHECK_RETURN(gco2D_Clear(egn2D, 1, &fillRect, 0xFF000000, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
			#else
			gcoSURF_Clear2D(s_Test2D.srcSurfs[i].srcSurf, 1, &fillRect, color, color);
			fillRect.left += 50;
			fillRect.top += 50;
			fillRect.right -= 50;
			fillRect.bottom -= 50;
			gcoSURF_Clear2D(s_Test2D.srcSurfs[i].srcSurf, 1, &fillRect, FULL_TRANSPARENT_COLOR, FULL_TRANSPARENT_COLOR);
			#endif
			//GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);
		}
		else{
			GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[i].srcSurf, 0xFF00FF00), gcvFALSE);
		}
	
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;
		GC320_LOG_NONE("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		if(i != 0){
			GalSetAlphaBlend();
		}
	}


	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &tmpRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->tmpPhyAddr, 1,
		&t2d->tmpStride, 1,
		gcvLINEAR,
		t2d->tmpFormat,
		gcvSURF_0_DEGREE,
		t2d->tmpWidth,
		t2d->tmpHeight), gcvFALSE);

	gctUINT8 horKernel = 5, verKernel = 5;
	// set kernel size
	//GC320_CHECK_RETURN(gco2D_SetKernelSize(egn2D, horKernel, verKernel), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, 0x0F, &tmpRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	
	/* after merge 3 surface into tmp surface */
	GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            &t2d->tmpPhyAddr, 1,
            &t2d->tmpStride, 1,
            gcvLINEAR,
            t2d->tmpFormat,
            gcvSURF_0_DEGREE,
            t2d->tmpWidth,
            t2d->tmpHeight), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &tmpRect), gcvFALSE);
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);
	
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);
	
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
    return gcvTRUE;
}

/* 专门测试 alpha blending  */
void Gc320PrepareTestCase7()
{
	uInt32 i = 0;
	for(i = 0; i < 1; i++){
		GC320_CHECK(gcoSURF_Construct(s_Runtime.hal, 
								1920,
								720,
								1,
								gcvSURF_BITMAP,
								s_Runtime.format,
								gcvPOOL_DEFAULT,
								&s_Test2D.srcSurfs[i].srcSurf));
		GC320_CHECK(gcoSURF_GetAlignedSize(s_Test2D.srcSurfs[i].srcSurf, 
											&s_Test2D.srcSurfs[i].srcWidth,
											&s_Test2D.srcSurfs[i].srcHeight, 
											&s_Test2D.srcSurfs[i].srcStrides[0]));

		GC320_CHECK(gcoSURF_Lock(s_Test2D.srcSurfs[i].srcSurf,
									s_Test2D.srcSurfs[i].srcPhyAddrs,
									s_Test2D.srcSurfs[i].srcLgcAddrs));

		gcoSURF_GetFormat(s_Test2D.srcSurfs[i].srcSurf, gcvNULL, &s_Test2D.srcSurfs[i].srcFormat);
		s_Test2D.srcSurfs[i].srcStrideNum = 1;
		s_Test2D.srcSurfs[i].srcAddrNum = 1;
		GC320_CHECK(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[i].srcSurf, 0xFF000000));
	}
}

gctBOOL Gc320RenderTest7(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT bgRect = {20, 30, 120, 130}, fgRect = {50, 60, 250, 260};
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gcoBRUSH bgBrush, fgBrush;
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status;
	gctUINT8 ROP;
	static gctUINT8 srcAlpha = 0x00, dstAlpha = 0x00;
	static gctUINT16 test = 0;

	// clear dest surface with black
	GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.srcSurfs[0].srcSurf, 0xFF000000), gcvFALSE);

	// draw dst rect
	GC320_CHECK_RETURN(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
				COLOR_ARGB8(dstAlpha, 0xFF, 0x00, 0x00), 0, &bgBrush), gcvFALSE);
	Gal2DRectangle(s_Runtime.hal, t2d->dstSurf, bgBrush, bgRect);
	GC320_CHECK_RETURN(gcoBRUSH_Destroy(bgBrush), gcvFALSE);

	// clear src surface with black
	GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, t2d->srcSurfs[0].srcSurf, COLOR_ARGB8(0xFF, 0x00, 0x00, 0x00)), gcvFALSE);

	// draw src rect
	GC320_CHECK_RETURN(gco2D_ConstructSingleColorBrush(egn2D, gcvTRUE,
				COLOR_ARGB8(srcAlpha, 0x00, 0x00, 0xFF), 0, &fgBrush), gcvFALSE);
	Gal2DRectangle(s_Runtime.hal, t2d->srcSurfs[0].srcSurf, fgBrush, fgRect);
	GC320_CHECK_RETURN(gcoBRUSH_Destroy(fgBrush), gcvFALSE);

	// set color source and src rect
	if (gcoHAL_IsFeatureAvailable(s_Runtime.hal, gcvFEATURE_2DPE20)){
		GC320_CHECK_RETURN(gco2D_SetColorSourceAdvanced(egn2D, t2d->srcSurfs[0].srcPhyAddrs[0], t2d->srcSurfs[0].srcStrides[0], t2d->srcSurfs[0].srcFormat,
						gcvSURF_0_DEGREE, t2d->srcSurfs[0].srcWidth, t2d->srcSurfs[0].srcHeight, gcvFALSE), gcvFALSE);
		ROP = 0xCC;
	}
	else{
		GC320_CHECK_RETURN(gco2D_SetColorSource(egn2D, t2d->srcSurfs[0].srcPhyAddrs[0], t2d->srcSurfs[0].srcStrides[0], t2d->srcSurfs[0].srcFormat,
						gcvSURF_0_DEGREE, t2d->srcSurfs[0].srcWidth, gcvFALSE, gcvSURF_OPAQUE, 0), gcvFALSE);
		ROP = 0x88;
	}

	GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &dstRect), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth), gcvFALSE);

	// enalbe alphablend
	if (gcoHAL_IsFeatureAvailable(s_Runtime.hal, gcvFEATURE_2DPE20)){
		switch(test)
		{
			case 0:
				GC320_CHECK_RETURN(gco2D_EnableAlphaBlendAdvanced(egn2D,
							gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
							gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
							gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT), gcvFALSE);
				break;
			case 1:
				GC320_CHECK_RETURN(gco2D_EnableAlphaBlendAdvanced(egn2D,
							gcvSURF_PIXEL_ALPHA_INVERSED, gcvSURF_PIXEL_ALPHA_INVERSED,
							gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
							gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT), gcvFALSE);
				break;
			case 2:
				GC320_CHECK_RETURN(gco2D_SetSourceGlobalColorAdvanced(egn2D, srcAlpha << 24), gcvFALSE);
				GC320_CHECK_RETURN(gco2D_SetTargetGlobalColorAdvanced(egn2D, dstAlpha << 24), gcvFALSE);
				GC320_CHECK_RETURN(gco2D_EnableAlphaBlendAdvanced(egn2D,
							gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
							gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
							gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT), gcvFALSE);
				break;
			case 3:
				GC320_CHECK_RETURN(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0x40000000), gcvFALSE);
				GC320_CHECK_RETURN(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0x80000000), gcvFALSE);

				GC320_CHECK_RETURN(gco2D_EnableAlphaBlendAdvanced(egn2D,
							gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
							gcvSURF_GLOBAL_ALPHA_SCALE, gcvSURF_GLOBAL_ALPHA_SCALE,
							gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT), gcvFALSE);
				break;
			case 4:
			{
				gceSURF_BLEND_FACTOR_MODE srcMode, dstMode;
				switch (frameNo % 16)
				{
				case 0:
					srcMode = gcvSURF_BLEND_ZERO;
					dstMode = gcvSURF_BLEND_ZERO;
					break;

				case 1:
					srcMode = gcvSURF_BLEND_ZERO;
					dstMode = gcvSURF_BLEND_ONE;
					break;

				case 2:
					srcMode = gcvSURF_BLEND_ZERO;
					dstMode = gcvSURF_BLEND_STRAIGHT;
					break;

				case 3:
					srcMode = gcvSURF_BLEND_ZERO;
					dstMode = gcvSURF_BLEND_INVERSED;
					break;

				case 4:
					srcMode = gcvSURF_BLEND_ONE;
					dstMode = gcvSURF_BLEND_ZERO;
					break;

				case 5:
					srcMode = gcvSURF_BLEND_ONE;
					dstMode = gcvSURF_BLEND_ONE;
					break;

				case 6:
					srcMode = gcvSURF_BLEND_ONE;
					dstMode = gcvSURF_BLEND_STRAIGHT;
					break;

				case 7:
					srcMode = gcvSURF_BLEND_ONE;
					dstMode = gcvSURF_BLEND_INVERSED;
					break;

				case 8:
					srcMode = gcvSURF_BLEND_STRAIGHT;
					dstMode = gcvSURF_BLEND_ZERO;
					break;

				case 9:
					srcMode = gcvSURF_BLEND_STRAIGHT;
					dstMode = gcvSURF_BLEND_ONE;
					break;

				case 10:
					srcMode = gcvSURF_BLEND_STRAIGHT;
					dstMode = gcvSURF_BLEND_STRAIGHT;
					break;

				case 11:
					srcMode = gcvSURF_BLEND_STRAIGHT;
					dstMode = gcvSURF_BLEND_INVERSED;
					break;

				case 12:
					srcMode = gcvSURF_BLEND_INVERSED;
					dstMode = gcvSURF_BLEND_ZERO;
					break;

				case 13:
					srcMode = gcvSURF_BLEND_INVERSED;
					dstMode = gcvSURF_BLEND_ONE;
					break;

				case 14:
					srcMode = gcvSURF_BLEND_INVERSED;
					dstMode = gcvSURF_BLEND_STRAIGHT;
					break;

				case 15:
					srcMode = gcvSURF_BLEND_INVERSED;
					dstMode = gcvSURF_BLEND_INVERSED;
					break;

				default:
					srcMode = gcvSURF_BLEND_ZERO;
					dstMode = gcvSURF_BLEND_ZERO;
					break;
				}
				GC320_LOG_ERR("srcMode = %d, dstMode = %d", srcMode, dstMode);
				GC320_CHECK_RETURN(gco2D_SetSourceGlobalColorAdvanced(egn2D, 0xC0000000), gcvFALSE);
				GC320_CHECK_RETURN(gco2D_SetTargetGlobalColorAdvanced(egn2D, 0xC0000000), gcvFALSE);
				GC320_CHECK_RETURN(gco2D_EnableAlphaBlendAdvanced(egn2D,
							gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
							gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
							srcMode, dstMode), gcvFALSE);
				break;
			}
		}
	}
	else{
		GC320_CHECK_RETURN(gco2D_EnableAlphaBlend(egn2D,
					0x0, 0x0,
					gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
					gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
					gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT,
					gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT), gcvFALSE);
	}

	GC320_CHECK_RETURN(gco2D_Blit(egn2D, 1, &dstRect, ROP, ROP, t2d->dstFormat), gcvFALSE);

	// disalbe alphablend
	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);

	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	dstAlpha -= 0x10;
	dstAlpha = dstAlpha < 0x00 ? 0x00 : dstAlpha;
	srcAlpha += 0x10;
	srcAlpha = srcAlpha > 0xff ? 0xff : srcAlpha;
	if((frameNo + 1) % 16 == 0){
		test = (test + 1) % 5;
	}
	
	*ppFrameData = t2d->dstLgcAddr;
	GC320_LOG_ERR("Test = %d, dstAlpha = 0x%x, srcAlpha = 0x%x", test, dstAlpha, srcAlpha);
    return gcvTRUE;
}

void Gc320PrepareTestCase8()
{
	const Int8* imageNames[] = {"./flyHouse_1920x720.bmp", "./NV12_1280x720.vimg", "./argb8_800x600.bmp", "./rgb_400x400.bmp"};
	uInt32 i = 0;
	for(i = 0; i < 4; i++){
		gctSTRING pSubStr = gcvNULL;
		pSubStr = strstr(imageNames[i], ".bmp");
		if(pSubStr){
			/* bmp */
			GalPrepareSurfaceForBmp(imageNames[i], i);
		}
		else{
			/* vimg */
			GalPrepareSurfaceForVimg(imageNames[i], i);
		}
	}

	/* hollow regions in surface */
	gcsRECT fillRects[1] = {{100, 200, 300, 400}};
	Gc320ClearSurfaceRegions(2, &fillRects[0], 1, 0x00);
	s_Test2D.srcSurfs[2].srcAlphaBlend = gcvTRUE;
	return;
}

//test gco2D_SetPorterDuffBlending()
gctBOOL Gc320RenderTest8(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	for(i = 0; i < 3; i++){
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;
		if(i == 0){
			srcRect.left = 200;
			srcRect.top = 200;
		}
		else if(i == 1){
			srcRect.left = 400;
			srcRect.top = 400;
		}

		/* vail area check */
		srcRect.left = (srcRect.left < 0) ? 0 : srcRect.left;
		srcRect.top = (srcRect.top < 0) ? 0 : srcRect.top;
		srcRect.right = (srcRect.right > pCurSrc->srcWidth) ? pCurSrc->srcWidth : srcRect.right;
		srcRect.bottom = (srcRect.bottom > pCurSrc->srcHeight) ? pCurSrc->srcHeight : srcRect.bottom;
		GC320_LOG_NOTIFY("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);

		/* 设置透明色，使挖的透过色的矩形位置生效 */
		if(pCurSrc->srcAlphaBlend == gcvTRUE){
			/* alpha = 0x00 是全透 */
			GC320_CHECK_RETURN(gco2D_SetPorterDuffBlending(egn2D, gcvPD_SRC_OVER), gcvFALSE);
		}

		#if 0
		switch((frameNo / 40) % 4){
			case 0:
				/* 不做镜像处理 */
				GC320_CHECK_RETURN(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvFALSE), gcvFALSE);
				break;
			case 1:
				/* 做垂直方向镜像处理 */
				GC320_CHECK_RETURN(gco2D_SetBitBlitMirror(egn2D, gcvTRUE, gcvFALSE), gcvFALSE);
				break;
			case 2:
				/* 做水平方向镜像处理 */
				GC320_CHECK_RETURN(gco2D_SetBitBlitMirror(egn2D, gcvFALSE, gcvTRUE), gcvFALSE);
				break;
			case 3:
				/* 做垂直和水平方向镜像处理 */
				GC320_CHECK_RETURN(gco2D_SetBitBlitMirror(egn2D, gcvTRUE, gcvTRUE), gcvFALSE);
				break;
			default:
				break;
		}
		#endif
	}
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, 0x07, &dstRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);

	/* 将第四个图片贴在前面三个融合的平面上 */
	GC320_CHECK_RETURN(gco2D_SetColorSourceEx(egn2D, 
		t2d->srcSurfs[3].srcPhyAddrs[0], 
		t2d->srcSurfs[3].srcStrides[0], 
		t2d->srcSurfs[3].srcFormat,
		gcvSURF_0_DEGREE,
		t2d->srcSurfs[3].srcWidth,
		t2d->srcSurfs[3].srcHeight,
		gcvFALSE,
		gcvSURF_OPAQUE,
		0), gcvFALSE);
	gcsRECT srcRect = {0, 0, t2d->srcSurfs[3].srcWidth, t2d->srcSurfs[3].srcHeight};
	GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_SetTargetEx(egn2D, t2d->dstPhyAddr, t2d->dstStride,
					gcvSURF_0_DEGREE, t2d->dstWidth, t2d->dstHeight), gcvFALSE);

	dstRect.left = 800;
	dstRect.top = 0;
	dstRect.right = 800 + t2d->srcSurfs[3].srcWidth;
	dstRect.bottom = t2d->srcSurfs[3].srcHeight;
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_Blit(egn2D, 1, &dstRect, 0xCC, 0xCC, t2d->dstFormat), gcvFALSE);

	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
    return gcvTRUE;
}


/*测试gcoSURF_EnableAlphaBlend（）函数*/
void Gc320PrepareTestCase9()
{
	const Int8* imageNames[] = {"./flyHouse_1920x720.bmp", "./NV12_1280x720.vimg", "./argb8_800x600.bmp", "./argb8_640x480.bmp"};
	uInt32 i = 0;
	for(i = 0; i < 4; i++){
		gctSTRING pSubStr = gcvNULL;
		pSubStr = strstr(imageNames[i], ".bmp");
		if(pSubStr){
			/* bmp */
			GalPrepareSurfaceForBmp(imageNames[i], i);
		}
		else{
			/* vimg */
			GalPrepareSurfaceForVimg(imageNames[i], i);
		}

		gcoSURF_EnableAlphaBlend(s_Test2D.srcSurfs[i].srcSurf, 0x0, 0x0,
								gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
								gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,
								gcvBLEND_SRC, gcvBLEND_SRC_OVER_DST,
								gcvSURF_COLOR_MULTIPLY, gcvSURF_COLOR_MULTIPLY);
	}

	/* hollow regions in surface */
	gcsRECT fillRects[1] = {{100, 200, 300, 400}};
	Gc320HollowSurfaceRegions(3, &fillRects[0], 1);
	return;
}

gctBOOL Gc320RenderTest9(gctUINT32 frameNo, void** ppFrameData)
{
	stTest2D *t2d = &s_Test2D;
	gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
	gco2D egn2D = s_Runtime.engine2d;
	gceSTATUS status = gcvSTATUS_OK;
	gctUINT8 ROP = 0;
	gctINT x, y;
	gctUINT i = 0;

	for(i = 0; i < 4; i++){
		gcsRECT srcRect = {0};
		stSrcSurfaceInfo *pCurSrc = &t2d->srcSurfs[i];
		GC320_LOG_NONE("[SrcSurf %d]: w = %d, h = %d, stride[0] = %d, stride[1] = %d, stride[2] = %d, " \
			"format = %d, phyAddr[0] = 0x%x, phyAddr[1] = 0x%x, phyAddr[2] = 0x%x", i,
			pCurSrc->srcWidth, pCurSrc->srcHeight, pCurSrc->srcStrides[0], pCurSrc->srcStrides[1], pCurSrc->srcStrides[2], 
			pCurSrc->srcFormat, pCurSrc->srcPhyAddrs[0], pCurSrc->srcPhyAddrs[1], pCurSrc->srcPhyAddrs[2]);
		/* srcRect assign which area of source surface to merge into destination surface.
		    attention: srcRect must in the area of source surface area.
		*/
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = pCurSrc->srcWidth;
		srcRect.bottom = pCurSrc->srcHeight;

		/* vail area check */
		srcRect.left = (srcRect.left < 0) ? 0 : srcRect.left;
		srcRect.top = (srcRect.top < 0) ? 0 : srcRect.top;
		srcRect.right = (srcRect.right > pCurSrc->srcWidth) ? pCurSrc->srcWidth : srcRect.right;
		srcRect.bottom = (srcRect.bottom > pCurSrc->srcHeight) ? pCurSrc->srcHeight : srcRect.bottom;
		GC320_LOG_NOTIFY("[SrcRect %d]: [%d %d; %d %d]", i, 
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		/* the order of the surface from lower to upper surface is 0 -> 2, index 0 is the lower surface */
		GC320_CHECK_RETURN(gco2D_SetCurrentSourceIndex(egn2D, i), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetGenericSource(
            egn2D,
            pCurSrc->srcPhyAddrs, pCurSrc->srcAddrNum,
            pCurSrc->srcStrides, pCurSrc->srcStrideNum,
            gcvLINEAR,
            pCurSrc->srcFormat,
            gcvSURF_0_DEGREE,
            pCurSrc->srcWidth,
            pCurSrc->srcHeight), gcvFALSE);

		GC320_CHECK_RETURN(gco2D_SetSource(egn2D, &srcRect), gcvFALSE);
		GC320_CHECK_RETURN(gco2D_SetROP(egn2D, 0xCC, 0xCC), gcvFALSE);
	}
	
	GC320_CHECK_RETURN(gco2D_SetClipping(egn2D, &dstRect), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_SetGenericTarget(
		egn2D, 
		&t2d->dstPhyAddr, 1,
		&t2d->dstStride, 1,
		gcvLINEAR,
		t2d->dstFormat,
		gcvSURF_0_DEGREE,
		t2d->dstWidth,
		t2d->dstHeight), gcvFALSE);

	/*
	the second parameter show the mask of which surface, the max num is 4 so is 0xf
	if you only have two surfaces, this value should be 0x03
	*/
	GC320_CHECK_RETURN(gco2D_MultiSourceBlit(egn2D, 0x0f, &dstRect, 1), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_Flush(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gco2D_DisableAlphaBlend(egn2D), gcvFALSE);
	GC320_CHECK_RETURN(gcoHAL_Commit(s_Runtime.hal, gcvTRUE), gcvFALSE);

	*ppFrameData = t2d->dstLgcAddr;
    return gcvTRUE;
}


void Gc320DestroyAllSurfaces()
{
	gceSTATUS status = gcvSTATUS_OK;
	stTest2D *t2d = &s_Test2D;
	uInt32 i = 0;
	/* destroy dest suface and unlock */
    if (t2d->dstSurf != gcvNULL){
		if (t2d->dstLgcAddr != gcvNULL){
			GC320_CHECK(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr));
			t2d->dstLgcAddr = gcvNULL;
		}

		GC320_CHECK(gcoSURF_Destroy(t2d->dstSurf));
    }

	for(i = 0; i < SRC_SURF_NUM; i++){
		if(t2d->srcSurfs[i].srcSurf != gcvNULL){
			if(t2d->srcSurfs[i].srcLgcAddrs[0] != NULL){
				GC320_CHECK(gcoSURF_Unlock(t2d->srcSurfs[i].srcSurf, t2d->srcSurfs[i].srcLgcAddrs[0]));
				t2d->srcSurfs[i].srcLgcAddrs[0] = gcvNULL;
			}

			GC320_CHECK(gcoSURF_Destroy(t2d->srcSurfs[i].srcSurf));
		}
	}

	printf("GC320 Destory All Surfaces Success!\n");
	return;
}


gctBOOL Gc320CleanDstSurface(gctUINT32 color)
{
	/* clear target surface to black. */
    GC320_CHECK_RETURN(Gal2DCleanSurface(s_Runtime.hal, s_Test2D.dstSurf, color), gcvFALSE);
	return gcvTRUE;
}

void Gc320Finalize()
{
#if defined(LINUX) || defined(ANDROID)
    if (g_Internal != gcvNULL){
        /* Unmap the local internal memory. */
        gcmVERIFY_OK(gcoHAL_UnmapMemory(s_Runtime.hal,
                                        g_InternalPhysical, g_InternalSize,
                                        g_Internal));
    }

    if (g_External != gcvNULL){
        /* Unmap the local external memory. */
        gcmVERIFY_OK(gcoHAL_UnmapMemory(s_Runtime.hal,
                                        g_ExternalPhysical, g_ExternalSize,
                                        g_External));
    }

    if (g_Contiguous != gcvNULL){
        /* Unmap the contiguous memory. */
        gcmVERIFY_OK(gcoHAL_UnmapMemory(s_Runtime.hal,
                                        g_ContiguousPhysical, g_ContiguousSize,
                                        g_Contiguous));
    }
#endif

    if (s_Runtime.hal != gcvNULL){
        gcoHAL_Commit(s_Runtime.hal, gcvTRUE);
        gcoHAL_Destroy(s_Runtime.hal);
    }

    if (s_Runtime.os != gcvNULL){
        gcoOS_Destroy(s_Runtime.os);
    }

	printf("GC320 Finalize success!\n");
	return;
}

static gceSTATUS GalQueryFeatureStr(
    gceFEATURE feature,
    char* name,
    char* message,
    gctBOOL* status)
{
    int i = 0, cnt = sizeof(c_featureInfos) / sizeof(c_featureInfos[0]);

    if (name == gcvNULL || message == gcvNULL || status == gcvNULL){
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    for (i = 0; i < cnt; i++){
        if (c_featureInfos[i].feature == feature) {
            memcpy(name, c_featureInfos[i].name, strlen(c_featureInfos[i].name)+1);
            memcpy(message, c_featureInfos[i].msg, strlen(c_featureInfos[i].msg)+1);
            *status = c_featureInfos[i].status;
            break;
        }
    }

	if(i >= cnt){
		return gcvSTATUS_NAME_NOT_FOUND;
	}
	else{
    	return gcvSTATUS_OK;
	}
}

void Gc320DumpFeatureAvailableInfo()
{
	/* log chip info */
    if (Gc320ChipCheck(&s_Runtime) < 0){
        GC320_LOG_ERR("*ERROR* Check chip info failed!");
        return;
    }

	/* check feature whether is available */
	gctBOOL featureStatus;
    char featureName[64], featureMsg[128];
	GC320_LOG_WARNING("");
	GC320_LOG_WARNING("++++++++++++++++++++++++++++++++++++++++++");
	int k = 0;
	for(k = 0; k < sizeof(FeatureList) / sizeof(FeatureList[0]); k++){
        if(GalQueryFeatureStr(FeatureList[k], featureName, featureMsg, &featureStatus) != gcvSTATUS_OK){
			continue;
		}
        if (gcoHAL_IsFeatureAvailable(s_Runtime.hal, FeatureList[k]) == featureStatus){
            GC320_LOG_WARNING("%s is not supported.", featureMsg);
        }
    }
	GC320_LOG_WARNING("++++++++++++++++++++++++++++++++++++++++++");
	GC320_LOG_WARNING("");
	return;
}

const stGalRuntime* Gc320GetHandle()
{
	return &s_Runtime;
}

gctBOOL Gc320GetSurfaceInfo(
	gctUINT *pAlignWidth, 
	gctUINT *pAlignHeight,
	gctINT *pAlignStride,
	gctUINT8 *bytesPix)
{
	gctUINT alignedWidth, alignedHeight;
    gctINT bitsStride, bitCount;
	gceSURF_FORMAT format;
	if(s_Test2D.dstSurf){
		GC320_CHECK_RETURN(gcoSURF_GetAlignedSize(s_Test2D.dstSurf, &alignedWidth, &alignedHeight, &bitsStride), gcvFALSE);
        GC320_CHECK_RETURN(gcoSURF_GetFormat(s_Test2D.dstSurf, NULL, &format), gcvFALSE);
        switch (format)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            bitCount = 32;
            break;

        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
            bitCount = 32;
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            bitCount = 32;
            break;

        case gcvSURF_B8G8R8A8:
        case gcvSURF_B8G8R8X8:
            bitCount = 32;
            break;

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
            bitCount = 16;
            break;

        case gcvSURF_R4G4B4X4:
        case gcvSURF_R4G4B4A4:
            bitCount = 16;
            break;

        case gcvSURF_X4B4G4R4:
        case gcvSURF_A4B4G4R4:
		    bitCount = 16;
            break;

        case gcvSURF_B4G4R4X4:
        case gcvSURF_B4G4R4A4:
            bitCount = 16;
            break;

        case gcvSURF_R5G6B5:
            bitCount = 16;
            break;

        case gcvSURF_B5G6R5:
            bitCount = 16;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            bitCount = 16;
            break;

        case gcvSURF_R5G5B5X1:
        case gcvSURF_R5G5B5A1:
            bitCount = 16;
            break;

        case gcvSURF_B5G5R5X1:
        case gcvSURF_B5G5R5A1:
            bitCount = 16;
            break;

        case gcvSURF_X1B5G5R5:
        case gcvSURF_A1B5G5R5:
            bitCount = 16;
            break;

        case gcvSURF_A8:
            bitCount = 8;
            break;
        default:
            // can not save and display
            return gcvFALSE;
        }
		*pAlignWidth = alignedWidth;
		*pAlignHeight = alignedHeight;
		*pAlignStride = bitsStride;
		*bytesPix = bitCount / 8;
		return gcvTRUE;
	}
	return gcvFALSE;
}

gctBOOL Gc320CheckAlignment(gceSURF_FORMAT surfFormat, void *pBuffer)
{
	gctUINT32 addressAlign;
    GC320_CHECK_RETURN(gcoSURF_GetAlignment(gcvSURF_BITMAP,
										surfFormat,
								        &addressAlign,
										NULL,
								        NULL), gcvFALSE);

	if((gctUINT32)pBuffer % addressAlign){
		GC320_LOG_ERR("Buffer passed to GC320 doesn't meet the alignment requirement, GC320 alignment need is 0x%x, received buffer[0x%p] with 0x%x aligned address", 
			addressAlign, pBuffer, ((gctUINT32)pBuffer % addressAlign));
		return gcvFALSE;
	}

	GC320_LOG_NOTIFY("Buffer passed to GC320 meet the alignment requirement, GC320 alignment need is 0x%x, received buffer[0x%p] with 0x%x aligned address", 
			addressAlign, pBuffer, ((gctUINT32)pBuffer % addressAlign));
	return gcvTRUE;
}

gctUINT32 Gc320GetAddrAlignment(gceSURF_FORMAT surfFormat)
{
	gctUINT32 addressAlign;
    GC320_CHECK_RETURN(gcoSURF_GetAlignment(gcvSURF_BITMAP,
										surfFormat,
								        &addressAlign,
										NULL,
								        NULL), 0);
	return addressAlign;
}
