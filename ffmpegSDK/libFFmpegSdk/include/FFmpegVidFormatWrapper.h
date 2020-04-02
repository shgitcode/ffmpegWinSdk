#ifndef __H_FFMPEG_VIDEO_FORMAT_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_FORMAT_WRAPPER_H_H

#include <string>
#include "FFmpegCommon.h"

class CFFVideoEncoderObj;

class CFFmpegVideoFormatWrapper
{
public:

	CFFmpegVideoFormatWrapper();
	virtual ~CFFmpegVideoFormatWrapper();

public:
	/*
	 * @ 初始化参数
	*/
	virtual s16 FFmpegFormatInitVideoParamter(const s8 *pchFile, const s8 *pchFormat, u16 keyFramNum);

	/*
	* @ 向视频输出格式集添加视频流
	*/
	virtual s16 FFmpegFormatAddVidOutputStream();

	/*
	* @ 输出视频流头信息
	*/
	virtual s16 FFmpegFormatWriteStreamHeader(void *ptCodecCtx);

	/*
	* @ 输出视频流（写入文件或推送视频流）
	*/
	virtual s16 FFmpegFormatWriteOutputPacket(void *pPkt);

	/*
	* @ 输出视频流尾信息
	*/
	virtual s16 FFmpegFormatWriteStreamTrailer();

	/*
	* @ 关闭视频封装器
	*/
	void FFmpegFormatCloseVideoFormat();

	/*
	* @ 输出编码速度
	*/
	void FFmpegFormatSetReportCurrentTimer();

private:

	
	void FFmpegFormatSetReportSatrtTimer();

	void FFmpegFormatPrintReport(u32 last);

private:

	AVRational m_CodecTimeBase;

	/* output to file or stream */
	AVFormatContext *m_ptVidOFmtCtx;
	AVStream        *m_ptOutStream;
	std::string     m_tFileName;
	std::string     m_tFormatName;    // rtp, flv
	u16             m_byKeyFrameNum;  // 输出关键帧数目
	u16             m_byAddFrameNum;

	/* print report speed */
	int64_t        m_timerStart;
	int64_t        m_curTime;
	int64_t        m_curPts;

	BOOL32          m_bCreate;
	BOOL32          m_bOutputFlag;   // 是否需输出
};
#endif
