#pragma once

class String;

namespace util {

class Image {
public:
	enum Channel { red = 0, green, blue, alpha };

	Image() {}
	virtual ~Image();

	unsigned int value(Channel rgb, unsigned int x, unsigned int y) const;

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
