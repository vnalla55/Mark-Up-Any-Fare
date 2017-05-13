#include "DataModel/SoloFmPath.h"

#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
namespace shpq
{

namespace
{

struct NullDeleter
{
  void operator()(void*) {}
};
}

SoloFmPathPtr
SoloFmPath::create(Trx& trx)
{
  return SoloFmPathPtr(trx.dataHandle().create<SoloFmPath>(), NullDeleter());
}
}
} // namespace tse::shpq
