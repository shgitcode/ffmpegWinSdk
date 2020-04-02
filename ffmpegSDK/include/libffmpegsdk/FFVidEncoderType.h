#ifndef __H_FF_VIDEO_ENCODER_TYPE_H_H
#define __H_FF_VIDEO_ENCODER_TYPE_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kdvtype.h"
#include "kdvdef.h"

#define  FF_FILE_NAME_SIZE 1024
/* ������������ */
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

/* ��Ƶ������� */
typedef struct FFVideoEncParam
{
	u8  m_byEncCodecType;           // ͼ���������
	u16 m_byMaxKeyFrameInterval;    // I֡�����P֡��Ŀ
	u8  m_byMaxQuant;               // �����������(20-40)
	u8  m_byMinQuant;               // ��С��������(20-40)
	u16 m_wBitRate;                 // ���������(Kbps,1K=1024)
	u16 m_wMaxBitRate;              // ������������
	u16 m_wMinBitRate;              // ������С������
	u16 m_wEncPicWidth;             // ��������ͼ���
	u16 m_wEncPicHeight;            // ��������ͼ���

	u16 m_wCRF;                     // can use CRF: 0-51
	u16 m_wNumBFrames;              // the numbers of B frames
	u8	m_byFrameRate;              // ֡��(default:25)
	u8  m_byEncProfile;	            // ������������
	u8  m_byPresetIndx;             // preset, default medium, can reduce encoded delay

	BOOL32 m_bAnnexb;               // annexb, default 
	BOOL32 m_bZlatency;             // for x264 zerolatency, can reduce encoded delay
	BOOL32 m_bForcedIdr;            // force key frame to IDR

	/* for test */
	u16 m_byKeyFrameNum;                 // �ؼ�֡��
	s8  m_chOutFile[FF_FILE_NAME_SIZE];  // ����ļ���URL
	s8  m_chFormatName[32];              // ���Э��
	BOOL32 m_bOutputFlag;                // �Ƿ���Ҫ���
}TFFVideoEncParam;

/* ָ����Ƶԭʼ���ݴ洢 */
typedef struct FFVideoYUVFrame
{
	u32 m_dwVidWidth;  // �����Ŀ��
	u32 m_dwVidHeight;
	em_FFVidPixIdx m_emInputPixIdx; // ͼ�����ظ�ʽ
	u8* m_pVidFrameY;  // ָ��Y�洢�ĵ�ַ
	u8* m_pVidFrameU;  // ָ��U�洢�ĵ�ַ����UV����洢��ַ
	u8* m_pVidFrameV;  // ָ��V�洢�ĵ�ַ
}TFFVideoYUVFrame;

#endif
