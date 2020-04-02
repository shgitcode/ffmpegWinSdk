#ifndef __H_FFMPEG_VIDEO_DECODER_COM_H_H
#define __H_FFMPEG_VIDEO_DECODER_COM_H_H


#include "FFmpegVidDecoderCommon.h"


class  CFFVideoDecoderCom
{
public:

	CFFVideoDecoderCom() = default;
	virtual ~CFFVideoDecoderCom() = default;

public:
	/*
	* @ 初始化视频解码器参数
	*/
	virtual s16 FFDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam) = 0;

	/*
	* @ 打开解码器
	*/
	virtual s16 FFDecOpenDecoder(AVCodecParameters *ptCodecPar) = 0;

	/*
	* @ 获取视频解码器
	*/
	virtual AVCodecContext* FFDecGetCodecCtx() = 0;

	/*
	* @ 解码一帧视频
	*/
	virtual s16 FFDecDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen) = 0;

	/*
	* @ 销毁视频解码器
	*/
	virtual void FFDecCloseDecoder() = 0;
};

#endif