#ifndef __H_FFMPEG_COMMON_H_H
#define __H_FFMPEG_COMMON_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus 
extern "C" {
#endif 

#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>

#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/timestamp.h>
#include <libavutil/time.h>
#include <libavutil/common.h>

#include <libswscale/swscale.h>
#ifdef __cplusplus 
}
#endif 

#include "kdvtype.h"
#include "kdvdef.h"


enum em_VideoHWCodecType
{
	em_UnknownCodec = -1,
	em_H264QsvCodec,
	em_HevcQsvCodec,
	em_H264AmfCodec,
	em_HevcAmfCodec,
	em_H264NvencCodec,
	em_HevcNvencCodec,
};

struct AVHWCodecInfo {
	const s8 *m_pchCodecName;
	const enum AVCodecID m_emCodecId;
	const enum AVHWDeviceType m_emtHWDevType;
	const enum AVPixelFormat m_emHWPixFmt;
	const enum AVPixelFormat m_emFrmPixFmt;
};

#endif