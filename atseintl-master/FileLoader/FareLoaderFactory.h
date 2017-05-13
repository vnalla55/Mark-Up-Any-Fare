#pragma once

#include "DBAccess/BoundFareDAO.h"
#include "FileLoader/FileLoaderBase.h"

#include <string>

namespace tse
{
namespace FareLoaderFactory
{
FileLoaderBase*
create(bool bUsePhase1Extract, const std::string& url, BoundFareCache* cache);
}
}

