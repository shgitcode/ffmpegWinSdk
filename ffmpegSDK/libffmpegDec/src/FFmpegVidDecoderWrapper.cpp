
#include "FFVideoDecoderType.h"
#include "FFmpegVidDecoderWrapper.h"

#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "swscale.lib")
#pragma comment (lib, "avdevice.lib")

/*
* AVCodec-H264 decoder : support hwaccel DXVA2(intel/cuda/amd)/NVDEC(cuda)
* AVCodec-HEVC decoder : support hwaccel DXVA2/NVDEC
* AVCodec-h264/hevc_qsv decoder  : support QSV codec
* AVCodec-h264/hevc_cuid decoder : support cuda codec
*/

CFFmpegVideoDecoderWrapper::CFFmpegVideoDecoderWrapper()
{
	m_emOutPixFmt = AV_PIX_FMT_NONE;

	/* codec */
	m_ptVidDecCodecCtx = nullptr;
	m_ptVidDecCodec = nullptr;

	/* swscale */
	m_ptPicSwsCtx = nullptr;

	m_ptVidCntFrm = nullptr;
	m_ptVidDecFrm = nullptr;

	m_dwOutPicWidth = 0;
	m_dwOutPicHeight = 0;

	m_tCodecName.clear();

	memset(m_chErrBuf, 0, ERR_BUF_SIZE);
}

CFFmpegVideoDecoderWrapper:: ~CFFmpegVideoDecoderWrapper()
{
	FFmpegDecCloseDecoder();
}

s16 CFFmpegVideoDecoderWrapper::FFmpegDecInitDecoderParameter(const TFFVideoDecoderParam *ptVidDecParam)
{
	if (!ptVidDecParam)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Init] Input paramter nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	/* set log level */
#ifdef _DEBUG
	av_log_set_level(AV_LOG_DEBUG);
#endif

	if (m_ptVidDecCodecCtx)
	{
		FFmpegDecCloseDecoder();
	}

	/* check input parameter */
	if (FFmpegCheckInputParam(ptVidDecParam) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Init] check input param error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	m_ptVidDecFrm = FFmpegDecMallocAVFrame();
	if (!m_ptVidDecFrm)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Init] Could not alloc AVFrame.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	return FF_ERR_NONE;
}

/* malloc CodecCtx with codec name : h264/hevc/h264_qsv/hevc_qsv */
s16 CFFmpegVideoDecoderWrapper::FFmpegDecCreateDecoder(enum AVCodecID codecId)
{
	u32 dwRet = 0;

	if ((codecId != AV_CODEC_ID_NONE) && 
		(codecId != AV_CODEC_ID_H264) &&
		(codecId != AV_CODEC_ID_HEVC)) // for device(gdigrab)
	{
		m_ptVidDecCodec = avcodec_find_decoder(codecId);
		if (!m_ptVidDecCodec)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [create] Couldn't find\
               [%d] Codec.\n", codecId);
			return FF_ERR_NOT_FOUND;
		}
	}
	else
	{
		m_ptVidDecCodec = avcodec_find_decoder_by_name(m_tCodecName.c_str());
		if (!m_ptVidDecCodec)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [create] Couldn't find\
               [%s] Codec.\n", m_tCodecName.c_str());
			return FF_ERR_NOT_FOUND;
		}
	}

	m_ptVidDecCodecCtx = avcodec_alloc_context3(m_ptVidDecCodec);
	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [create] Couldn't allocate CodecCtx.\n");
		return FF_ERR_MEMORY_ALLOC;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoDecoderWrapper::FFmpegDecOpenDecoder(AVCodecParameters *ptCodecPar)
{
	u32 dwRet = 0;

	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [open] CodecCtx not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	/* Copy codec parameters from input stream to codec context */
	if (ptCodecPar)
	{
		dwRet = avcodec_parameters_to_context(m_ptVidDecCodecCtx, ptCodecPar);
		if (dwRet < 0)
		{
			av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [open] copy codec parameters \
                   to CodecCtx error [%s]\n", m_chErrBuf);
			return FF_ERR_INVALID_VIDEO_PARAM;
		}
	}

	dwRet = avcodec_open2(m_ptVidDecCodecCtx, m_ptVidDecCodec, nullptr);
	if (dwRet < 0)
	{
		av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [open] Decoder error [%s].\n", m_chErrBuf);
		return FF_ERR_UNSUPPORTED;
	}

	av_log(m_ptVidDecCodecCtx, AV_LOG_DEBUG, "FFMDec [open] CodecCtx [w/h] [%d/%d].\n",
		   m_ptVidDecCodecCtx->width, m_ptVidDecCodecCtx->height);

	return FF_ERR_NONE;
}

/*
* malloc AVFrame
*/
AVFrame* CFFmpegVideoDecoderWrapper::FFmpegDecMallocAVFrame()
{
	AVFrame *pFrame = nullptr;

	/* malloc AVFrame */
	pFrame = av_frame_alloc();
	if (!pFrame)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [MallocFrame] alloc AVFrame error.\n");
	}

	return pFrame;
}

s16 CFFmpegVideoDecoderWrapper::FFmpegDecGetAVFrameBuffer(AVFrame *pFrame, enum AVPixelFormat emPixFmt,
	u32 dwWidth, u32 dwHeight)
{
	u32 dwRet = FF_ERR_NONE;

	if (!pFrame)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [GetBuf] Frame NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	pFrame->format = emPixFmt;
	pFrame->width  = dwWidth;
	pFrame->height = dwHeight;

	dwRet = av_frame_get_buffer(pFrame, 32);
	if (dwRet < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [GetBuf] get AVFrame buffer error.\n");
		return FF_ERR_NOT_ENOUGH_BUFFER;
	}

	return FF_ERR_NONE;
}


void CFFmpegVideoDecoderWrapper::FFmpegDecCloseDecoder()
{
	m_emOutPixFmt = AV_PIX_FMT_NONE;

	m_dwOutPicWidth = 0;
	m_dwOutPicHeight = 0;

	m_tCodecName.clear();

	if (m_ptVidDecFrm)
	{
		av_frame_free(&m_ptVidDecFrm);
		m_ptVidDecFrm = nullptr;
	}

	if (m_ptVidCntFrm)
	{
		av_frame_free(&m_ptVidCntFrm);
		m_ptVidCntFrm = nullptr;
	}

	if (m_ptPicSwsCtx)
	{
		sws_freeContext(m_ptPicSwsCtx);
		m_ptPicSwsCtx = nullptr;
	}

	/* codec context */
	if (m_ptVidDecCodecCtx)
	{
		avcodec_free_context(&m_ptVidDecCodecCtx);
		m_ptVidDecCodecCtx = nullptr;
		m_ptVidDecCodec = nullptr;
	}
}

s16 CFFmpegVideoDecoderWrapper::FFmpegDecCreatePicSwsContext(enum AVPixelFormat emPixFmt, u32 dwWidth, u32 dwHeight)
{

	if (m_emOutPixFmt == AV_PIX_FMT_NONE)
	{
		m_emOutPixFmt = emPixFmt;
	}

	if (m_ptPicSwsCtx)
	{
		return FF_ERR_NONE;
	}

	if ((emPixFmt == m_emOutPixFmt)
		&& (dwWidth == m_dwOutPicWidth)
		&& (dwHeight == m_dwOutPicHeight))
	{
		return FF_ERR_NONE;
	}

	/* create scaling context */
	m_ptPicSwsCtx = sws_getContext(dwWidth, dwHeight, emPixFmt,
		                           m_dwOutPicWidth, m_dwOutPicHeight, m_emOutPixFmt,
		                           SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!m_ptPicSwsCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [SwsCtx] Couldn't create scale context.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoDecoderWrapper::FFmpegDecRunPicSwsScale(AVFrame *ptFrm)
{
	u32 dwRet = 0;

	if (!ptFrm)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [SwsScale] input frame NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (m_ptPicSwsCtx)
	{
		/* allocate convert destination picture buffers,  must be free in end */
		if (!m_ptVidCntFrm)
		{
			m_ptVidCntFrm = FFmpegDecMallocAVFrame();
			if (!m_ptVidCntFrm)
			{
				av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [SwsScale] Could not alloc Converted AVFrame.\n");
				return FF_ERR_MEMORY_ALLOC;
			}

			if (FFmpegDecGetAVFrameBuffer(m_ptVidCntFrm, m_emOutPixFmt, m_dwOutPicWidth, m_dwOutPicHeight) < 0)
			{
				av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [SwsScale] Could not alloc AVFrame data buffer.\n");
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
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [SwsScale] Swscale error [%s].\n", m_chErrBuf);
			return FF_ERR_UNSUPPORTED;
		}

		dwRet = av_frame_copy_props(m_ptVidCntFrm, ptFrm);
		if (dwRet < 0) {
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [SwsScale] copy frame props error.\n");
			return FF_ERR_INVALID_VIDEO_PARAM;
		}

		av_log(m_ptVidDecCodecCtx, AV_LOG_DEBUG, "FFMDec [SwsScale] frame pts [%lld].\n", m_ptVidCntFrm->pts);
	}

	return FF_ERR_NONE;
}

/*
* return value:
* -1: error
* 0:  again
* 2:  have video frame
*/
s16  CFFmpegVideoDecoderWrapper::FFmpegDecDecodeAVPacket(AVPacket *pPkt)
{
	u32  dwRet = 0;

	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [packet] Codec not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_ptVidDecFrm)
	{
		m_ptVidDecFrm = FFmpegDecMallocAVFrame();
		if (!m_ptVidDecFrm)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [packet] Could not alloc AVFrame.\n");
			return FF_ERR_MEMORY_ALLOC;
		}
	}
	
	dwRet = avcodec_send_packet(m_ptVidDecCodecCtx, pPkt);
	if (dwRet < 0)
	{
		av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [packet] send packet error [%s].\n", m_chErrBuf);
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	av_frame_unref(m_ptVidDecFrm);

	while (dwRet >= 0)
	{
		dwRet = avcodec_receive_frame(m_ptVidDecCodecCtx, m_ptVidDecFrm);
		if (dwRet == AVERROR(EAGAIN))
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_WARNING, "FFMDec [packet] need put packet again.\n");
			return FF_ERR_NONE;
		}
		else if (dwRet == AVERROR_EOF)
		{
			av_log(m_ptVidDecCodecCtx, AV_LOG_DEBUG, "FFMDec [packet] receive frame EOF.\n");
			return FF_STS_EOF;
		}
		else if (dwRet < 0)
		{
			av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
			av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [packet] receive frame error [%s].\n", m_chErrBuf);
			return FF_ERR_INVALID_VIDEO_PARAM;
		}
		break;
	}

	return FF_STS_HAVE_PKT;
}

s16  CFFmpegVideoDecoderWrapper::FFmpegDecCopyAVFrame(AVFrame *pFrm, u8 *pbyOutData, u32 &dwOutLen)
{
	u32 dwRet = 0;
	u32 dwDataSize = 0;

	if (!pbyOutData)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Copy] output buffer nullptr.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!pFrm)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Copy] input frame NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Copy] Format not Create.\n");
		return FF_ERR_NULL_PTR;
	}

	dwDataSize = av_image_get_buffer_size((enum AVPixelFormat)pFrm->format, pFrm->width, pFrm->height, 1);
	
	dwDataSize = (dwOutLen < dwDataSize) ? dwOutLen : dwDataSize;

	dwRet = av_image_copy_to_buffer(pbyOutData, dwDataSize,
		                            (const uint8_t * const *)pFrm->data, (const int *)pFrm->linesize,
		                            (enum AVPixelFormat)pFrm->format, pFrm->width, pFrm->height, 1);
	if (dwRet < 0)
	{
		av_strerror(dwRet, m_chErrBuf, ERR_BUF_SIZE);
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [Copy] copy frame to pbyOutData error [%s].\n", m_chErrBuf);
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	dwOutLen = dwDataSize;

	//av_frame_unref(pFrm);

	return FF_ERR_NONE;
}

///////////////////////////////////////////////////////////////////////////////////////
/* private function */

s16 CFFmpegVideoDecoderWrapper::FFmpegCheckInputParam(const TFFVideoDecoderParam *ptDecParam)
{
	if (!ptDecParam)
	{
		return FF_ERR_NULL_PTR;
	}

	m_tCodecName.append(ptDecParam->m_chCodecName);

	if (m_tCodecName.empty())
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [check] Codec Name NULL.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	m_dwOutPicWidth  = ptDecParam->m_dwPicWidth;
	m_dwOutPicHeight = ptDecParam->m_dwPicHeight;

	/* set outPixel: convert pixel index to AVpixelFormat */
	if (FFmpegConvertPixFmt(ptDecParam) < 0)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_ERROR, "FFMDec [check] convert pixel format error.\n");
		return FF_ERR_INVALID_VIDEO_PARAM;
	}

	return FF_ERR_NONE;
}


s16 CFFmpegVideoDecoderWrapper::FFmpegConvertPixFmt(const TFFVideoDecoderParam *ptDecParam)
{
	if (!ptDecParam)
	{
		return FF_ERR_NULL_PTR;
	}

	switch (ptDecParam->m_emPixIdx)
	{
	case FF_VID_DEC_PIX_UNKNOWN:
		m_emOutPixFmt = AV_PIX_FMT_NONE; // use raw pixel format
		break;

	case FF_VID_DEC_PIX_I420:
		m_emOutPixFmt = AV_PIX_FMT_YUV420P;
		break;

	case FF_VID_DEC_PIX_NV12:
		m_emOutPixFmt = AV_PIX_FMT_NV12;
		break;

	default:
		m_emOutPixFmt = AV_PIX_FMT_YUV420P;
		break;
	}

	return FF_ERR_NONE;
}

void CFFmpegVideoDecoderWrapper::FFmpegDumpDecoderParam()
{
	if (m_ptVidDecCodecCtx)
	{
		av_log(m_ptVidDecCodecCtx, AV_LOG_INFO, "FFDecoder video pixel format [%d].\n", m_ptVidDecCodecCtx->pix_fmt);
		av_log(m_ptVidDecCodecCtx, AV_LOG_INFO, "FFDecoder video width [%d] height[%d].\n", m_ptVidDecCodecCtx->width,
			m_ptVidDecCodecCtx->height);
	}
}

