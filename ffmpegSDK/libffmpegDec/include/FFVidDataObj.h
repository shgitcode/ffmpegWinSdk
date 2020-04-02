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
	* @ 初始化操作数据流参数
	*/
	virtual s16 FFDataObjInitParameter(const TFFVideoDecoderParam *ptVidDecParam);

	/*
	* @ 打开数据流句柄
	*/
	virtual s16 FFDataObjOpenHandle();

	/*
	* @ 获取CodecParam,须在打开数据流之后
	*/
	virtual AVCodecParameters* FFDataObjGetCodecParam();

	/*
	* @ 解析一帧视频数据
	*/
	virtual s16 FFDataObjParsePacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen);

	/*
	* @ 获取一帧视频数据
	*/
	virtual AVPacket* FFDataObjGetAVPacket();

	/*
	* @ 解关联一帧视频数据
	*/
	virtual void FFDataObjUnrefAVPacket();

	/*
	* @ 关闭数据流句柄
	*/
	virtual void FFDataObjCloseHandle();

private:
	std::unique_ptr<CFFVideoDataCom> m_tFFVidData;
	em_FFVidDataType m_bDirectData;
};

#endif