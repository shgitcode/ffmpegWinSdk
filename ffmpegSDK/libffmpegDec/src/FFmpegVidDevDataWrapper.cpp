
#include "FFVideoDecoderType.h"
#include "FFmpegVidDevDataWrapper.h"


CFFmpegVideoDevDataWrapper::CFFmpegVideoDevDataWrapper()
{
	/* format */
	m_ptInFmt = nullptr;
	m_ptOptions = nullptr;

	m_tFmtName.clear();
}

CFFmpegVideoDevDataWrapper:: ~CFFmpegVideoDevDataWrapper()
{
	FFDataCloseDataStream();
}

s16 CFFmpegVideoDevDataWrapper::FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Dev [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	avdevice_register_all();

	if (CFFmpegVideoFormatDataWrapper::FFDataInitVidParameter(ptVidDecParam) < 0)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Dev [Init] init paramter error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	m_tFmtName.append(ptVidDecParam->m_chFormatName);
	if (m_tFmtName.empty())
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Dev [Init] Format Name nullptr.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoDevDataWrapper::FFDataOpenDataStream()
{
	m_ptInFmt = av_find_input_format(m_tFmtName.c_str());
	if (!m_ptInFmt)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Dev [open] find format [%s] error.\n", m_tFmtName.c_str());
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (FFFormatOpenInputFile(m_ptInFmt, m_ptOptions) < 0)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Dev [open] open input file error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}

void CFFmpegVideoDevDataWrapper::FFDataCloseDataStream()
{
	m_ptInFmt = nullptr;
	m_ptOptions = nullptr;

	m_tFmtName.clear();

	CFFmpegVideoFormatDataWrapper::FFDataCloseDataStream();
}

