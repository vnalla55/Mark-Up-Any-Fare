#include "DataModel/DirFMPathList.h"

#include "DataModel/DirFMPath.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"

#include <algorithm>

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

struct DirFMPathLess
{
  bool operator()(const DirFMPathPtr lhp, const DirFMPathPtr rhp)
  {
    return lhp->lowerBound() < rhp->lowerBound();
  }
};

} // namespace

DirFMPathListPtr
DirFMPathList::create(Trx& trx)
{
  return DirFMPathListPtr(trx.dataHandle().create<DirFMPathList>(), NullDeleter());
}

void
DirFMPathList::insert(DirFMPathPtr fmSeg)
{
  iterator it = std::lower_bound(begin(), end(), fmSeg, DirFMPathLess());
  _dirFMPath.insert(it, fmSeg);
}

MoneyAmount
DirFMPathList::lowerBound() const
{
  return (*begin())->lowerBound();
}
}
} // namespace tse::shpq
