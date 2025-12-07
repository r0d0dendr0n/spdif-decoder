/*
 * resample.c
 *
 *  Created on: 20.04.2015
 *      Author: sebastian
 */
#include "resample.h"

#include <libavutil/opt.h>

SwrContext* resample_init(){
	return swr_alloc();
}

void resample_deinit(SwrContext* swr){
	swr_free(&swr);
}

void resample_loadFromCodec(SwrContext *swr, AVCodecContext* audioCodec){
	int ret = 0;
	// Set up SWR context once you've got codec information
//	ret |= av_opt_set_int(swr, "in_channel_layout",  audioCodec->ch_layout.u.mask, 0);
//	ret |= av_opt_set_int(swr, "out_channel_layout", audioCodec->ch_layout.u.mask,  0);
	ret |= av_opt_set_chlayout(swr, "in_chlayout",   &audioCodec->ch_layout, 0);
	ret |= av_opt_set_chlayout(swr, "out_chlayout",  &audioCodec->ch_layout, 0);
	ret |= av_opt_set_int(swr, "in_sample_rate",     audioCodec->sample_rate, 0);
	ret |= av_opt_set_int(swr, "out_sample_rate",    audioCodec->sample_rate, 0);
	ret |= av_opt_set_sample_fmt(swr, "in_sample_fmt",  audioCodec->sample_fmt, 0);
	ret |= av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
	if(ret < 0){
		fprintf(stderr, "Unable to set av opt in resample.c\n");
		//swr_free(&swr); // not maintained here
		return;
	}
	swr_init(swr);

}

void resample_do(SwrContext* swr, AVFrame *audioFrame, uint8_t* outputBuffer){
	swr_convert(swr,
			&outputBuffer,
			audioFrame->nb_samples,
			(const uint8_t**)audioFrame->data,
			audioFrame->nb_samples);
}
