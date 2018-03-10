#include <map>

#include "constants.hh"
#include "worldmap/MapColor.hh"
#include "worldmap/Worldmap.hh"
#include "util/Image.hh"
#include "declare.hh"
#include "Sector.hh"
#include "Logging.hh"
#include "worldmap/Coordinate.hh"

namespace worldmap {

const MapColor MapColor::uncolored(0,0,0);

const std::map<MapColor, Sector> color_sector_map = {
//             red, grn, blue
	{ MapColor(  0,   0,   0), Sector::none        },
	{ MapColor{255, 255, 255}, Sector::city             },
	{ MapColor(  0, 255,   0), Sector::field       },
	{ MapColor(  0, 199,  12), Sector::forest_sparse      },
	{ MapColor(  0, 148,   5), Sector::forest_medium      },
	{ MapColor(  0, 102,   0), Sector::forest_dense      },
	{ MapColor(203,  98,  12), Sector::road      },
	{ MapColor(255,   0, 153), Sector::area_entrance      },
	{ MapColor(  0,   0, 255), Sector::water_noswim      },
	{ MapColor( 51, 153, 255), Sector::water_swim      },
};

Worldmap::
Worldmap() {
	util::Image image;

	if (!image.load(WORLDMAP_IMAGE_FILE)) {
		Logging::bugf("Worldmap(): failed to load image file %s.", WORLDMAP_IMAGE_FILE);
		return;
	}

	_height = image.height();
	_width = image.width();

	if (height() > 0 && width() > 0) {
		rows = new unsigned char *[height()];

		for(unsigned int y = 0; y < height(); y++) {
			rows[y] = new unsigned char[width()];

			for (unsigned int x = 0; x < width(); x++) {
				Sector sector(Sector::none);

				if (image.value(util::Image::Channel::alpha, x, y) > 0) {
					MapColor color(
						image.value(util::Image::Channel::red, x, y),
						image.value(util::Image::Channel::green, x, y),
						image.value(util::Image::Channel::blue, x, y)
					);

					sector = color_to_sector(color);

					if (sector == Sector::unknown) {
						Logging::bugf("color_to_sector: unknown sector for color %d,%d,%d.",
							color.red, color.green, color.blue);
						sector = Sector::none;
					}
				}

				rows[y][x] = static_cast<unsigned char>(sector);
			}
		}
	}
}

Worldmap::
~Worldmap() {
	if (rows != nullptr) {
		for (unsigned int h = 0; h < height(); h++)
			delete[] rows[h];

		delete[] rows;
	}
}

const Sector Worldmap::
get_sector(const Coordinate& coord) const {
	unsigned char ret = 0;

	if (coord.is_valid()
	 && (unsigned int)coord.x < width()
	 && (unsigned int)coord.y < height())
		ret = rows[coord.y][coord.x];

	return static_cast<Sector>(ret);
}

const Sector Worldmap::
color_to_sector(const MapColor& color) const {
	const auto& pair = color_sector_map.find(color);

	if (pair == color_sector_map.cend()) {
		Logging::bugf("color_to_sector: unknown sector for color %d,%d,%d.",
			color.red, color.green, color.blue);
		return Sector::unknown;
	}

	return pair->second;
}

} // namespace worldmap
