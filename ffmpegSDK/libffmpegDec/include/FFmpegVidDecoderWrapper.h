#ifndef __H_FFMPEG_VIDEO_DECODER_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_DECODER_WRAPPER_H_H


#include "FFmpegVidDecoderCommon.h"


class  CFFmpegVideoDecoderWrapper
{
public:

	CFFmpegVideoDecoderWrapper();
	virtual ~CFFmpegVideoDecoderWrapper();

protected:
	/*
	* @ ��ʼ����Ƶ����������
	*/
	virtual s16 FFmpegDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam);

	/*
	* @ ����������
	*/
	virtual s16 FFmpegDecCreateDecoder(enum AVCodecID codecId);

	/*
	* @ �򿪽�����
	*/
	virtual s16 FFmpegDecOpenDecoder(AVCodecParameters *ptCodecPar);

	/*
	* @ ����һ֡��Ƶ
	*/
	virtual s16 FFmpegDecDecodeAVPacket(AVPacket *pkt);

	/*
	* @ ����֡ת����
	*/
	virtual s16 FFmpegDecCreatePicSwsContext(enum AVPixelFormat emPixFmt, u32 dwWidth, u32 dwHeight);

	/*
	* @ ִ��֡ת����
	*/
	virtual s16 FFmpegDecRunPicSwsScale(AVFrame *pFrm);

	/*
	* @ ����AVFrame
	*/
	virtual AVFrame* FFmpegDecMallocAVFrame();

	/*
	* @ ����AVFrame��buffer
	*/
	virtual s16 FFmpegDecGetAVFrameBuffer(AVFrame *pFrame, enum AVPixelFormat emPixFmt,
		u32 dwWidth, u32 dwHeight);
	
	/*
	* @ ����һ֡��Ƶ
	*/
	virtual s16 FFmpegDecCopyAVFrame(AVFrame *pFrm, u8 *pbyOutData, u32 &dwOutLen);

	/*
	* @ ������Ƶ������
	*/
	virtual void FFmpegDecCloseDecoder();

private:

	s16 FFmpegCheckInputParam(const TFFVideoDecoderParam *ptDecParam);

	s16 FFmpegConvertPixFmt(const TFFVideoDecoderParam *ptDecParam);

	void FFmpegDumpDecoderParam();

protected:

	AVCodecContext  *m_ptVidDecCodecCtx;
	AVCodec         *m_ptVidDecCodec;

	AVFrame        *m_ptVidDecFrm;
	AVFrame        *m_ptVidCntFrm;   // ��m_ptVidDecFrmת��Ϊ֡���ָ����ʽ

	s8 m_chErrBuf[ERR_BUF_SIZE];

private:

	/* scale convert */
	struct SwsContext *m_ptPicSwsCtx;

	u32 m_dwOutPicWidth;   // �����ͼ���
	u32 m_dwOutPicHeight;  // �����ͼ���
	enum AVPixelFormat   m_emOutPixFmt; // �����ͼ�����ظ�ʽ

	std::string m_tCodecName;
};

#endif
