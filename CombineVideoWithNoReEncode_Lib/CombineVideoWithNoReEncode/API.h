
#include "stdafx.h"

#define LIB_EXPORT_API __declspec(dllexport)
#define MAX_FILES 20

#ifdef __cplusplus
extern"C"
{
#endif
#include <libavformat/avformat.h>
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libswresample\swresample.h"
#include "libavutil\fifo.h"
#include "libavutil/audio_fifo.h"


#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
	//#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
	//#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
	//#pragma comment(lib, "swscale.lib")
#ifdef __cplusplus
};
#endif

#include <string>
#include <vector>

typedef struct
{
	//输入文件
	std::string strFile;	
	//起始时间点
	int64_t time_begin;
	//结束时间点
	int64_t time_end;

}MEDIAIN;

typedef std::vector<MEDIAIN> VECMEDIAIN;

int LIB_EXPORT_API mergeVideo(VECMEDIAIN, char* outFileName);