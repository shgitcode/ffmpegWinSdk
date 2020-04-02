#ifndef __H_FF_VIDEO_ENCODER_OBJ_H_H
#define __H_FF_VIDEO_ENCODER_OBJ_H_H

#include <memory>
#include "FFVidEncoderType.h"


class CFFmpegVideoEncoderCom;

class  CFFVideoEncoderObj
{
public:
	CFFVideoEncoderObj();

	virtual ~CFFVideoEncoderObj();

public:
	virtual s16	InitFFVideoEncoder(em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam);

	virtual s16 OpenFFVideoEncoder();

	virtual void *GetFFVideoEncoder();

	virtual void CloseFFVideoEncoder();

	virtual s16	EncodeFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData);

	virtual s16 FlushFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag);

	virtual void *GetFFVideoAVPacket();

	virtual s16 RequestKeyFrame();

protected:
	std::unique_ptr<CFFmpegVideoEncoderCom> m_pFFVidCodecCom;
	em_FFVideoEncoderType    m_emFFVideoType;
};

#endif
