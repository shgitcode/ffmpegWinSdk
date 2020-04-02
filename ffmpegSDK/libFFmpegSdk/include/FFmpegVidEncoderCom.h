#ifndef __H_FFMPEG_VIDEO_ENCODER_COM_H_H
#define __H_FFMPEG_VIDEO_ENCODER_COM_H_H

#include "kdvtype.h"

struct FFVideoYUVFrame;
typedef struct FFVideoYUVFrame TFFVideoYUVFrame;

struct FFVideoEncParam;
typedef struct FFVideoEncParam TFFVideoEncParam;

class CFFmpegVideoEncoderCom
{
public:
	CFFmpegVideoEncoderCom() {};

	virtual ~CFFmpegVideoEncoderCom() {};

public:
	/*
	* @ ��ʼ��������
	*/
	virtual s16	InitFFVideoEncoderCodecCtxCom(const TFFVideoEncParam *ptVdioEncParam, enum em_FFVideoEncoderType emVidType) = 0;

	/*
	* @ �򿪱�����
	*/
	virtual s16 OpenFFVideoEncoderCom() = 0;

	/*
	* @ ��ȡ������������
	*/
	virtual void* GetFFVideoEncoderCom() = 0;

	/*
	* @ ���ٱ�����
	*/
	virtual void CloseFFVideoEncoderCom() = 0;

	/*
	* @ ����һ֡
	*/
	virtual s16	EncodeFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData) = 0;

	/*
	* @ ˢ�±�����(ͣ�����������п��ܻỺ�漸֡���ݻ���)
	*/
	virtual s16 FlushFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag) = 0;

	/*
	* @ ��ȡ���������
	*/
	virtual void* GetFFVideoAVPacketCom() = 0;

	/*
	* @ �������һ���ؼ�֡
	*/
	virtual void RequestFFVideoKeyFrameCom() = 0;
};

#endif
