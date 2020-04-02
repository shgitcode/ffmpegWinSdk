#ifndef __H_HW_VIDEO_ENCODER_WRAPPER_H_H
#define __H_HW_VIDEO_ENCODER_WRAPPER_H_H

#include "FFmpegVidEncoderWrapper.h"
#include "FFmpegVidEncoderCom.h"

class CHWVideoEncoderWrapper :virtual public CFFmpegVideoEncoderWrapper,
	                          virtual public CFFmpegVideoEncoderCom
{
public:
	CHWVideoEncoderWrapper();
	virtual ~CHWVideoEncoderWrapper();

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
	* @ �򿪱�����ǰ����ȡHW�豸������HW֡
	*/
	s16 FFmpegSetHWDeviceCtx(enum AVHWDeviceType emHWDevType);

	/*
	* @ ����һ֡ǰ����ȡHW֡buffer�ڴ�
	*/
	s16 FFmpegGetHWFrameBuffer();

	/*
	* @ ����һ֡ǰ����SW֡ת��ΪHW֡
	*/
	s16 FFmpegTransToHWFrame();

	/*
	* @ ����һ֡ǰ��ΪHW֡��������ڴ�
	*/
	s16 FFmpegSetHWFrameCtx(AVCodecContext *ctx, AVBufferRef *ptHWDevCtx);

	/*
	 * �ͷ�HW�豸��֡�ڴ�
	*/
	void FFmpegFreeHWContext();

private:

	AVBufferRef    *m_ptHWDevCtx;
	AVFrame        *m_ptHWVidEncFrm;
	enum em_VideoHWCodecType   m_emHWCodecType;
};
#endif

