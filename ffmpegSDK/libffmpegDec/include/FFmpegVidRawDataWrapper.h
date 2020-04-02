#ifndef __H_FFMPEG_VIDEO_RAW_DATA_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_RAW_DATA_WRAPPER_H_H

#include "FFVidDataCom.h"
#include "FFmpegVidDecoderCommon.h"


class  CFFmpegVideoRawDataWrapper: virtual public CFFVideoDataCom
{
public:

	CFFmpegVideoRawDataWrapper();
	virtual ~CFFmpegVideoRawDataWrapper();

public:
	/*
	* @ ��ʼ����Ƶ���װ����
	*/
	virtual s16 FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecoderParam) override;

	/*
	* @ ��������
	*/
	virtual s16 FFDataOpenDataStream() override;

	/*
	* @ ��ȡCodecParam,���ڴ�������֮��
	*/
	virtual AVCodecParameters* FFDataGetCodecParam() override;

	/*
	* @ ���װһ֡��Ƶ
	*/
	virtual s16 FFDataReadPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen) override;

	/*
	* @ ��ȡһ֡��Ƶ
	*/
	virtual AVPacket* FFDataGetAVPacket() override;

	/*
	* @ �����һ֡��Ƶ
	*/
	virtual void FFDataUnrefAVPacket() override;

	/*
	* @ �ر�������
	*/
	virtual void FFDataCloseDataStream() override;

private:

	s16 FFRawInitParameter(const TFFVideoDecoderParam *ptVidDec);
	
	s16 FFRawCreateParserCtx();

	/* need AVCodecContext */
	s16 FFRawParseToAVPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen);

	AVPacket* FFRawGetAVPacket();

	void FFRawCloseParserCtx();

private:

	s16 FFmpegRawConvertCodecId(const TFFVideoDecoderParam *ptDecParam);

	s16 FFmpegRawPutPacketToList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd, AVPacket *pPkt);

	void FFmpegRawFreePacketList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd);

	s16 FFmpegRawGetPacketFromList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd, AVPacket *pPkt);

private:

	enum AVCodecID  m_emCodecId;

	AVCodecParserContext *m_ptCodecParserCtx;

	struct AVPacketList *m_ptPktBuffer;
	struct AVPacketList *m_ptPktBufferEnd;

	AVPacket       *m_ptVidDecPkt;

	s8 m_chErrBuf[ERR_BUF_SIZE];
};

#endif
