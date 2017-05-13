//----------------------------------------------------------------------------
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class CircleTripRuleItem : public RuleItemInfo
{
public:
  CircleTripRuleItem()
    : _unavailtag(' '), _sameCarrierInd(' '), _atwInd(' '), _breakPoints(' '), _inhibit(' ')
  {
  }
  virtual bool operator==(const CircleTripRuleItem& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailtag == rhs._unavailtag) &&
            (_sameCarrierInd == rhs._sameCarrierInd) && (_atwInd == rhs._atwInd) &&
            (_breakPoints == rhs._breakPoints) && (_stopoverCnt == rhs._stopoverCnt) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(CircleTripRuleItem& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailtag = 'A';
    obj._sameCarrierInd = 'B';
    obj._atwInd = 'C';
    obj._breakPoints = 'D';
    obj._stopoverCnt = "aaaaaaaa";
    obj._inhibit = 'E';
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailtag;
  Indicator _sameCarrierInd;
  Indicator _atwInd;
  Indicator _breakPoints;
  std::string _stopoverCnt;
  Indicator _inhibit; // Inhibit now checked at App Level

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _sameCarrierInd);
    FLATTENIZE(archive, _atwInd);
    FLATTENIZE(archive, _breakPoints);
    FLATTENIZE(archive, _stopoverCnt);
    FLATTENIZE(archive, _inhibit);
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  Indicator& sameCarrierInd() { return _sameCarrierInd; }
  const Indicator& sameCarrierInd() const { return _sameCarrierInd; }

  Indicator& atwInd() { return _atwInd; }
  const Indicator& atwInd() const { return _atwInd; }

  Indicator& breakPoints() { return _breakPoints; }
  const Indicator& breakPoints() const { return _breakPoints; }

  std::string& stopoverCnt() { return _stopoverCnt; }
  const std::string& stopoverCnt() const { return _stopoverCnt; }

  const Indicator inhibit() const
  {
    return _inhibit;
  };
  Indicator& inhibit()
  {
    return _inhibit;
  };
};
}
