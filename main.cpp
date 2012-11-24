#include "videolib.hpp"

int main (int argc, char **argv)
{
	/*
	 * Variable declarations:
	*/
	Video v;
	sf::RenderWindow window;
	sf::Texture texture;
	sf::Uint8 *buf;
	/*
	 * Load video:
	*/
	if (!v.load("testvid.mp4"))
		return 1;
	/*
	 * Init window & texture:
	*/
	//window.create(sf::VideoMode(v.get_width(), v.get_height(), 24), "FLOATING");
	//texture.create(v.get_width(), v.get_height());
	/*
	 * Get frames:
	*/
	buf = new sf::Uint8[v.get_bufsize()];
	while (v.get_vframe(buf))
	{
		
	}
	delete[] buf;
	/*
	 * Unload video:
	*/
	v.unload();
	return 0;
}
