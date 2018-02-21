#pragma once

#include "String.hh"

namespace util {

class Image {
public:
	enum RGB { red = 0, green, blue, alpha };

	Image() {}
	virtual ~Image() {
		if (rows != nullptr) {
			for (unsigned int h = 0; h < height(); h++)
				delete rows[h];

			delete rows;
		}
	}

	unsigned int value(RGB rgb, unsigned int x, unsigned int y) const {
		if (rows == nullptr || x > width() || y > height())
			return 0;

		return rows[y][x*4+rgb];
	}

	unsigned int height() const { return _height; }
	unsigned int width() const { return _width; }
	bool load(const String& filename);

private:
	Image(const Image&);
	Image& operator=(const Image&);

	void height(unsigned int v) { _height = v; }
	void width(unsigned int v) { _width = v; }

	unsigned int _width = 0;
	unsigned int _height = 0;
	unsigned char **rows = nullptr;
};

} // namespace util
