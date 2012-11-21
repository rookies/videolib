/*
 * g++ -o libavcodec-test -lavcodec -lavformat -lavutil -lswscale -lsfml-window -lsfml-graphics libavcodec-test.cpp
*/
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
extern "C" {
#if __WORDSIZE == 64
#	define INT64_C(c)	c ## L
#	define UINT64_C(c)	c ## UL
#else
#	define INT64_C(c)	c ## LL
#	define UINT64_C(c)	c ## ULL
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main (int argc, char **argv)
{
	/*
	 * Variable definitions:
	*/
	/*
	 * videofile_format_context
	 * video_stream
	 * videofile_codec_context_video
	 * sws_ctx
	 * 
	 * videofile_packet
	 * videofile_video_frame_finished
	 * videofile_video_frame
	 * videofile_video_frame_rgb
	 */
	AVFormatContext *videofile_format_context = NULL;
	AVCodecContext *videofile_codec_context_video = NULL;
	AVCodec *videofile_codec_video = NULL;
	AVDictionary *options_dict = NULL;
	int video_stream = -1;
	int i;
	AVFrame *videofile_video_frame = NULL;
	AVFrame *videofile_video_frame_rgb = NULL;
	sf::Uint8 *videofile_buffer;
	AVPacket videofile_packet;
	int videofile_video_frame_finished;
	sf::RenderWindow window;
	sf::Event event;
	sf::Texture texture;
	sf::Sprite sprite;
	struct SwsContext *sws_ctx = NULL;
	/*
	 * Check arguments:
	*/
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " file" << std::endl;
		return 1;
	};
	/*
	 * Register formats & codecs:
	*/
	av_register_all();
	/*
	 * Open video file:
	*/
	if (avformat_open_input(&videofile_format_context, argv[1], NULL, NULL) !=0)
	{
		std::cerr << "Error: avformat_open_input() failed! Maybe the file doesn't exist?" << std::endl;
		return 1;
	};
	/*
	 * Get stream information:
	*/
	if (avformat_find_stream_info(videofile_format_context, NULL) < 0)
	{
		std::cerr << "Error: avformat_find_stream_info() failed!" << std::endl;
		return 1;
	};
	/*
	 * Dump file information:
	*/
	av_dump_format(videofile_format_context, 0, argv[1], 0);
	/*
	 * Find first video stream:
	*/
	for (i=0; i < videofile_format_context->nb_streams; i++)
	{
		if (videofile_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_stream = i;
			break;
		};
	}
	if (video_stream == -1)
	{
		std::cerr << "Error: Couldn't find a video stream!" << std::endl;
		return 1;
	};
	videofile_codec_context_video = videofile_format_context->streams[video_stream]->codec;
	/*
	 * Find a decoder:
	*/
	videofile_codec_video = avcodec_find_decoder(videofile_codec_context_video->codec_id);
	if (videofile_codec_video == NULL)
	{
		std::cerr << "Error: Unsupported video codec!" << std::endl;
		return 1;
	};
	/*
	 * Open codec:
	*/
	if (avcodec_open2(videofile_codec_context_video, videofile_codec_video, &options_dict) < 0)
	{
		std::cerr << "Error: Couldn't open video codec!" << std::endl;
		return 1;
	};
	/*
	 * Create window & texture:
	*/
	window.create(
		sf::VideoMode(
			videofile_codec_context_video->width,
			videofile_codec_context_video->height,
			24
		),
		"FLOATING"
	);
	texture.create(
		videofile_codec_context_video->width,
		videofile_codec_context_video->height
	);
	/*
	 * Allocate video frames:
	*/
	videofile_video_frame = avcodec_alloc_frame();
	videofile_video_frame_rgb = avcodec_alloc_frame();
	/*
	 * Init sw_scale:
	*/
	int numBytes = avpicture_get_size(PIX_FMT_RGBA, videofile_codec_context_video->width, videofile_codec_context_video->height);
	videofile_buffer = (sf::Uint8 *)av_malloc(numBytes * sizeof(sf::Uint8));
	avpicture_fill(
		(AVPicture *)videofile_video_frame_rgb,
		videofile_buffer,
		PIX_FMT_RGBA,
		videofile_codec_context_video->width,
		videofile_codec_context_video->height
	);
	sws_ctx = sws_getContext(
		videofile_codec_context_video->width,
		videofile_codec_context_video->height,
		videofile_codec_context_video->pix_fmt,
		videofile_codec_context_video->width,
		videofile_codec_context_video->height,
		PIX_FMT_RGBA,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);
	/*
	 * Read frames:
	*/
	while (av_read_frame(videofile_format_context, &videofile_packet) >= 0)
	{
		if (videofile_packet.stream_index == video_stream)
		{
			avcodec_decode_video2(videofile_codec_context_video, videofile_video_frame, &videofile_video_frame_finished, &videofile_packet);
			if (videofile_video_frame_finished)
			{
				sws_scale(
					sws_ctx,
					videofile_video_frame->data,
					videofile_video_frame->linesize,
					0,
					videofile_codec_context_video->height,
					videofile_video_frame_rgb->data,
					videofile_video_frame_rgb->linesize
				);
				texture.update(videofile_buffer);
				sprite.setTexture(texture);
				window.draw(sprite);
				window.display();
			};
		};
		av_free_packet(&videofile_packet);
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					return 1; // exit
					break;
			}
		}
	}
	return 0;
}
