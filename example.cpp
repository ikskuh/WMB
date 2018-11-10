#include <iostream>
#include <map>
#include "wmb.hpp"
using namespace WMB;

/*
 * This example lists all textures in the level file.
 */

std::map<WMB::Texture::Format, std::string> formats =
{
	{ WMB::Texture::RGBA8888, "RGBA8888" },
	{ WMB::Texture::RGB888,   "RGB888" },
	{ WMB::Texture::RGB565,   "RGB565" },
	{ WMB::Texture::DDS,      "DDS" },
};

int main(int argc, char ** argv)
{
	if(argc < 2) {
		std::cout << "Usage: wmb [filename]" << std::endl;
		return 0;
	}

	auto level = WMB::load(argv[1]);
	if(not level) {
		std::cerr << "Failed to load '" << argv[1] << "'." << std::endl;
		return 1;
	}

	for(auto const & tex : level->textures)
	{
		auto fmt = formats.find(tex.format);
		std::cout << "size="   << tex.width << "*" << tex.height << ",\t";

		if(fmt != formats.end())
			std::cout << "format=" << fmt->second<< ",\t";
		else
			std::cout << "format=unknown(" << tex.format << "),\t";

		std::cout << "name='"  << tex.name << "'" << std::endl;
	}

	return 0;
}
