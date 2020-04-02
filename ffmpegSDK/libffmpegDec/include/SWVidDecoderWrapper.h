#ifndef __H_SW_FFMPEG_VIDEO_DECODER_WRAPPER_H_H
#define __H_SW_FFMPEG_VIDEO_DECODER_WRAPPER_H_H


#include "FFmpegVidDecoderCommon.h"
#include "FFVidDecoderCom.h"
#include "FFmpegVidDecoderWrapper.h"


class  CFFSWVideoDecoderWrapper : virtual public CFFmpegVideoDecoderWrapper,
	                              virtual public  CFFVideoDecoderCom
{
public:

	CFFSWVideoDecoderWrapper();
	virtual ~CFFSWVideoDecoderWrapper();

public:
	/*
	* @ 创建及初始化视频解码器参数
	*/
	virtual s16 FFDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam) override;

	/*
	* @ 打开解码器
	*/
	virtual s16 FFDecOpenDecoder(AVCodecParameters *ptCodecPar) override;

	/*
	* @ 获取视频解码器
	*/
	virtual AVCodecContext* FFDecGetCodecCtx() override;

	/*
	* @ 解码一帧视频
	*/
	virtual s16 FFDecDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen) override;

	/*
	* @ 销毁视频解码器
	*/
	virtual void FFDecCloseDecoder() override;
};

#endif