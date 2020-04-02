#ifndef __H_FF_VIDEO_DATA_DECODER_OBJ_H_H
#define __H_FF_VIDEO_DATA_DECODER_OBJ_H_H

#include <memory>
#include "FFmpegVidDecoderCommon.h"

class  CFFVideoDataObj;
class  CFFVideoDecoderObj;

class  CFFVidDataDecoderObj
{
public:
	CFFVidDataDecoderObj();

	virtual ~CFFVidDataDecoderObj();

public:

	virtual s16	InitFFDataVideoDecoder(const TFFVideoDecoderParam *ptVidDecParam);

	virtual void CloseFFDataVideoDecoder();

	virtual s16	DecodeFFDataVideoFrame(u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen);

	virtual s16 FlushFFDataVideoFrame(u8 *pbyOutData, u32 &dwOutLen);

private:

	std::unique_ptr<CFFVideoDecoderObj>  m_tFFVidDecObj;
	std::unique_ptr<CFFVideoDataObj> m_tFFVidDataObj;

	std::mutex m_tFFMutex;

	BOOL32     m_bCreate;

	em_FFVidDataType m_bDirectData;

};

#endif