#include "FileLoader/FareLoaderFactory.h"

#include "DBAccess/BoundFareAdditionalInfo.h"
#include "DBAccess/Cache.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/HashKey.h"
#include "FileLoader/FileLoader.h"
#include "FileLoader/FileLoaderBase.h"
#include "FileLoader/FileLoaderExtract.h"

namespace tse
{
namespace FareLoaderFactory
{
FileLoaderBase*
create(bool bUsePhase1Extract, const std::string& url, BoundFareCache* cache)
{
  FileLoaderBase* loader = nullptr;
  if (bUsePhase1Extract)
  {
    loader = new FileLoaderExtract(url, cache);
  }
  else
  {
    loader = new FileLoader(url, cache);
  }
  return loader;
}
}
}
