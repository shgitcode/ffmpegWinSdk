#ifndef __H_FFMPEG_VIDEO_DATA_OBJ_H_H
#define __H_FFMPEG_VIDEO_DATA_OBJ_H_H


#include "FFmpegVidDecoderCommon.h"

class  CFFVideoDataCom;

class  CFFVideoDataObj
{
public:

	CFFVideoDataObj();
	virtual ~CFFVideoDataObj();

public:
	/*
	* @ ��ʼ����������������
	*/
	virtual s16 FFDataObjInitParameter(const TFFVideoDecoderParam *ptVidDecParam);

	/*
	* @ �����������
	*/
	virtual s16 FFDataObjOpenHandle();

	/*
	* @ ��ȡCodecParam,���ڴ�������֮��
	*/
	virtual AVCodecParameters* FFDataObjGetCodecParam();

	/*
	* @ ����һ֡��Ƶ����
	*/
	virtual s16 FFDataObjParsePacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen);

	/*
	* @ ��ȡһ֡��Ƶ����
	*/
	virtual AVPacket* FFDataObjGetAVPacket();

	/*
	* @ �����һ֡��Ƶ����
	*/
	virtual void FFDataObjUnrefAVPacket();

	/*
	* @ �ر����������
	*/
	virtual void FFDataObjCloseHandle();

private:
	std::unique_ptr<CFFVideoDataCom> m_tFFVidData;
	em_FFVidDataType m_bDirectData;
};

#endif