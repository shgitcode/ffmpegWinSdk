#ifndef __H_FFMPEG_VIDEO_FORMAT_DATA_WRAPPER_H_H
#define __H_FFMPEG_VIDEO_FORMAT_DATA_WRAPPER_H_H

#include "FFVidDataCom.h"
#include "FFmpegVidDecoderCommon.h"


class  CFFmpegVideoFormatDataWrapper : virtual public CFFVideoDataCom
{
public:

	CFFmpegVideoFormatDataWrapper();
	virtual ~CFFmpegVideoFormatDataWrapper();

public:
	/*
	* @ 初始化视频解封装参数
	*/
	virtual s16 FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecParam) override;

	/*
	* @ 打开数据流
	*/
	virtual s16 FFDataOpenDataStream() override;

	/*
	* @ 获取CodecParam,须在打开数据流之后
	*/
	virtual AVCodecParameters* FFDataGetCodecParam() override;

	/*
	* @ 解封装一帧视频
	*/
	virtual s16 FFDataReadPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen) override;

	/*
	* @ 获取一帧视频
	*/
	virtual AVPacket* FFDataGetAVPacket() override;

	/*
	* @ 解关联一帧视频
	*/
	virtual void FFDataUnrefAVPacket() override;

	/*
	* @ 关闭数据流
	*/
	virtual void FFDataCloseDataStream() override;

protected:
	virtual 	s16 FFFormatOpenInputFile(AVInputFormat *ptInFmt, AVDictionary *ptOpt);

private:

	s16 FFFormatInitParameter(const TFFVideoDecoderParam *ptVidDecParam);
	s16 FFFormatReadAVPacket();
	void FFFormatCloseFormatCtx();

public:
	AVFormatContext *m_ptVidIFmtCtx;

private:

	AVCodecParameters *m_ptCodecPar;

	u32             m_dwVidStmIdx;

	AVPacket       *m_ptVidDecPkt;

	std::string     m_tInputFile;

	s8 m_chErrBuf[ERR_BUF_SIZE];
};

#endif

