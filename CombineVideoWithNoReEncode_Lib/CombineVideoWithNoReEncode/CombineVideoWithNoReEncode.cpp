
#include "stdafx.h"
#include "API.h"

AVFormatContext *in_fmtctx[MAX_FILES],  *out_fmtctx = NULL;
AVStream *out_video_stream = NULL, *out_audio_stream = NULL;
int video_stream_index = -1, audio_stream_index = -1;
int m_nInFiles = 0;
int startAPts = 0;
int startVPts = 0;
int startADts = 0;
int startVDts = 0;
bool bIsAStart = false;
bool bIsVStart = false;

int end_merge(VECMEDIAIN  vecMediaIn)
{
	for (int i=0; i<vecMediaIn.size(); i++)
	{
		avformat_close_input(&in_fmtctx[i]);
	}

	/* close output */
	if (out_fmtctx && !(out_fmtctx->oformat->flags & AVFMT_NOFILE))
		avio_close(out_fmtctx->pb);

	avformat_free_context(out_fmtctx);
	return 0;
}

int open_input(MEDIAIN mediaIn)
{
	int ret = -1;
	if ((ret = avformat_open_input(&in_fmtctx[m_nInFiles], mediaIn.strFile.c_str(), NULL, NULL)) < 0)
	{
		printf("can not open the first input context!\n");
		return ret;
	}
	if ((ret = avformat_find_stream_info(in_fmtctx[m_nInFiles], NULL)) < 0)
	{
		printf("can not find the first input stream info!\n");
		return ret;
	}
	m_nInFiles++;
}

int open_output(const char * out_name)
{
	int ret = -1;
	if ((ret = avformat_alloc_output_context2(&out_fmtctx, NULL, NULL, out_name)) < 0)
	{
		printf("can not alloc context for output!\n");
		return ret;
	}

	//new stream for out put
	for (int i = 0; i < in_fmtctx[0]->nb_streams; i++)
	{
		if (in_fmtctx[0]->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_stream_index = i;
			out_video_stream = avformat_new_stream(out_fmtctx, NULL);
			if (!out_video_stream)
			{
				printf("Failed allocating output1 video stream\n");
				ret = AVERROR_UNKNOWN;
				return ret;
			}
			if ((ret = avcodec_copy_context(out_video_stream->codec, in_fmtctx[0]->streams[i]->codec)) < 0)
			{
				printf("can not copy the video codec context!\n");
				return ret;
			}
			out_video_stream->codec->codec_tag = 0;
			if(out_fmtctx->oformat->flags & AVFMT_GLOBALHEADER)
			{
				out_video_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}
		}
		else if (in_fmtctx[0]->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_stream_index = i;
			out_audio_stream = avformat_new_stream(out_fmtctx, NULL);

			if (!out_audio_stream)
			{
				printf("Failed allocating output1 video stream\n");
				ret = AVERROR_UNKNOWN;
				return ret;
			}
			if ((ret = avcodec_copy_context(out_audio_stream->codec, in_fmtctx[0]->streams[i]->codec)) < 0)
			{
				printf("can not copy the video codec context!\n");
				return ret;
			}
			out_audio_stream->codec->codec_tag = 0;
			if(out_fmtctx->oformat->flags & AVFMT_GLOBALHEADER)
			{
				out_audio_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}
		}
	}

	//open output file
	if (!(out_fmtctx->oformat->flags & AVFMT_NOFILE))
	{
		if ((ret = avio_open(&out_fmtctx->pb, out_name, AVIO_FLAG_WRITE)) < 0)
		{
			printf("can not open the out put file handle!\n");
			return ret;
		}
	}

	//write out  file header
	if ((ret = avformat_write_header(out_fmtctx, NULL)) < 0)
	{
		printf( "Error occurred when opening video output file\n");
		return ret;
	}
}

int mergeVideo(VECMEDIAIN  vecMediaIn, char*outFileName)
{
	unsigned long frameNum = 0;
	unsigned long frameStart = 0;
	unsigned long frameEnd = 0;
	av_register_all();	

	for (unsigned int i=0; i<vecMediaIn.size(); i++)
	{
		if (!open_input(vecMediaIn[i]))
		{
			return false;
		}
	}

	if(0 > open_output(outFileName))
	{
		end_merge(vecMediaIn);
		return false;
	}

	AVFormatContext *input_ctx;
	AVPacket pkt;
	int pts_v = 0, pts_a = 0, dts_v = 0, dts_a = 0, pts_sv = 0, dts_sv = 0, pts_sa = 0, dts_sa = 0;

	for (int i=0; i<vecMediaIn.size(); i++)
	{
		bIsAStart = false;
		bIsVStart = false;
		startAPts = 0;
		startVPts = 0;
		frameNum = 0;
		std::vector<MEDIAIN>::iterator it = vecMediaIn.begin();
		int j = 0;
		for(j = 0; j < i; j++)
		{
			it++;
		}
		frameStart = it->time_begin * 25;
		frameEnd = it->time_end * 25;

		input_ctx = in_fmtctx[i];
		while(1)
		{
			if(0 > av_read_frame(input_ctx, &pkt))
			{
				float vedioDuraTime, audioDuraTime;
				vedioDuraTime = ((float)input_ctx->streams[video_stream_index]->time_base.num / 
					(float)input_ctx->streams[video_stream_index]->time_base.den) * ((float)pts_v);
				audioDuraTime = ((float)input_ctx->streams[audio_stream_index]->time_base.num / 
					(float)input_ctx->streams[audio_stream_index]->time_base.den) * ((float)pts_a);

				if (audioDuraTime > vedioDuraTime)
				{
					dts_v = pts_v = audioDuraTime / ((float)input_ctx->streams[video_stream_index]->time_base.num / 
						(float)input_ctx->streams[video_stream_index]->time_base.den);
					dts_a++;
					pts_a++;
				}
				else
				{
					dts_a = pts_a = vedioDuraTime / ((float)input_ctx->streams[audio_stream_index]->time_base.num / 
						(float)input_ctx->streams[audio_stream_index]->time_base.den);
					dts_v++;
					pts_v++;
				}

				pts_sa = pts_a;
				dts_sa = dts_a;
				pts_sv = pts_v;
				dts_sv = dts_v;
				break;
			}
			if (pkt.stream_index == video_stream_index)
			{
				frameNum++;
			}

			if((frameNum < frameStart))
			{
				continue;
			}

			if((frameEnd) && (frameNum > frameEnd))
			{
				float vedioDuraTime, audioDuraTime;
				vedioDuraTime = ((float)input_ctx->streams[video_stream_index]->time_base.num / 
					(float)input_ctx->streams[video_stream_index]->time_base.den) * ((float)pts_v);
				audioDuraTime = ((float)input_ctx->streams[audio_stream_index]->time_base.num / 
					(float)input_ctx->streams[audio_stream_index]->time_base.den) * ((float)pts_a);

				if (audioDuraTime > vedioDuraTime)
				{
					dts_v = pts_v = audioDuraTime / ((float)input_ctx->streams[video_stream_index]->time_base.num / 
						(float)input_ctx->streams[video_stream_index]->time_base.den);
					dts_a++;
					pts_a++;
				}
				else
				{
					dts_a = pts_a = vedioDuraTime / ((float)input_ctx->streams[audio_stream_index]->time_base.num / 
						(float)input_ctx->streams[audio_stream_index]->time_base.den);
					dts_v++;
					pts_v++;
				}

				pts_sa = pts_a;
				dts_sa = dts_a;
				pts_sv = pts_v;
				dts_sv = dts_v;
				break;
			}

			if(!bIsVStart)
			{
				if (pkt.stream_index == video_stream_index)
				{
					bIsVStart = true;
					startVPts = pkt.pts;
					startVDts = pkt.dts;
				}
			}

			if(!bIsAStart)
			{
				if (pkt.stream_index == audio_stream_index)
				{
					bIsAStart = true;
					startAPts = pkt.pts;
					startADts = pkt.dts;
				}
			}

			if (pkt.stream_index == video_stream_index)
			{
				pkt.pts -= startVPts;
				pkt.dts -= startVDts;

				pkt.pts += pts_sv;
				pkt.dts += dts_sv;
				pts_v = pkt.pts;
				dts_v = pkt.dts;
			}
			if (pkt.stream_index == audio_stream_index)
			{
				pkt.pts -= startAPts;
				pkt.dts -= startADts;

				pkt.pts += pts_sa;
				pkt.dts += dts_sa;
				pts_a = pkt.pts;
				dts_a = pkt.dts;
			}

			pkt.pts = av_rescale_q_rnd(pkt.pts, input_ctx->streams[pkt.stream_index]->time_base, 
				out_fmtctx->streams[pkt.stream_index]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, input_ctx->streams[pkt.stream_index]->time_base, 
				out_fmtctx->streams[pkt.stream_index]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
			pkt.pos = -1;

			if (av_interleaved_write_frame(out_fmtctx, &pkt) < 0) 
			{
				printf( "Error muxing packet\n");
				return -1;
				//break;
			}
			av_free_packet(&pkt);		
		}
	}
	av_write_trailer(out_fmtctx);

	end_merge(vecMediaIn);
	return 0;
}

