//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class NoPNRFareTypeGroup
{
public:
  NoPNRFareTypeGroup() : _fareTypeGroup(0) {}

  ~NoPNRFareTypeGroup()
  {
    std::vector<FareTypeMatrix*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    { // Nuke 'em!
      delete *SegIt;
    }
  }

  int& fareTypeGroup() { return _fareTypeGroup; }
  const int& fareTypeGroup() const { return _fareTypeGroup; }

  // int &fareTypeDesig() { return _fareTypeDesig;}
  // const int &fareTypeDesig() const { return _fareTypeDesig; }

  std::vector<FareTypeMatrix*>& segs() { return _segs; }
  const std::vector<FareTypeMatrix*>& segs() const { return _segs; }

  bool operator==(const NoPNRFareTypeGroup& rhs) const
  {
    bool eq((_fareTypeGroup == rhs._fareTypeGroup) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(NoPNRFareTypeGroup& obj)
  {
    obj._fareTypeGroup = 1;

    FareTypeMatrix* ftm1 = new FareTypeMatrix;
    FareTypeMatrix* ftm2 = new FareTypeMatrix;

    FareTypeMatrix::dummyData(*ftm1);
    FareTypeMatrix::dummyData(*ftm2);

    obj._segs.push_back(ftm1);
    obj._segs.push_back(ftm2);
  }

protected:
  // Join fields (w/Child: FareTypeMatrix)
  int _fareTypeGroup;
  std::vector<FareTypeMatrix*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareTypeGroup);
    FLATTENIZE(archive, _segs);
  }

protected:
private:
  NoPNRFareTypeGroup(const NoPNRFareTypeGroup&);
  NoPNRFareTypeGroup& operator=(const NoPNRFareTypeGroup&);
};
}

