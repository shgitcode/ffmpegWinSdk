#ifndef __H_FFMPEG_VIDEO_DATA_COM_H_H
#define __H_FFMPEG_VIDEO_DATA_COM_H_H


#include "FFmpegVidDecoderCommon.h"


class  CFFVideoDataCom
{
public:

	CFFVideoDataCom() = default;
	virtual ~CFFVideoDataCom() = default;

public:

	/*
	* @ 初始化视频解封装参数
	*/
	virtual s16 FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecoderParam) = 0;

	/*
	* @ 打开数据流
	*/
	virtual s16 FFDataOpenDataStream() = 0;

	/*
	* @ 获取CodecParam,须在打开数据流之后
	*/
	virtual AVCodecParameters* FFDataGetCodecParam() = 0;

	/*
	* @ 解封装一帧视频
	*/
	virtual s16 FFDataReadPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen) = 0;

	/*
	* @ 获取一帧视频
	*/
	virtual AVPacket* FFDataGetAVPacket() = 0;

	/*
	* @ 解关联一帧视频
	*/
	virtual void FFDataUnrefAVPacket() = 0;

	/*
	* @ 关闭数据流
	*/
	virtual void FFDataCloseDataStream() = 0;
};

#endif
