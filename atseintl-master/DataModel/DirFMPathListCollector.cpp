#include "DataModel/DirFMPathListCollector.h"

#include "DataModel/DirFMPath.h"
#include "DataModel/DirFMPathList.h"
#include "DataModel/ShoppingTrx.h"
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

DirFMPathListCollectorPtr
DirFMPathListCollector::create(ShoppingTrx& trx)
{
  return DirFMPathListCollectorPtr(&trx.dataHandle().safe_create<DirFMPathListCollector>(trx),
                                   NullDeleter());
}

void
DirFMPathListCollector::insert(DirFMPathPtr dirFMPath)
{
  SolutionType solutionType = dirFMPath->getType();
  if (!_dirFMPathListMap[solutionType])
    _dirFMPathListMap[solutionType] = DirFMPathList::create(_trx);
  _dirFMPathListMap[solutionType]->insert(dirFMPath);
}
}
} // namespace tse::shpq
