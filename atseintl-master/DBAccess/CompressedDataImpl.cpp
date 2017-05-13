#include "DBAccess/CompressedDataImpl.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/TSSCacheCommon.h"
#include "DBAccess/CompressedData.h"

#include <lz4.h>
#include <snappy.h>

enum
{
  BLOCK_BYTES = 1024 * 64
};

namespace
{

bool useBufferConfig()
{
  bool useBuffer(false);
  if (tse::Global::hasConfig())
  {
    std::string useBufferCfg;
    tse::Global::config().getValue("USE_DECOMPRESSION_BUFFER", useBufferCfg, "TSE_SERVER");
    useBuffer = useBufferCfg == "Y" || useBufferCfg == "y";
  }
  return useBuffer;
}

bool lz4Config()
{
  bool lz4lib(false);
  if (tse::Global::hasConfig())
  {
    std::string LZ4cfg;
    tse::Global::config().getValue("LZ4_LIBRARY", LZ4cfg, "TSE_SERVER");
    lz4lib = LZ4cfg == "Y" || LZ4cfg == "y";
  }
  return lz4lib;
}

bool lz4()
{
  static bool lz4lib(lz4Config());
  return lz4lib;
}

void lzCompress(const char*input,
                size_t inputSz,
                std::vector<char>& compressed)
{
  LZ4_stream_t lz4Stream = {};
  char inpBuf[2][BLOCK_BYTES];
  int inpBufIndex(0);
  size_t remainBytes(inputSz);
  size_t vecttorIdx(0);
  for (;;)
  {
    char* const inpPtr = inpBuf[inpBufIndex];
    size_t inpBytes(std::min(static_cast<size_t>(BLOCK_BYTES), remainBytes));
    if (inpBytes > 0)
    {
      std::memcpy(&inpBuf[inpBufIndex][0], input + vecttorIdx, inpBytes);
      vecttorIdx += inpBytes;
      remainBytes -= inpBytes;
    }
    else
    {
      break;
    }
    if(0 == inpBytes)
    {
      break;
    }
    char cmpBuf[LZ4_COMPRESSBOUND(BLOCK_BYTES)];
    int cmpBytes(LZ4_compress_continue(&lz4Stream, inpPtr, cmpBuf, inpBytes));
    if(cmpBytes <= 0)
    {
      break;
    }
    size_t prvSize(compressed.size());
    compressed.resize(prvSize + sizeof(cmpBytes) + cmpBytes);
    std::memcpy(&compressed[prvSize], &cmpBytes, sizeof(cmpBytes));
    std::memcpy(&compressed[prvSize + sizeof(cmpBytes)], cmpBuf, cmpBytes);
    inpBufIndex = (inpBufIndex + 1) % 2;
  }
  int dummy(0);
  size_t prvSize(compressed.size());
  assert(!compressed.empty());
  compressed.resize(prvSize + sizeof(dummy));
  std::memcpy(&compressed[prvSize], &dummy, sizeof(dummy));
}

void lzDecompress(std::vector<char>& decompressed,
                  const std::vector<char>& compressed)
{
  LZ4_streamDecode_t lz4StreamDecode = {};
  char decBuf[2][BLOCK_BYTES];
  int decBufIndex(0);
  size_t comressedInd(0);
  size_t decomressedInd(0);
  for (;;) 
  {
    char cmpBuf[LZ4_COMPRESSBOUND(BLOCK_BYTES)];
    int cmpBytes(0);
    std::memcpy(&cmpBytes, &compressed[comressedInd], sizeof(cmpBytes));
    comressedInd += sizeof(cmpBytes);
    std::memcpy(cmpBuf, &compressed[comressedInd], cmpBytes);
    comressedInd += cmpBytes;
    char* const decPtr = decBuf[decBufIndex];
    int decBytes(LZ4_decompress_safe_continue(&lz4StreamDecode,
                                              cmpBuf,
                                              decPtr,
                                              cmpBytes,
                                              BLOCK_BYTES));
    if(decBytes <= 0)
    {
      break;
    }
    assert(decomressedInd + decBytes <= decompressed.size());
    std::memcpy(&decompressed[decomressedInd], decPtr, decBytes);
    decomressedInd += decBytes;
    decBufIndex = (decBufIndex + 1) % 2;
  }
}

}// namespace

namespace tse
{

namespace CompressedDataImpl
{

sfc::CompressedData* compress(const char* input,
                              size_t inputSz)
{
  std::vector<char> compressed;
  size_t compressedSz(0);
  if (UNLIKELY(lz4()))
  {
    lzCompress(input, inputSz, compressed);
    compressedSz = compressed.size();
  }
  else
  {
    compressed.resize(snappy::MaxCompressedLength(inputSz));
    snappy::RawCompress(input, inputSz, &compressed[0], &compressedSz);
  }
  return new sfc::CompressedData(compressed, inputSz, compressedSz);
}

std::vector<char>* uncompress(const sfc::CompressedData& compressedData,
                              std::vector<char>& uncompressed)
{
  if (!compressedData._deflated.empty() && compressedData._inflatedSz > 0)
  {
    static bool useBuffer(useBufferConfig());
    std::vector<char>* buffer(nullptr);
    size_t bufferSize(useBuffer ? tsscache::memoryBufferSize() : 0);
    if (bufferSize > 0
        && compressedData._inflatedSz <= bufferSize)
    {
      std::vector<char>* pooledBuffer(tsscache::getMemoryBuffer());
      if (pooledBuffer)
      {
        buffer = pooledBuffer;
      }
    }
    if (nullptr == buffer || buffer->empty())
    {
      uncompressed.resize(compressedData._inflatedSz);
      buffer = &uncompressed;
    }
    if (lz4())
    {
      lzDecompress(*buffer, compressedData._deflated);
      return buffer;
    }
    else
    {
      if (snappy::RawUncompress(
          &compressedData._deflated[0], compressedData._deflated.size(), &(*buffer)[0]))
      {
        return buffer;
      }
    }
  }
  return nullptr;
}

}// CompressedDataImpl

}// tse
