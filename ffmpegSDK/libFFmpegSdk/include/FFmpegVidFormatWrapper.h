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
	 * @ ��ʼ������
	*/
	virtual s16 FFmpegFormatInitVideoParamter(const s8 *pchFile, const s8 *pchFormat, u16 keyFramNum);

	/*
	* @ ����Ƶ�����ʽ�������Ƶ��
	*/
	virtual s16 FFmpegFormatAddVidOutputStream();

	/*
	* @ �����Ƶ��ͷ��Ϣ
	*/
	virtual s16 FFmpegFormatWriteStreamHeader(void *ptCodecCtx);

	/*
	* @ �����Ƶ����д���ļ���������Ƶ����
	*/
	virtual s16 FFmpegFormatWriteOutputPacket(void *pPkt);

	/*
	* @ �����Ƶ��β��Ϣ
	*/
	virtual s16 FFmpegFormatWriteStreamTrailer();

	/*
	* @ �ر���Ƶ��װ��
	*/
	void FFmpegFormatCloseVideoFormat();

	/*
	* @ ��������ٶ�
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
	u16             m_byKeyFrameNum;  // ����ؼ�֡��Ŀ
	u16             m_byAddFrameNum;

	/* print report speed */
	int64_t        m_timerStart;
	int64_t        m_curTime;
	int64_t        m_curPts;

	BOOL32          m_bCreate;
	BOOL32          m_bOutputFlag;   // �Ƿ������
};
#endif
