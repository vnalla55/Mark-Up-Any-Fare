//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef NOPNROPTIONSSEG_H
#define NOPNROPTIONSSEG_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class NoPNROptionsSeg
{
public:
  NoPNROptionsSeg() : _seqNo(0), _noDisplayOptions(0), _fareTypeGroup(0) {}

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  int& noDisplayOptions() { return _noDisplayOptions; }
  const int& noDisplayOptions() const { return _noDisplayOptions; }

  int& fareTypeGroup() { return _fareTypeGroup; }
  const int& fareTypeGroup() const { return _fareTypeGroup; }

  bool operator==(const NoPNROptionsSeg& rhs) const
  {
    return ((_seqNo == rhs._seqNo) && (_noDisplayOptions == rhs._noDisplayOptions) &&
            (_fareTypeGroup == rhs._fareTypeGroup));
  }

  static void dummyData(NoPNROptionsSeg& obj)
  {
    obj._seqNo = 1;
    obj._noDisplayOptions = 2;
    obj._fareTypeGroup = 3;
  }

private:
  int _seqNo;
  int _noDisplayOptions;
  int _fareTypeGroup;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _noDisplayOptions);
    FLATTENIZE(archive, _fareTypeGroup);
  }
};
}

#endif // NOPNROPTIONSSEG_H
