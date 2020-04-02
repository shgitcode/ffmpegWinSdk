#ifndef __H_FFMPEG_VIDEO_DATA_COM_H_H
#define __H_FFMPEG_VIDEO_DATA_COM_H_H


#include "FFmpegVidDecoderCommon.h"


class  CFFVideoDataCom
{
public:

	CFFVideoDataCom() = default;
	virtual ~CFFVideoDataCom() = default;

public:

	/*
	* @ ��ʼ����Ƶ���װ����
	*/
	virtual s16 FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecoderParam) = 0;

	/*
	* @ ��������
	*/
	virtual s16 FFDataOpenDataStream() = 0;

	/*
	* @ ��ȡCodecParam,���ڴ�������֮��
	*/
	virtual AVCodecParameters* FFDataGetCodecParam() = 0;

	/*
	* @ ���װһ֡��Ƶ
	*/
	virtual s16 FFDataReadPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen) = 0;

	/*
	* @ ��ȡһ֡��Ƶ
	*/
	virtual AVPacket* FFDataGetAVPacket() = 0;

	/*
	* @ �����һ֡��Ƶ
	*/
	virtual void FFDataUnrefAVPacket() = 0;

	/*
	* @ �ر�������
	*/
	virtual void FFDataCloseDataStream() = 0;
};

#endif
