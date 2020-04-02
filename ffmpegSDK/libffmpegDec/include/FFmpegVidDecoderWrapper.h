#ifndef __H_FFMPEG_VIDEO_DECODER_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_DECODER_WRAPPER_H_H


#include "FFmpegVidDecoderCommon.h"


class  CFFmpegVideoDecoderWrapper
{
public:

	CFFmpegVideoDecoderWrapper();
	virtual ~CFFmpegVideoDecoderWrapper();

protected:
	/*
	* @ 初始化视频解码器参数
	*/
	virtual s16 FFmpegDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam);

	/*
	* @ 创建解码器
	*/
	virtual s16 FFmpegDecCreateDecoder(enum AVCodecID codecId);

	/*
	* @ 打开解码器
	*/
	virtual s16 FFmpegDecOpenDecoder(AVCodecParameters *ptCodecPar);

	/*
	* @ 解码一帧视频
	*/
	virtual s16 FFmpegDecDecodeAVPacket(AVPacket *pkt);

	/*
	* @ 创建帧转换器
	*/
	virtual s16 FFmpegDecCreatePicSwsContext(enum AVPixelFormat emPixFmt, u32 dwWidth, u32 dwHeight);

	/*
	* @ 执行帧转换器
	*/
	virtual s16 FFmpegDecRunPicSwsScale(AVFrame *pFrm);

	/*
	* @ 分配AVFrame
	*/
	virtual AVFrame* FFmpegDecMallocAVFrame();

	/*
	* @ 分配AVFrame的buffer
	*/
	virtual s16 FFmpegDecGetAVFrameBuffer(AVFrame *pFrame, enum AVPixelFormat emPixFmt,
		u32 dwWidth, u32 dwHeight);
	
	/*
	* @ 拷贝一帧视频
	*/
	virtual s16 FFmpegDecCopyAVFrame(AVFrame *pFrm, u8 *pbyOutData, u32 &dwOutLen);

	/*
	* @ 销毁视频解码器
	*/
	virtual void FFmpegDecCloseDecoder();

private:

	s16 FFmpegCheckInputParam(const TFFVideoDecoderParam *ptDecParam);

	s16 FFmpegConvertPixFmt(const TFFVideoDecoderParam *ptDecParam);

	void FFmpegDumpDecoderParam();

protected:

	AVCodecContext  *m_ptVidDecCodecCtx;
	AVCodec         *m_ptVidDecCodec;

	AVFrame        *m_ptVidDecFrm;
	AVFrame        *m_ptVidCntFrm;   // 将m_ptVidDecFrm转换为帧输出指定格式

	s8 m_chErrBuf[ERR_BUF_SIZE];

private:

	/* scale convert */
	struct SwsContext *m_ptPicSwsCtx;

	u32 m_dwOutPicWidth;   // 解码后图像宽
	u32 m_dwOutPicHeight;  // 解码后图像高
	enum AVPixelFormat   m_emOutPixFmt; // 解码后图像像素格式

	std::string m_tCodecName;
};

#endif
