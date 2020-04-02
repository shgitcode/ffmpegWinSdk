#ifndef __H_FF_VIDEO_ENCODER_TYPE_H_H
#define __H_FF_VIDEO_ENCODER_TYPE_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kdvtype.h"
#include "kdvdef.h"

#define  FF_FILE_NAME_SIZE 1024
/* 编码能力级别 */
#define FFMPEG_VIDEO_BASE_PROFILE   0 // BP  
#define FFMPEG_VIDEO_MAIN_PROFILE   1   
#define FFMPEG_VIDEO_HIGH_PROFILE   2   

typedef enum em_FFVidPixIdx
{
	FF_VID_PIX_UNKNOWN = -1,
	FF_VID_PIX_I420,
	FF_VID_PIX_NV12,
}em_FFVidPixIdx;

/* Error message */
enum  em_FFStatus
{
	FF_STS_HAVE_PKT = 2,                    /* after encoding frame, have video packet */
	FF_STS_EOF = 1,                         /* end to encoded frame */

	FF_ERR_NONE = 0,                        /* no error */

	FF_ERR_UNKNOWN = -1,                    /* unknown error. */
	FF_ERR_NULL_PTR = -2,                   /* null pointer */
	FF_ERR_UNSUPPORTED = -3,                /* undeveloped feature */
	FF_ERR_MEMORY_ALLOC = -4,               /* failed to allocate memory */
	FF_ERR_INVALID_HANDLE = -5,             /* invalid handle */
	FF_ERR_NOT_FOUND = -6,                  /* the specified object is not found */
	FF_ERR_INVALID_VIDEO_PARAM = -7,        /* invalid video parameters */
	FF_ERR_DEVICE_UNSUPPORTED = -8,         /* HW device not supported */
	FF_ERR_NOT_ENOUGH_BUFFER = -9,          /* insufficient buffer for data */
	FF_ERR_NOT_INITIALIZED = -10,           /* not initialized */
};

enum em_FFPresetIndx
{
	em_FF_PRESET_UNKNOWN = -1,
	em_FF_PRESET_ULTRAFAST = 0,
	em_FF_PRESET_SUPERFAST,
	em_FF_PRESET_VERYFAST,
	em_FF_PRESET_FASTER,
	em_FF_PRESET_FAST,
	em_FF_PRESET_MEDIUM,
	em_FF_PRESET_SLOW,
	em_FF_PRESET_SLOWER,
	em_FF_PRESET_VERYSLOW,
	em_FF_PRESET_PLACEBO,
};

enum em_FFVideoEncoderType
{
	em_FF_SW = 0,
	em_FF_INTELHW,
	em_FF_NVIDIAHW,
	em_FF_AMDHW,
	em_FF_ENCNONE,
};

/* 视频编码参数 */
typedef struct FFVideoEncParam
{
	u8  m_byEncCodecType;           // 图像编码类型
	u16 m_byMaxKeyFrameInterval;    // I帧间最大P帧数目
	u8  m_byMaxQuant;               // 最大量化参数(20-40)
	u8  m_byMinQuant;               // 最小量化参数(20-40)
	u16 m_wBitRate;                 // 编码比特率(Kbps,1K=1024)
	u16 m_wMaxBitRate;              // 编码最大比特率
	u16 m_wMinBitRate;              // 编码最小比特率
	u16 m_wEncPicWidth;             // 编码设置图像宽
	u16 m_wEncPicHeight;            // 编码设置图像高

	u16 m_wCRF;                     // can use CRF: 0-51
	u16 m_wNumBFrames;              // the numbers of B frames
	u8	m_byFrameRate;              // 帧率(default:25)
	u8  m_byEncProfile;	            // 编码能力档次
	u8  m_byPresetIndx;             // preset, default medium, can reduce encoded delay

	BOOL32 m_bAnnexb;               // annexb, default 
	BOOL32 m_bZlatency;             // for x264 zerolatency, can reduce encoded delay
	BOOL32 m_bForcedIdr;            // force key frame to IDR

	/* for test */
	u16 m_byKeyFrameNum;                 // 关键帧数
	s8  m_chOutFile[FF_FILE_NAME_SIZE];  // 输出文件或URL
	s8  m_chFormatName[32];              // 输出协议
	BOOL32 m_bOutputFlag;                // 是否需要输出
}TFFVideoEncParam;

/* 指向视频原始数据存储 */
typedef struct FFVideoYUVFrame
{
	u32 m_dwVidWidth;  // 对齐后的宽度
	u32 m_dwVidHeight;
	em_FFVidPixIdx m_emInputPixIdx; // 图像像素格式
	u8* m_pVidFrameY;  // 指向Y存储的地址
	u8* m_pVidFrameU;  // 指向U存储的地址或者UV交错存储地址
	u8* m_pVidFrameV;  // 指向V存储的地址
}TFFVideoYUVFrame;

#endif
