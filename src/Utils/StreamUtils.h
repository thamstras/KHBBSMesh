#pragma once
#include <iostream>
#include <vector>

// TODO: replace all usage of this function with the one below.
template<typename T1>
void ReadStream(std::istream& stream, T1& out_var) { stream.read((char*)(&out_var), sizeof(T1)); }

template<typename T1>
T1 ReadStream(std::istream& stream)
{
	T1 var{};
	stream.read((char*)(&var), sizeof(T1));
	return var;
}

template<typename T1>
void ReadStreamArr(std::istream& stream, T1* arr, size_t count)
{
	for(size_t i = 0; i < count; i++) arr[i] = ReadStream<T1>(stream);
}

std::vector<uint8_t> ReadBlob(std::istream& stream, size_t size);

void Realign(std::istream& stream, size_t alignment);

uint16_t PeekShort(std::istream& stream);