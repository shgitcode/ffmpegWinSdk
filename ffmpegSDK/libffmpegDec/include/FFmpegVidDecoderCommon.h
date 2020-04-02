#ifndef __H_FFMPEG_VIDEO_DECODER_COMMON_H_H
#define __H_FFMPEG_VIDEO_DECODER_COMMON_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
#include <string>
#include "kdvtype.h"
#include "FFVideoDecoderType.h"

#ifdef __cplusplus 
extern "C" {
#endif 

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/macros.h>

#include <libswscale/swscale.h>

#ifdef __cplusplus 
}
#endif 


#define ERR_BUF_SIZE 1024


#endif

