#ifndef __H_FF_VID_ENCODER_FORMAT_OBJ_H_H
#define __H_FF_VID_ENCODER_FORMAT_OBJ_H_H

#include <memory>
#include <mutex>
#include "FFVidEncoderType.h"

class CFFmpegVideoFormatWrapper;
class CFFVideoEncoderObj;

class CFFVidEndFormatObj
{
public:
	CFFVidEndFormatObj();

	virtual ~CFFVidEndFormatObj();

public:
	virtual s16	InitFFVidFormatEncoder(em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam);

	virtual void CloseFFVidFormatEncoder();

	virtual s16	EncodeFFVidFormatVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData);

	virtual s16 FlushFFVidFormatVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag);

	virtual s16 RequestVidFormatKeyFrame();

protected:
	std::mutex m_FFMutex;
	BOOL32     m_bCreate;

	std::unique_ptr<CFFVideoEncoderObj> m_pFFVidEncoder;

#ifdef FF_ADD_FORMAT
	std::unique_ptr<CFFmpegVideoFormatWrapper> m_pFFVidFormat;
#endif

};

#endif