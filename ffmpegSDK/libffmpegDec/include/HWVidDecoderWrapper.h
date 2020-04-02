#ifndef __H_HW_FFMPEG_VIDEO_DECODER_WRAPPER_H_H
#define __H_HW_FFMPEG_VIDEO_DECODER_WRAPPER_H_H

#ifdef __cplusplus 
extern "C" {
#endif 
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_qsv.h>
#include <libavcodec/qsv.h>
#ifdef __cplusplus 
}
#endif 

#include "FFmpegVidDecoderCommon.h"
#include "FFVidDecoderCom.h"
#include "FFmpegVidDecoderWrapper.h"


class  CFFHWVideoDecoderWrapper: virtual public CFFmpegVideoDecoderWrapper,
	                             virtual public CFFVideoDecoderCom
{
public:

	CFFHWVideoDecoderWrapper();
	virtual ~CFFHWVideoDecoderWrapper();

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
	virtual s16 FFDecDecodeAVPacket(AVPacket *pkt, u8 *pbyOutData, u32 &dwOutLen) override;

	/*
	* @ 销毁视频解码器
	*/
	virtual void FFDecCloseDecoder() override;

private:
	// callback function must be static type
	static enum AVPixelFormat HWDecGetHWPixFormatCB(AVCodecContext *ptAvctx, const enum AVPixelFormat *pPixFmts);
	// callback function must be static type
	static enum AVPixelFormat HWDecSetHWFrameCtxCB(AVCodecContext *ptAvctx, const enum AVPixelFormat *pPixFmts);

	s16 HWDecCreateHWDeviceContext();

	s16 HWDecGetHWPixFormatFromConfigure();

	s16 HWDecRetriveHWFrame();

	s16 HWDecConvertDeviceType(const TFFVideoDecoderParam *ptDecParam);

private:

	AVBufferRef     *m_ptHWDevCtx;
	AVFrame         *m_ptVidHW2SWFrm;
	enum AVPixelFormat   m_emHWPixFmt;
	enum AVHWDeviceType  m_emDevType;
	BOOL32          m_bSetHWFrameCtx;
};

#endif
