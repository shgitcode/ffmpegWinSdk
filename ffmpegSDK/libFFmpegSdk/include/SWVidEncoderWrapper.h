#ifndef __H_SW_VIDEO_ENCODER_WRAPPER_H_H
#define __H_SW_VIDEO_ENCODER_WRAPPER_H_H

#include "FFmpegVidEncoderWrapper.h"
#include "FFmpegVidEncoderCom.h"

class CSWVideoEncoderWrapper : virtual public CFFmpegVideoEncoderWrapper,
	                           virtual public CFFmpegVideoEncoderCom
{
public:

	CSWVideoEncoderWrapper();
	virtual ~CSWVideoEncoderWrapper();

public:
	/*
	* @ ��ʼ��������
	*/
	virtual s16	InitFFVideoEncoderCodecCtxCom(const TFFVideoEncParam *ptVdioEncParam,
		                              enum em_FFVideoEncoderType emVidType) override;

	/*
	* @ �򿪱�����
	*/
	virtual s16 OpenFFVideoEncoderCom() override;


	/*
	* @ ��ȡ������������
	*/
	virtual void* GetFFVideoEncoderCom() override;


	/*
	* @ ���ٱ�����
	*/
	virtual void CloseFFVideoEncoderCom() override;

	/*
	* @ ����һ֡
	*/
	virtual s16	EncodeFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, 
		                              const TFFVideoYUVFrame *pTVidRawData) override;

	/*
	* @ ˢ�±�����(ͣ�����������п��ܻỺ�漸֡���ݻ���)
	*/
	virtual s16 FlushFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag) override;

	/*
	* @ ��ȡ���������
	*/
	virtual void* GetFFVideoAVPacketCom() override;

	/*
	* @ �������һ���ؼ�֡
	*/
	virtual void RequestFFVideoKeyFrameCom() override;

private:

	/*
	* @ �򿪱�����ǰ�������������ز���
	*/
	virtual s16 FFmpegSetAttrBeforeOpenVideoEncoder(const TFFVideoEncParam *ptVidEncParam);

private:
	s16 m_emCodecIndx;
	enum AVPixelFormat m_emSWPixFmt;
};

#endif