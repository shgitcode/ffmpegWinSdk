#include "HWVidDecoderWrapper.h"
#include "SWVidDecoderWrapper.h"
#include "FFVidDecoderObj.h"

CFFVideoDecoderObj::CFFVideoDecoderObj()
{
	m_tFFVidDec.reset(nullptr);
}


CFFVideoDecoderObj::~CFFVideoDecoderObj()
{
	FFDecObjCloseHandle();
}


s16 CFFVideoDecoderObj::FFDecObjInitParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		return -1;
	}

	if (ptVidDecParam->m_emDecType == FF_VID_HW_DEC_TYPE_NONE) // software
	{
		m_tFFVidDec.reset(new CFFSWVideoDecoderWrapper());
	}
	else // hardware
	{
		m_tFFVidDec.reset(new CFFHWVideoDecoderWrapper());
	}

	if (!m_tFFVidDec.get())
	{
		return -1;
	}

	return m_tFFVidDec->FFDecInitDecoderParameter(ptVidDecParam);

}


s16 CFFVideoDecoderObj::FFDecObjOpenHandle(AVCodecParameters *ptCodecPar)
{
	if (m_tFFVidDec.get())
	{
		return m_tFFVidDec->FFDecOpenDecoder(ptCodecPar);
	}

	return -1;
}

AVCodecContext* CFFVideoDecoderObj::FFDecObjGetCodecCtx()
{
	if (m_tFFVidDec.get())
	{
		return m_tFFVidDec->FFDecGetCodecCtx();
	}

	return nullptr;
}

s16 CFFVideoDecoderObj::FFDecObjDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen)
{
	if (m_tFFVidDec.get())
	{
		return m_tFFVidDec->FFDecDecodeAVPacket(pPkt, pbyOutData, dwOutLen);
	}

	return -1;
}

void CFFVideoDecoderObj::FFDecObjCloseHandle()
{
	if (m_tFFVidDec.get())
	{
		m_tFFVidDec->FFDecCloseDecoder();
	}
}