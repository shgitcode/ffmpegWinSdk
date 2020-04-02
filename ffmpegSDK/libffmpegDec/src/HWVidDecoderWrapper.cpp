
#include "FFVideoDecoderType.h"
#include "HWVidDecoderWrapper.h"


CFFHWVideoDecoderWrapper::CFFHWVideoDecoderWrapper()
{
	m_ptHWDevCtx    = nullptr;
	m_ptVidHW2SWFrm = nullptr;

	m_emDevType  = AV_HWDEVICE_TYPE_NONE;
	m_emHWPixFmt = AV_PIX_FMT_NONE;

	m_bSetHWFrameCtx = false;
}

CFFHWVideoDecoderWrapper:: ~CFFHWVideoDecoderWrapper()
{
	FFDecCloseDecoder();
}

s16 CFFHWVideoDecoderWrapper::FFDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (FFmpegDecInitDecoderParameter(ptVidDecParam) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Init] init decoder error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (HWDecConvertDeviceType(ptVidDecParam) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Init] convert device type error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (m_emDevType == AV_HWDEVICE_TYPE_NONE)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Init] device type not support.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFHWVideoDecoderWrapper::FFDecOpenDecoder(AVCodecParameters *ptCodecPar)
{
	enum AVCodecID  emCodecId = ptCodecPar ? ptCodecPar->codec_id : AV_CODEC_ID_NONE;

	if (FFmpegDecCreateDecoder(emCodecId) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Init] create Decoder error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	/* get HWpixel Format */
	if (HWDecGetHWPixFormatFromConfigure() < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [open] Couldn't Get HWPixelFormat from configure.\n");
		return FF_ERR_UNSUPPORTED;
	}

	/* creat HWDevice Context */
	if (HWDecCreateHWDeviceContext() < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [open] Create HWDevice context error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (FFmpegDecOpenDecoder(ptCodecPar) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [open] open Decoder error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	return FF_ERR_NONE;
}

AVCodecContext* CFFHWVideoDecoderWrapper::FFDecGetCodecCtx()
{
	return m_ptVidDecCodecCtx;
}

s16 CFFHWVideoDecoderWrapper::FFDecDecodeAVPacket(AVPacket *pPkt, u8 *pbyOutData, u32 &dwOutLen)
{
	u32 dwRet = 0;
	AVFrame *pFrm = nullptr;

	if (!pbyOutData)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Decode] output buffer nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	/* save frame to m_ptVidDecFrm */
	dwRet = FFmpegDecDecodeAVPacket(pPkt);
	if (dwRet < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Decode] decode avpacket error.\n");
		return FF_ERR_UNSUPPORTED;
	}

	/* dwRet equal FF_STS_HAVE_PKT, mean have frame */
	if (dwRet == FF_STS_HAVE_PKT)
	{
		if (HWDecRetriveHWFrame() < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Decode] retrive avframe error.\n");
			return FF_ERR_UNSUPPORTED;
		}

		/* create swsContext to convert frame  */
		if (FFmpegDecCreatePicSwsContext((enum AVPixelFormat)m_ptVidHW2SWFrm->format, m_ptVidHW2SWFrm->width, m_ptVidHW2SWFrm->height) < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFmDec [packet] Couldn't create SwsContext.\n");
			return FF_ERR_UNSUPPORTED;
		}

		if (FFmpegDecRunPicSwsScale(m_ptVidHW2SWFrm) < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFmDec [packet] swsScale AVFrame error.\n");
			return FF_ERR_UNSUPPORTED;
		}

		pFrm = m_ptVidHW2SWFrm;

		if (m_ptVidCntFrm)
		{
			pFrm = m_ptVidCntFrm;
		}
	
		if (FFmpegDecCopyAVFrame(pFrm, pbyOutData, dwOutLen) < 0)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Decode] copy avframe error.\n");
			return FF_ERR_UNSUPPORTED;
		}
		dwRet = FF_STS_HAVE_PKT;
	}
	else
	{
		dwOutLen = 0;
	}

	av_frame_unref(m_ptVidDecFrm);
	av_frame_unref(m_ptVidHW2SWFrm);

	return dwRet;
}

void CFFHWVideoDecoderWrapper::FFDecCloseDecoder()
{
	if (m_ptVidHW2SWFrm)
	{
		av_frame_free(&m_ptVidHW2SWFrm);
		m_ptVidHW2SWFrm = nullptr;
	}

	if (m_ptHWDevCtx)
	{
		av_buffer_unref(&m_ptHWDevCtx);
		m_ptHWDevCtx = nullptr;
	}

	m_bSetHWFrameCtx = false;

	m_emDevType  = AV_HWDEVICE_TYPE_NONE;
	m_emHWPixFmt = AV_PIX_FMT_NONE;

	FFmpegDecCloseDecoder();
}


/////////////////////////////////private///////////////////////////////////////////////////

/* this function can be called before avcodec_open2()  */
s16 CFFHWVideoDecoderWrapper::HWDecCreateHWDeviceContext()
{
	u32 dwRet = 0;

	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Device] CodecCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	/* open the hardware device */
	dwRet = av_hwdevice_ctx_create(&m_ptHWDevCtx, m_emDevType, NULL, NULL, 0);
	if (dwRet < 0)
	{
		av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Device] open the [%s] device, error [%s].\n",
			   av_hwdevice_get_type_name(m_emDevType), m_chErrBuf);
		return FF_ERR_DEVICE_UNSUPPORTED;
	}

	if (m_bSetHWFrameCtx)
	{
		/* must be set callback :
		 * for AVcodec h264_qsv: qsv_decode_frame->ff_qsv_process_data->ff_get_format
		 */
		m_ptVidDecCodecCtx->opaque     = m_ptHWDevCtx;
		m_ptVidDecCodecCtx->get_format = HWDecSetHWFrameCtxCB;

		av_log(m_ptVidDecCodecCtx, AV_LOG_DEBUG, "HWDec [Device] set device [%s] HWFrame context .\n",
			   av_hwdevice_get_type_name(m_emDevType));
	}
	else
	{
		// for AVHWaccel struct
		m_ptVidDecCodecCtx->hw_device_ctx = av_buffer_ref(m_ptHWDevCtx);
		m_ptVidDecCodecCtx->opaque = &m_emHWPixFmt;
		m_ptVidDecCodecCtx->get_format = HWDecGetHWPixFormatCB;
	}

	return FF_ERR_NONE;
}


enum AVPixelFormat CFFHWVideoDecoderWrapper::HWDecGetHWPixFormatCB(AVCodecContext *ptAvctx, const enum AVPixelFormat *pPixFmts)
{
	const enum AVPixelFormat *pPixFmt = nullptr;
	enum AVPixelFormat *pHWPixFmt = static_cast<enum AVPixelFormat *> (ptAvctx->opaque);

	av_log(ptAvctx, AV_LOG_WARNING, "HWDec [GetFmtCB] HWSurface coded w/h [%d/%d].\n", ptAvctx->coded_width, ptAvctx->coded_height);

	for (pPixFmt = pPixFmts; *pPixFmt != -1; pPixFmt++)
	{
		if (*pPixFmt == *pHWPixFmt)
			return *pPixFmt;
	}

	av_log(ptAvctx, AV_LOG_ERROR, "HWDec [GetFmtCB] Failed to get HW pixel Format.\n");

	return AV_PIX_FMT_NONE;
}

enum AVPixelFormat CFFHWVideoDecoderWrapper::HWDecSetHWFrameCtxCB(AVCodecContext *ptAVCtx, const enum AVPixelFormat *pPixFmts)
{
	u32 dwI = 0;
	u32 dwRet = 0;

	AVBufferRef *pHWDevRef = (AVBufferRef *)ptAVCtx->opaque;
	AVHWDeviceContext *pHWDevCtx = (AVHWDeviceContext*)pHWDevRef->data;
	enum AVPixelFormat emHWPixFmt = AV_PIX_FMT_NONE;

	av_log(ptAVCtx, AV_LOG_WARNING, "HWDec [SetFrmCtxCB] HWSurface coded format [%d/%d].\n", ptAVCtx->pix_fmt, ptAVCtx->sw_pix_fmt);

	for (dwI = 0; ; dwI++)
	{
		const AVCodecHWConfig *ptHWConfig = avcodec_get_hw_config(ptAVCtx->codec, dwI);
		if (!ptHWConfig)
		{
			av_log(ptAVCtx, AV_LOG_ERROR, "HWDec [SetFrmCtxCB] codec %s does not support device [%s].\n",
				   ptAVCtx->codec->name, av_hwdevice_get_type_name(pHWDevCtx->type));
			return AV_PIX_FMT_NONE;
		}

		/*
		 * h264/hevc_qsv method support AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX,
		 */
		if (ptHWConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX &&
			ptHWConfig->device_type == pHWDevCtx->type)
		{
			emHWPixFmt = ptHWConfig->pix_fmt;
			av_log(ptAVCtx, AV_LOG_WARNING, "HWDec [SetFrmCtxCB] HWFrameCtx [%d].\n", emHWPixFmt);
			break;
		}
	}

	if (emHWPixFmt == AV_PIX_FMT_NONE)
	{
		return AV_PIX_FMT_NONE;
	}

	while (*pPixFmts != AV_PIX_FMT_NONE)
	{
		av_log(ptAVCtx, AV_LOG_WARNING, "HWDec [SetFrmCtxCB] HWSurface coded format [%d]=[%d].\n", *pPixFmts, emHWPixFmt);

		if (*pPixFmts == emHWPixFmt)
		{
			AVHWFramesContext  *pHWFrameCtx = nullptr;

			/* create a pool of surfaces to be used by the decoder */
			ptAVCtx->hw_frames_ctx = av_hwframe_ctx_alloc(pHWDevRef);
			if (!ptAVCtx->hw_frames_ctx)
			{
				av_log(ptAVCtx, AV_LOG_ERROR, "HWDec [SetFrmCtxCB] Failed to set HWFrame context.\n");
				return AV_PIX_FMT_NONE;
			}
		
			pHWFrameCtx = (AVHWFramesContext*)ptAVCtx->hw_frames_ctx->data;

			av_log(ptAVCtx, AV_LOG_WARNING, "HWDec [SetFrmCtxCB] HWSurface format [%d] wh[%d/%d].\n", 
				   emHWPixFmt, ptAVCtx->coded_width, ptAVCtx->coded_height);

			pHWFrameCtx->format = emHWPixFmt;
			pHWFrameCtx->sw_format = ptAVCtx->sw_pix_fmt;
			pHWFrameCtx->width = FFALIGN(ptAVCtx->coded_width, 32);
			pHWFrameCtx->height = FFALIGN(ptAVCtx->coded_height, 32);
			pHWFrameCtx->initial_pool_size = 32;
			
			// for QSV decoder
			if (*pPixFmts == AV_PIX_FMT_QSV)
			{
				AVQSVFramesContext *pQSVFrameCtx = nullptr;
				pQSVFrameCtx = (AVQSVFramesContext *)pHWFrameCtx->hwctx;

				pQSVFrameCtx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;
			}

			dwRet = av_hwframe_ctx_init(ptAVCtx->hw_frames_ctx);
			if (dwRet < 0)
			{
				return AV_PIX_FMT_NONE;
			}

			return emHWPixFmt;
		}

		pPixFmts++;
	}

	av_log(ptAVCtx, AV_LOG_ERROR, "HWDec [SetFrmCtxCB] Failed to get HW pixel format.\n");

	return AV_PIX_FMT_NONE;
}

/* this function can be called before avio_open2() */
s16 CFFHWVideoDecoderWrapper::HWDecGetHWPixFormatFromConfigure()
{
	u32 dwI = 0;

	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [HWPix] CodecCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	for (dwI = 0; ; dwI++)
	{
		const AVCodecHWConfig *ptHWConfig = avcodec_get_hw_config(m_ptVidDecCodec, dwI);
		if (!ptHWConfig)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [HWPix] codec %s does not support device [%s].\n",
				   m_ptVidDecCodec->name, av_hwdevice_get_type_name(m_emDevType));
			return FF_ERR_DEVICE_UNSUPPORTED;
		}

		if (ptHWConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
			ptHWConfig->device_type == m_emDevType)
		{
			m_emHWPixFmt = ptHWConfig->pix_fmt;
			break;
		}

		/* h264/hevc_qsv method support AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX,
		 * need to set AVHWFrameCtx in get_format() callback
		*/
		if (ptHWConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX &&
			ptHWConfig->device_type == m_emDevType)
		{
			m_emHWPixFmt = ptHWConfig->pix_fmt;
			av_log(m_ptVidDecCodecCtx, AV_LOG_WARNING, "HWDec [HWPix] HWFrameCtx [%d].\n", m_emHWPixFmt);
			m_bSetHWFrameCtx = true;
			break;
		}
	}

	return FF_ERR_NONE;
}

s16 CFFHWVideoDecoderWrapper::HWDecRetriveHWFrame()
{
	u32 dwRet = 0;

	if (!m_ptVidHW2SWFrm)
	{
		m_ptVidHW2SWFrm = FFmpegDecMallocAVFrame();
		if (!m_ptVidHW2SWFrm)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Retrive] malloc HWframe error.\n");
			return FF_ERR_MEMORY_ALLOC;
		}
	}

	dwRet = av_hwframe_transfer_data(m_ptVidHW2SWFrm, m_ptVidDecFrm, 0);
	if (dwRet < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Retrive] transfer HWframe to SWFrame error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	dwRet = av_frame_copy_props(m_ptVidHW2SWFrm, m_ptVidDecFrm);
	if (dwRet < 0) 
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "HWDec [Retrive] copy props error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}
s16 CFFHWVideoDecoderWrapper::HWDecConvertDeviceType(const TFFVideoDecoderParam *ptDecParam)
{
	if (!ptDecParam)
	{
		return FF_ERR_NULL_PTR;
	}

	switch (ptDecParam->m_emDecType)
	{
	case FF_VID_HW_DEC_TYPE_NONE:
		m_emDevType = AV_HWDEVICE_TYPE_NONE;
		break;

	case FF_VID_HW_DEC_TYPE_DXVA2:
		m_emDevType = AV_HWDEVICE_TYPE_DXVA2;
		break;

	case FF_VID_HW_DEC_TYPE_QSV:
		m_emDevType = AV_HWDEVICE_TYPE_QSV;
		break;

	case FF_VID_HW_DEC_TYPE_CUDA:
		m_emDevType = AV_HWDEVICE_TYPE_CUDA;
		break;

	default:
		m_emDevType = AV_HWDEVICE_TYPE_NONE;
		break;
	}
	return FF_ERR_NONE;
}