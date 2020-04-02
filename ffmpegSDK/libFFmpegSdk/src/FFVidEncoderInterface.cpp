#include "FFVidEncoderFormatObj.h"
#include "FFVidEncoderInterface.h"


CFFVidEncoderInterface::CFFVidEncoderInterface()
{
	m_pFFVidEndFmtObj = NULL;
}

CFFVidEncoderInterface::~CFFVidEncoderInterface()
{
	CloseFFVideoEncoder();
}

s16	CFFVidEncoderInterface::InitFFVideoEncoder(em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam)
{
	m_pFFVidEndFmtObj = new CFFVidEndFormatObj();

	if (m_pFFVidEndFmtObj)
	{
		return m_pFFVidEndFmtObj->InitFFVidFormatEncoder(emType, ptVdioEncParam);
	}

	return -1;
}

void CFFVidEncoderInterface::CloseFFVideoEncoder()
{
	if (m_pFFVidEndFmtObj)
	{
		m_pFFVidEndFmtObj->CloseFFVidFormatEncoder();
		
		delete m_pFFVidEndFmtObj;
		m_pFFVidEndFmtObj = nullptr;
	}
}


s16	CFFVidEncoderInterface::EncodeFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData)
{
	if (m_pFFVidEndFmtObj)
	{
		return m_pFFVidEndFmtObj->EncodeFFVidFormatVideoFrame(pbyOutData, dwOutLen, bKeyFlag, pTVidRawData);
	}

	return -1;
}


s16 CFFVidEncoderInterface::FlushFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag)
{
	if (m_pFFVidEndFmtObj)
	{
		return m_pFFVidEndFmtObj->FlushFFVidFormatVideoFrame(pbyOutData, dwOutLen, bKeyFlag);
	}

	return -1;
}


s16 CFFVidEncoderInterface::RequestKeyFrame()
{
	if (m_pFFVidEndFmtObj)
	{
		return m_pFFVidEndFmtObj->RequestVidFormatKeyFrame();
	}

	return -1;
}


/* called interface */
void* FFVideoEnc_CreateInstance()
{
	CFFVidEncoderInterface* pObj = new CFFVidEncoderInterface();

	return pObj;
}

s16 FFVideoEnc_DestoryInstance(void* pHandle)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidEncoderInterface* pObj = (CFFVidEncoderInterface*)pHandle;

	delete pObj;
	pObj = NULL;

	return 0;
}

s16 FFVideoEnc_InitEncoder(void* pHandle, em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidEncoderInterface* pObj = (CFFVidEncoderInterface*)pHandle;

	return pObj->InitFFVideoEncoder(emType, ptVdioEncParam);
}

s16 FFVideoEnc_EncodeFrame(void* pHandle, u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidEncoderInterface* pObj = (CFFVidEncoderInterface*)pHandle;

	return pObj->EncodeFFVideoFrame(pbyOutData, dwOutLen, bKeyFlag, pTVidRawData);
}

s16   FFVideoEnc_FlushFrame(void* pHandle, u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidEncoderInterface* pObj = (CFFVidEncoderInterface*)pHandle;

	return pObj->FlushFFVideoFrame(pbyOutData, dwOutLen, bKeyFlag);
}

void FFVideoEnc_CloseEncoder(void* pHandle)
{
	if (pHandle == NULL)
	{
		return;
	}

	CFFVidEncoderInterface* pObj = (CFFVidEncoderInterface*)pHandle;

	return pObj->CloseFFVideoEncoder();
}

s16 FFVideoEnc_RequestIDR(void* pHandle)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidEncoderInterface* pObj = (CFFVidEncoderInterface*)pHandle;

	return pObj->RequestKeyFrame();
}