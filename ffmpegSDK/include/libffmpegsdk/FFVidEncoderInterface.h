#ifndef __H_FF_VIDEO_ENCODER_INTERFACE_H_H
#define __H_FF_VIDEO_ENCODER_INTERFACE_H_H

#include "FFVidEncoderType.h"

#ifdef FFVIDEOENCODEROBJ_EXPORTS
#define FF_VIDEO_ENCODER_API _declspec(dllexport)
#else
#define FF_VIDEO_ENCODER_API _declspec(dllimport)
#endif

class CFFVidEndFormatObj;

class FF_VIDEO_ENCODER_API CFFVidEncoderInterface
{
public:
	CFFVidEncoderInterface();

	virtual ~CFFVidEncoderInterface();

public:
	virtual s16	InitFFVideoEncoder(em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam);

	virtual void CloseFFVideoEncoder();

	virtual s16	EncodeFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData);

	virtual s16 FlushFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag);

	virtual s16 RequestKeyFrame();

protected:
	CFFVidEndFormatObj  *m_pFFVidEndFmtObj;
};

/*
* create video encoder instance
* return instance or NULL
*/
extern "C" FF_VIDEO_ENCODER_API void* FFVideoEnc_CreateInstance();

/*
* destory video encoder instance
* return -1(failed) or other value(success)
*/
extern "C" FF_VIDEO_ENCODER_API s16	  FFVideoEnc_DestoryInstance(void* pHandle);

/*
* init video encoder codec
* return -1(failed) or 0(success)
*/
extern "C" FF_VIDEO_ENCODER_API s16   FFVideoEnc_InitEncoder(void* pHandle, em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam);

/*
* encode one frame each time
* return value:
*             < 0  : error
*               0  : no error == continue
*               1  : EOF
*               2  : must be packet
*/
extern "C" FF_VIDEO_ENCODER_API s16   FFVideoEnc_EncodeFrame(void* pHandle, u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData);

/*
* flush one frame each time
* return value:
*             < 0  : error
*               0  : no error == continue
*               1  : EOF
*               2  : must be packet
*/
extern "C" FF_VIDEO_ENCODER_API s16   FFVideoEnc_FlushFrame(void* pHandle, u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag);

/*
* close video encoder codec
* no return value
*/
extern "C" FF_VIDEO_ENCODER_API void  FFVideoEnc_CloseEncoder(void* pHandle);

/*
* force to request to encode key frame
* return -1(failed) or 0(success)
*/
extern "C" FF_VIDEO_ENCODER_API s16   FFVideoEnc_RequestIDR(void* pHandle);

#endif
