#include "FFmpegVidFormatDataWrapper.h"
#include "FFmpegVidRawDataWrapper.h"
#include "FFmpegVidDevDataWrapper.h"
#include "FFVidDataObj.h"

CFFVideoDataObj::CFFVideoDataObj()
{
	m_tFFVidData.reset(nullptr);
}

CFFVideoDataObj::~CFFVideoDataObj()
{
	FFDataObjCloseHandle();
}

s16 CFFVideoDataObj::FFDataObjInitParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		return -1;
	}

	m_bDirectData = ptVidDecParam->m_bDirectData;

	if (m_bDirectData == FF_VID_DATA_TYPE_FILE)
	{
		m_tFFVidData.reset(new CFFmpegVideoFormatDataWrapper());
	}
	else if (m_bDirectData == FF_VID_DATA_TYPE_RAW)
	{
		m_tFFVidData.reset(new CFFmpegVideoRawDataWrapper());
	}
	else
	{
		m_tFFVidData.reset(new CFFmpegVideoDevDataWrapper());
	}

	if (!m_tFFVidData.get())
	{
		return -1;
	}

	return m_tFFVidData->FFDataInitVidParameter(ptVidDecParam);
}


s16 CFFVideoDataObj::FFDataObjOpenHandle()
{
	if (m_tFFVidData.get())
	{
		return m_tFFVidData->FFDataOpenDataStream();
	}

	return -1;
}

AVCodecParameters* CFFVideoDataObj::FFDataObjGetCodecParam()
{
	if (m_tFFVidData.get())
	{
		return m_tFFVidData->FFDataGetCodecParam();
	}

	return nullptr;
}


s16 CFFVideoDataObj::FFDataObjParsePacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen)
{

	if (m_tFFVidData.get())
	{
		return m_tFFVidData->FFDataReadPacket(ptCodecCtx, pbyInData, dwInLen);
	}

	return -1;
}


AVPacket* CFFVideoDataObj::FFDataObjGetAVPacket()
{
	if (m_tFFVidData.get())
	{
		return m_tFFVidData->FFDataGetAVPacket();
	}

	return nullptr;
}

void CFFVideoDataObj::FFDataObjUnrefAVPacket()
{
	if (m_tFFVidData.get())
	{
		m_tFFVidData->FFDataUnrefAVPacket();
	}
}

void CFFVideoDataObj::FFDataObjCloseHandle()
{
	if (m_tFFVidData.get())
	{
		m_tFFVidData->FFDataCloseDataStream();
	}
}