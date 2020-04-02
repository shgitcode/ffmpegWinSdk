#include "FFVidEncoderType.h"
#include "FFmpegVidFormatWrapper.h"
#include "FFVidEncoderObj.h"

CFFmpegVideoFormatWrapper::CFFmpegVideoFormatWrapper()
{
	m_bCreate     = false;
	m_bOutputFlag = false;

	m_ptVidOFmtCtx = nullptr;
	m_ptOutStream  = nullptr;

	m_byKeyFrameNum = 0;
	m_byAddFrameNum = 0;

	m_tFileName.clear();
	m_tFormatName.clear();
}

CFFmpegVideoFormatWrapper::~CFFmpegVideoFormatWrapper()
{
	FFmpegFormatCloseVideoFormat();
}

s16 CFFmpegVideoFormatWrapper::FFmpegFormatInitVideoParamter(const s8 *pchFile, const s8 *pchFormat, 
	                                                         u16 keyFramNum)
{
	m_tFileName.append(pchFile);
	m_tFormatName.append(pchFormat);
	m_byKeyFrameNum = keyFramNum;

	if (m_tFileName.empty())
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_WARNING, "FFmpegFormat [Init] not set \
               outPut file, so ignoring.\n");
		return FF_ERR_NONE;
	}

	m_bOutputFlag = true;
	m_bCreate = true;

	return FF_ERR_NONE;
}

/* before avio_open2 , but AVCodecContext must be active */
s16 CFFmpegVideoFormatWrapper::FFmpegFormatAddVidOutputStream()
{
	u32 dwRet = 0;
	AVOutputFormat *ptVidOfmt = nullptr;

	if (!m_bCreate)
	{
		return FF_ERR_NOT_INITIALIZED;
	}

	if (!m_bOutputFlag)
	{
		return FF_ERR_NONE;
	}

	if (m_tFormatName.size())
	{
		ptVidOfmt = av_guess_format(m_tFormatName.c_str(), NULL, NULL);
	}

	avformat_alloc_output_context2(&m_ptVidOFmtCtx, ptVidOfmt, NULL, m_tFileName.c_str());
	if (!m_ptVidOFmtCtx)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Add] create OFormatCtx error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	m_ptOutStream = avformat_new_stream(m_ptVidOFmtCtx, NULL);
	if (!m_ptOutStream)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Add] allocate OStream error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	return FF_ERR_NONE;
}

/* after avio_open2 */
s16 CFFmpegVideoFormatWrapper::FFmpegFormatWriteStreamHeader(void *ptCodecCtx)
{
	u32 dwRet = 0;
	u32 dwStep = 0;
	AVCodecContext *ptVidCodecCtx = nullptr;

	if (!m_bCreate)
	{
		return FF_ERR_NOT_INITIALIZED;
	}

	if (!m_bOutputFlag)
	{
		return FF_ERR_NONE;
	}
	
	//ptVidCodecCtx = static_cast<AVCodecContext *> (ptCodecCtx);
	ptVidCodecCtx = (AVCodecContext *) (ptCodecCtx);
	if (!ptVidCodecCtx)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Header] codecContext NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_ptVidOFmtCtx)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Header] OFormatContext NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	m_ptOutStream->id = m_ptVidOFmtCtx->nb_streams - 1;
	m_ptOutStream->time_base = ptVidCodecCtx->time_base;
	m_ptOutStream->avg_frame_rate = av_inv_q(ptVidCodecCtx->time_base);

	m_CodecTimeBase = ptVidCodecCtx->time_base;

	av_log(m_ptVidOFmtCtx, AV_LOG_DEBUG, "FFmpegFormat [Add] OStream timebase[%d/%d].\n",
		m_ptOutStream->time_base.num, m_ptOutStream->time_base.den);

	/* copy codec parameters */
	dwRet = avcodec_parameters_from_context(m_ptOutStream->codecpar, ptVidCodecCtx);
	if (dwRet < 0)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Header] copy parameters from codecCtx error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	/* copy side data */
	for (dwStep = 0; dwStep < ptVidCodecCtx->nb_coded_side_data; dwStep++)
	{
		if (AV_PKT_DATA_CPB_PROPERTIES == ptVidCodecCtx->coded_side_data[dwStep].type)
		{
			dwRet = av_stream_add_side_data(m_ptOutStream, AV_PKT_DATA_CPB_PROPERTIES,
				    ptVidCodecCtx->coded_side_data[dwStep].data,
				    ptVidCodecCtx->coded_side_data[dwStep].size);
			if (dwRet < 0)
			{
				av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Header] copy side data error.\n");
				return FF_ERR_UNSUPPORTED;
			}
			break;
		}
	}

	m_ptOutStream->codecpar->codec_tag = 0;

	if (!(m_ptVidOFmtCtx->oformat->flags & AVFMT_NOFILE))
	{
		dwRet = avio_open(&m_ptVidOFmtCtx->pb, m_tFileName.c_str(), AVIO_FLAG_WRITE);
		if (dwRet < 0)
		{
			av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Header] open \
                   output file '%s' error.", m_tFileName.c_str());
			return FF_ERR_UNSUPPORTED;
		}
	}

	/* write header */
	dwRet = avformat_write_header(m_ptVidOFmtCtx, NULL);
	if (dwRet < 0)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Header] Write header error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	av_dump_format(m_ptVidOFmtCtx, 0, m_tFileName.c_str(), 1);

	FFmpegFormatSetReportSatrtTimer();

	/* for test : for rtp protocol, must be generate SDP  */
	if (!m_tFormatName.compare("rtp"))
	{
		s8 sdp[2048] = { 0 };
		FILE *pf = nullptr;

		av_sdp_create(&m_ptVidOFmtCtx, 1, sdp, sizeof(sdp));

		fopen_s(&pf, "rtp.sdp", "wb");
		if (pf)
		{
			fwrite(sdp, 1, strlen(sdp), pf);
			fclose(pf);
		}
	}

	return FF_ERR_NONE;
}

s16 CFFmpegVideoFormatWrapper::FFmpegFormatWriteOutputPacket(void *pPkt)
{
	u32 dwRet = 0;
	int64_t iQPts = 0;
	AVRational tRBase;
	AVPacket *pVidPkt = nullptr;

	if (!m_bCreate)
	{
		return FF_ERR_NOT_INITIALIZED;
	}

	if (!m_bOutputFlag)
	{
		return FF_ERR_NONE;
	}

	if (!m_ptVidOFmtCtx)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Packet] oFormatContext NULL.\n");
		return FF_ERR_NULL_PTR;
	}
	
	//pVidPkt = static_cast<AVPacket *> (pPkt);
	pVidPkt = (AVPacket *) (pPkt);
	if (!pVidPkt)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Packet] AVPacket NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	/* rescale output packet timestamp values from codec timebase to stream timebase */
	av_packet_rescale_ts(pVidPkt, m_CodecTimeBase, m_ptOutStream->time_base);

	av_log(m_ptVidOFmtCtx, AV_LOG_DEBUG, "FFmpegFormat [Packet] OStream timebase[%d/%d].\n",
		m_ptOutStream->time_base.num, m_ptOutStream->time_base.den);

	av_log(m_ptVidOFmtCtx, AV_LOG_DEBUG, "FFmpegFormat [Packet] OPacket pts[%lld]dts[%lld].\n",
		pVidPkt->pts, pVidPkt->dts);

	/* accumulate curretn pts */
	m_curPts = INT64_MIN + 1;
	if (av_stream_get_end_pts(m_ptOutStream) != AV_NOPTS_VALUE)
	{
		tRBase.num = 1;
		tRBase.den = AV_TIME_BASE;

		iQPts = av_rescale_q(av_stream_get_end_pts(m_ptOutStream),
			m_ptOutStream->time_base, tRBase);

		m_curPts = FFMAX(m_curPts, iQPts);
	}

	av_log(m_ptVidOFmtCtx, AV_LOG_DEBUG, "FFmpegFormat [Packet] current pts[%lld].\n", m_curPts);

	/* count keyFrame */
	if (m_byKeyFrameNum && (pVidPkt->flags & AV_PKT_FLAG_KEY))
	{
		m_byAddFrameNum++;

		if (m_byAddFrameNum > m_byKeyFrameNum)
		{
			m_bOutputFlag = false;
			m_byAddFrameNum = 0;
		}
	}

	pVidPkt->stream_index = m_ptOutStream->index;

	dwRet = av_interleaved_write_frame(m_ptVidOFmtCtx, pVidPkt);
	if (dwRet < 0)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Packet] Write packet error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	FFmpegFormatPrintReport(0);

	/* close file */
	if (!m_bOutputFlag)
	{
		av_write_trailer(m_ptVidOFmtCtx);

		av_log(m_ptVidOFmtCtx, AV_LOG_DEBUG, "FFmpegFormat [Packet] Write trailer.\n");

		if (m_ptVidOFmtCtx)
		{
			if (!(m_ptVidOFmtCtx->oformat->flags & AVFMT_NOFILE))
			{
				/* Close the output file. */
				avio_closep(&m_ptVidOFmtCtx->pb);
			}

			m_ptOutStream = nullptr;
			avformat_free_context(m_ptVidOFmtCtx);
			m_ptVidOFmtCtx = nullptr;
		}
		FFmpegFormatPrintReport(1);
	}

	return FF_ERR_NONE;
}

s16 CFFmpegVideoFormatWrapper::FFmpegFormatWriteStreamTrailer()
{
	if (!m_ptVidOFmtCtx)
	{
		av_log(m_ptVidOFmtCtx, AV_LOG_ERROR, "FFmpegFormat [Trailer] oFormat context NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	av_log(m_ptVidOFmtCtx, AV_LOG_DEBUG, "FFmpegFormat [Trailer] write Trailer.\n");

	av_write_trailer(m_ptVidOFmtCtx);

	FFmpegFormatPrintReport(1);

	return FF_ERR_NONE;
}

void CFFmpegVideoFormatWrapper::FFmpegFormatSetReportCurrentTimer()
{
	m_curTime = av_gettime_relative();
	return;
}

void CFFmpegVideoFormatWrapper::FFmpegFormatCloseVideoFormat()
{
	if (!m_bCreate)
	{
		return;
	}

	m_bCreate = false;
	m_bOutputFlag = false;

	m_byKeyFrameNum = 0;
	m_byAddFrameNum = 0;

	m_tFileName.clear();
	m_tFormatName.clear();

	if (m_ptVidOFmtCtx)
	{
		if (!(m_ptVidOFmtCtx->oformat->flags & AVFMT_NOFILE))
		{
			/* Close the output file. */
			avio_closep(&m_ptVidOFmtCtx->pb);
		}

		avformat_free_context(m_ptVidOFmtCtx);
	}
}

///////////////////////////////private////////////////////////////////////


void CFFmpegVideoFormatWrapper::FFmpegFormatSetReportSatrtTimer()
{
	m_timerStart = av_gettime_relative();
	return;
}

void CFFmpegVideoFormatWrapper::FFmpegFormatPrintReport(u32 last)
{
	float ft;
	double speed;
	s8 buf[1024] = { 0 };
	const char end = last ? '\n' : '\r';

	ft = (m_curTime - m_timerStart) / 1000000.0;

	speed = (ft != 0.0) ? (double)m_curPts / AV_TIME_BASE / ft : -1;

	if (speed < 0)
	{
		sprintf_s(buf, " speed=N/A%c", end);
	}
	else
	{
		sprintf_s(buf, " speed=%4.3gx%c", speed, end);
	}

	av_log(m_ptVidOFmtCtx, AV_LOG_INFO, "%s", buf);

	return;
}
