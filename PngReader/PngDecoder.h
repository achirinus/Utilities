#pragma once

struct PngInfo
{
	int32_t Width;
	int32_t Height;
	int8_t BitDepth; // Per channel, not pixel
	int8_t ColorType;
	int8_t CompressionMethod;
	int8_t FilterMethod;
	int8_t InterlaceMethod;
	bool IsSRGB;
};

class PngDecoder
{
public:
	PngDecoder();
	~PngDecoder();
};

