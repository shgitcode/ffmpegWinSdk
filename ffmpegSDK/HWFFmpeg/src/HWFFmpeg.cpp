#include <iostream>
#include <chrono>
#include "FFVidEncoderType.h"
#include "FFVidFrameType.h"
#include "FFVidEncoderInterface.h"

#define VIDEO_PKT_SIZE   (1024*1000)
#define VIDEO_PKT_NUM   100

/* encoder last senconds */
#define ENCODED_FRAME_LAST_TIME    2

using namespace std::chrono;

FILE *g_pInputF = nullptr;
u8  g_aOutPkt[VIDEO_PKT_SIZE] = {0};

/* generate NV12 video frame data */
void GenerateNV12VideoFrame(TFFVideoYUVFrame *tVidFrm, s16 s16EncodeStep)
{
	s16 x = 0, y = 0;
	u32 dwLineSize = 0;

	/* prepare a dummy image: pixel format is nv12 */
	/* Y */
	dwLineSize = tVidFrm->m_dwVidWidth;
	for (y = 0; y < tVidFrm->m_dwVidHeight; y++) {
		for (x = 0; x < tVidFrm->m_dwVidWidth; x++) {
			tVidFrm->m_pVidFrameY[y * dwLineSize + x] = x + y + s16EncodeStep * 3;
		}
	}

	/* Cb and Cr */
	for (y = 0; y < tVidFrm->m_dwVidHeight>>1; y++) {
		for (x = 0; x < tVidFrm->m_dwVidWidth>>1; x++) {
			tVidFrm->m_pVidFrameU[y * dwLineSize + 2*x] = 128 + y + s16EncodeStep * 2;
			tVidFrm->m_pVidFrameU[y * dwLineSize + 2*x + 1] = 64 + x + s16EncodeStep * 5;
		}
	}
}


/* generate YUV420P video frame data */
void GenerateI420VideoFrame(TFFVideoYUVFrame *tVidFrm, s16 s16EncodeStep)
{
	s16 x = 0, y = 0;
	u32 dwLineSize = 0;

	/* prepare a dummy image: pixel format is I420 */
	/* Y */
	dwLineSize = tVidFrm->m_dwVidWidth;
	for (y = 0; y < tVidFrm->m_dwVidHeight; y++) {
		for (x = 0; x < tVidFrm->m_dwVidWidth; x++) {
			tVidFrm->m_pVidFrameY[y * dwLineSize + x] = x + y + s16EncodeStep * 3;
		}
	}

	/* Cb and Cr */
	dwLineSize = tVidFrm->m_dwVidWidth >> 1;
	for (y = 0; y < tVidFrm->m_dwVidHeight >> 1; y++) {
		for (x = 0; x < tVidFrm->m_dwVidWidth >> 1; x++) {
			tVidFrm->m_pVidFrameU[y * dwLineSize + x] = 128 + y + s16EncodeStep * 2;
			tVidFrm->m_pVidFrameV[y * dwLineSize + x + 1] = 64 + x + s16EncodeStep * 5;
		}
	}
}

/*
*  rc:  X264显式支持的一趟码率控制方法有：ABR, CQP, CRF. 缺省方法是CRF。这三种方式的优先级是ABR > CQP > CRF.
*  if ( bitrate )                rc_method = ABR;
*  else if ( qp || qp_constant ) rc_method = CQP;
*  else                          rc_method = CRF;
*/
u32 TestHWVideoEncoder(const TFFVideoEncParam &tVidEncParam, em_FFVidPixIdx emVidPix)
{
	s16 flushCount = 0;
	s16 s16Ret = 0;
	s16 s16EncodeStep = 0;
	u32 dwPicSize = 0;
	u32 dwYCbCrSize = 0;
	u32 dwReadSize = 0;
	u32 dwOutSize = 0;
	FILE *pf = NULL;
	void *pHandle = NULL;
	TFFVideoYUVFrame tVidFrm;
	BOOL32 bKeyFlag = true;
	BOOL32 bEOF = false;

	/* open file to store video packet */
#ifdef _DEBUG
	errno_t err;
	err = fopen_s(&pf, "HWVid.h264", "wb");
	if (err != 0)
	{
		std::cout << "Could not open test file\n" << std::endl;
		return -1;
	}
#endif

	//tVidFrm = new TFFVideoYUVFrame;
	
	tVidFrm.m_dwVidWidth  = tVidEncParam.m_wEncPicWidth;
	tVidFrm.m_dwVidHeight = tVidEncParam.m_wEncPicHeight;
	
	tVidFrm.m_emInputPixIdx = emVidPix;

	dwPicSize = tVidEncParam.m_wEncPicWidth * tVidEncParam.m_wEncPicHeight;
	dwYCbCrSize = dwPicSize * 3 / 2;

	tVidFrm.m_pVidFrameY = new u8[dwYCbCrSize];

	if (emVidPix == FF_VID_PIX_NV12)
	{
		tVidFrm.m_pVidFrameU = tVidFrm.m_pVidFrameY + dwPicSize;
	}
	else if (emVidPix == FF_VID_PIX_I420)
	{
		tVidFrm.m_pVidFrameU = tVidFrm.m_pVidFrameY + dwPicSize;
		//tVidFrm.m_pVidFrameV = tVidFrm.m_pVidFrameU + (dwPicSize >> 2);
		tVidFrm.m_pVidFrameV = tVidFrm.m_pVidFrameY + dwPicSize * 5 / 4;
	}
	else
	{
		return -1;
	}

	// create instance handle
	pHandle = FFVideoEnc_CreateInstance();
	if (!pHandle)
	{
		std::cout << "Create SW video encoder hamdle error\n" << std::endl;
		return -1;
	}

	s16Ret = FFVideoEnc_InitEncoder(pHandle, em_FF_INTELHW, &tVidEncParam);
	if (s16Ret < 0)
	{
		std::cout << "Init SW video encoder codec error\n" << std::endl;
		return -1;
	}

	auto encEnd = system_clock::now() + seconds(ENCODED_FRAME_LAST_TIME);
	auto encStart = system_clock::now();

	/* encode one video frame */
	//while (encStart <= encEnd)
	while(1)
	{
		/* prepare a dummy image */
		memset(tVidFrm.m_pVidFrameY, 0, dwYCbCrSize);

		/* read video frame from file */
		if (g_pInputF)
		{
			// read Y
			dwReadSize = fread((uint8_t*)(tVidFrm.m_pVidFrameY), dwPicSize, 1, g_pInputF);
			if (dwReadSize <= 0)
			{
				std::cout << "AVC Encode  read Y end" << std::endl;
				break;
			}

			if (emVidPix == FF_VID_PIX_NV12)
			{
				// read U and V
				dwReadSize = fread((uint8_t*)(tVidFrm.m_pVidFrameU), dwPicSize >> 1, 1, g_pInputF);
				if (dwReadSize <= 0)
				{
					std::cout << "AVC Encode  read U&V end" << std::endl;
					break;
				}
			}
			else if (emVidPix == FF_VID_PIX_I420)
			{
				// read U
				dwReadSize = fread((uint8_t*)(tVidFrm.m_pVidFrameU), dwPicSize >> 2, 1, g_pInputF);
				if (dwReadSize <= 0)
				{
					std::cout << "AVC Encode  read U end" << std::endl;
					break;
				}

				// read V
				dwReadSize = fread((uint8_t*)(tVidFrm.m_pVidFrameV), dwPicSize >> 2, 1, g_pInputF);
				if (dwReadSize <= 0)
				{
					std::cout << "AVC Encode  read V end" << std::endl;
					break;
				}
			}

		}
		else
		{
			if (s16EncodeStep > VIDEO_PKT_NUM)
			{
				break;
			}
	
			if (emVidPix == FF_VID_PIX_NV12)
			{
				GenerateNV12VideoFrame(&tVidFrm, s16EncodeStep);
			}
			else if (emVidPix == FF_VID_PIX_I420)
			{
				GenerateI420VideoFrame(&tVidFrm, s16EncodeStep);
			}

			s16EncodeStep++;
		}

		memset(g_aOutPkt, 0, VIDEO_PKT_SIZE);
		dwOutSize = 0;

#ifdef _DEBUG
		/* encode the image */
		printf("Encode [%d] frame!\n", flushCount++);
#endif

		s16Ret = FFVideoEnc_EncodeFrame(pHandle, g_aOutPkt, dwOutSize, bKeyFlag, &tVidFrm);
		if (s16Ret < 0)
		{
			std::cout << "Encode video frame error!\n" << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_EOF)
		{
			bEOF = true;
			std::cout << "Encode video frame end!\n" << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_HAVE_PKT)
		{
#ifdef _DEBUG
			printf("Write encode frame size [%d]!\n", dwOutSize);
			fwrite(g_aOutPkt, 1, dwOutSize, pf);
#endif
		}
	
		encStart = system_clock::now();
	}

	flushCount = 0;

	/* flush the encoder */
	while (!bEOF)
	{
		flushCount++;
		memset(g_aOutPkt, 0, VIDEO_PKT_SIZE);
		dwOutSize = 0;

		s16Ret = FFVideoEnc_FlushFrame(pHandle, g_aOutPkt, dwOutSize, bKeyFlag);
		if (s16Ret < 0)
		{
			std::cout << "Flush video frame error!\n" << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_EOF)
		{
			bEOF = true;
			std::cout << "Flush video frame end!\n" << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_HAVE_PKT)
		{
#ifdef _DEBUG
			printf("Flush [%d] encode frame size [%d]!\n", flushCount, dwOutSize);
			fwrite(g_aOutPkt, 1, dwOutSize, pf);
#endif
		}
	}

#ifdef _DEBUG
	fclose(pf);
#endif

	std::chrono::duration<double> diff = system_clock::now() - encStart;
	std::cout << "HW encoder spend :\n" << diff.count() << std::endl;

	delete[] tVidFrm.m_pVidFrameY;

	FFVideoEnc_CloseEncoder(pHandle);

	FFVideoEnc_DestoryInstance(pHandle);

	std::cout << "This HW Encoder program end!\n" << std::endl;

	return 0;
}

void usage_help(char *name)
{
	fprintf(stderr, "Usage: %s <width> <height> <input_pixel> [<input file>] \n", name);
	fprintf(stderr, "      <input_pixel> : pixel format is NV12 or YUV420P, 0 : yuv420p, 1 : nv12!\n");
	fprintf(stderr, "      <input_file>  : if not NULL, pixel of file must be consistent of input_pixel!\n");
	fprintf(stderr, "      <width> : width of rawvideo!\n");
	fprintf(stderr, "      <height> : height of rawvideo!\n");

	return;
}

// 采用如下命令生成nv12格式文件：
// ffmpeg -i 0001.mp4 -f rawvideo -pix_fmt nv12 -y 0001_640_480_nv12.nv12
// NV12编码
// ffmpeg -f rawvideo -s:v 640x480 -pix_fmt nv12 -i 0001_640_480_nv12.nv12 -profile:v high -vcodec h264_qsv -s 1920x1080 -y hwtest.mp4

int main(int argc, char **argv)
{
	em_FFVidPixIdx   emVidPix = FF_VID_PIX_NV12;
	TFFVideoEncParam tVidEncParam;

	// 输入判断
	if (argc>1 && !strcmp(argv[1], "-h"))
	{
		usage_help(argv[0]);
		return -1;
	}

	if (argc > 5 || argc < 4)
	{
		usage_help(argv[0]);
		return -1;
	}

	if (argc > 3)
	{
		emVidPix = (em_FFVidPixIdx)atoi(argv[3]);
	}

	// 打开输入文件
	if (argc == 5)
	{
		errno_t err;
		err = fopen_s(&g_pInputF, argv[4], "rb");
		if (err != 0)
		{
			std::cout << "AVC Could not open input  file: " << argv[3] << std::endl;
			return -1;
		}
	}

	// 初始化参数
	memset(&tVidEncParam, 0, sizeof(tVidEncParam));

	tVidEncParam.m_byEncCodecType = MEDIA_TYPE_H264;
	tVidEncParam.m_byFrameRate = 30;
	tVidEncParam.m_byMaxKeyFrameInterval = 60;
	tVidEncParam.m_byMaxQuant = 51;
	tVidEncParam.m_byMinQuant = 20;
	tVidEncParam.m_wBitRate = 4096;
	tVidEncParam.m_wEncPicHeight = atoi(argv[2]);
	tVidEncParam.m_wEncPicWidth = atoi(argv[1]);
	tVidEncParam.m_wMaxBitRate = 20480;
	tVidEncParam.m_wMinBitRate = tVidEncParam.m_wBitRate;
	tVidEncParam.m_byEncProfile = FFMPEG_VIDEO_BASE_PROFILE; // FFMPEG_VIDEO_BASE_PROFILE; //;FFMPEG_VIDEO_HIGH_PROFILE

	/* save to local file */
	memcpy(tVidEncParam.m_chOutFile, "hwRawVid.h264", FF_FILE_NAME_SIZE);

	/* push video in rtp protocol */
	//memcpy(tVidEncParam.m_chOutFile, "rtp://127.0.0.1:56782", FF_FILE_NAME_SIZE);
	//memcpy(tVidEncParam.m_chFormatName, "rtp", 32);

	tVidEncParam.m_byKeyFrameNum = 0;
	tVidEncParam.m_bOutputFlag = true;

	if (TestHWVideoEncoder(tVidEncParam, emVidPix) < 0)
	{
		std::cout << "HW video encoder failed!\n" << std::endl;
		return -1;
	}

	if (g_pInputF)
	{
		fclose(g_pInputF);
	}

	return 0;
}
