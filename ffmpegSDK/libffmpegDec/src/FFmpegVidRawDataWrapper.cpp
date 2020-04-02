
#include "FFVideoDecoderType.h"
#include "FFmpegVidRawDataWrapper.h"


CFFmpegVideoRawDataWrapper::CFFmpegVideoRawDataWrapper()
{
	m_emCodecId = AV_CODEC_ID_NONE;

	m_ptVidDecPkt = nullptr;

	/* Directly send data */
	m_ptCodecParserCtx = nullptr;
	m_ptPktBuffer = nullptr;
	m_ptPktBufferEnd = nullptr;

	memset(m_chErrBuf, 0, ERR_BUF_SIZE);
}

CFFmpegVideoRawDataWrapper:: ~CFFmpegVideoRawDataWrapper()
{
	FFRawCloseParserCtx();
}

s16 CFFmpegVideoRawDataWrapper::FFDataInitVidParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (FFRawInitParameter(ptVidDecParam) < 0)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Init] Raw paramter error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoRawDataWrapper::FFDataOpenDataStream()
{

	if (m_emCodecId == AV_CODEC_ID_NONE)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Open] not support codecID.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (FFRawCreateParserCtx() < 0)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Open] create ParserCtx error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}

AVCodecParameters*  CFFmpegVideoRawDataWrapper::FFDataGetCodecParam()
{
	return nullptr;
}

s16 CFFmpegVideoRawDataWrapper::FFDataReadPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen)
{
	s16 s16Ret = 0;

	if (!pbyInData)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Read] input buffer nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!ptCodecCtx)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Read] CodecCtx nullptr.\n");
		return FF_ERR_NOT_INITIALIZED;
	}

	if (!m_ptCodecParserCtx)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Read] Parser not Create.\n");
		return FF_ERR_NOT_INITIALIZED;
	}

	s16Ret = FFRawParseToAVPacket(ptCodecCtx, pbyInData, dwInLen);
	if (s16Ret < 0)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFData-Raw [Read] Parse packet error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return s16Ret;
}


AVPacket* CFFmpegVideoRawDataWrapper::FFDataGetAVPacket()
{
	return FFRawGetAVPacket();
}

void CFFmpegVideoRawDataWrapper::FFDataUnrefAVPacket()
{
   // nothing
}

void CFFmpegVideoRawDataWrapper::FFDataCloseDataStream()
{
	FFRawCloseParserCtx();
}


///////////////////////////////////////////////////////////////////////////////////////
/* private function */

s16 CFFmpegVideoRawDataWrapper::FFRawInitParameter(const TFFVideoDecoderParam *ptVidDec)
{

	if (m_ptCodecParserCtx)
	{
		FFRawCloseParserCtx();
	}

	if (FFmpegRawConvertCodecId(ptVidDec) < 0)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [Init] convert codecId error.\n");
		return FF_ERR_NULL_PTR;
	}

	m_ptVidDecPkt = av_packet_alloc();
	if (!m_ptVidDecPkt)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [Init] convert codecId error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoRawDataWrapper::FFRawParseToAVPacket(AVCodecContext* ptCodecCtx, u8 *pbyInData, u32 &dwInLen)
{
	u32 dwI = 0;
	u32 dwRet = 0;
	u32 dwInSize = 0;
	u8 *pbyInPkt = nullptr;
	AVPacket *pkt = nullptr;

	pkt = av_packet_alloc();
	if (!pkt)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [Parse] AVPacket malloc error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	dwInSize = dwInLen;
	pbyInPkt = pbyInData;

	/* parse input data to AVPacket */
	while (dwInSize > 0)
	{
		pkt->data = NULL;
		pkt->size = 0;

		dwRet = av_parser_parse2(m_ptCodecParserCtx, ptCodecCtx, &pkt->data, &pkt->size,
				pbyInPkt, dwInSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (dwRet < 0)
		{
			av_packet_free(&pkt);
			av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
			av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [Parse] parse pkt error [%s].\n", m_chErrBuf);
			return FF_ERR_UNSUPPORTED;
		}

		pbyInPkt += dwRet;
		dwInSize -= dwRet;

		/* put AVPacket into pktlist*/
		if (pkt->size)
		{
			pkt->stream_index = 0;
			dwRet = FFmpegRawPutPacketToList(&m_ptPktBuffer, &m_ptPktBufferEnd, pkt);
			if (dwRet < 0)
			{
				av_packet_free(&pkt);
				av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [Parse] put pkt to list error.\n");
				return FF_ERR_UNSUPPORTED;
			}
		}
		av_packet_unref(pkt);
	}

	av_packet_free(&pkt);

	if (m_ptPktBuffer)
	{
		return FF_STS_HAVE_PKT;
	}

	return FF_ERR_NONE;
}


AVPacket* CFFmpegVideoRawDataWrapper::FFRawGetAVPacket()
{
	u32 dwRet = 0;

	av_packet_unref(m_ptVidDecPkt);

	m_ptVidDecPkt->data = nullptr;
	m_ptVidDecPkt->size = 0;

	/* if pktlist have AVPacket, we get AVPacket from pktList */
	if (m_ptPktBuffer)
	{
		dwRet = FFmpegRawGetPacketFromList(&m_ptPktBuffer, &m_ptPktBufferEnd, m_ptVidDecPkt);
		if (dwRet < 0)
		{
			av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [Get] pkt from list error.\n");
			return nullptr;
		}
	}

	return m_ptVidDecPkt->size ? m_ptVidDecPkt : nullptr;
}


void CFFmpegVideoRawDataWrapper::FFRawCloseParserCtx()
{

	m_emCodecId = AV_CODEC_ID_NONE;

	if (m_ptVidDecPkt)
	{
		av_packet_free(&m_ptVidDecPkt);
		m_ptVidDecPkt = nullptr;
	}

	if (m_ptCodecParserCtx)
	{
		av_parser_close(m_ptCodecParserCtx);
		m_ptCodecParserCtx = nullptr;
	}

	FFmpegRawFreePacketList(&m_ptPktBuffer, &m_ptPktBufferEnd);
	m_ptPktBuffer = nullptr;
	m_ptPktBufferEnd = nullptr;
}


s16 CFFmpegVideoRawDataWrapper::FFRawCreateParserCtx()
{

	m_ptCodecParserCtx = av_parser_init(m_emCodecId);
	if (!m_ptCodecParserCtx)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [create] Couldn't \
				   find parser context for codecID [%d].\n", m_emCodecId);
		return FF_ERR_UNSUPPORTED;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoRawDataWrapper::FFmpegRawConvertCodecId(const TFFVideoDecoderParam *ptDecParam)
{

	switch (ptDecParam->m_emDecCodec)
	{
	case FF_VID_DEC_CODEC_UNKNOWN:
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFmpegRaw [convert] not support codec ID.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;

	case FF_VID_DEC_CODEC_H264:
		m_emCodecId = AV_CODEC_ID_H264;
		break;

	case FF_VID_DEC_CODEC_HEVC:
		m_emCodecId = AV_CODEC_ID_HEVC;
		break;

	default:
		m_emCodecId = AV_CODEC_ID_H264;
		break;
	}

	return FF_ERR_NONE;
}

s16 CFFmpegVideoRawDataWrapper::FFmpegRawPutPacketToList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd, AVPacket *pPkt)
{
	u32 dwRet = 0;;

	AVPacketList *pktl = (AVPacketList *)av_mallocz(sizeof(AVPacketList));
	if (!pktl)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [put] Could not alloc pktList.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	dwRet = av_packet_ref(&pktl->pkt, pPkt);
	if (dwRet < 0)
	{
		av_free(pktl);
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [put] Could not ref pkt.\n");
		return FF_ERR_UNSUPPORTED;
	}

	if (*pPktBuf)
	{
		(*pPktBufEnd)->next = pktl;
	}
	else
	{
		*pPktBuf = pktl;
	}

	*pPktBufEnd = pktl;

	return FF_ERR_NONE;
}


s16 CFFmpegVideoRawDataWrapper::FFmpegRawGetPacketFromList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd, AVPacket *pPkt)
{
	AVPacketList *pktl;

	if (*pPktBuf == nullptr)
	{
		av_log(m_ptCodecParserCtx, AV_LOG_ERROR, "FFRaw [get] pktList empty.\n");
		return FF_ERR_NULL_PTR;
	}

	pktl = *pPktBuf;
	*pPkt = pktl->pkt;
	*pPktBuf = pktl->next;

	if (!pktl->next)
	{
		*pPktBufEnd = nullptr;
	}

	av_freep(&pktl);

	return FF_ERR_NONE;
}

void CFFmpegVideoRawDataWrapper::FFmpegRawFreePacketList(AVPacketList **pPktBuf, AVPacketList **pPktBufEnd)
{
	AVPacketList *tmp = *pPktBuf;

	while (tmp)
	{
		AVPacketList *pktl = tmp;
		tmp = pktl->next;
		av_packet_unref(&pktl->pkt);
		av_freep(&pktl);
	}
	*pPktBuf = NULL;
	*pPktBufEnd = NULL;
}
