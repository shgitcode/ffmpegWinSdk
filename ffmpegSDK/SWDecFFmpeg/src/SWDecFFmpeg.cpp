
#include "FFVideoDecoderType.h"
#include "FFVidDecoderInterface.h"
#include <iostream>
#include <chrono>

#define DECODER_FRAME_SIZE  4 * 1024 *1024
#define INPUT_FRAME_SIZE  4 * 1024

#define DEV_FRAME_NUM      2000

#define CFFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

u8  g_aOutData[DECODER_FRAME_SIZE] = { 0 };


u32 TestFFVideoDecoder(TFFVideoDecoderParam &tVidDecParam)
{
	s16 s16Step = 0;
	s16 s16Ret  = 0;
	FILE *pYuvFile = nullptr;  // save decoded data to YUV
	FILE *pRawFile = nullptr;  // h264 file
	void *pHandle  = nullptr;  // decoder handle
	u32 dwOutSize  = 0;
	u8 *pInData    = nullptr;
	u32 dwInSize   = 0;

	/* open file to store decoded video packet */
	errno_t err;
	err = fopen_s(&pYuvFile, "DecRaw.yuv", "wb");
	if (err != 0)
	{
		std::cout << "Decoder Could not open YUV file." << std::endl;
		return -1;
	}

	if (tVidDecParam.m_bDirectData == FF_VID_DATA_TYPE_RAW)
	{
		/* open source video file : f.h264 */
		err = fopen_s(&pRawFile, tVidDecParam.m_chInputFile, "rb");
		if (err != 0)
		{
			std::cout << "Decoder Could not open decRaw.h264 file." << std::endl;
			return -1;
		}

		pInData = new u8[INPUT_FRAME_SIZE];
	}
	else if (tVidDecParam.m_bDirectData == FF_VID_DATA_TYPE_DEV)
	{
		/* open source video data from deskstop */
		memcpy(tVidDecParam.m_chFormatName, "gdigrab", DECODER_NAME_SIZE);
	}

	/* create decoder instance handle */
	pHandle = FFVideoDec_CreateInstance();
	if (!pHandle)
	{
		std::cout << "Decoder Create video Decoder handle error." << std::endl;
		return -1;
	}

	/* Init decoder instance by parameters */
	s16Ret = FFVideoDec_InitDecoder(pHandle, &tVidDecParam);
	if (s16Ret < 0)
	{
		std::cout << "Decoder Init video Decoder error." << std::endl;
		return -1;
	}

	std::cout << "Decoder decode one video frame." << std::endl;

	auto startTime = std::chrono::system_clock::now();
	auto endTime   = startTime;

	/* decode one video frame */
	while (1)
	{
		memset(g_aOutData, 0, DECODER_FRAME_SIZE);
		dwOutSize = DECODER_FRAME_SIZE;

		/* directly read packet from input file */
		if (tVidDecParam.m_bDirectData == FF_VID_DATA_TYPE_RAW)
		{
			memset(pInData, 0, INPUT_FRAME_SIZE);
			dwInSize = fread(pInData, 1, INPUT_FRAME_SIZE, pRawFile);
			if (!dwInSize)
			{
				std::cout << "Decoder open decRaw.h264 file finished!" << std::endl;
				break;
			}

            #ifdef _DEBUG
			std::cout << "Decoder read video packet size: " << dwInSize << std::endl;
            #endif
		}

		/* decode one frame */
		s16Ret = FFVideoDec_DecodeFrame(pHandle, pInData, dwInSize, g_aOutData, dwOutSize);
		if (s16Ret == FF_STS_EOF)
		{
			std::cout << "Decoder video frame finished!" << std::endl;
			break;
		}
		else if (s16Ret < 0)
		{
			std::cout << "Decoder video frame error!" << std::endl;
			break;
		}

        #ifdef _DEBUG
		std::cout << "Decoder video frame size: " << dwOutSize << std::endl;
        #endif

		if (s16Ret == FF_STS_HAVE_PKT)
		{
			fwrite(g_aOutData, 1, dwOutSize, pYuvFile);
			s16Step++;
		}

		if (tVidDecParam.m_bDirectData == FF_VID_DATA_TYPE_DEV)
		{
			if (s16Step > DEV_FRAME_NUM)
			{
				break;
			}
		}
	}

	endTime = std::chrono::system_clock::now();

	std::cout << "Total Time Cost : " 
		      << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() 
		      << std::endl;

	/* flush the decoder:decoded frame can saved in decoder buffer */
	while (1)
	{
		memset(g_aOutData, 0, DECODER_FRAME_SIZE);
		dwOutSize = DECODER_FRAME_SIZE;

		std::cout << "Decoder Flush video frame: " << s16Step << std::endl;

		s16Ret = FFVideoDec_FlushFrame(pHandle, g_aOutData, dwOutSize);
		if (s16Ret == FF_STS_EOF)
		{
			std::cout << "Decoder Flush video frame finished!" << std::endl;
			break;
		}
		else if (s16Ret < 0)
		{
			std::cout << "Decoder Flush video frame error!" << std::endl;
			break;
		}

		std::cout << "Decoder Flush video frame size: " << dwOutSize << std::endl;

		if (s16Ret == FF_STS_HAVE_PKT)
		{
			fwrite(g_aOutData, 1, dwOutSize, pYuvFile);
			s16Step++;
		}
		else
		{
			std::cout << "Decoder Flush video frame end!" << std::endl;
			break;
		}
	}

	std::cout << "Total frames : " << s16Step << std::endl;

	if (tVidDecParam.m_bDirectData)
	{
		fclose(pRawFile);

		delete []pInData;
	}

	fclose(pYuvFile);

	std::cout << "Close Decoder Instance!" << std::endl;

	FFVideoDec_CloseDecoder(pHandle);

	FFVideoDec_DestoryInstance(pHandle);
	
	std::cout << "This Decoder program end!" << std::endl;

	return 0;
}


void usage_help(char *name)
{
	fprintf(stderr, "\nUsage: %s <dataType> <devType> <width> <height> <codec> <input file> \n", name);
	fprintf(stderr, "  <dataType> : 0 - read packet from file or stream in decoder instance;\n");
	fprintf(stderr, "               1 - directly send raw packet to decoder instance,\n ");
	fprintf(stderr, "                   so must read packet from rawvideo file outside instance;\n");
	fprintf(stderr, "               2 - read packet from device (camera/screen recoder)in decoder instance.\n ");
	fprintf(stderr, "   <devType> : 0 - software decoder;\n");
	fprintf(stderr, "               1 - dxva2 device;\n");
	fprintf(stderr, "               2 - qsv device;\n");
	fprintf(stderr, "               3 - cuda device;.\n");
	fprintf(stderr, "     <codec> : h264/hevc/h264_qsv/hevc_qsv/h264_cuvid/hevc_cuvid.\n");
	fprintf(stderr, "<input file> : for dataType 0 : 11.mp4.\n");
	fprintf(stderr, "               for dataType 1 : 11.h264.\n");
	fprintf(stderr, "               for dataType 2 : desktop/video=screen-capture-recorder/video=Integrated Camera.\n");

	return;
}


int main(int argc, char **argv)
{
	TFFVideoDecoderParam tVidDecParam;

	// 输入判断
	if (argc>1 && !strcmp(argv[1], "-h"))
	{
		usage_help(argv[0]);
		return -1;
	}

	if (argc != 7)
	{
		usage_help(argv[0]);
		return -1;
	}

	memset(&tVidDecParam, 0, sizeof(tVidDecParam));

	tVidDecParam.m_bDirectData = (em_FFVidDataType)atoi(argv[1]);
	tVidDecParam.m_emDecType   = (em_FFVidHWDecType)atoi(argv[2]);//FF_VID_HW_DEC_TYPE_NONE;
	tVidDecParam.m_dwPicWidth  = atoi(argv[3]);
	tVidDecParam.m_dwPicHeight = atoi(argv[4]);

	memcpy(tVidDecParam.m_chCodecName, argv[5], DECODER_NAME_SIZE);

	// 解码库从文件读取的情况 (m_bDirectData = 0)
	memcpy(tVidDecParam.m_chInputFile, argv[6], DECODER_FILE_SIZE);

	// 逐步向解码库传入压缩数据情况,(m_bDirectData = 1)
	tVidDecParam.m_emDecCodec = FF_VID_DEC_CODEC_H264;

	tVidDecParam.m_emPixIdx = FF_VID_DEC_PIX_I420;

	if (TestFFVideoDecoder(tVidDecParam) < 0)
	{
		std::cout << "FFvideo Decoder failed!" << std::endl;
		return -1;
	}

	std::cout << "FFvideo Decoder success!" << std::endl;

	return 0;
}

