/*
 * codechandler.c
 *
 *  Created on: 25.04.2015
 *      Author: sebastian
 */

#include <stdio.h>
#include <err.h>
#include "resample.h"
#include "codechandler.h"

void CodecHandler_init(CodecHandler* h){
	h->codec = NULL;
	h->codecContext = NULL;
	h->currentChannelCount = 0;
	h->currentCodecID = AV_CODEC_ID_NONE;
	h->currentSampleRate = 0;
	h->swr = resample_init();
	h->frame = av_frame_alloc();
}
void CodecHandler_deinit(CodecHandler* h){
	resample_deinit(h->swr);
	av_frame_free(&h->frame);
}

int CodecHandler_loadCodec(CodecHandler * handler, AVFormatContext * formatcontext){
	if (formatcontext->nb_streams == 0){
		printf("could not find a stream\n");
		handler->currentCodecID = AV_CODEC_ID_NONE;
		return -1;
	}

	if(handler->currentCodecID == formatcontext->streams[0]->codecpar->codec_id){
		//printf("Codec already loaded\n");
		return 0;
	}

	if(handler->codecContext != NULL){
		CodecHandler_closeCodec(handler);
	}
	handler->currentCodecID = AV_CODEC_ID_NONE;

	handler->codec = avcodec_find_decoder(formatcontext->streams[0]->codecpar->codec_id);
	if (!handler->codec) {
		printf("could not find codec\n");
		return -1;
	}else{
		printf("found codec %s (%s)\n", handler->codec->name, handler->codec->long_name);
	}
	handler->codecContext = avcodec_alloc_context3(handler->codec);
// Tu jeszcze nie
// printf("2.loadCodec/h->codecContext->ch_layout.nb_channels=%d\n", handler->codecContext->ch_layout.nb_channels);
// printf("2.loadCodec/h->codecContext->ch_layout.u.mask=%lu\n", handler->codecContext->ch_layout.u.mask);
// printf("2.loadCodec/h->codecContext->sample_rate=%d\n", handler->codecContext->sample_rate);
	if (!handler->codecContext){
		errx(1, "cannot allocate codec");
	}
	if (avcodec_open2(handler->codecContext, handler->codec, NULL) != 0){
		errx(1, "cannot open codec");
	}
// Tu jeszcze nie
// printf("2.loadCodec/h->codecContext->ch_layout.nb_channels=%d\n", handler->codecContext->ch_layout.nb_channels);
// printf("2.loadCodec/h->codecContext->ch_layout.u.mask=%lu\n", handler->codecContext->ch_layout.u.mask);
// printf("2.loadCodec/h->codecContext->sample_rate=%d\n", handler->codecContext->sample_rate);
	handler->currentCodecID = formatcontext->streams[0]->codecpar->codec_id;
// Tu jeszcze nie
// printf("3.loadCodec/formatcontext->streams[0]->codecpar->sample_rate=%d\n", formatcontext->streams[0]->codecpar->sample_rate);
// printf("3.loadCodec/formatcontext->streams[0]->codecpar->ch_layout.nb_channels=%d\n", formatcontext->streams[0]->codecpar->ch_layout.nb_channels);
// printf("3.loadCodec/formatcontext->streams[0]->codecpar->ch_layout.u.mask=%lu\n", formatcontext->streams[0]->codecpar->ch_layout.u.mask);
	// handler->codecContext->sample_rate = formatcontext->streams[0]->codecpar->sample_rate;
	// handler->codecContext->ch_layout = formatcontext->streams[0]->codecpar->ch_layout;
	return 0;
}

// https://ffmpeg.org/doxygen/5.0/demuxing_decoding_8c-example.html#a43
static int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame* frame)
{
	int ret = 0;

	// submit the packet to the decoder
	ret = avcodec_send_packet(dec, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
		return ret;
	}

	ret = avcodec_receive_frame(dec, frame);
	if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)){
		return 0; // brak bramki do wczytania
	}
	if (ret < 0) {
		fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
		return ret;
	}
	return pkt->size;
}

int CodecHandler_decodeCodec(CodecHandler * h, AVPacket * pkt,
		uint8_t *outbuffer, uint32_t* bufferfilled){
	// int got_frame;
	// int processed_len = avcodec_decode_audio4(h->codecContext, h->frame, &got_frame, pkt);
	int processed_len = decode_packet(h->codecContext, pkt, h->frame); // Not returning size...
	if (processed_len < 0){
		errx(1, "cannot decode input");
	}

	h->codecContext->ch_layout= h->frame->ch_layout;
	h->codecContext->sample_rate = h->frame->sample_rate;

	int ret = 0;

// Tu juz tak
// printf("decodeCodec/h->codecContext->ch_layout.nb_channels=%d\n", h->codecContext->ch_layout.nb_channels);
// printf("decodeCodec/h->codecContext->ch_layout.u.mask=%lu\n", h->codecContext->ch_layout.u.mask);
// printf("decodeCodec/h->codecContext->sample_rate=%d\n", h->codecContext->sample_rate);
	pkt->data += processed_len;
	pkt->size -= processed_len;
	if(h->currentChannelCount != h->codecContext->ch_layout.nb_channels
			|| h->currentSampleRate != h->codecContext->sample_rate
			|| h->currentChannelLayout != h->codecContext->ch_layout.u.mask){
		resample_loadFromCodec(h->swr, h->codecContext);
		printf("c: %d, s: %d\n",h->codecContext->ch_layout.nb_channels, h->codecContext->sample_rate);
		ret = 1;
	}

	swr_convert(h->swr, &outbuffer, h->frame->nb_samples, (const uint8_t **)h->frame->data, h->frame->nb_samples);
	*bufferfilled = av_samples_get_buffer_size(NULL,
			   h->codecContext->ch_layout.nb_channels,
			   h->frame->nb_samples,
			   AV_SAMPLE_FMT_S16,
			   1);

	h->currentChannelCount = h->codecContext->ch_layout.nb_channels;
	h->currentSampleRate = h->codecContext->sample_rate;
	h->currentChannelLayout = h->codecContext->ch_layout.u.mask;
	return ret;
}


int CodecHandler_closeCodec(CodecHandler * handler){
	if(handler->codecContext != NULL){
		//avcodec_close(handler->codecContext); // deprecated?
		avcodec_free_context(&handler->codecContext);
	}
	handler->codec = NULL;
	handler->codecContext = NULL;
	return 0;
}
