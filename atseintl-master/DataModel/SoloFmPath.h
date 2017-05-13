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


#include <memory>
#include <vector>

namespace tse
{

class Trx;

namespace shpq
{

class DirFMPathListCollector;
class SoloFmPath;
typedef std::shared_ptr<SoloFmPath> SoloFmPathPtr;

class SoloFmPath
{
  typedef std::shared_ptr<DirFMPathListCollector> DirFMPathListCollectorPtr;
  typedef std::vector<DirFMPathListCollectorPtr> LegDirFmPathVector;

public:
  typedef LegDirFmPathVector::iterator iterator;
  typedef LegDirFmPathVector::const_iterator const_iterator;
  typedef LegDirFmPathVector::value_type value_type;
  typedef LegDirFmPathVector::size_type size_type;

  static SoloFmPathPtr create(Trx&);

  void insertLeg(DirFMPathListCollectorPtr leg) { _legDirFmPath.push_back(leg); }

  iterator begin() { return _legDirFmPath.begin(); }
  iterator end() { return _legDirFmPath.end(); }

  const_iterator begin() const { return _legDirFmPath.begin(); }
  const_iterator end() const { return _legDirFmPath.end(); }

  value_type operator[](size_type legDirFmPathNumber) { return _legDirFmPath[legDirFmPathNumber]; }
  const value_type operator[](size_type legDirFmPathNumber) const
  {
    return _legDirFmPath[legDirFmPathNumber];
  }

  size_type size() const { return _legDirFmPath.size(); }

private:
  LegDirFmPathVector _legDirFmPath;
};
}
} // namespace tse::shpq

