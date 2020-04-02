#ifndef __H_FFMPEG_VIDEO_DECODER_OBJ_H_H
#define __H_FFMPEG_VIDEO_DECODER_OBJ_H_H

#include "FFmpegVidDecoderCommon.h"

class  CFFVideoDecoderCom;

class  CFFVideoDecoderObj
{
public:

	CFFVideoDecoderObj();
	virtual ~CFFVideoDecoderObj();

public:
	/*
	* @ 初始化视频解码器参数
	*/
	virtual s16 FFDecObjInitParameter(const TFFVideoDecoderParam *ptVidDecParam);

	/*
	* @ 打开解码句柄
	*/
	virtual s16 FFDecObjOpenHandle(AVCodecParameters *ptCodecPar);

	/*
	* @ 获取视频解码器
	*/
	virtual AVCodecContext* FFDecObjGetCodecCtx();

	/*
	* @ 解码一帧视频
	*/
	virtual s16 FFDecObjDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen);

	/*
	* @ 关闭视频解码句柄
	*/
	virtual void FFDecObjCloseHandle();

private:
	std::unique_ptr<CFFVideoDecoderCom> m_tFFVidDec;
};

#endif
