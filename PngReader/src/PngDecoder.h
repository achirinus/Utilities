#pragma once

#include <cstdint>

extern char PngHeader[];

#define PNG_HEADER_SIZE 8

enum ChunkType : int
{
	IHDR = 0x52444849,
	PLTE = 0x45544C50,
	IDAT = 0x54414449,
	IEND = 0x444E4549,
	bKGD = 0x44474B62,
	cHRM = 0x4D524863,
	dSIG = 0x47495364,
	eXIf = 0x66495865,
	gAMA = 0x414D4167,
	hIST = 0x54534968,
	iCCP = 0x50434369,
	iTXt = 0x74585469,
	pHYs = 0x73594870,
	sBIT = 0x54494273,
	sPLT = 0x544C5073,
	sRGB = 0x42475273,
	sTER = 0x52455473,
	tEXt = 0x74584574,
	tIME = 0x454D4974,
	tRNS = 0x534E5274,
	zTXt = 0x7458547A
};

enum PngColorType
{
	GREYSCALE = 0,
	TRUECOLOR = 2,
	INDEXED = 3,
	GREYSCALE_A = 4,
	TRUECOLOR_A = 6
};

struct PngChunk
{
	int Length;
	int Type;
	char* Data;
	int ChunkChecksum;
	bool IsValid;
	int TotalSize;
};

struct FileContent
{
	int Size;
	char* Data;
};

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

