// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/ShpqTypes.h"

#include <memory>
#include <map>

namespace tse
{

class ShoppingTrx;

namespace shpq
{

class DirFMPath;
class DirFMPathList;
class DirFMPathListCollector;
typedef std::shared_ptr<DirFMPathListCollector> DirFMPathListCollectorPtr;

class DirFMPathListCollector
{
public:
  typedef std::shared_ptr<DirFMPathList> DirFMPathListPtr;
  typedef std::map<SolutionType, DirFMPathListPtr> DirFMPathListMap;
  typedef std::shared_ptr<DirFMPath> DirFMPathPtr;
  typedef DirFMPathListMap::size_type size_type;

  typedef DirFMPathListMap::iterator iterator;
  typedef DirFMPathListMap::const_iterator const_iterator;

  static DirFMPathListCollectorPtr create(ShoppingTrx&);

  DirFMPathListCollector(ShoppingTrx& trx) : _trx(trx) {}

  iterator begin() { return _dirFMPathListMap.begin(); }
  iterator end() { return _dirFMPathListMap.end(); }
  const_iterator begin() const { return _dirFMPathListMap.begin(); }
  const_iterator end() const { return _dirFMPathListMap.end(); }

  size_type size() const { return _dirFMPathListMap.size(); }

  DirFMPathListPtr getDirFMPathList(SolutionType solutionType)
  {
    return _dirFMPathListMap[solutionType];
  }
  void insert(DirFMPathPtr);

private:
  ShoppingTrx& _trx;
  DirFMPathListMap _dirFMPathListMap;
};
}
} // namespace tse::shpq

