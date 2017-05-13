#include "FileLoader/FileLoaderBase.h"

#include "Common/Logger.h"
#include "DBAccess/Cache.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/HashKey.h"
#include "FileLoader/BFUtils.h"
#include "FileLoader/GZStream.h"

#include <string>

#include <errno.h>
#include <time.h>

namespace tse
{
log4cxx::LoggerPtr
getLogger()
{
  return log4cxx::Logger::getLogger("atseintl.BFFileLoader");
}

FileLoaderBase::FileLoaderBase(const std::string& url, BoundFareCache* cache)
  : _gzStream(new GZStream(url)),
    _cache(cache),
    _pVector(nullptr),
    _badEntriesThreshold(BFUtils::getBadEntriesThreshold())
{
}

FileLoaderBase::~FileLoaderBase()
{
  delete _gzStream;
  delete _pVector;
}

void
FileLoaderBase::parseDateTime(const char* pBuffer, size_t length, DateTime& result)
{
  struct tm tm;
  static const char* format = "%Y-%m-%d %H:%M:%S";
  // strptime ignores the rest of string after reading in format
  if (nullptr == strptime(pBuffer, format, &tm))
  {
    std::string dateTimeStr(pBuffer, length);
    LOG4CXX_WARN(getLogger(), "FileLoader::parseDateTime:bad entry:" << dateTimeStr);
  }
  else
  {
    result =
        DateTime(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // cerr << result << std::endl;
  }
}
}
