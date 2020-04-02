#ifndef __H_FFMPEG_VIDEO_ENCODER_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_ENCODER_WRAPPER_H_H

#include "FFmpegCommon.h"

#define ERR_BUF_SIZE 1024

struct FFVideoYUVFrame;
typedef struct FFVideoYUVFrame TFFVideoYUVFrame;

struct FFVideoEncParam;
typedef struct FFVideoEncParam TFFVideoEncParam;


class CFFmpegVideoEncoderWrapper
{
public:

	CFFmpegVideoEncoderWrapper();
	virtual ~CFFmpegVideoEncoderWrapper();

protected:
	/*
	* @ ��ʼ����Ƶ����������
	* *ptVidEncParam: ��Ƶ�������ò���
	* *pchCodecName�����������ƣ�libx264/h264_qsv��
	* emPixFmt:�������ظ�ʽ��AV_PIX_FMT_YUV420P/AV_PIX_FMT_QSV��
	*/
	virtual s16 FFmpegInitVideoEncodeParameter(const TFFVideoEncParam *ptVidEncParam, 
		                                       const s8 *pchCodecName,
		                                       enum AVPixelFormat emPixFmt);

	/*
	* @ ����Ƶ������
	*/
	virtual s16 FFmpegOpenVideoEncoder();

	/*
	* @ ����һ֡��Ƶ
	*/
	virtual s16 FFmpegEncodeOneFrame(u8 *pbyOutData, u32 &dwOutLen, 
		                             AVFrame *ptVidEncFrm, BOOL32 &bKeyFlag);

	/*
	* @ ��ȡ�������Ƶ����
	*/
	virtual AVPacket* FFmpegGetVideoAVPacket();
	/*
	* @ ����һ֡��Ƶǰ,����AVFrame
	*/
	virtual AVFrame* FFmpegMallocAVFrame();

	/*
	* @ ����һ֡ǰ����ȡ֡buffer�ڴ�
	*/
	virtual s16 FFmpegGetAVFrameBuffer(AVFrame *ptFrm, enum AVPixelFormat emPixFmt,
		                               u32 dwWidth, u32 dwHeight);

	/*
	* @ ����һ֡��Ƶǰ,����AVPacket
	*/
	virtual AVPacket* FFmpegMallocAVPacket();

	/*
	* @ ��������ת��ΪAVFrame��װ��ʽ
	*/
	virtual s16 FFmpegConvertVideoToAVFrame(const TFFVideoYUVFrame *ptVidRawData);

	/*
	* @ ������Ƶ������
	*/
	virtual void FFmpegCloseVideoEncoder();

	/*
	* @ ������Ƶ����ؼ�֡
	*/
	virtual void FFmpegSetRequestIDR();

	/*
	* @ ������Ƶ���֡���ظ�ʽ
	*/
	virtual void FFmpegSetOutPixFormat(enum AVPixelFormat emSWPixFmt);

	/*
	* @ ��ʾ�����������Ϣ
	*/
	virtual void FFmpegDumpEncoderContext();

	/*
	 * @���������ظ�ʽתΪAV���ظ�ʽ
	*/
	virtual enum AVPixelFormat FFmpegConvertInputFmtToAVPixFmt(em_FFVidPixIdx emInputPixIdx);

private:

	/*
	 * @��������ת����
	*/
	virtual s16 FFmpegCreatePicSwsContext(enum AVPixelFormat emPixFmt, u32 dwWidth, u32 dwHeight);
 
	/*
	 * @ִ������ת����
	*/
	virtual s16 FFmpegRunPicSwsScale(AVFrame *ptFrm);

protected:

	AVCodecContext *m_ptVidEncCodecCtx;
	AVFrame        *m_ptVidEncFrm;
	BOOL32          m_bCreate;

	/* store error info */
	s8 m_chErrBuf[ERR_BUF_SIZE];

private:

	AVPacket       *m_ptVidEncPkt;
	AVCodec        *m_ptVidEncCodec;
	enum AVPixelFormat m_emInPixFmt;
	BOOL32         m_bReqIDR;

	enum AVPixelFormat m_emOutPixFmt;

	/* scale convert */
	struct SwsContext *m_ptPicSwsCtx;
	AVFrame           *m_ptVidCntFrm;
};
#endif