#ifndef __H_FF_VIDEO_DECODER_TYPE_H_H
#define __H_FF_VIDEO_DECODER_TYPE_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kdvtype.h"

#define  DECODER_FILE_SIZE 1024 
#define  DECODER_NAME_SIZE 16

typedef enum em_FFVidDecPixIdx
{
	FF_VID_DEC_PIX_UNKNOWN = -1,
	FF_VID_DEC_PIX_I420,
	FF_VID_DEC_PIX_NV12,
}em_FFVidDecPixIdx;


typedef enum em_FFVidHWDecType
{
	FF_VID_HW_DEC_TYPE_NONE = 0,     // for software decoder(codecName : h264/hevc)
	FF_VID_HW_DEC_TYPE_DXVA2,    // for INTEL/AMD/NAVIDA chip(codecName : h264/hevc)
	FF_VID_HW_DEC_TYPE_QSV,      // for QSV   (codecName : h264_qsv/hevc_qsv)
	FF_VID_HW_DEC_TYPE_CUDA,     // for CUDA  (codecName : h264_cuvid/hevc_cuvid/h264/hevc)
}em_FFVidHWDecType;


typedef enum em_FFVidDecCodec
{
	FF_VID_DEC_CODEC_UNKNOWN = -1,
	FF_VID_DEC_CODEC_H264,
	FF_VID_DEC_CODEC_HEVC,
}em_FFVidDecCodec;

// data type
typedef enum em_FFVidDataType
{
	FF_VID_DATA_TYPE_FILE = 0, // file or stream url(rtmp)
	FF_VID_DATA_TYPE_RAW,      // raw data (h264/hevc), not formated data(mp4/flv)
	FF_VID_DATA_TYPE_DEV,      // device data(camera/screen recoder)
}em_FFVidDataType;


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

/* 视频解码帧输出参数 */
typedef struct FFVideoDecoderParam
{
	u32 m_dwPicWidth;                // 解码后图像宽
	u32 m_dwPicHeight;               // 解码后图像高
	em_FFVidDecPixIdx	m_emPixIdx;  // 解码后像素索引

	em_FFVidDecCodec    m_emDecCodec;    // Raw数据时解码Codec(h264/hevc)
	
	em_FFVidHWDecType   m_emDecType;     // 硬解码时设备类型

	s8 m_chCodecName[DECODER_NAME_SIZE]; // 解码器名称(h264/hevc/h264_qsv/hevc_qsv/h264_cuvid/hevc_cuvid)
	
	s8 m_chFormatName[DECODER_NAME_SIZE]; // 输入设备名称(dshow/gdigrab)--无需给出解码器名称

	/* 
	 * 输入文件/网络流地址/设备URL
	 * 对于设备：(BMP
	 *   gdigrab:desktop
	 *   dshow:video=screen-capture-recorder
	 *   dshow:video=Integrated Camera
	 *   )
	*/
	s8 m_chInputFile[DECODER_FILE_SIZE]; 

	em_FFVidDataType m_bDirectData;
}TFFVideoDecoderParam;

#endif
