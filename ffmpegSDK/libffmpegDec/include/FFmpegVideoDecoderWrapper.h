#ifndef __H_FFMPEG_VIDEO_DECODER_H_H
#define __H_FFMPEG_VIDEO_DECODER_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
#include <string>
#include "kdvtype.h"
#include "FFVideoDecoder.h"

#ifdef __cplusplus 
extern "C" {
#endif 

#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/macros.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_qsv.h>
#include <libavcodec/qsv.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>

#ifdef __cplusplus 
}
#endif 

#define ERR_BUF_SIZE 1024


typedef struct FFmpegDecDevContext {
	AVBufferRef *m_ptHWDevRef;
} FFmpegDecDevCtx;


class  CFFmpegVideoDecoderWrapper
{
public:

	CFFmpegVideoDecoderWrapper();
	virtual ~CFFmpegVideoDecoderWrapper();

public:
	/*
	* @ 初始化视频解码器参数
	*/
	virtual s16 FFmpegDecoderInitVideoParameter(const TFFVideoDecoderParam *ptVidDecoderParam);

	/*
	* @ 解码一帧视频
	*/
	virtual s16 FFmpegDecoderDecodeOneFrame(u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen);

	/*
	* @ 刷新一帧视频
	*/
	virtual s16 FFmpegDecoderFlushOneFrame(u8 *pbyOutData, u32 &dwOutLen);

	/*
	* @ 销毁视频解码器
	*/
	virtual void FFmpegDecoderCloseVideoDecoder();

private:

	// callback function must be static type
	static enum AVPixelFormat FFmpegGetHWPixFormat(AVCodecContext *ptAvctx, const enum AVPixelFormat *pPixFmts);

	// callback function must be static type
	static enum AVPixelFormat FFmpegSetHWFrameCtx(AVCodecContext *ptAvctx, const enum AVPixelFormat *pPixFmts);

	s16 FFmpegCreateHWDeviceContext();

	s16 FFmpegGetHWPixFormatFromConfigure();

	s16 FFmpegSetHWFrameCtx(AVCodecContext *ptCodecCtx, AVBufferRef *ptBufRef);

	/* for pkt Directly send */
	s16 FFmpegPutPacketToList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd, AVPacket *pPkt);

	void FFmpegFreePacketList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd);

	s16 FFmpegGetPacketFromList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd, AVPacket *pPkt);

	
	s16 FFmpegOpenInputFile();

	s16 FFmpegOpenVideoDecoder();

	s16 FFmpegCreatePictureSwsContext(AVFrame *pFrm);

	s16 FFmpegMallocAVFrameAndAVPacket();

	s16 FFmpegDecodePacket(AVPacket *pkt);

	s16 FFmpegCopyFrame(u8 *pbyOutData, u32 &dwOutLen);


	s16 FFmpegCheckInputParam(const TFFVideoDecoderParam *ptDecParam);

	s16 FFmpegConvertPixFmt(const TFFVideoDecoderParam *ptDecParam);

	s16 FFmpegConvertCodecId(const TFFVideoDecoderParam *ptDecParam);

	s16 FFmpegConvertDeviceType(const TFFVideoDecoderParam *ptDecParam);

	void FFmpegDumpDecoderParam();

private:

	AVFormatContext *m_ptVidDecoderFmtCtx;

	AVCodecContext  *m_ptVidDecCodecCtx;
	AVCodec         *m_ptVidDecCodec;
	AVCodecParameters *m_ptCodecPar;
	enum AVCodecID  m_emCodecId;

	AVStream        *m_ptVidStream;
	u32             m_dwVideoIdx;

	/* for HW decoder */
	AVBufferRef     *m_ptHWDevCtx;
	FFmpegDecDevCtx m_tHWDevCtx;
	AVFrame         *m_ptVidHW2SWFrm;
	enum AVPixelFormat   m_emHWPixFmt;
	enum AVHWDeviceType  m_emDevType;

	/* scale convert */
	struct SwsContext *m_ptPicSwsCtx;

	/* Directly send data */
	AVCodecParserContext *m_ptCodecParserCtx;
	struct AVPacketList *m_ptPktBuffer;
	struct AVPacketList *m_ptPktBufferEnd;

	AVPacket       *m_ptVidDecPkt;
	AVFrame        *m_ptVidDecFrm;
	AVFrame        *m_ptVidCntFrm;       // 将m_ptVidDecFrm转换为帧输出指定格式

	u32 m_dwOutPicWidth;   // 解码后图像宽
	u32 m_dwOutPicHeight;  // 解码后图像高
	enum AVPixelFormat   m_emOutPixFmt; // 解码后图像像素格式
	
	u32 m_dwDirectData;

	std::string m_tCodecName;
	std::string m_tInputFile;

	s8 m_chErrBuf[ERR_BUF_SIZE];

	std::mutex      m_mutex;

	BOOL32          m_bSetHWFrameCtx;

	BOOL32          m_bCreate;
};

#endif

