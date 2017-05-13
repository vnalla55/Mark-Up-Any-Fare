//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef PFCCOLLECTEXCPT_H
#define PFCCOLLECTEXCPT_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class PfcCollectExcpt
{
public:
  PfcCollectExcpt() : _collectOption(' ') {}
  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  Indicator& collectOption() { return _collectOption; }
  const Indicator& collectOption() const { return _collectOption; }

  bool operator==(const PfcCollectExcpt& rhs) const
  {
    return ((_nation == rhs._nation) && (_collectOption == rhs._collectOption));
  }

  static void dummyData(PfcCollectExcpt& obj)
  {
    obj._nation = "ABCD";
    obj._collectOption = 'E';
  }

private:
  NationCode _nation;
  Indicator _collectOption;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _collectOption);
  }
};
}

#endif
