#ifndef __H_FFMPEG_VIDEO_DECODER_OBJ_H_H
#define __H_FFMPEG_VIDEO_DECODER_OBJ_H_H

#include "FFmpegVidDecoderCommon.h"

class  CFFVideoDecoderCom;

class  CFFVideoDecoderObj
{
public:

	CFFVideoDecoderObj();
	virtual ~CFFVideoDecoderObj();

public:
	/*
	* @ ��ʼ����Ƶ����������
	*/
	virtual s16 FFDecObjInitParameter(const TFFVideoDecoderParam *ptVidDecParam);

	/*
	* @ �򿪽�����
	*/
	virtual s16 FFDecObjOpenHandle(AVCodecParameters *ptCodecPar);

	/*
	* @ ��ȡ��Ƶ������
	*/
	virtual AVCodecContext* FFDecObjGetCodecCtx();

	/*
	* @ ����һ֡��Ƶ
	*/
	virtual s16 FFDecObjDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen);

	/*
	* @ �ر���Ƶ������
	*/
	virtual void FFDecObjCloseHandle();

private:
	std::unique_ptr<CFFVideoDecoderCom> m_tFFVidDec;
};

#endif
