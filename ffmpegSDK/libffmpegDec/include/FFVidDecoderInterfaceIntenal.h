#ifndef __H_FF_VIDEO_DECODER_INTERFACE_INTENAL_H_H
#define __H_FF_VIDEO_DECODER_INTERFACE_INTENAL_H_H

#include "FFVideoDecoderType.h"

class CFFVidDataDecoderObj;

class  CFFVidDecoderInterface
{
public:
	CFFVidDecoderInterface();

	virtual ~CFFVidDecoderInterface();

public:
	virtual s16	InitFFVideoDecoder(const TFFVideoDecoderParam *ptVidDecParam);

	virtual void CloseFFVideoDecoder();

	virtual s16	DecodeFFVideoFrame(u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen);

	virtual s16 FlushFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen);

private:
	CFFVidDataDecoderObj  *m_pFFVidDataDecObj;
};

#endif
