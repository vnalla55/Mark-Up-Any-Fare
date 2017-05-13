//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class OpenJawRestriction : public RuleItemInfo
{
public:
  OpenJawRestriction() : _category(0), _orderNo(0), _set1ApplInd(' '), _set2ApplInd(' ') {}

  virtual ~OpenJawRestriction() {}

  virtual bool operator==(const OpenJawRestriction& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_category == rhs._category) &&
            (_orderNo == rhs._orderNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_set1ApplInd == rhs._set1ApplInd) &&
            (_set1Loc1 == rhs._set1Loc1) && (_set1Loc2 == rhs._set1Loc2) &&
            (_set2ApplInd == rhs._set2ApplInd) && (_set2Loc1 == rhs._set2Loc1) &&
            (_set2Loc2 == rhs._set2Loc2));
  }

private:
  int _category;
  int _orderNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _set1ApplInd;
  LocKey _set1Loc1;
  LocKey _set1Loc2;
  Indicator _set2ApplInd;
  LocKey _set2Loc1;
  LocKey _set2Loc2;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _category);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _set1ApplInd);
    FLATTENIZE(archive, _set1Loc1);
    FLATTENIZE(archive, _set1Loc2);
    FLATTENIZE(archive, _set2ApplInd);
    FLATTENIZE(archive, _set2Loc1);
    FLATTENIZE(archive, _set2Loc2);
  }

  int& category() { return _category; }
  const int& category() const { return _category; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& set1ApplInd() { return _set1ApplInd; }
  const Indicator& set1ApplInd() const { return _set1ApplInd; }

  LocKey& set1Loc1() { return _set1Loc1; }
  const LocKey& set1Loc1() const { return _set1Loc1; }

  LocKey& set1Loc2() { return _set1Loc2; }
  const LocKey& set1Loc2() const { return _set1Loc2; }

  Indicator& set2ApplInd() { return _set2ApplInd; }
  const Indicator& set2ApplInd() const { return _set2ApplInd; }

  LocKey& set2Loc1() { return _set2Loc1; }
  const LocKey& set2Loc1() const { return _set2Loc1; }

  LocKey& set2Loc2() { return _set2Loc2; }
  const LocKey& set2Loc2() const { return _set2Loc2; }
};
}

