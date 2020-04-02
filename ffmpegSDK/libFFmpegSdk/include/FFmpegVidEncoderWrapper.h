#ifndef __H_FFMPEG_VIDEO_ENCODER_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_ENCODER_WRAPPER_H_H

#include "FFmpegCommon.h"

#define ERR_BUF_SIZE 1024

struct FFVideoYUVFrame;
typedef struct FFVideoYUVFrame TFFVideoYUVFrame;

struct FFVideoEncParam;
typedef struct FFVideoEncParam TFFVideoEncParam;


class CFFmpegVideoEncoderWrapper
{
public:

	CFFmpegVideoEncoderWrapper();
	virtual ~CFFmpegVideoEncoderWrapper();

protected:
	/*
	* @ 初始化视频编码器参数
	* *ptVidEncParam: 视频编码配置参数
	* *pchCodecName：编码器名称（libx264/h264_qsv）
	* emPixFmt:编码像素格式（AV_PIX_FMT_YUV420P/AV_PIX_FMT_QSV）
	*/
	virtual s16 FFmpegInitVideoEncodeParameter(const TFFVideoEncParam *ptVidEncParam, 
		                                       const s8 *pchCodecName,
		                                       enum AVPixelFormat emPixFmt);

	/*
	* @ 打开视频编码器
	*/
	virtual s16 FFmpegOpenVideoEncoder();

	/*
	* @ 编码一帧视频
	*/
	virtual s16 FFmpegEncodeOneFrame(u8 *pbyOutData, u32 &dwOutLen, 
		                             AVFrame *ptVidEncFrm, BOOL32 &bKeyFlag);

	/*
	* @ 获取编码后视频数据
	*/
	virtual AVPacket* FFmpegGetVideoAVPacket();
	/*
	* @ 编码一帧视频前,分配AVFrame
	*/
	virtual AVFrame* FFmpegMallocAVFrame();

	/*
	* @ 编码一帧前，获取帧buffer内存
	*/
	virtual s16 FFmpegGetAVFrameBuffer(AVFrame *ptFrm, enum AVPixelFormat emPixFmt,
		                               u32 dwWidth, u32 dwHeight);

	/*
	* @ 编码一帧视频前,分配AVPacket
	*/
	virtual AVPacket* FFmpegMallocAVPacket();

	/*
	* @ 编码数据转换为AVFrame封装格式
	*/
	virtual s16 FFmpegConvertVideoToAVFrame(const TFFVideoYUVFrame *ptVidRawData);

	/*
	* @ 销毁视频编码器
	*/
	virtual void FFmpegCloseVideoEncoder();

	/*
	* @ 设置视频编码关键帧
	*/
	virtual void FFmpegSetRequestIDR();

	/*
	* @ 设置视频输出帧像素格式
	*/
	virtual void FFmpegSetOutPixFormat(enum AVPixelFormat emSWPixFmt);

	/*
	* @ 显示编码器相关信息
	*/
	virtual void FFmpegDumpEncoderContext();

	/*
	 * @将输入像素格式转为AV像素格式
	*/
	virtual enum AVPixelFormat FFmpegConvertInputFmtToAVPixFmt(em_FFVidPixIdx emInputPixIdx);

private:

	/*
	 * @创建像素转换器
	*/
	virtual s16 FFmpegCreatePicSwsContext(enum AVPixelFormat emPixFmt, u32 dwWidth, u32 dwHeight);
 
	/*
	 * @执行像素转换器
	*/
	virtual s16 FFmpegRunPicSwsScale(AVFrame *ptFrm);

protected:

	AVCodecContext *m_ptVidEncCodecCtx;
	AVFrame        *m_ptVidEncFrm;
	BOOL32          m_bCreate;

	/* store error info */
	s8 m_chErrBuf[ERR_BUF_SIZE];

private:

	AVPacket       *m_ptVidEncPkt;
	AVCodec        *m_ptVidEncCodec;
	enum AVPixelFormat m_emInPixFmt;
	BOOL32         m_bReqIDR;

	enum AVPixelFormat m_emOutPixFmt;

	/* scale convert */
	struct SwsContext *m_ptPicSwsCtx;
	AVFrame           *m_ptVidCntFrm;
};
#endif