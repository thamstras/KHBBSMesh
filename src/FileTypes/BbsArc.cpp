#include "BbsArc.h"
#include <stdexcept>
#include "..\Utils\StreamUtils.h"
#include "..\Utils\BitCast.h"
#include "..\Utils\MagicCode.h"

constexpr uint32_t arcMagic = MagicCode('A', 'R', 'C', '\0');

static ArcEntry ParseArcEntry(std::ifstream& file)
{
	ArcEntry entry = {};
	entry.directoryHash = ReadStream<uint32_t>(file);
	entry.dataOffset = ReadStream<uint32_t>(file);
	entry.dataLength = ReadStream<uint32_t>(file);
	entry.reserved = ReadStream<uint32_t>(file);
	ReadStreamArr(file, entry.fileName, 16);
	return entry;
}

bool ArcEntry::IsLink()
{
	return directoryHash != 0;
}

static ArcHeader ParseArcHeader(std::ifstream& file)
{
	ArcHeader header = {};
	header.magic = ReadStream<uint32_t>(file);
	if (header.magic != arcMagic)
	{
		throw std::runtime_error("File is not a valid ARC file! (Magic code fail)");
	}
	header.version = ReadStream<uint16_t>(file);
	header.entryCount = ReadStream<uint16_t>(file);
	header.reserved0 = ReadStream<uint32_t>(file);
	header.reserved1 = ReadStream<uint32_t>(file);
	return header;
}

BbsArc BbsArc::ReadArcFile(std::ifstream& file, std::streamoff base)
{
	BbsArc arc;
	arc.header = ParseArcHeader(file);
	for (int i = 0; i < arc.header.entryCount; i++)
		arc.entries.push_back(ParseArcEntry(file));
	/*for (ArcEntry& entry : arc.entries)
	{
		std::vector<uint8_t> data;
		if (!(entry.IsLink() || entry.dataOffset == 0 || entry.dataLength == 0))
		{
			file.seekg(base + (std::streamoff)entry.dataOffset, std::ios_base::beg);
			data = ReadBlob(file, entry.dataLength);
		}
		arc.datas.push_back(data);
	}*/
	return arc;
}