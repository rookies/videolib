#ifndef VIDEOLIB_HPP
#	define VIDEOLIB_HPP

#	include <iostream>
	extern "C" {
#	if __WORDSIZE == 64
#		define INT64_C(c)	c ## L
#		define UINT64_C(c)	c ## UL
#	else
#		define INT64_C(c)	c ## LL
#		define UINT64_C(c)	c ## ULL
#	endif
#	include <libavcodec/avcodec.h>
#	include <libavformat/avformat.h>
#	include <libswscale/swscale.h>
	}
#	include <SFML/Graphics.hpp>
#	include <SFML/Window.hpp>

	class Video
	{
		public:
			Video();
			virtual ~Video();

			bool load(std::string file);
			void unload(void);
			int get_bufsize(void);
			int get_width(void);
			int get_height(void);
			bool get_vframe(sf::Uint8 *buf);
		private:
			bool m_pleasefree;
			AVFormatContext *m_format_context;
			AVCodecContext *m_codec_context_video;
			int m_stream_video;
			AVFrame *m_videoframe;
			AVFrame *m_videoframe_rgb;
			sf::Uint8 *m_videobuffer;
			int videoframe_finished;
			struct SwsContext *m_sws_ctx;
	};
#endif // VIDEOLIB_HPP
