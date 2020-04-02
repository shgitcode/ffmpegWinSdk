#include "FFVidEncoderObj.h"
#include "FFVidEncoderFormatObj.h"
#include "FFmpegVidFormatWrapper.h"

CFFVidEndFormatObj::CFFVidEndFormatObj()
{
	m_bCreate = false;

#ifdef ENABLE_FF_FORMAT
	m_pFFVidFormat.reset(nullptr);
#endif

	m_pFFVidEncoder.reset(nullptr);
}

CFFVidEndFormatObj::~CFFVidEndFormatObj()
{
	CloseFFVidFormatEncoder();
}

s16	CFFVidEndFormatObj::InitFFVidFormatEncoder(em_FFVideoEncoderType emType, const TFFVideoEncParam *ptVdioEncParam)
{
	s16 s16Ret = 0;
	
	std::lock_guard<std::mutex> lock(m_FFMutex);

	if (m_bCreate)
	{
		m_bCreate = false;

		if (m_pFFVidEncoder.get())
		{
			m_pFFVidEncoder->CloseFFVideoEncoder();
		}

#ifdef ENABLE_FF_FORMAT
		if (m_pFFVidFormat.get())
		{
			m_pFFVidFormat->FFmpegFormatCloseVideoFormat();
		}
#endif
	}

	m_pFFVidEncoder.reset(new CFFVideoEncoderObj);

	if (m_pFFVidEncoder.get())
	{
		s16Ret = m_pFFVidEncoder->InitFFVideoEncoder(emType, ptVdioEncParam);
		if (s16Ret < 0)
		{
			return -1;
		}
	}

#ifdef ENABLE_FF_FORMAT
	/* create output format context */
	if (ptVdioEncParam->m_bOutputFlag)
	{
		m_pFFVidFormat.reset(new CFFmpegVideoFormatWrapper);
	}

	if (m_pFFVidFormat.get())
	{
		if (!m_pFFVidEncoder.get())
		{
			return -1;
		}
	
		s16Ret = m_pFFVidFormat->FFmpegFormatInitVideoParamter(ptVdioEncParam->m_chOutFile,
			                                                   ptVdioEncParam->m_chFormatName, 
			                                                   ptVdioEncParam->m_byKeyFrameNum);
		if (s16Ret < 0)
		{
			return -1;
		}

		s16Ret = m_pFFVidFormat->FFmpegFormatAddVidOutputStream();
		if (s16Ret < 0)
		{
			return -1;
		}
	}
#endif

	/* open encoder codec context */
	if (m_pFFVidEncoder.get())
	{
		s16Ret = m_pFFVidEncoder->OpenFFVideoEncoder();
		if (s16Ret < 0)
		{
			return -1;
		}
	}

#ifdef ENABLE_FF_FORMAT
	/* write format header */
	if (m_pFFVidFormat.get())
	{
		if (!m_pFFVidEncoder.get())
		{
			return -1;
		}

		s16Ret = m_pFFVidFormat->FFmpegFormatWriteStreamHeader(m_pFFVidEncoder->GetFFVideoEncoder());
		if (s16Ret < 0)
		{
			return -1;
		}
	}
#endif

	m_bCreate = true;

	return 0;
}


void CFFVidEndFormatObj::CloseFFVidFormatEncoder()
{
	std::lock_guard<std::mutex> lock(m_FFMutex);

	m_bCreate = false;

	if (m_pFFVidEncoder.get())
	{
		m_pFFVidEncoder->CloseFFVideoEncoder();
	}

#ifdef ENABLE_FF_FORMAT
	if (m_pFFVidFormat.get())
	{
		m_pFFVidFormat->FFmpegFormatCloseVideoFormat();
	}
#endif
}


s16	CFFVidEndFormatObj::EncodeFFVidFormatVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, const TFFVideoYUVFrame *pTVidRawData)
{
	s16 s16Ret = -1;

	std::lock_guard<std::mutex> lock(m_FFMutex);

	if (!m_bCreate)
	{
		return s16Ret;
	}

	if (m_pFFVidEncoder.get())
	{
		s16Ret = m_pFFVidEncoder->EncodeFFVideoFrame(pbyOutData, dwOutLen, bKeyFlag, pTVidRawData);
		if (s16Ret < 0)
		{
			return -1;
		}
	}

#ifdef ENABLE_FF_FORMAT
	if ((s16Ret > FF_ERR_NONE) && m_pFFVidFormat.get())
	{
		if (!m_pFFVidEncoder.get())
		{
			return -1;
		}

		if (s16Ret == FF_STS_HAVE_PKT)
		{
			if (m_pFFVidFormat->FFmpegFormatWriteOutputPacket(m_pFFVidEncoder->GetFFVideoAVPacket()) < 0)
			{
				return -1;
			}
		}
		else if (s16Ret == FF_STS_EOF) 
		{
			if (m_pFFVidFormat->FFmpegFormatWriteStreamTrailer() < 0)
			{
				return -1;
			}
		}
	}
#endif
	return s16Ret;
}


s16 CFFVidEndFormatObj::FlushFFVidFormatVideoFrame(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag)
{
	s16 s16Ret = -1;

	std::lock_guard<std::mutex> lock(m_FFMutex);

	if (!m_bCreate)
	{
		return s16Ret;
	}

	if (m_pFFVidEncoder.get())
	{
		s16Ret = m_pFFVidEncoder->FlushFFVideoFrame(pbyOutData, dwOutLen, bKeyFlag);
		if (s16Ret < 0)
		{
			return -1;
		}
	}

#ifdef ENABLE_FF_FORMAT
	if ((s16Ret > FF_ERR_NONE) && m_pFFVidFormat.get())
	{
		if (!m_pFFVidEncoder.get())
		{
			return -1;
		}

		if (s16Ret == FF_STS_HAVE_PKT)
		{
			if (m_pFFVidFormat->FFmpegFormatWriteOutputPacket(m_pFFVidEncoder->GetFFVideoAVPacket()) < 0)
			{
				return -1;
			}
		}
		else if (s16Ret == FF_STS_EOF)
		{
			if (m_pFFVidFormat->FFmpegFormatWriteStreamTrailer() < 0)
			{
				return -1;
			}
		}
	}
#endif

	return s16Ret;
}

s16 CFFVidEndFormatObj::RequestVidFormatKeyFrame()
{
	std::lock_guard<std::mutex> lock(m_FFMutex);

	if (!m_bCreate)
	{
		return -1;
	}

	if (m_pFFVidEncoder.get())
	{
		if (m_pFFVidEncoder->RequestKeyFrame() < 0)
		{
			return -1;
		}
	}

	return -1;
}