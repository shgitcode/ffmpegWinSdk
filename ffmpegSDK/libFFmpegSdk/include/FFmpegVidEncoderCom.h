#ifndef __H_FFMPEG_VIDEO_ENCODER_COM_H_H
#define __H_FFMPEG_VIDEO_ENCODER_COM_H_H

#include "kdvtype.h"

struct FFVideoYUVFrame;
typedef struct FFVideoYUVFrame TFFVideoYUVFrame;

struct FFVideoEncParam;
typedef struct FFVideoEncParam TFFVideoEncParam;

class CFFmpegVideoEncoderCom
{
public:
	CFFmpegVideoEncoderCom() {};

	virtual ~CFFmpegVideoEncoderCom() {};

public:
	/*
	* @ 初始化编码器
	*/
	virtual s16	InitFFVideoEncoderCodecCtxCom(const TFFVideoEncParam *ptVdioEncParam, enum em_FFVideoEncoderType emVidType) = 0;

	/*
	* @ 打开编码器
	*/
	virtual s16 OpenFFVideoEncoderCom() = 0;

	/*
	* @ 获取编码器上下文
	*/
	virtual void* GetFFVideoEncoderCom() = 0;

	/*
	* @ 销毁编码器
	*/
	virtual void CloseFFVideoEncoderCom() = 0;

	/*
	* @ 编码一帧
	*/
	virtual s16	EncodeFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData) = 0;

	/*
	* @ 刷新编码器(停编码后编码器中可能会缓存几帧数据缓存)
	*/
	virtual s16 FlushFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag) = 0;

	/*
	* @ 获取编码后数据
	*/
	virtual void* GetFFVideoAVPacketCom() = 0;

	/*
	* @ 请求编码一个关键帧
	*/
	virtual void RequestFFVideoKeyFrameCom() = 0;
};

#endif
