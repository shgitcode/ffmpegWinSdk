#ifndef __H_FFMPEG_VIDEO_DEVICE_DATA_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_DEVICE_DATA_WRAPPER_H_H

#ifdef __cplusplus 
extern "C" {
#endif 
#include <libavdevice/avdevice.h>
#ifdef __cplusplus 
}
#endif 

#include "FFVidDataCom.h"
#include "FFmpegVidDecoderCommon.h"
#include "FFmpegVidFormatDataWrapper.h"


class  CFFmpegVideoDevDataWrapper : virtual public CFFmpegVideoFormatDataWrapper
{
public:

	CFFmpegVideoDevDataWrapper();
	virtual ~CFFmpegVideoDevDataWrapper();

public:
	/*
	* @ 初始化视频解封装参数
	*/
	virtual s16 FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecParam) override;

	/*
	* @ 打开数据流
	*/
	virtual s16 FFDataOpenDataStream() override;

	/*
	* @ 关闭数据流
	*/
	virtual void FFDataCloseDataStream() override;

private:

	AVInputFormat *m_ptInFmt;
	AVDictionary  *m_ptOptions;

	std::string     m_tFmtName;
};

#endif


