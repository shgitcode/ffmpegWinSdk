#include "FFVidDataDecoderObj.h"
#include "FFVidDecoderInterfaceIntenal.h"
#include "FFVidDecoderInterface.h"


CFFVidDecoderInterface::CFFVidDecoderInterface()
{
	m_pFFVidDataDecObj = NULL;
}

CFFVidDecoderInterface::~CFFVidDecoderInterface()
{
	CloseFFVideoDecoder();
}

s16	CFFVidDecoderInterface::InitFFVideoDecoder(const TFFVideoDecoderParam *ptVidDecParam)
{
	m_pFFVidDataDecObj = new CFFVidDataDecoderObj();

	if (m_pFFVidDataDecObj)
	{
		return m_pFFVidDataDecObj->InitFFDataVideoDecoder(ptVidDecParam);
	}

	return -1;
}

void CFFVidDecoderInterface::CloseFFVideoDecoder()
{
	if (m_pFFVidDataDecObj)
	{
		m_pFFVidDataDecObj->CloseFFDataVideoDecoder();

		delete m_pFFVidDataDecObj;
		m_pFFVidDataDecObj = nullptr;
	}
}


s16	CFFVidDecoderInterface::DecodeFFVideoFrame(u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen)
{
	if (m_pFFVidDataDecObj)
	{
		return m_pFFVidDataDecObj->DecodeFFDataVideoFrame(pbyInData, dwInLen, pbyOutData, dwOutLen);
	}

	return -1;
}


s16 CFFVidDecoderInterface::FlushFFVideoFrame(u8 *pbyOutData, u32 &dwOutLen)
{
	if (m_pFFVidDataDecObj)
	{
		return m_pFFVidDataDecObj->FlushFFDataVideoFrame(pbyOutData, dwOutLen);
	}

	return -1;
}


/* called interface */
void* FFVideoDec_CreateInstance()
{
	CFFVidDecoderInterface* pObj = new CFFVidDecoderInterface();

	return pObj;
}

s16 FFVideoDec_DestoryInstance(void* pHandle)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidDecoderInterface* pObj = (CFFVidDecoderInterface*)pHandle;

	delete pObj;
	pObj = NULL;

	return 0;
}

s16 FFVideoDec_InitDecoder(void* pHandle, const TFFVideoDecoderParam *ptVidDecParam)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidDecoderInterface* pObj = (CFFVidDecoderInterface*)pHandle;

	return pObj->InitFFVideoDecoder(ptVidDecParam);
}

s16 FFVideoDec_DecodeFrame(void* pHandle, u8 *pbyInData, u32 &dwInLen, u8 *pbyOutData, u32 &dwOutLen)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidDecoderInterface* pObj = (CFFVidDecoderInterface*)pHandle;

	return pObj->DecodeFFVideoFrame(pbyInData, dwInLen, pbyOutData, dwOutLen);
}

s16   FFVideoDec_FlushFrame(void* pHandle, u8 *pbyOutData, u32 &dwOutLen)
{
	if (pHandle == NULL)
	{
		return -1;
	}

	CFFVidDecoderInterface* pObj = (CFFVidDecoderInterface*)pHandle;

	return pObj->FlushFFVideoFrame(pbyOutData, dwOutLen);
}

void FFVideoDec_CloseDecoder(void* pHandle)
{
	if (pHandle == NULL)
	{
		return;
	}

	CFFVidDecoderInterface* pObj = (CFFVidDecoderInterface*)pHandle;

	return pObj->CloseFFVideoDecoder();
}