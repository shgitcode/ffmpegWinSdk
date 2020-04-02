#include "FFVidEncoderType.h"
#include "FFmpegVidEncoderWrapper.h"

#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "swscale.lib")
#pragma comment (lib, "avdevice.lib")

CFFmpegVideoEncoderWrapper::CFFmpegVideoEncoderWrapper()
{
	m_bCreate = false;
	m_bReqIDR = false;

	m_emInPixFmt  = AV_PIX_FMT_NONE;
	m_emOutPixFmt = AV_PIX_FMT_NONE;

	m_ptVidEncFrm = nullptr;
	m_ptVidEncPkt = nullptr;
	m_ptVidEncCodec = nullptr;
	m_ptVidEncCodecCtx = nullptr;

	/* swscale */
	m_ptPicSwsCtx = nullptr;
	m_ptVidCntFrm = nullptr;

	/* error info */
	memset(m_chErrBuf, 0, ERR_BUF_SIZE);
}

CFFmpegVideoEncoderWrapper::~CFFmpegVideoEncoderWrapper()
{
	FFmpegCloseVideoEncoder();
}


///////////////////////////////ÊµÏÖ¼Ì³Ð/////////////////////////////////////////
/*
 * free AVFrame
 * free AVPacket
 * close CodecContext
 */
void CFFmpegVideoEncoderWrapper::FFmpegCloseVideoEncoder()
{
	m_bCreate = false;
	m_bReqIDR = false;

	m_emInPixFmt  = AV_PIX_FMT_NONE;
	m_emOutPixFmt = AV_PIX_FMT_NONE;

	if (m_ptVidEncFrm)
	{
		av_frame_free(&m_ptVidEncFrm);
		m_ptVidEncFrm = nullptr;
	}

	if (m_ptVidEncPkt)
	{
		av_packet_free(&m_ptVidEncPkt);
		m_ptVidEncPkt = nullptr;
	}

	if (m_ptVidEncCodecCtx)
	{
		m_ptVidEncCodec = nullptr;
		avcodec_free_context(&m_ptVidEncCodecCtx);
		m_ptVidEncCodecCtx = nullptr;
	}

	/* swsscale */
	if (m_ptPicSwsCtx)
	{
		sws_freeContext(m_ptPicSwsCtx);
		m_ptPicSwsCtx = nullptr;
	}

	if (m_ptVidCntFrm)
	{
		av_frame_free(&m_ptVidCntFrm);
		m_ptVidCntFrm = nullptr;
	}
}

/* 
 * find videoCodec by pchCodecName,  
 * malloc codecContext by videoCodec, 
 * and then init codecContext by ptVidEncParam
 */
s16 CFFmpegVideoEncoderWrapper::FFmpegInitVideoEncodeParameter(const TFFVideoEncParam *ptVidEncParam, 
	                                                           const s8 *pchCodecName,
	                                                           enum AVPixelFormat emPixFmt)
{
	if (!ptVidEncParam)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Init] input paramter NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!pchCodecName)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Init] codec Name NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (m_bCreate && m_ptVidEncCodecCtx)
	{
		return FF_ERR_NONE;
	}

	if (m_ptVidEncCodecCtx)
	{
		FFmpegCloseVideoEncoder();
	}

	/* set log level */
#ifdef _DEBUG
	av_log_set_level(AV_LOG_DEBUG);
#else
	av_log_set_level(AV_LOG_INFO);
#endif

	/* find the video encoder codec */
	m_ptVidEncCodec = avcodec_find_encoder_by_name(pchCodecName);
	if (!m_ptVidEncCodec)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Init] [%s] codec not found.\n", pchCodecName);
		return FF_ERR_NOT_FOUND;
	}

	m_ptVidEncCodecCtx = avcodec_alloc_context3(m_ptVidEncCodec);
	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Init]  allocate [%s] codecContext error.\n", pchCodecName);
		return FF_ERR_MEMORY_ALLOC;
	}

	/* 
	 ********* X264 rate control *********
	 * ABR : must be set bitrate, can set VBV(matrate>bitrate, buffersize=bitrate)
	 * CRF : must be set crf, can set VBV(matrate>bitrate, buffersize=bitrate)
	 * CBR : must be set matrate=buffersize=bitrate
     ********* intel QSV rate control *********
	 * VBR : must be set bitrate, can set VBV(matrate>bitrate, buffersize=bitrate)
	 * AVBR: can not set matrate
	 * CBR : must be set matrate=bitrate ,buffersize=bitrate
	*/
	m_ptVidEncCodecCtx->bit_rate       = ptVidEncParam->m_wBitRate*1000;
	m_ptVidEncCodecCtx->rc_max_rate    = ptVidEncParam->m_wMaxBitRate*1000;
	m_ptVidEncCodecCtx->rc_min_rate    = ptVidEncParam->m_wMinBitRate*1000;
	m_ptVidEncCodecCtx->rc_buffer_size = m_ptVidEncCodecCtx->rc_max_rate;//m_ptVidEncCodecCtx->bit_rate;

	m_ptVidEncCodecCtx->qmax = ptVidEncParam->m_byMaxQuant;
	m_ptVidEncCodecCtx->qmin = ptVidEncParam->m_byMinQuant;

	/* video resolution */
	m_ptVidEncCodecCtx->width  = ptVidEncParam->m_wEncPicWidth;
	m_ptVidEncCodecCtx->height = ptVidEncParam->m_wEncPicHeight;

	/* frames per second */
	m_ptVidEncCodecCtx->time_base.num = 1;
	m_ptVidEncCodecCtx->time_base.den = ptVidEncParam->m_byFrameRate;
	m_ptVidEncCodecCtx->framerate.num = ptVidEncParam->m_byFrameRate;
	m_ptVidEncCodecCtx->framerate.den = 1;

	/* gop */
	m_ptVidEncCodecCtx->gop_size   = ptVidEncParam->m_byMaxKeyFrameInterval;
	m_ptVidEncCodecCtx->keyint_min = ptVidEncParam->m_byMaxKeyFrameInterval;

	/* set pixel format, eg: AV_PIX_FMT_YUV420P or AV_PIX_FMT_QSV; */
	m_ptVidEncCodecCtx->pix_fmt = emPixFmt;

	/* SAR */
	m_ptVidEncCodecCtx->sample_aspect_ratio.num = 1;
	m_ptVidEncCodecCtx->sample_aspect_ratio.den = 1;

	/* set video encoder profile */
	if (m_ptVidEncCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		/* set AVC video profile */
		switch (ptVidEncParam->m_byEncProfile)
		{
		case FFMPEG_VIDEO_BASE_PROFILE:
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "profile", "baseline", 0);
			break;

		case FFMPEG_VIDEO_MAIN_PROFILE:
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "profile", "main", 0);
			break;

		case FFMPEG_VIDEO_HIGH_PROFILE:
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "profile", "high", 0);
			break;

		default:
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "profile", "high", 0);
			break;
		}
	}
	else if (m_ptVidEncCodecCtx->codec_id == AV_CODEC_ID_H265)
	{
		av_opt_set(m_ptVidEncCodecCtx->priv_data, "profile", "main", 0);
	}

	/* malloc AVFrame */
	m_ptVidEncFrm = FFmpegMallocAVFrame();
	if (!m_ptVidEncFrm)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Init] Malloc AVframe memory error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	/* malloc AVPacket */
	m_ptVidEncPkt = FFmpegMallocAVPacket();
	if (!m_ptVidEncPkt)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Init] Malloc AVpacket memory error.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	return FF_ERR_NONE;
}

/* 
 * open video encoder CodecContext 
 */
s16 CFFmpegVideoEncoderWrapper::FFmpegOpenVideoEncoder()
{
	u32 dwRet = FF_ERR_NONE;

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [openCodec] codecContext NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	dwRet = avcodec_open2(m_ptVidEncCodecCtx, m_ptVidEncCodec, nullptr);
	if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [openCodec] open codecContext error.\n");
		return FF_ERR_INVALID_HANDLE;
	}

	return FF_ERR_NONE;
}

/*
 * malloc AVPacket
 */
AVPacket* CFFmpegVideoEncoderWrapper::FFmpegMallocAVPacket()
{
	u32 dwRet = FF_ERR_NONE;
	AVPacket* pPkt = nullptr;

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [MallocPacket] codecContext NULL.\n");
		return nullptr;
	}

	/* malloc AVPacket */
	pPkt = av_packet_alloc();
	if (!pPkt)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [MallocPacket] alloc AVPacket error.\n");
	}

	return pPkt;
}

/*
* malloc AVFrame
*/
AVFrame* CFFmpegVideoEncoderWrapper::FFmpegMallocAVFrame()
{
	AVFrame *pFrame = nullptr;

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [MallocFrame] codecContext NULL.\n");
		return nullptr;
	}

	/* malloc AVFrame */
	pFrame = av_frame_alloc();
	if (!pFrame)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [MallocFrame] alloc AVFrame error.\n");
	}

	return pFrame;
}

s16 CFFmpegVideoEncoderWrapper::FFmpegGetAVFrameBuffer(AVFrame *pFrame, enum AVPixelFormat emPixFmt, 
	                                                   u32 dwWidth, u32 dwHeight)
{
	u32 dwRet = FF_ERR_NONE;

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [GetBuffer] codecContext NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!pFrame)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [GetBuffer] Frame NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	pFrame->format = emPixFmt;
	pFrame->width  = dwWidth;
	pFrame->height = dwHeight;

	dwRet = av_frame_get_buffer(pFrame, 32);
	if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [GetBuffer] get AVFrame buffer error.\n");
		return FF_ERR_NOT_ENOUGH_BUFFER;
	}

	return FF_ERR_NONE;
}


enum AVPixelFormat CFFmpegVideoEncoderWrapper::FFmpegConvertInputFmtToAVPixFmt(em_FFVidPixIdx emInputPixIdx)
{
	enum AVPixelFormat emPixFmt = AV_PIX_FMT_NONE;

	switch (emInputPixIdx)
	{
	case FF_VID_PIX_UNKNOWN:
		break;

	case FF_VID_PIX_I420:
		emPixFmt = AV_PIX_FMT_YUV420P;
		break;

	case FF_VID_PIX_NV12:
		emPixFmt = AV_PIX_FMT_NV12;
		break;

	default:
		emPixFmt = AV_PIX_FMT_YUV420P;
		break;
	}

	return emPixFmt;
}

/*
 * convert input raw data to AVFrame(yuv420p/nv12)
*/
s16 CFFmpegVideoEncoderWrapper::FFmpegConvertVideoToAVFrame(const TFFVideoYUVFrame *ptVidRawData)
{
	u32 dwPicSize = 0;

	if (!ptVidRawData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Convert] input Rawdata NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_bCreate || !m_ptVidEncCodecCtx || !m_ptVidEncFrm)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Convert] codecContext or AVFrame NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	m_emInPixFmt = FFmpegConvertInputFmtToAVPixFmt(ptVidRawData->m_emInputPixIdx);
	if (m_emInPixFmt == AV_PIX_FMT_NONE)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Convert] not support pixel.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	dwPicSize = ptVidRawData->m_dwVidWidth * ptVidRawData->m_dwVidHeight;

	m_ptVidEncFrm->width  = ptVidRawData->m_dwVidWidth;
	m_ptVidEncFrm->height = ptVidRawData->m_dwVidHeight;

	m_ptVidEncFrm->pts = m_ptVidEncCodecCtx->frame_number;

	m_ptVidEncFrm->key_frame = 0;
	m_ptVidEncFrm->pict_type = AV_PICTURE_TYPE_NONE;

	if (FFmpegGetAVFrameBuffer(m_ptVidEncFrm, m_emInPixFmt, m_ptVidEncFrm->width, m_ptVidEncFrm->height) < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsScale] Could not alloc AVFrame data buffer.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	av_log(m_ptVidEncCodecCtx, AV_LOG_DEBUG, "FFmpegEncoder [SwsScale] frame number[%lld].\n", m_ptVidEncCodecCtx->frame_number);

	/* set key frame */
	if (m_bReqIDR)
	{
		m_ptVidEncFrm->key_frame = 1;
		m_ptVidEncFrm->pict_type = AV_PICTURE_TYPE_I;
		m_bReqIDR = false;
	}

	switch (m_emInPixFmt)
	{
	case AV_PIX_FMT_YUV420P: // for SW encoder
		memcpy(m_ptVidEncFrm->data[0], ptVidRawData->m_pVidFrameY, dwPicSize);
		memcpy(m_ptVidEncFrm->data[1], ptVidRawData->m_pVidFrameU, dwPicSize >> 2);
		memcpy(m_ptVidEncFrm->data[2], ptVidRawData->m_pVidFrameV, dwPicSize >> 2);

		break;

	case AV_PIX_FMT_NV12: // for HW encoder
		memcpy(m_ptVidEncFrm->data[0], ptVidRawData->m_pVidFrameY, dwPicSize);
		memcpy(m_ptVidEncFrm->data[1], ptVidRawData->m_pVidFrameU, dwPicSize >> 1);

		break;

	default:
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (m_emInPixFmt != m_emOutPixFmt)
	{

		if (FFmpegCreatePicSwsContext(m_emInPixFmt, ptVidRawData->m_dwVidWidth, ptVidRawData->m_dwVidHeight) < 0)
		{
			av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Convert] not support pixel.\n");
			return FF_ERR_INVALID_VIDEO_PARAM;
		}

		if (FFmpegRunPicSwsScale(m_ptVidEncFrm) < 0)
		{
			av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [Convert] swscale error.\n");
			return FF_ERR_INVALID_VIDEO_PARAM;
		}

		av_frame_ref(m_ptVidEncFrm, m_ptVidCntFrm);

	}
	return FF_ERR_NONE;
}

/*
 * encode one frame
 * return value:
 *                 < 0 : error
 *    FF_ERR_NONE    0  : no error
 *    FF_STS_EOF     1  : EOF
 *  FF_STS_HAVE_PKT  2  : must be packet
 */
s16 CFFmpegVideoEncoderWrapper::FFmpegEncodeOneFrame(u8 *pbyOutData, u32 &dwOutLen, 
	                                                 AVFrame *ptVidEncFrm, BOOL32 &bKeyFlag)
{
	s8 chbuf[1024] = {0};
	u32 dwRet = FF_ERR_NONE;

	if (!pbyOutData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [encodeFrame] output buffer NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_bCreate || !m_ptVidEncCodecCtx || !m_ptVidEncPkt)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [encodeFrame] codeContext or AVPacket NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	dwOutLen = 0;
	bKeyFlag = false;

	av_packet_unref(m_ptVidEncPkt);

	/* send the frame to the encoder */
	dwRet = avcodec_send_frame(m_ptVidEncCodecCtx, ptVidEncFrm);
	if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [encodeFrame] Send frame encoding error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	dwRet = avcodec_receive_packet(m_ptVidEncCodecCtx, m_ptVidEncPkt);
	if (dwRet == AVERROR(EAGAIN))
	{
		av_strerror(dwRet, chbuf, 1024);
		av_log(m_ptVidEncCodecCtx, AV_LOG_WARNING, "FFmpegEncoder [encodeFrame] recvive pkt return EAGAIN [%d][%s].\n", dwRet, chbuf);
		return FF_ERR_NONE;
	}
	else if (dwRet == AVERROR_EOF)
	{
		av_strerror(dwRet, chbuf, 1024);
		av_log(m_ptVidEncCodecCtx, AV_LOG_DEBUG, "FFmpegEncoder [encodeFrame] recvive pkt return EOF [%d][%s].\n", dwRet, chbuf);
		return FF_STS_EOF;
	}
	else if (dwRet < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [encodeFrame] recvive pkt return Error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (m_ptVidEncPkt->flags & AV_PKT_FLAG_KEY)
	{
		bKeyFlag = true;
	}

	av_log(m_ptVidEncCodecCtx, AV_LOG_DEBUG, "FFmpegEncoder [encodeFrame] Encoded packet keyFlag [%d]  pts %lld (size=%d).\n",
		   bKeyFlag, m_ptVidEncPkt->pts, m_ptVidEncPkt->size);

	m_ptVidEncPkt->stream_index = 0;
	memcpy(pbyOutData, m_ptVidEncPkt->data, m_ptVidEncPkt->size);

	dwOutLen = m_ptVidEncPkt->size;

	if (ptVidEncFrm)
	{
		av_frame_unref(ptVidEncFrm);
	}

	return FF_STS_HAVE_PKT;
}

AVPacket* CFFmpegVideoEncoderWrapper::FFmpegGetVideoAVPacket()
{
	return m_ptVidEncPkt;
}

void CFFmpegVideoEncoderWrapper::FFmpegSetRequestIDR()
{
	m_bReqIDR = true;
}


void CFFmpegVideoEncoderWrapper::FFmpegSetOutPixFormat(enum AVPixelFormat emSWPixFmt)
{
	m_emOutPixFmt = emSWPixFmt;
}

void CFFmpegVideoEncoderWrapper::FFmpegDumpEncoderContext()
{
	if (!m_bCreate)
	{
		return;
	}

	if (m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "FFmpegEncoder some infomation.\n");
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "Encoder codecId [%d]. pixel format [%d]\n", 
			   m_ptVidEncCodecCtx->codec_id, m_ptVidEncCodecCtx->pix_fmt);
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "Frame width [%d] height [%d].\n", 
			   m_ptVidEncCodecCtx->width, m_ptVidEncCodecCtx->height);
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "Frame framerate [%f] GOP [%d].\n",
			   av_q2d(m_ptVidEncCodecCtx->framerate), m_ptVidEncCodecCtx->gop_size);
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "Frame bitrate [%d].\n", m_ptVidEncCodecCtx->bit_rate);
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "Frame rc max bitrate [%d] min bitrate [%d] buffer size [%d].\n",
			   m_ptVidEncCodecCtx->rc_max_rate, m_ptVidEncCodecCtx->rc_min_rate, m_ptVidEncCodecCtx->rc_buffer_size);
		av_log(m_ptVidEncCodecCtx, AV_LOG_INFO, "Frame max qp [%d]  min qp [%d].\n",
			   m_ptVidEncCodecCtx->qmax, m_ptVidEncCodecCtx->qmin);
	} 
}

s16 CFFmpegVideoEncoderWrapper::FFmpegCreatePicSwsContext(enum AVPixelFormat emPixFmt, u32 dwWidth, u32 dwHeight)
{
	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsCtx] FormatCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	if (m_emOutPixFmt == AV_PIX_FMT_NONE)
	{
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	if (m_ptPicSwsCtx)
	{
		return FF_ERR_NONE;
	}

	/*
	 * only convert pixel format for input AVframe
	*/
	if (emPixFmt == m_emOutPixFmt)
	{
		return FF_ERR_NONE;
	}

	/* create scaling context */
	m_ptPicSwsCtx = sws_getContext(dwWidth, dwHeight, emPixFmt,
		                           dwWidth, dwHeight, m_emOutPixFmt,
		                           SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!m_ptPicSwsCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsCtx] Couldn't create scale context.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}


	return FF_ERR_NONE;
}

s16 CFFmpegVideoEncoderWrapper::FFmpegRunPicSwsScale(AVFrame *ptFrm)
{
	u32 dwRet = 0;

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsScale] FormatCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	if (m_ptPicSwsCtx)
	{
		/* allocate convert destination picture buffers,  must be free in end */
		if (!m_ptVidCntFrm)
		{
			m_ptVidCntFrm = FFmpegMallocAVFrame();
			if (!m_ptVidCntFrm)
			{
				av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsScale] Could not alloc Converted AVFrame.\n");
				return FF_ERR_MEMORY_ALLOC;
			}

			if (FFmpegGetAVFrameBuffer(m_ptVidCntFrm, m_emOutPixFmt, ptFrm->width, ptFrm->height) < 0)
			{
				av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsScale] Could not alloc AVFrame data buffer.\n");
				return FF_ERR_MEMORY_ALLOC;
			}
		}

		/* convert to destination pixel format */
		dwRet = sws_scale(m_ptPicSwsCtx, (const uint8_t * const*)ptFrm->data,
			              ptFrm->linesize, 0, ptFrm->height,
			              m_ptVidCntFrm->data, m_ptVidCntFrm->linesize);

		if (dwRet <= 0)
		{
			av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
			av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFmpegEncoder [SwsScale] Swscale error [%s].\n", m_chErrBuf);
			return FF_ERR_UNSUPPORTED;
		}

		m_ptVidCntFrm->pts = ptFrm->pts;
		m_ptVidCntFrm->key_frame = ptFrm->key_frame;
		m_ptVidCntFrm->pict_type = ptFrm->pict_type;
	
		av_log(m_ptVidEncCodecCtx, AV_LOG_DEBUG, "FFmpegEncoder [SwsScale] frame pts [%lld].\n", m_ptVidCntFrm->pts);

		av_frame_unref(ptFrm);
	}

	return FF_ERR_NONE;
}