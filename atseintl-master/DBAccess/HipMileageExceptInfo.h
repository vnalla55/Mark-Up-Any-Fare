//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class HipMileageExceptInfo : public RuleItemInfo
{
public:
  HipMileageExceptInfo() : _connectStopInd(' '), _noHipInd(' '), _unavailTag(' '), _inhibit(' ') {}

  virtual ~HipMileageExceptInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& connectStopInd() { return _connectStopInd; }
  const Indicator& connectStopInd() const { return _connectStopInd; }

  Indicator& noHipInd() { return _noHipInd; }
  const Indicator& noHipInd() const { return _noHipInd; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const HipMileageExceptInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
            (_connectStopInd == rhs._connectStopInd) && (_noHipInd == rhs._noHipInd) &&
            (_unavailTag == rhs._unavailTag) && (_inhibit == rhs._inhibit));
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _connectStopInd;
  Indicator _noHipInd;
  Indicator _unavailTag;
  Indicator _inhibit;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _connectStopInd);
    FLATTENIZE(archive, _noHipInd);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _inhibit);
  }

};
}
