#include "videolib.hpp"

Video::Video()
{
	m_pleasefree = false;
}
Video::~Video()
{
	if (m_pleasefree)
		unload();
}
bool Video::load(std::string file)
{
	/*
	 * Variable declarations:
	*/
	int i;
	AVCodec *codec_video;
	AVDictionary *options_dict;
	const char *filename;
	/*
	 * Convert filename:
	*/
	filename = file.c_str();
	/*
	 * Register formats & codecs:
	*/
	av_register_all();
	/*
	 * Open file:
	*/
	if (avformat_open_input(&m_format_context, filename, NULL, NULL) != 0)
	{
		std::cerr << "avformat_open_input() failed!" << std::endl;
		return false;
	};
	/*
	 * Get stream information:
	*/
	if (avformat_find_stream_info(m_format_context, NULL) < 0)
	{
		std::cerr << "avformat_find_stream_info() failed!" << std::endl;
		return false;
	};
	/*
	 * Dump file information:
	*/
	av_dump_format(m_format_context, 0, filename, 0);
	/*
	 * Find the first video stream:
	*/
	m_stream_video = -1;
	for (i=0; i < m_format_context->nb_streams; i++)
	{
		if (m_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_stream_video = i;
			break;
		};
	}
	if (-1 == m_stream_video)
	{
		std::cerr << "Couldn't find a video stream!" << std::endl;
		return false;
	};
	m_codec_context_video = m_format_context->streams[m_stream_video]->codec;
	/*
	 * Find the decoder:
	*/
	codec_video = avcodec_find_decoder(m_codec_context_video->codec_id);
	if (codec_video == NULL)
	{
		std::cerr << "avcodec_find_decoder() failed! (unsupported video codec)" << std::endl;
		return false;
	};
	/*
	 * Open the video stream:
	*/
	options_dict = NULL;
	if (avcodec_open2(m_codec_context_video, codec_video, &options_dict) < 0)
	{
		std::cerr << "avcodec_open2() failed! (video)" << std::endl;
		return false;
	};
	/*
	 * Allocate video frames:
	*/
	m_videoframe = avcodec_alloc_frame();
	m_videoframe_rgb = avcodec_alloc_frame();
	/*
	 * Init video buffer:
	*/
	m_videobuffer = new sf::Uint8[avpicture_get_size(PIX_FMT_RGBA, m_codec_context_video->width, m_codec_context_video->height)];
	avpicture_fill(
		(AVPicture *)m_videoframe_rgb,
		m_videobuffer,
		PIX_FMT_RGBA,
		m_codec_context_video->width,
		m_codec_context_video->height
	);
	/*
	 * Init swscale:
	*/
	m_sws_ctx = sws_getContext(
		m_codec_context_video->width,
		m_codec_context_video->height,
		m_codec_context_video->pix_fmt,
		m_codec_context_video->width,
		m_codec_context_video->height,
		PIX_FMT_RGBA,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);
	/*
	 * Return success:
	*/
	m_pleasefree = true;
	return true;
}
void Video::unload(void)
{
	if (!m_pleasefree)
		return;
	delete[] m_videobuffer;
	av_free(m_videoframe);
	av_free(m_videoframe_rgb);
	avcodec_close(m_codec_context_video);
	avformat_close_input(&m_format_context);
	sws_freeContext(m_sws_ctx);
	m_pleasefree = false;
}
int Video::get_bufsize(void)
{
	return avpicture_get_size(PIX_FMT_RGBA, m_codec_context_video->width, m_codec_context_video->height);
}
int Video::get_width(void)
{
	return m_codec_context_video->width;
}
int Video::get_height(void)
{
	return m_codec_context_video->height;
}
bool Video::get_vframe(sf::Uint8 *buf)
{
	/*
	 * Variable declarations:
	*/
	AVPacket packet;
	int frame_finished;
	/*
	 * Read frame:
	*/
	do {
		if (packet.size > 0)
			av_free_packet(&packet);
		if (av_read_frame(m_format_context, &packet) < 0)
			return false;
	}
	while (packet.stream_index != m_stream_video);
	/*
	 * Decode:
	*/
	avcodec_decode_video2(m_codec_context_video, m_videoframe, &frame_finished, &packet);
	if (frame_finished)
	{
		/*
		 * Convert:
		*/
		sws_scale(
			m_sws_ctx,
			m_videoframe->data,
			m_videoframe->linesize,
			0,
			m_codec_context_video->height,
			m_videoframe_rgb->data,
			m_videoframe_rgb->linesize
		);
		buf = m_videobuffer;
	};
	/*
	 * Free:
	*/
	av_free_packet(&packet);
	return true;
}
