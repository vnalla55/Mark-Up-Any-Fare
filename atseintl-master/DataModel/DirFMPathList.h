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

#include "Common/TsePrimitiveTypes.h"

#include <memory>
#include <vector>

namespace tse
{

class Trx;

namespace shpq
{

class DirFMPath;
class DirFMPathList;
typedef std::shared_ptr<DirFMPathList> DirFMPathListPtr;

class DirFMPathList
{
  typedef std::shared_ptr<DirFMPath> DirFMPathPtr;
  typedef std::vector<DirFMPathPtr> DirFMPathVector;

public:
  typedef DirFMPathVector::iterator iterator;
  typedef DirFMPathVector::const_iterator const_iterator;

  static DirFMPathListPtr create(Trx&);

  iterator begin() { return _dirFMPath.begin(); }
  iterator end() { return _dirFMPath.end(); }

  const_iterator begin() const { return _dirFMPath.begin(); }
  const_iterator end() const { return _dirFMPath.end(); }

  void insert(DirFMPathPtr);
  MoneyAmount lowerBound() const;
  DirFMPathVector::size_type size() { return _dirFMPath.size(); }

private:
  DirFMPathVector _dirFMPath;
};
}
} // namespace tse::shpq

