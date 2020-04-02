
#include "FFVideoDecoderType.h"
#include "SWVidDecoderWrapper.h"


CFFSWVideoDecoderWrapper::CFFSWVideoDecoderWrapper()
{

}

CFFSWVideoDecoderWrapper:: ~CFFSWVideoDecoderWrapper()
{
	FFDecCloseDecoder();
}

s16 CFFSWVideoDecoderWrapper::FFDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (ptVidDecParam->m_emDecType != FF_VID_HW_DEC_TYPE_NONE)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Init] Input paramter error type [%d].\n", ptVidDecParam->m_emDecType);
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (FFmpegDecInitDecoderParameter(ptVidDecParam) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Init] init decoder error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFSWVideoDecoderWrapper::FFDecOpenDecoder(AVCodecParameters *ptCodecPar)
{
	enum AVCodecID   emCodecId = ptCodecPar ? ptCodecPar->codec_id : AV_CODEC_ID_NONE;

	if (FFmpegDecCreateDecoder(emCodecId) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [open] create Decoder error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	if (FFmpegDecOpenDecoder(ptCodecPar) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [open] open Decoder error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	return FF_ERR_NONE;
}

AVCodecContext* CFFSWVideoDecoderWrapper::FFDecGetCodecCtx()
{
	return m_ptVidDecCodecCtx;
}

s16 CFFSWVideoDecoderWrapper::FFDecDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen)
{
	u32 dwRet = 0;
	AVFrame *pFrm = nullptr;

	if (!pbyOutData)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Decode] output buffer nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Decode] CodecCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	/* save frame to m_ptVidDecFrm */
	dwRet = FFmpegDecDecodeAVPacket(pPkt);
	if (dwRet < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Decode] decode avpacket error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	/* dwRet equal FF_STS_HAVE_PKT, mean have frame */
	if (dwRet == FF_STS_HAVE_PKT)
	{
		/* create swsContext to convert frame  */
		if (FFmpegDecCreatePicSwsContext((enum AVPixelFormat)m_ptVidDecFrm->format, m_ptVidDecFrm->width, m_ptVidDecFrm->height) < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFmDec [packet] Couldn't create SwsContext.\n");
			return FF_ERR_UNSUPPORTED;
		}

		if (FFmpegDecRunPicSwsScale(m_ptVidDecFrm) < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFmDec [packet] swsScale AVFrame error.\n");
			return FF_ERR_UNSUPPORTED;
		}

		pFrm = m_ptVidDecFrm;

		if (m_ptVidCntFrm)
		{
			pFrm = m_ptVidCntFrm;
		}

		if (FFmpegDecCopyAVFrame(pFrm, pbyOutData, dwOutLen) < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "SWDec [Decode] copy avframe error.\n");
			return FF_ERR_UNSUPPORTED;
		}
		dwRet = FF_STS_HAVE_PKT;
	}
	else
	{
		dwOutLen = 0;
	}

	av_frame_unref(m_ptVidDecFrm);

	return dwRet;
}

void CFFSWVideoDecoderWrapper::FFDecCloseDecoder()
{

	FFmpegDecCloseDecoder();

}