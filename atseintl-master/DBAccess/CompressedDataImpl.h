#pragma once

#include <cstring>
#include <vector>

namespace sfc
{
class CompressedData;
}

namespace tse
{

namespace CompressedDataImpl
{

sfc::CompressedData* compress(const char* input,
                              size_t inputSz);

std::vector<char>* uncompress(const sfc::CompressedData& compressedData,
                              std::vector<char>& uncompressed);

}// CompressedDataImpl

}// tse

