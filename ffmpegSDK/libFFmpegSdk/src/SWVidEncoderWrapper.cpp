#include "FFVidEncoderType.h"
#include "SWVidEncoderWrapper.h"
#include "FFmpegVidFormatWrapper.h"

static const s8 * const g_pchPreset[] = {
	"ultrafast",
	"superfast",
	"veryfast",
	"faster",
	"fast",
	"medium",
	"slow",
	"slower",
	"veryslow",
	"placebo",
};

static const s8 * const g_pchSWEncoderName[] = {
	"libx264",
	"libx265",
};

enum em_SWCodecIndx
{
	em_SW_LIBX264_INDX = 0,
	em_SW_LIBX265_INDX,
};


/*************************** public interface *********************************************/

CSWVideoEncoderWrapper::CSWVideoEncoderWrapper()
{
	m_emCodecIndx = em_SW_LIBX264_INDX;
	m_emSWPixFmt = AV_PIX_FMT_NONE;
}

CSWVideoEncoderWrapper::~CSWVideoEncoderWrapper()
{
	CloseFFVideoEncoderCom();
}

/* 
 * init and open SW video encoder codec context
 */
s16  CSWVideoEncoderWrapper::InitFFVideoEncoderCodecCtxCom(const TFFVideoEncParam *ptVidEncParam, 
	                                                       enum em_FFVideoEncoderType emVidType)
{
	s16 s16Ret = FF_ERR_NONE;

	if (!ptVidEncParam)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFSW_ENCODER [Init] input paramter NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if ((ptVidEncParam->m_byEncCodecType != MEDIA_TYPE_H264) &&
		(ptVidEncParam->m_byEncCodecType != MEDIA_TYPE_H265))
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFSW_ENCODER [Init] not support codec type [%d].\n",
			   ptVidEncParam->m_byEncCodecType);
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

	/* Get AVC or HEVC encode codecId */
	switch (ptVidEncParam->m_byEncCodecType)
	{
	case MEDIA_TYPE_H264:

		m_emCodecIndx = em_SW_LIBX264_INDX;
		m_emSWPixFmt = AV_PIX_FMT_YUV420P;
		break;

	case MEDIA_TYPE_H265:

		m_emCodecIndx = em_SW_LIBX265_INDX;
		m_emSWPixFmt = AV_PIX_FMT_YUV420P;
		break;

	default:
		m_emCodecIndx = em_SW_LIBX264_INDX;
		m_emSWPixFmt = AV_PIX_FMT_YUV420P;

		break;
	}

	/* Init codec context */
	s16Ret = FFmpegInitVideoEncodeParameter(ptVidEncParam, 
		                                    g_pchSWEncoderName[m_emCodecIndx],
		                                    m_emSWPixFmt);
	if (s16Ret < FF_ERR_NONE)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFSW_ENCODER [Init] [%s] codec context error.\n",
			g_pchSWEncoderName[m_emCodecIndx]);
		return s16Ret;
	}

	/* before avio_open2() */
	s16Ret = FFmpegSetAttrBeforeOpenVideoEncoder(ptVidEncParam);
	if (s16Ret < FF_ERR_NONE)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFSW_ENCODER [Init] Set [%s] codec \
               attribution error\n", g_pchSWEncoderName[m_emCodecIndx]);
		return s16Ret;
	}

	/* set output pixel format */
	FFmpegSetOutPixFormat(m_emSWPixFmt);

	return FF_ERR_NONE;
}

/*
* open AVCodecContext with avio_open2
*/
s16 CSWVideoEncoderWrapper::OpenFFVideoEncoderCom()
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
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "FFSW_ENCODER [OpenCodec] [%s] encoder codec error.\n",
			   g_pchSWEncoderName[m_emCodecIndx]);
		return s16Ret;
	}

	/* display codec context information */
	FFmpegDumpEncoderContext();

	m_bCreate = true;

	return FF_ERR_NONE;
}

void* CSWVideoEncoderWrapper::GetFFVideoEncoderCom()
{
	return m_ptVidEncCodecCtx;
}

/*
 * first, convert raw data to AVFrame
 * then, encode AVFrame to packet
 * return value:
 *             < 0  : error
 *               0  : no error
 *               1  : EOF
 *               2  : must be packet
*/
s16 CSWVideoEncoderWrapper::EncodeFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag, 
	                                              const TFFVideoYUVFrame *ptVidRawData)
{
	s16 s16Ret = 0;

	if (!pbyOutData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Encode output stored packet data NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!ptVidRawData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Encode input raw data NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_bCreate || !m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Encode codec not initialized.\n");
		return FF_ERR_NOT_INITIALIZED;
	}

	s16Ret = FFmpegConvertVideoToAVFrame(ptVidRawData);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Encode Convert frame error.\n");
		return s16Ret;
	}

	s16Ret = FFmpegEncodeOneFrame(pbyOutData, dwOutLen, m_ptVidEncFrm, bKeyFlag);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Encode one frame error.\n");
	}

	return s16Ret;
}

/*
* flush buffered frame to packet
* return value:
*             < 0  : error
*               0  : no error
*               1  : EOF
*               2  : must be packet
*/
s16 CSWVideoEncoderWrapper::FlushFFVideoFrameCom(u8 *pbyOutData, u32 &dwOutLen, BOOL32 &bKeyFlag)
{
	s16 s16Ret = 0;

	if (!pbyOutData)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Flush output stored packet data NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_bCreate || !m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Flush codec not initialized.\n");
		return FF_ERR_NOT_INITIALIZED;
	}

	s16Ret = FFmpegEncodeOneFrame(pbyOutData, dwOutLen, NULL, bKeyFlag);
	if (s16Ret < 0)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW Flush frame error.\n");
	}

	return s16Ret;
}

void* CSWVideoEncoderWrapper::GetFFVideoAVPacketCom()
{
	return FFmpegGetVideoAVPacket();
}

void CSWVideoEncoderWrapper::CloseFFVideoEncoderCom()
{
	m_emCodecIndx = AV_CODEC_ID_NONE;
	m_emSWPixFmt = AV_PIX_FMT_NONE;

	FFmpegCloseVideoEncoder();
}


void CSWVideoEncoderWrapper::RequestFFVideoKeyFrameCom()
{
	FFmpegSetRequestIDR();
}

/******************************* private function **********************************************************/

/* set video attribute to AVCodecContext */
s16 CSWVideoEncoderWrapper::FFmpegSetAttrBeforeOpenVideoEncoder(const TFFVideoEncParam *ptVidEncParam)
{
	if (!ptVidEncParam)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW set video attribution input NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (!m_ptVidEncCodecCtx)
	{
		av_log(m_ptVidEncCodecCtx, AV_LOG_ERROR, "SW set video attribution codec Context NULL.\n");
		return FF_ERR_NULL_PTR;
	}

	if (m_ptVidEncCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		/* CRF */
		if (ptVidEncParam->m_wCRF > 0)
		{
			//av_opt_set(m_ptVidEncCodecCtx->priv_data, "x264-params", "crf=24", 0);
			av_opt_set_double(m_ptVidEncCodecCtx->priv_data, "crf", ptVidEncParam->m_wCRF, 0);

		}
		
		/* set preset:default medium -- 30fps */
		if ((ptVidEncParam->m_byPresetIndx > em_FF_PRESET_UNKNOWN) && (ptVidEncParam->m_byPresetIndx <= em_FF_PRESET_PLACEBO))
		{
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "preset", g_pchPreset[ptVidEncParam->m_byPresetIndx], 0);
			av_log(m_ptVidEncCodecCtx, AV_LOG_WARNING, "SW set video preset [%s].\n", g_pchPreset[ptVidEncParam->m_byPresetIndx]);
		}

		/* if not set,about 47 frame delay(frame-based threading), 
		 * if want no delay with seting zerolatency(slice-based threading), 
		 * then sliced-threads equal one
		 */
		if (ptVidEncParam->m_bZlatency)
		{
			/* start zerolatency, can use slice-based threading, 
			 * number of slice which one frame contain equal number of thread
			 */
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "tune", "zerolatency", 0);
			/* frame-based encode */
			//av_opt_set(m_ptVidEncCodecCtx->priv_data, "x264-params", "sliced-threads=0", 0);
		}

		if (0) // not set, please use multi-thread
		{
			/* close slice-based threading, then delay number threads -1 , 
			 * so set threads = 1, so can use frame-based encode 
			 */
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "x264-params", "sliced-threads=0", 0);
			//av_opt_set(m_ptVidEncCodecCtx->priv_data, "x264-params", "threads=1", 0); // thread_count
		}

		/* set x264-params :: can call x264_param_parse function */
		/* x264: b_annexb default, then can use x264-params set b_annexb */
		if (!ptVidEncParam->m_bAnnexb)
		{
			av_opt_set(m_ptVidEncCodecCtx->priv_data, "x264-params", "annexb=0", 0);
			//av_opt_set(m_ptVidEncCodecCtx->priv_data, "x264opts", "annexb=0", 0);
		}

		/* force I frame to IDR */
		if (ptVidEncParam->m_bForcedIdr)
		{
			av_opt_set_int(m_ptVidEncCodecCtx->priv_data, "forced-idr", 1, 0);
		}

		/* flags: AV_CODEC_FLAG_GLOBAL_HEADER == not repeat sps pps in key frame header */
		//m_ptVidEncCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	return FF_ERR_NONE;
}
