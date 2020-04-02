
#include "FFVideoDecoderType.h"
#include "FFmpegVidFormatDataWrapper.h"


CFFmpegVideoFormatDataWrapper::CFFmpegVideoFormatDataWrapper()
{
	m_dwVidStmIdx = -1;

	/* codec */
	m_ptCodecPar = nullptr;

	/* format */
	m_ptVidIFmtCtx = nullptr;

	m_ptVidDecPkt = nullptr;

	m_tInputFile.clear();
	memset(m_chErrBuf, 0, ERR_BUF_SIZE);
}

CFFmpegVideoFormatDataWrapper:: ~CFFmpegVideoFormatDataWrapper()
{
	FFDataCloseDataStream();
}

s16 CFFmpegVideoFormatDataWrapper::FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Format [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (FFFormatInitParameter(ptVidDecParam) < 0)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Format [Init] init paramter error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoFormatDataWrapper::FFDataOpenDataStream()
{
	if (FFFormatOpenInputFile(nullptr, nullptr) < 0)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Format [open] open input file error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}

AVCodecParameters*  CFFmpegVideoFormatDataWrapper::FFDataGetCodecParam()
{
	return m_ptCodecPar;
}

s16 CFFmpegVideoFormatDataWrapper::FFDataReadPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen)
{
	s16 s16Ret = 0;

	if (!m_ptVidIFmtCtx)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Format [Read] FormatCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_ptVidDecPkt)
	{
		m_ptVidDecPkt = av_packet_alloc();
		if (!m_ptVidDecPkt)
		{
			av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Format [Read] malloc AVPacket error.\n");
			return FF_ERR_MEMORY_ALLOC;
		}
	}

	s16Ret = FFFormatReadAVPacket();
	if (s16Ret < 0)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFData-Format [read] packet error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return s16Ret;
}


AVPacket* CFFmpegVideoFormatDataWrapper::FFDataGetAVPacket()
{
	return 	m_ptVidDecPkt->size ? m_ptVidDecPkt : nullptr;
}

void CFFmpegVideoFormatDataWrapper::FFDataUnrefAVPacket()
{
	av_packet_unref(m_ptVidDecPkt);
	m_ptVidDecPkt->data = nullptr;
	m_ptVidDecPkt->size = 0;

}

void CFFmpegVideoFormatDataWrapper::FFDataCloseDataStream()
{
	FFFormatCloseFormatCtx();
}

////////////////////////////////////////private/////////////////////////////////////////////////////////

s16 CFFmpegVideoFormatDataWrapper::FFFormatInitParameter(const TFFVideoDecoderParam *ptVidDecParam)
{

	if (!ptVidDecParam)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (m_ptVidIFmtCtx)
	{
		FFFormatCloseFormatCtx();
	}

	m_tInputFile.append(ptVidDecParam->m_chInputFile);
	if (m_tInputFile.empty())
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [Init] Input file nullptr.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	m_ptVidDecPkt = av_packet_alloc();
	if (!m_ptVidDecPkt)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [Init] malloc AVPacket error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoFormatDataWrapper::FFFormatReadAVPacket()
{
	u32 dwRet = 0;

	/* read AVpacket from file */
	av_packet_unref(m_ptVidDecPkt);
	m_ptVidDecPkt->data = nullptr;
	m_ptVidDecPkt->size = 0;

	dwRet = av_read_frame(m_ptVidIFmtCtx, m_ptVidDecPkt);
	if (dwRet == AVERROR_EOF || avio_feof(m_ptVidIFmtCtx->pb))
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_WARNING, "FFFormat [Read] read packet finished.\n");
		return FF_STS_EOF;
	}
	else if (dwRet < 0)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [Read] read packet error.\n");
		return FF_ERR_UNKNOWN;
	}

	if (m_dwVidStmIdx == m_ptVidDecPkt->stream_index)
	{
		return FF_STS_HAVE_PKT;
	}

	return FF_ERR_NONE;
}


void CFFmpegVideoFormatDataWrapper::FFFormatCloseFormatCtx()
{
	m_dwVidStmIdx = -1;

	m_tInputFile.clear();

	if (m_ptVidDecPkt)
	{
		av_packet_free(&m_ptVidDecPkt);
		m_ptVidDecPkt = nullptr;
	}

	/* format context */
	if (m_ptVidIFmtCtx)
	{
		avformat_close_input(&m_ptVidIFmtCtx);
		m_ptVidIFmtCtx = nullptr;
	}
}


s16 CFFmpegVideoFormatDataWrapper::FFFormatOpenInputFile(AVInputFormat *ptInFmt, AVDictionary *ptOpt)
{
	u32 dwRet = 0;
	u32 dwI = 0;

	dwRet = avformat_open_input(&m_ptVidIFmtCtx, m_tInputFile.c_str(), ptInFmt, &ptOpt);
	if (dwRet != 0)
	{
		av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [open]  file [%s], error [%s].\n",
			m_tInputFile.c_str(), m_chErrBuf);
		return FF_ERR_UNSUPPORTED;
	}

	dwRet = avformat_find_stream_info(m_ptVidIFmtCtx, nullptr);
	if (dwRet < 0)
	{
		av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [open] find stream error [%s].\n", m_chErrBuf);
		return FF_ERR_UNSUPPORTED;
	}

	for (dwI = 0; dwI < m_ptVidIFmtCtx->nb_streams; dwI++)
	{
		if (m_ptVidIFmtCtx->streams[dwI]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_dwVidStmIdx = dwI;
			m_ptCodecPar = m_ptVidIFmtCtx->streams[dwI]->codecpar;
			av_log(m_ptVidIFmtCtx, AV_LOG_WARNING, "FFFormat [open] find a codec id [%d].\n", m_ptCodecPar->codec_id);

			break;
		}
	}

	if (m_dwVidStmIdx == -1)
	{
		av_log(m_ptVidIFmtCtx, AV_LOG_ERROR, "FFFormat [open] Couldn't find a video stream.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}