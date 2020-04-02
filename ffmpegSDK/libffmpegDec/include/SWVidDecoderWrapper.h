#ifndef __H_SW_FFMPEG_VIDEO_DECODER_WRAPPER_H_H
#define __H_SW_FFMPEG_VIDEO_DECODER_WRAPPER_H_H


#include "FFmpegVidDecoderCommon.h"
#include "FFVidDecoderCom.h"
#include "FFmpegVidDecoderWrapper.h"


class  CFFSWVideoDecoderWrapper : virtual public CFFmpegVideoDecoderWrapper,
	                              virtual public  CFFVideoDecoderCom
{
public:

	CFFSWVideoDecoderWrapper();
	virtual ~CFFSWVideoDecoderWrapper();

public:
	/*
	* @ ��������ʼ����Ƶ����������
	*/
	virtual s16 FFDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam) override;

	/*
	* @ �򿪽�����
	*/
	virtual s16 FFDecOpenDecoder(AVCodecParameters *ptCodecPar) override;

	/*
	* @ ��ȡ��Ƶ������
	*/
	virtual AVCodecContext* FFDecGetCodecCtx() override;

	/*
	* @ ����һ֡��Ƶ
	*/
	virtual s16 FFDecDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen) override;

	/*
	* @ ������Ƶ������
	*/
	virtual void FFDecCloseDecoder() override;
};

#endif