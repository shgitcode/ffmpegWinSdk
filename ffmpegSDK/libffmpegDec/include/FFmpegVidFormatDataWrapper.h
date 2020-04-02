#ifndef __H_FFMPEG_VIDEO_FORMAT_DATA_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_FORMAT_DATA_WRAPPER_H_H

#include "FFVidDataCom.h"
#include "FFmpegVidDecoderCommon.h"


class  CFFmpegVideoFormatDataWrapper : virtual public CFFVideoDataCom
{
public:

	CFFmpegVideoFormatDataWrapper();
	virtual ~CFFmpegVideoFormatDataWrapper();

public:
	/*
	* @ ��ʼ����Ƶ���װ����
	*/
	virtual s16 FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecParam) override;

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

protected:
	virtual 	s16 FFFormatOpenInputFile(AVInputFormat *ptInFmt, AVDictionary *ptOpt);

private:

	s16 FFFormatInitParameter(const TFFVideoDecoderParam *ptVidDecParam);
	s16 FFFormatReadAVPacket();
	void FFFormatCloseFormatCtx();

public:
	AVFormatContext *m_ptVidIFmtCtx;

private:

	AVCodecParameters *m_ptCodecPar;

	u32             m_dwVidStmIdx;

	AVPacket       *m_ptVidDecPkt;

	std::string     m_tInputFile;

	s8 m_chErrBuf[ERR_BUF_SIZE];
};

#endif

