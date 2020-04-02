
#include "FFVidEncoderObj.h"
#include "HWVidEncoderWrapper.h"
#include "SWVidEncoderWrapper.h"


CFFVideoEncoderObj::CFFVideoEncoderObj()
{
	m_pFFVidCodecCom.reset(nullptr);
	m_emFFVideoType    = em_FF_ENCNONE;
}

CFFVideoEncoderObj::~CFFVideoEncoderObj()
{
	CloseFFVideoEncoder();
}

s16	CFFVideoEncoderObj::InitFFVideoEncoder(em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam)
{
	s16 s16Ret = -1;

	if (!ptVdioEncParam || emType >= em_FF_ENCNONE)
	{
		return -1;
	}

	if ((m_emFFVideoType == emType) && m_pFFVidCodecCom.get())
	{
		return 0;
	}

	if (m_pFFVidCodecCom.get())
	{
		CloseFFVideoEncoder();
	}

	switch (emType)
	{
	case em_FF_SW:
		m_pFFVidCodecCom.reset( new CSWVideoEncoderWrapper );
		//m_pFFVidCodecCom = std::make_unique<CSWVideoEncoderWrapper>();
		break;

	default:
		m_pFFVidCodecCom.reset(new CHWVideoEncoderWrapper);
		break;
	}

	if (m_pFFVidCodecCom.get())
	{
		s16Ret = m_pFFVidCodecCom->InitFFVideoEncoderCodecCtxCom(ptVdioEncParam, emType);
	}

	if (0 == s16Ret)
	{
		m_emFFVideoType = emType;
	}

	return s16Ret;
}

s16	CFFVideoEncoderObj::OpenFFVideoEncoder()
{
	if (m_pFFVidCodecCom.get())
	{
		return m_pFFVidCodecCom->OpenFFVideoEncoderCom();
	}

	return -1;
}

void* CFFVideoEncoderObj::GetFFVideoEncoder()
{
	if (m_pFFVidCodecCom.get())
	{
		return m_pFFVidCodecCom->GetFFVideoEncoderCom();
	}

	return nullptr;
}

void CFFVideoEncoderObj::CloseFFVideoEncoder()
{
	if (m_pFFVidCodecCom.get())
	{
		m_pFFVidCodecCom->CloseFFVideoEncoderCom();
	}

	m_pFFVidCodecCom.reset(nullptr);

	m_emFFVideoType = em_FF_ENCNONE;
}


s16	CFFVideoEncoderObj::EncodeFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData)
{
	if (m_pFFVidCodecCom.get())
	{
		return m_pFFVidCodecCom->EncodeFFVideoFrameCom(pbyOutData, dwOutLen, bKeyFlag, pTVidRawData);
	}

	return -1;
}


s16 CFFVideoEncoderObj::FlushFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag)
{
	if (m_pFFVidCodecCom.get())
	{
		return m_pFFVidCodecCom->FlushFFVideoFrameCom(pbyOutData, dwOutLen, bKeyFlag);
	}

	return -1;
}

void* CFFVideoEncoderObj::GetFFVideoAVPacket()
{
	if (m_pFFVidCodecCom.get())
	{
		return m_pFFVidCodecCom->GetFFVideoAVPacketCom();
	}

	return nullptr;

}

s16 CFFVideoEncoderObj::RequestKeyFrame()
{
	if (m_pFFVidCodecCom.get())
	{
		m_pFFVidCodecCom->RequestFFVideoKeyFrameCom();

		return 0;
	}

	return -1;
}
