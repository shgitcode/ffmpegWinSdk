#include "FFVidEncoderType.h"
#include "HWVidEncoderWrapper.h"

static const AVHWCodecInfo g_tHWCodecInfo[] = {
	{ "h264_qsv",   AV_CODEC_ID_H264, AV_HWDEVICE_TYPE_QSV,     AV_PIX_FMT_QSV,   AV_PIX_FMT_NV12 },
	{ "hevc_qsv",   AV_CODEC_ID_HEVC, AV_HWDEVICE_TYPE_QSV,     AV_PIX_FMT_QSV,   AV_PIX_FMT_NV12 },
	{ "h264_amf",   AV_CODEC_ID_H264, AV_HWDEVICE_TYPE_D3D11VA, AV_PIX_FMT_D3D11, AV_PIX_FMT_NV12 },
	{ "hevc_amf",   AV_CODEC_ID_HEVC, AV_HWDEVICE_TYPE_D3D11VA, AV_PIX_FMT_D3D11, AV_PIX_FMT_NV12 },
	{ "nvenc",      AV_CODEC_ID_H264, AV_HWDEVICE_TYPE_CUDA,    AV_PIX_FMT_CUDA,  AV_PIX_FMT_NV12 },
	{ "hevc_nvenc", AV_CODEC_ID_HEVC, AV_HWDEVICE_TYPE_CUDA,    AV_PIX_FMT_CUDA,  AV_PIX_FMT_NV12 },
	{ nullptr,      AV_CODEC_ID_NONE, AV_HWDEVICE_TYPE_NONE,    AV_PIX_FMT_NONE,  AV_PIX_FMT_NONE },
};


/*************************** public interface *********************************************/

CHWVideoEncoderWrapper::CHWVideoEncoderWrapper()
{
	m_emHWCodecType = em_UnknownCodec;
	m_ptHWVidEncFrm = nullptr;
	m_ptHWDevCtx = nullptr;
}

CHWVideoEncoderWrapper::~CHWVideoEncoderWrapper()
{
	CloseFFVideoEncoderCom();
}

/*
* init and open HW(Intel/Amd/Navida) video encoder codec context
*/
s16  CHWVideoEncoderWrapper::InitFFVideoEncoderCodecCtxCom(const TFFVideoEncParam *ptVidEncParam,
	                                                       enum em_FFVideoEncoderType emVidType)
{
	s16 s16Ret = 0;

	if (!ptVidEncParam)
	{
		av_log(NULL, AV_LOG_ERROR, "FFHW_ENCODER [Init] video encoder input param  NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if ((emVidType <= em_FF_SW) || (emVidType >= em_FF_ENCNONE))
	{
		av_log(NULL, AV_LOG_ERROR, "FFHW_ENCODER [Init] Init not support hw type [%d].\n", emVidType);
		return FF_ERR_UNSUPPORTED;
	}

	if ((ptVidEncParam->m_byEncCodecType != MEDIA_TYPE_H264) &&
		(ptVidEncParam->m_byEncCodecType != MEDIA_TYPE_H265))
	{
		av_log(NULL, AV_LOG_ERROR, "FFHW_ENCODER [Init] not support codec type [%d].\n", ptVidEncParam->m_byEncCodecType);
		return FF_ERR_UNSUPPORTED;
	}

	if (m_bCreate && m_ptVidEncCodecCtx)
	{
		return FF_ERR_NONE;
	}

	if (m_ptVidEncCodecCtx)
	{
		FFmpegCloseVideoEncoder();
	}

	switch (emVidType)
	{
	case em_FF_INTELHW:
		if (ptVidEncParam->m_byEncCodecType == MEDIA_TYPE_H264)
		{
			m_emHWCodecType = em_H264QsvCodec; // h264_qsv
		}
		else
		{
			m_emHWCodecType = em_HevcQsvCodec; // hevc_qsv
		}

		break;

	case em_FF_AMDHW:
		if (ptVidEncParam->m_byEncCodecType == MEDIA_TYPE_H264)
		{
			m_emHWCodecType = em_H264AmfCodec; // h264_amf
		}
		else
		{
			m_emHWCodecType = em_HevcAmfCodec; // hevc_amf
		}

		break;

	case em_FF_NVIDIAHW:
		if (ptVidEncParam->m_byEncCodecType == MEDIA_TYPE_H264)
		{
			m_emHWCodecType = em_H264NvencCodec; // h264_nvenc
		}
		else
		{
			m_emHWCodecType = em_HevcNvencCodec; // hevc_nvenc
		}

		break;

	default:
		break;
	}

	if (m_emHWCodecType == em_UnknownCodec)
	{
		av_log(NULL, AV_LOG_ERROR, "FFHW_ENCODER [Init] Not support codec type [%d].\n", 
			   ptVidEncParam->m_byEncCodecType);
		return FF_ERR_UNSUPPORTED;
	}

	s16Ret = FFmpegInitVideoEncodeParameter(ptVidEncParam, 
		                                    g_tHWCodecInfo[m_emHWCodecType].m_pchCodecName,
		                                    g_tHWCodecInfo[m_emHWCodecType].m_emHWPixFmt);
	if (s16Ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "HW Init [%s] codec context error.\n", 
			   g_tHWCodecInfo[m_emHWCodecType].m_pchCodecName);
		return s16Ret;
	}

	s16Ret = FFmpegSetHWDeviceCtx(g_tHWCodecInfo[m_emHWCodecType].m_emtHWDevType);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW override Set [%s] codec context attribution error.\n",
			   g_tHWCodecInfo[m_emHWCodecType].m_pchCodecName);
		return s16Ret;
	}

	/* set output pixel format */
	FFmpegSetOutPixFormat(g_tHWCodecInfo[m_emHWCodecType].m_emFrmPixFmt);

	return 0;
}

/*
 * open AVCodecContext with avio_open2 
*/
s16 CHWVideoEncoderWrapper::OpenFFVideoEncoderCom()
{
	s16 s16Ret = FF_ERR_NONE;

	if (m_bCreate)
	{
		return s16Ret;
	}

	/* open codec encoder */
	s16Ret = FFmpegOpenVideoEncoder();
	if (s16Ret < FF_ERR_NONE)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFHW_ENCODER [OpenCodec] [%s] encoder codec error.\n",
			   g_tHWCodecInfo[m_emHWCodecType].m_pchCodecName);
		return s16Ret;
	}

	/* display codec context information */
	FFmpegDumpEncoderContext();

	m_bCreate = true;

	return FF_ERR_NONE;
}

void* CHWVideoEncoderWrapper::GetFFVideoEncoderCom()
{
	return m_ptVidEncCodecCtx;
}

/*
* first, convert raw data to AVFrame
* then, transfer SWFrame to HWFrame
* then, encode HWframe to packet
* return value:
*             < 0  : error
*               0  : no error
*               1  : EOF
*               2  : must be packet
*/

s16 CHWVideoEncoderWrapper::EncodeFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, 
	                                              const TFFVideoYUVFrame *ptVidRawData)
{
	s16 s16Ret = 0;

	if (!pbyOutData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Encode output stored packet data NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!ptVidRawData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Encode input raw data NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_bCreate || !m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Encode codec not initialized.\n");
		return FF_ERR_NOT_INITIALIZED;
	}

	s16Ret = FFmpegConvertVideoToAVFrame(ptVidRawData);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Encode Convert raw data to sw/hwFrame error\n");
		return s16Ret;
	}

	s16Ret = FFmpegGetHWFrameBuffer();
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW override Malloc [%s] Hwframe error.\n",
			g_tHWCodecInfo[m_emHWCodecType].m_pchCodecName);
		return s16Ret;
	}

	s16Ret = FFmpegTransToHWFrame();
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Transfer swframe to hwframe error.\n");
		return s16Ret;
	}

	s16Ret = FFmpegEncodeOneFrame(pbyOutData, dwOutLen, m_ptHWVidEncFrm, bKeyFlag);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Encode one hwframe error\n");
	}

	return s16Ret;
}

/*
* flush buffered HWframe to packet
* return value:
*             < 0  : error
*               0  : no error
*               1  : EOF
*               2  : must be packet
*/
s16 CHWVideoEncoderWrapper::FlushFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag)
{
	s16 s16Ret = 0;

	if (!pbyOutData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Flush output stored packet data NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_bCreate || !m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Flush codec not initialized.\n");
		return FF_ERR_NOT_INITIALIZED;
	}

	s16Ret = FFmpegEncodeOneFrame(pbyOutData, dwOutLen, NULL, bKeyFlag);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Flush Encode one hwframe error\n");
	}

	return s16Ret;
}

void* CHWVideoEncoderWrapper::GetFFVideoAVPacketCom()
{
	return FFmpegGetVideoAVPacket();
}

void CHWVideoEncoderWrapper::CloseFFVideoEncoderCom()
{
	FFmpegFreeHWContext();
	FFmpegCloseVideoEncoder();
}

void CHWVideoEncoderWrapper::RequestFFVideoKeyFrameCom()
{
	FFmpegSetRequestIDR();
}


/******************************* private function **********************************************************/

s16 CHWVideoEncoderWrapper::FFmpegSetHWFrameCtx(AVCodecContext *ptCodecCtx, AVBufferRef *ptBufRef)
{
	s16 s16Ret = FF_ERR_NONE;
	AVBufferRef       *ptHWRef = NULL;
	AVHWFramesContext *ptHWCtx = NULL;

	ptHWRef = av_hwframe_ctx_alloc(ptBufRef);
	if (!ptHWRef)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to create hwframe context.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	ptHWCtx = (AVHWFramesContext *)(ptHWRef->data);
	ptHWCtx->format    = ptCodecCtx->pix_fmt;
	ptHWCtx->sw_format = g_tHWCodecInfo[m_emHWCodecType].m_emFrmPixFmt;
	ptHWCtx->width  = ptCodecCtx->width;
	ptHWCtx->height = ptCodecCtx->height;
	ptHWCtx->initial_pool_size = 32;

	if (av_hwframe_ctx_init(ptHWRef) < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to initialize hwframe context.\n");
		av_buffer_unref(&ptHWRef);
		return FF_ERR_UNSUPPORTED;
	}

	ptCodecCtx->hw_frames_ctx = av_buffer_ref(ptHWRef);
	if (!ptCodecCtx->hw_frames_ctx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW hw_frames_ctx NULL\n");
		s16Ret = FF_ERR_NULL_PTR;
	}

	av_buffer_unref(&ptHWRef);

	return s16Ret;
}


/* before avio_open2() */
s16 CHWVideoEncoderWrapper::FFmpegSetHWDeviceCtx(enum AVHWDeviceType emHWDevType)
{
	u32 dwRet = 0;

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Video encoder codec context NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	/* create the video encoder HW device */
	dwRet = av_hwdevice_ctx_create(&m_ptHWDevCtx, emHWDevType, NULL, NULL, 0); // AV_HWDEVICE_TYPE_QSV
	if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to create a [%d] device.\n", emHWDevType);
		return FF_ERR_DEVICE_UNSUPPORTED;
	}

	/* set hw_frames_ctx for encoder's AVCodecContext */
	if (FFmpegSetHWFrameCtx(m_ptVidEncCodecCtx, m_ptHWDevCtx) < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to set hw [%d] frame context.\n", emHWDevType);
		return FF_ERR_UNSUPPORTED;
	}

	if (g_tHWCodecInfo[m_emHWCodecType].m_emCodecId == AV_CODEC_ID_H264)
	{
		if (m_emHWCodecType == AV_HWDEVICE_TYPE_QSV)
		{
			{
				/* QSV: set async_depth equal 1, zerolatency , if async_depth = 6, then latency 6 frame */
				//av_opt_set_int(m_ptVidEncCodecCtx->priv_data, "async_depth", 6, 0);
			}

			{
				/* QSV: for multi-slice */
				//m_ptVidEncCodecCtx->slices = m_ptVidEncCodecCtx->height / 16;
				//av_opt_set_int(m_ptVidEncCodecCtx->priv_data, "max_slice_size", m_ptVidEncCodecCtx->width * 16, 0);
			}
		}
		/* flags: AV_CODEC_FLAG_GLOBAL_HEADER == not repeat sps pps in key frame header */
		//m_ptVidEncCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	return FF_ERR_NONE;
}


s16  CHWVideoEncoderWrapper::FFmpegGetHWFrameBuffer()
{
	u32 dwRet = 0;

	if (!m_ptHWVidEncFrm)
	{
		m_ptHWVidEncFrm = FFmpegMallocAVFrame();
		if (!m_ptHWVidEncFrm)
		{
			av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Malloc hwframe error.\n");
			return FF_ERR_MEMORY_ALLOC;
		}
	}

	dwRet = av_hwframe_get_buffer(m_ptVidEncCodecCtx->hw_frames_ctx, m_ptHWVidEncFrm, 0);
	if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to get buffer for hwframe.\n");
		return FF_ERR_NOT_ENOUGH_BUFFER;
	}

	if (!m_ptHWVidEncFrm->hw_frames_ctx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to get hw_frames_ctx for hwframe.\n");
		return FF_ERR_NULL_PTR;
	}

	return FF_ERR_NONE;
}

s16 CHWVideoEncoderWrapper::FFmpegTransToHWFrame()
{
	u32 dwRet = 0;

	dwRet = av_hwframe_transfer_data(m_ptHWVidEncFrm, m_ptVidEncFrm, 0);
	if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "HW Failed to transfer swframe to hwframe.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	m_ptHWVidEncFrm->pts = m_ptVidEncFrm->pts;
	m_ptHWVidEncFrm->pict_type = m_ptVidEncFrm->pict_type;
	m_ptHWVidEncFrm->key_frame = m_ptVidEncFrm->key_frame;
	
	av_log(m_ptVidEncCodecCtx, AV_LOG_DEBUG, "HW Failed to transfer swframe to hwframe [%ld].\n", m_ptHWVidEncFrm->pts);

	av_frame_unref(m_ptVidEncFrm);

	return FF_ERR_NONE;
}

void CHWVideoEncoderWrapper::FFmpegFreeHWContext()
{
	m_emHWCodecType = em_UnknownCodec;

	if (m_ptHWVidEncFrm)
	{
		av_frame_free(&m_ptHWVidEncFrm);
		m_ptHWVidEncFrm = NULL;
	}

	if (m_ptHWDevCtx)
	{
		av_buffer_unref(&m_ptHWDevCtx);
		m_ptHWDevCtx = NULL;
	}
}