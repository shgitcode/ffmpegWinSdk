#include <iostream>
#include <chrono>

#include "FFVidEncoderInterface.h"
#include "FFVidEncoderType.h"
#include "FFVidFrameType.h"

using namespace std::chrono;

#define VIDEO_PKT_SIZE   (1024*1000)

/* encoder last senconds */
#define ENCODED_FRAME_LAST_TIME    2

FILE *g_pInputF = nullptr;

void InitSWVideoEncParam(TFFVideoEncParam &tVidEncParam, u32 dwWidth, u32 dwHeight)
{
	memset(&tVidEncParam, 0, sizeof(tVidEncParam));

	tVidEncParam.m_byEncCodecType = MEDIA_TYPE_H264;
	tVidEncParam.m_byFrameRate = 30;
	tVidEncParam.m_byMaxKeyFrameInterval = 60;
	tVidEncParam.m_byMaxQuant = 51;
	tVidEncParam.m_byMinQuant = 20;
	tVidEncParam.m_wBitRate = 4096;
	tVidEncParam.m_wEncPicHeight = dwHeight;
	tVidEncParam.m_wEncPicWidth = dwWidth;
	tVidEncParam.m_wMaxBitRate = 10240;
	tVidEncParam.m_wMinBitRate = tVidEncParam.m_wBitRate;
	tVidEncParam.m_byEncProfile = FFMPEG_VIDEO_BASE_PROFILE; // FFMPEG_VIDEO_BASE_PROFILE; //;FFMPEG_VIDEO_HIGH_PROFILE

	//tVidEncParam.m_bZlatency = true;
	//tVidEncParam.m_wCRF = 24;
	tVidEncParam.m_bAnnexb = true;
	tVidEncParam.m_bForcedIdr = true;
	tVidEncParam.m_byPresetIndx = em_FF_PRESET_UNKNOWN; // em_FF_PRESET_UNKNOWN;;em_FF_PRESET_VERYFAST

	/* save to local file */
	memcpy(tVidEncParam.m_chOutFile, "swVid.mp4", FF_FILE_NAME_SIZE);

	/* push video in rtp protocol */
	//memcpy(tVidEncParam.m_chOutFile, "rtp://127.0.0.1:56782", FF_FILE_NAME_SIZE);
    //memcpy(tVidEncParam.m_chFormatName, "rtp", 32);

	tVidEncParam.m_byKeyFrameNum = 0;
	tVidEncParam.m_bOutputFlag = true;
}


/* generate video yuv420p frame data */
void GenerateVideoFrame(TFFVideoYUVFrame *tVidFrm, s16 s16EncodeStep)
{
	s16 x = 0, y = 0;
	u32 dwLineSize = 0;

	/* prepare a dummy image: pixel format is yuv420p */
	/* Y */
	dwLineSize = tVidFrm->m_dwVidWidth;
	for (y = 0; y < tVidFrm->m_dwVidHeight; y++) {
		for (x = 0; x < tVidFrm->m_dwVidWidth; x++) {
			tVidFrm->m_pVidFrameY[y * dwLineSize + x] = x + y + s16EncodeStep * 3;
		}
	}

	/* Cb and Cr */
	dwLineSize = tVidFrm->m_dwVidWidth >> 1;
	for (y = 0; y < tVidFrm->m_dwVidHeight / 2; y++) {
		for (x = 0; x < tVidFrm->m_dwVidWidth / 2; x++) {
			tVidFrm->m_pVidFrameU[y * dwLineSize + x] = 128 + y + s16EncodeStep * 2;
			tVidFrm->m_pVidFrameV[y * dwLineSize + x] = 64 + x + s16EncodeStep * 5;
		}
	}
}


u32 TestAVCVideoEncoder(const TFFVideoEncParam &tVidEncParam)
{
	s16 s16EncodeStep = 0, s16FlushStep = 0;
	s16 s16Ret = 0;
	u32 dwWidth = 0;
	u32 dwSize = 0;
	u32 dwReadSize = 0;
	u8  *cOutPkt = NULL;
	u32 dwOutSize = 0;

#ifdef _DEBUG
	FILE *pf = NULL;
#endif

	void *pHandle = NULL;
	TFFVideoYUVFrame *tVidFrm = NULL;
	BOOL32 bKeyFlag = true;
	BOOL32 bEOF = false;

#ifdef _DEBUG
	errno_t err;
	err = fopen_s(&pf, "SWVid.h264", "wb");
	if (err != 0)
	{
		std::cout << "AVC Could not open test file." << std::endl;
		return -1;
	}
#endif

	// malloc memory
	cOutPkt = new u8[VIDEO_PKT_SIZE];

	tVidFrm = new TFFVideoYUVFrame;
	
	tVidFrm->m_dwVidWidth  = tVidEncParam.m_wEncPicWidth;
	tVidFrm->m_dwVidHeight = tVidEncParam.m_wEncPicHeight;
	
	dwSize = tVidEncParam.m_wEncPicWidth * tVidEncParam.m_wEncPicHeight;

	tVidFrm->m_pVidFrameY = new u8[dwSize * 3 / 2];
	tVidFrm->m_pVidFrameU = tVidFrm->m_pVidFrameY + dwSize;
	tVidFrm->m_pVidFrameV = tVidFrm->m_pVidFrameY + dwSize * 5 / 4;

	// create instance handle
	pHandle = FFVideoEnc_CreateInstance();
	if (!pHandle)
	{
		std::cout << "AVC Create SW video encoder handle error." << std::endl;
		return -1;
	}

	s16Ret = FFVideoEnc_InitEncoder(pHandle, em_FF_SW, &tVidEncParam);
	if (s16Ret < 0)
	{
		std::cout << "AVC Init SW video encoder codec error." << std::endl;
		return -1;
	}

	auto encEnd   = system_clock::now() + seconds(ENCODED_FRAME_LAST_TIME);
	auto encStart = system_clock::now();

	/* encode one video frame */
	//while (encStart <= encEnd)
	while(1)
	{

#ifdef _DEBUG
		auto encTmpStart = system_clock::now();
#endif
		memset(cOutPkt, 0, VIDEO_PKT_SIZE);
		dwOutSize = 0;

		memset(tVidFrm->m_pVidFrameY, 0, dwSize);
		memset(tVidFrm->m_pVidFrameU, 0, dwSize >> 2);
		memset(tVidFrm->m_pVidFrameV, 0, dwSize >> 2);

		/* generate video frame */
		//GenerateVideoFrame(tVidFrm, s16EncodeStep);
		// read Y
		dwReadSize = fread((uint8_t*)(tVidFrm->m_pVidFrameY), dwSize, 1, g_pInputF);
		if (dwReadSize <= 0)
		{
			std::cout << "AVC Encode  read Y end" << std::endl;
			break;
		}
		// read U
		dwReadSize = fread((uint8_t*)(tVidFrm->m_pVidFrameU), dwSize >> 2, 1, g_pInputF);
		if (dwReadSize <= 0)
		{
			std::cout << "AVC Encode  read U end" << std::endl;
			break;
		}
		// read V
		dwReadSize = fread((uint8_t*)(tVidFrm->m_pVidFrameV), dwSize >> 2, 1, g_pInputF);
		if (dwReadSize <= 0)
		{
			std::cout << "AVC Encode  read V end" << std::endl;
			break;
		}

		/* encode the image */
#ifdef _DEBUG
		std::cout << "AVC Encode  " << s16EncodeStep << " frame." << std::endl;
#endif

		s16Ret = FFVideoEnc_EncodeFrame(pHandle, cOutPkt, dwOutSize, bKeyFlag, tVidFrm);
		if (s16Ret < 0)
		{
			std::cout << "AVC Encode video frame error!." << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_EOF)
		{
			bEOF = true;
			std::cout << "AVC Encode video frame end!." << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_HAVE_PKT)
		{
#ifdef _DEBUG
			std::cout << "AVC Write packet size " << "[ " << dwOutSize  << " ]"<< std::endl;
			if (dwOutSize > 0)
			{
				fwrite(cOutPkt, 1, dwOutSize, pf);
			}
#endif
		}

		encStart = system_clock::now();

#ifdef _DEBUG
		std::chrono::duration<double> diff = encStart - encTmpStart;
		std::cout << "AVC Encode  " << s16EncodeStep << " frame last " << diff.count() << std::endl;
#endif

		s16EncodeStep++;

		if (bKeyFlag)
		{
			s16EncodeStep = 0;
		}
	}

	/* flush the encoder */
	while (!bEOF)
	{
		s16FlushStep++;

		memset(cOutPkt, 0, VIDEO_PKT_SIZE);
		dwOutSize = 0;

		s16Ret = FFVideoEnc_FlushFrame(pHandle, cOutPkt, dwOutSize, bKeyFlag);
		if (s16Ret < 0)
		{
			std::cout << "AVC Flush video frame error!." << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_EOF)
		{
			bEOF = true;
			std::cout << "AVC Flush video frame end!." << std::endl;
			break;
		}
		else if (s16Ret == FF_STS_HAVE_PKT)
		{
#ifdef _DEBUG
			std::cout << "AVC Flush " << s16FlushStep << " packet size " << "[ " << dwOutSize << " ]" << std::endl;

			if (dwOutSize > 0)
			{
				fwrite(cOutPkt, 1, dwOutSize, pf);
			}
#endif
		}
	}

#ifdef _DEBUG
	fclose(pf);
#endif

	std::chrono::duration<double> diff = system_clock::now() - encStart;
	std::cout << "SW encoder spend [ " << diff.count()  << " ]" << std::endl;

	delete[] tVidFrm->m_pVidFrameY;
	delete tVidFrm;
	delete[] cOutPkt;

	FFVideoEnc_CloseEncoder(pHandle);

	FFVideoEnc_DestoryInstance(pHandle);

	std::cout << "This AVC program end!." << std::endl;

	return 0;
}

void usage_help(char *name)
{
	fprintf(stderr, "Usage: %s <width> <height> <input file> \n", name);
	fprintf(stderr, "      <input file> : must be rawvideo and pixel format must be YUV420P!\n");
	fprintf(stderr, "      <width> : width of rawvideo!\n");
	fprintf(stderr, "      <height> : height of rawvideo!\n");
}

// ffmpeg -f rawvideo  -s:v 640x480 -pix_fmt yuv420p -i 0001_640_480_I420.yuv  -vcodec libx264 -s 1920x1080 -y swtest.mp4
int main(int argc, char **argv)
{
	TFFVideoEncParam tVidEncParam;

	// ÊäÈëÅÐ¶Ï
	if (argc>1 && !strcmp(argv[1], "-h"))
	{
		usage_help(argv[0]);
		return -1;
	}

	if (argc < 4) 
	{
		usage_help(argv[0]);
		return -1;
	}

	errno_t err;
	err = fopen_s(&g_pInputF, argv[3], "rb");
	if (err != 0)
	{
		std::cout << "AVC Could not open input  file: " << argv[3] << std::endl;
		return -1;
	}

	InitSWVideoEncParam(tVidEncParam, atoi(argv[1]), atoi(argv[2]));

	if (TestAVCVideoEncoder(tVidEncParam) < 0)
	{
		std::cout << "AVC video encoder failed!\n" << std::endl;
		return -1;
	}

	if (g_pInputF)
	{
		fclose(g_pInputF);
	}

	return 0;
}