#include "FFVidDataObj.h"
#include "FFVidDecoderObj.h"
#include "FFVidDataDecoderObj.h"

CFFVidDataDecoderObj::CFFVidDataDecoderObj()
{
	m_bCreate = false;

	m_tFFVidDataObj.reset(nullptr);

	m_tFFVidDecObj.reset(nullptr);
}

CFFVidDataDecoderObj::~CFFVidDataDecoderObj()
{
	CloseFFDataVideoDecoder();
}

s16	CFFVidDataDecoderObj::InitFFDataVideoDecoder(const TFFVideoDecoderParam *ptVidDecParam)
{
	s16 s16Ret = 0;

	std::lock_guard<std::mutex> lock(m_tFFMutex);

	if (!ptVidDecParam)
	{
		return -1;
	}

	if (m_bCreate)
	{
		return 0;
	}

	m_bDirectData = ptVidDecParam->m_bDirectData;

	/* check obj is active */
	if (m_tFFVidDataObj.get())
	{
		m_tFFVidDataObj->FFDataObjCloseHandle();
		m_tFFVidDataObj.reset(nullptr);
	}

	if (m_tFFVidDecObj.get())
	{
		m_tFFVidDecObj->FFDecObjCloseHandle();
		m_tFFVidDecObj.reset(nullptr);
	}

	/* create obj */
	m_tFFVidDataObj.reset(new CFFVideoDataObj());
	if (!m_tFFVidDataObj.get())
	{
		return -1;
	}

	m_tFFVidDecObj.reset(new CFFVideoDecoderObj());
	if (!m_tFFVidDecObj.get())
	{
		return -1;
	}

	/* for obj init paramter */
	s16Ret = m_tFFVidDataObj->FFDataObjInitParameter(ptVidDecParam);
	if (s16Ret < 0)
	{
		return -1;
	}

	s16Ret = m_tFFVidDecObj->FFDecObjInitParameter(ptVidDecParam);
	if (s16Ret < 0)
	{
		return -1;
	}

	/* create handle */
	s16Ret = m_tFFVidDataObj->FFDataObjOpenHandle();
	if (s16Ret < 0)
	{
		return -1;
	}

	s16Ret = m_tFFVidDecObj->FFDecObjOpenHandle(m_tFFVidDataObj->FFDataObjGetCodecParam());
	if (s16Ret < 0)
	{
		return -1;
	}

	m_bCreate = true;

	return 0;
}


void CFFVidDataDecoderObj::CloseFFDataVideoDecoder()
{
	std::lock_guard<std::mutex> lock(m_tFFMutex);

	if (!m_bCreate)
	{
		return;
	}

	if (m_tFFVidDecObj.get())
	{
		m_tFFVidDecObj->FFDecObjCloseHandle();
	}

	if (m_tFFVidDataObj.get())
	{
		m_tFFVidDataObj->FFDataObjCloseHandle();
	}

	m_bCreate = false;
}


s16	CFFVidDataDecoderObj::DecodeFFDataVideoFrame(u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen)
{
	s16 s16Ret = 0;

	std::lock_guard<std::mutex> lock(m_tFFMutex);

	if (!m_bCreate)
	{
		return -1;
	}

	if (!pbyOutData)
	{
		return -1;
	}

	if (!m_tFFVidDecObj.get() || !m_tFFVidDataObj.get())
	{
		return -1;
	}

	s16Ret = m_tFFVidDataObj->FFDataObjParsePacket(m_tFFVidDecObj->FFDecObjGetCodecCtx(), pbyInData, dwInLen);
	if (s16Ret < 0)
	{
		return -1;
	}

	if (s16Ret > FF_ERR_NONE)// have one packet or EOF
	{
		s16Ret = m_tFFVidDecObj->FFDecObjDecodeAVPacket(m_tFFVidDataObj->FFDataObjGetAVPacket(), pbyOutData, dwOutLen);
		if (s16Ret < 0)
		{
			return -1;
		}
	}
	else
	{
		dwOutLen = 0;
	}

	return s16Ret;
}


s16 CFFVidDataDecoderObj::FlushFFDataVideoFrame(u8 *pbyOutData, u32 &dwOutLen)
{
	s16 s16Ret = 0;

	std::lock_guard<std::mutex> lock(m_tFFMutex);

	if (!m_bCreate)
	{
		return -1;
	}

	if (!pbyOutData)
	{
		return -1;
	}

	if (!m_tFFVidDecObj.get() || !m_tFFVidDataObj.get())
	{
		return -1;
	}

	if (m_bDirectData == FF_VID_DATA_TYPE_DEV)
	{
		m_tFFVidDataObj->FFDataObjUnrefAVPacket();
	}

	s16Ret = m_tFFVidDecObj->FFDecObjDecodeAVPacket(m_tFFVidDataObj->FFDataObjGetAVPacket(), pbyOutData, dwOutLen);
	if (s16Ret < 0)
	{
		return -1;
	}

	return s16Ret;
}
