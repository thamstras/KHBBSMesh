#pragma once
#include <vector>
#include <fstream>
#include <cstdint>

struct ArcEntry
{
	uint32_t directoryHash;
	uint32_t dataOffset;
	uint32_t dataLength;
	uint32_t reserved;
	char fileName[16];

	bool IsLink();
};

struct ArcHeader
{
	uint32_t magic;
	uint16_t version;
	uint16_t entryCount;
	uint32_t reserved0;
	uint32_t reserved1;
};

class BbsArc
{
public:
	ArcHeader header;
	std::vector<ArcEntry> entries;
	//std::vector<std::vector<uint8_t>> datas;

	static BbsArc ReadArcFile(std::ifstream& file, std::streamoff base = 0);
};