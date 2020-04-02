#ifndef __H_HW_VIDEO_ENCODER_WRAPPER_H_H
#define __H_HW_VIDEO_ENCODER_WRAPPER_H_H

#include "FFmpegVidEncoderWrapper.h"
#include "FFmpegVidEncoderCom.h"

class CHWVideoEncoderWrapper :virtual public CFFmpegVideoEncoderWrapper,
	                          virtual public CFFmpegVideoEncoderCom
{
public:
	CHWVideoEncoderWrapper();
	virtual ~CHWVideoEncoderWrapper();

public:
	/*
	* @ 初始化编码器
	*/
	virtual s16	InitFFVideoEncoderCodecCtxCom(const TFFVideoEncParam *ptVdioEncParam,
		enum em_FFVideoEncoderType emVidType) override;

	/*
	* @ 打开编码器
	*/
	virtual s16 OpenFFVideoEncoderCom() override;

	/*
	* @ 获取编码器上下文
	*/
	virtual void* GetFFVideoEncoderCom() override;


	/*
	* @ 销毁编码器
	*/
	virtual void CloseFFVideoEncoderCom() override;

	/*
	* @ 编码一帧
	*/
	virtual s16	EncodeFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, 
		                              const TFFVideoYUVFrame *pTVidRawData) override;

	/*
	* @ 刷新编码器(停编码后编码器中可能会缓存几帧数据缓存)
	*/
	virtual s16 FlushFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag) override;

	/*
	* @ 获取编码后数据
	*/
	virtual void* GetFFVideoAVPacketCom() override;

	/*
	* @ 请求编码一个关键帧
	*/
	virtual void RequestFFVideoKeyFrameCom() override;

private:

	/*
	* @ 打开编码器前，获取HW设备及分配HW帧
	*/
	s16 FFmpegSetHWDeviceCtx(enum AVHWDeviceType emHWDevType);

	/*
	* @ 编码一帧前，获取HW帧buffer内存
	*/
	s16 FFmpegGetHWFrameBuffer();

	/*
	* @ 编码一帧前，将SW帧转换为HW帧
	*/
	s16 FFmpegTransToHWFrame();

	/*
	* @ 编码一帧前，为HW帧分配管理内存
	*/
	s16 FFmpegSetHWFrameCtx(AVCodecContext *ctx, AVBufferRef *ptHWDevCtx);

	/*
	 * 释放HW设备及帧内存
	*/
	void FFmpegFreeHWContext();

private:

	AVBufferRef    *m_ptHWDevCtx;
	AVFrame        *m_ptHWVidEncFrm;
	enum em_VideoHWCodecType   m_emHWCodecType;
};
#endif

