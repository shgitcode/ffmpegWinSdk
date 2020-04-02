#ifndef __H_FF_VIDEO_DECODER_INTERFACE_H_H
#define __H_FF_VIDEO_DECODER_INTERFACE_H_H

#include "FFVideoDecoderType.h"

#ifdef FFVIDEODECODEROBJ_EXPORTS
#define FF_VIDEO_DECODER_API _declspec(dllexport)
#else
#define FF_VIDEO_DECODER_API _declspec(dllimport)
#endif

/*
* create video Decoder instance
* return instance or NULL
*/
extern "C" FF_VIDEO_DECODER_API void* FFVideoDec_CreateInstance();

/*
* destory video Decoder instance
* return -1(failed) or other value(success)
*/
extern "C" FF_VIDEO_DECODER_API s16	  FFVideoDec_DestoryInstance(void* pHandle);

/*
* init video Decoder codec
* return -1(failed) or 0(success)
*/
extern "C" FF_VIDEO_DECODER_API s16   FFVideoDec_InitDecoder(void* pHandle, const TFFVideoDecoderParam *ptVidDecParam);

/*
* decode one frame each time
* return value:
*             < 0  : error
*               0  : no error == continue
*               1  : EOF
*               2  : must be frame
*/
extern "C" FF_VIDEO_DECODER_API s16   FFVideoDec_DecodeFrame(void* pHandle, u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen);

/*
* flush one frame each time
* return value:
*             < 0  : error
*               0  : no error == continue
*               1  : EOF
*               2  : must be frame
*/
extern "C" FF_VIDEO_DECODER_API s16   FFVideoDec_FlushFrame(void* pHandle, u8 *pbyOutData, u32 &dwOutLen);

/*
* close video Decoder codec
* no return value
*/
extern "C" FF_VIDEO_DECODER_API void  FFVideoDec_CloseDecoder(void* pHandle);

#endif