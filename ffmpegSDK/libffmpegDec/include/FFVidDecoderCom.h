#ifndef __H_FFMPEG_VIDEO_DECODER_COM_H_H
#define __H_FFMPEG_VIDEO_DECODER_COM_H_H


#include "FFmpegVidDecoderCommon.h"


class  CFFVideoDecoderCom
{
public:

	CFFVideoDecoderCom() = default;
	virtual ~CFFVideoDecoderCom() = default;

public:
	/*
	* @ ��ʼ����Ƶ����������
	*/
	virtual s16 FFDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam) = 0;

	/*
	* @ �򿪽�����
	*/
	virtual s16 FFDecOpenDecoder(AVCodecParameters *ptCodecPar) = 0;

	/*
	* @ ��ȡ��Ƶ������
	*/
	virtual AVCodecContext* FFDecGetCodecCtx() = 0;

	/*
	* @ ����һ֡��Ƶ
	*/
	virtual s16 FFDecDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen) = 0;

	/*
	* @ ������Ƶ������
	*/
	virtual void FFDecCloseDecoder() = 0;
};

#endif