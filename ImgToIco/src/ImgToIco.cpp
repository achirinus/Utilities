// ImgToIco.cpp : Defines the entry point for the console application.
//


#include <cstdio>
#include <cstdint>


struct IcoEntry
{
	int8_t Width;
	int8_t Height;
	int8_t Colors;
	int8_t Reserved;
	int16_t ColorPlanes;
	int16_t Bpp;
	int32_t SizeOfData;
	int32_t Offset;
};

struct IcoDir
{
	int16_t ReservedBytes;
	int16_t ImageType;
	int16_t NumOfImages;
};

//For now I just assume that the image is 256 x 256 and a png

int main(int argc, char* argv[])
{
	if (argc < 2) return -1;
	//TODO Make this usable for more images
	char* PngFileName = argv[1];
	FILE* IcoFile;
	FILE* PngFile;
	fopen_s(&IcoFile, "TestIcoFile.ico", "wb+");
	fopen_s(&PngFile, PngFileName, "rb");
	fseek(PngFile, 0, SEEK_END);
	size_t SizeOfPng = ftell(PngFile);
	fseek(PngFile, 0, SEEK_SET);
	char* PngContent = new char[SizeOfPng];
	fread(PngContent, SizeOfPng, 1, PngFile);
	fclose(PngFile);

	fseek(IcoFile, 0, SEEK_SET);
	IcoDir IcoDirectory{};
	IcoDirectory.ImageType = 1;
	IcoDirectory.NumOfImages = 1;
	IcoEntry Entry{};
	Entry.Bpp = 32;
	Entry.SizeOfData = SizeOfPng;
	Entry.Offset = sizeof(IcoEntry) + sizeof(IcoDir);
	Entry.ColorPlanes = 1;
	
	fwrite(&IcoDirectory, sizeof(IcoDir), 1, IcoFile);
	fwrite(&Entry, sizeof(IcoEntry), 1, IcoFile);
	fwrite(PngContent, SizeOfPng, 1, IcoFile);
	fclose(IcoFile);
	delete[] PngContent;
    return 0;
}

