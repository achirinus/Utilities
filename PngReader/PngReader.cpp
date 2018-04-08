// PngReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdint>
#include <cstdio>





bool IsValidChunk(char* Data);
bool IsChunkCritical(int Type);
PngChunk GetNextChunk(char* Data);
int SwapEndian(int SizeInBigEndian);

FileContent GetFileContent(FILE* file);

int main(int argc, char* argv[])
{
	//if (argc < 2) return -1;
	char* FileName = argv[1];
	FILE* PngFile;
	fopen_s(&PngFile, FileName, "rb");
	FileContent PngContent = GetFileContent(PngFile);
	char* TempData = PngContent.Data;
	TempData += PNG_HEADER_SIZE;

	FILE* NewPngFile;
	fopen_s(&NewPngFile, "NewTestPng.png", "wb+");
	int wrote  = fwrite(&PngHeader[0], 1, 8, NewPngFile);
	//fwrite(&Header1, 4, 1, NewPngFile);
	//fwrite(&Header2, 4, 1, NewPngFile);
	PngChunk LastChunk;
	PngInfo Info;
	do 
	{
		LastChunk = GetNextChunk(TempData);
		//Do something with the chunks
		if (LastChunk.Type == ChunkType::IHDR)
		{
			char* TempData = LastChunk.Data;
			
			Info.Width = SwapEndian(*(int*)TempData);
			TempData += 4;
			Info.Height = SwapEndian(*(int*)TempData);
			TempData += 4;
			Info.BitDepth = *TempData;
			TempData++;
			Info.ColorType = *TempData;
			TempData++;
			Info.CompressionMethod = *TempData;
			TempData++;
			Info.FilterMethod = *TempData;
			TempData++;
			Info.InterlaceMethod = *TempData;
		}
		if (LastChunk.Type != ChunkType::iTXt)
		{
			int FixedEndian = SwapEndian(LastChunk.Length);
			fwrite(&FixedEndian, 4, 1, NewPngFile);
			fwrite(&LastChunk.Type, 4, 1, NewPngFile);
			fwrite(LastChunk.Data, LastChunk.Length, 1, NewPngFile);
			fwrite(&LastChunk.ChunkChecksum, 4, 1, NewPngFile);
		}
		TempData += LastChunk.TotalSize;
	} 
	while(LastChunk.IsValid && LastChunk.Type != ChunkType::IEND);
	fclose(NewPngFile);
	fclose(PngFile);
    return 0;
}

bool IsChunkCritical(int Type)
{
	int FirstChar = Type & 0xFF;
	
	if ((FirstChar >= 65) && (FirstChar <= 90)) return true;
	return false;
}

//This is not checking everything now, it should also compute and check the checksum
bool IsValidChunk(char* Data)
{
	int* ChunkTypeP = (int*)(Data + 4);
	bool result = false;
	switch (int tt = *ChunkTypeP)
	{
		case ChunkType::IHDR:
		case ChunkType::PLTE:
		case ChunkType::IDAT:
		case ChunkType::IEND:
		case ChunkType::bKGD:
		case ChunkType::cHRM:
		case ChunkType::dSIG:
		case ChunkType::eXIf:
		case ChunkType::gAMA:
		case ChunkType::hIST:
		case ChunkType::iCCP:
		case ChunkType::iTXt:
		case ChunkType::pHYs:
		case ChunkType::sBIT:
		case ChunkType::sPLT:
		case ChunkType::sRGB:
		case ChunkType::sTER:
		case ChunkType::tEXt:
		case ChunkType::tIME:
		case ChunkType::tRNS:
		case ChunkType::zTXt:
			result = true;
			break;
		default: result = false;
	}
	return result;
}

PngChunk GetNextChunk(char* Data)
{
	PngChunk result{};

	if (bool IsChunkValid = IsValidChunk(Data))
	{
		int* LengthP = (int*)Data;
		result.Length = SwapEndian(*LengthP);
		result.Type = *(int*)(Data + 4);
		result.IsValid = IsChunkValid;
		result.Data = Data + 8;
		result.ChunkChecksum = *(int*)(result.Data + result.Length);
		result.TotalSize = 12 + result.Length;
	}
	return result;
}

int SwapEndian(int SizeInBigEndian)
{
	int result = 0;
	int temp = 0;
	result |= (SizeInBigEndian & 0xFF000000) >> 24;
	result |= (SizeInBigEndian & 0x00FF0000) >> 8;
	result |= (SizeInBigEndian & 0x0000FF00) << 8;
	result |= (SizeInBigEndian & 0x000000FF) << 24;
	return result;
}


FileContent GetFileContent(FILE* file)
{
	FileContent result{};
	fseek(file, 0, SEEK_END);
	result.Size = ftell(file);
	fseek(file, 0, SEEK_SET);
	result.Data = new char[result.Size];
	fread(result.Data, result.Size, 1, file);
	return result;
}
