#include "FileLoader/GZStream.h"

#include "Common/Logger.h"

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <zlib.h>

namespace
{
log4cxx::LoggerPtr
getLogger()
{
  return log4cxx::Logger::getLogger("atseintl.BFFileLoader");
}
}

namespace tse
{
GZStream::GZStream(const std::string& url) : _gzFile(nullptr)
{
  _gzFile = gzopen(url.c_str(), "rb");
  if (nullptr == _gzFile)
  {
    LOG4CXX_ERROR(getLogger(), "GZStream:gzopen failed for " << url);
  }
}

GZStream::~GZStream()
{
  if (_gzFile != nullptr && gzclose(_gzFile) != Z_OK)
  {
    LOG4CXX_ERROR(getLogger(), "~GZStream:gzclose failed");
  }
}

std::streamsize
GZStream::read(void* pBuffer, std::streamsize size)
{
  if (_gzFile == nullptr)
    throw std::runtime_error("GZIP stream not open");
  std::streamsize result(0);
  if (_gzFile != nullptr && size > 0 && (result = gzread(_gzFile, pBuffer, size)) <= 0 && !gzeof(_gzFile))
  {
    int err(0);
    LOG4CXX_ERROR(getLogger(), "GZStream::read:" << gzerror(_gzFile, &err));
    throw std::runtime_error("GZIP stream read failed");
  }
  return result;
}
}
