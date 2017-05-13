//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class RoundTripRuleItem : public RuleItemInfo
{
public:
  RoundTripRuleItem()
    : _itemNo(0),
      _textTblItemNo(0),
      _overrideDateTblItemNo(0),
      _sameCarrierInd(' '),
      _unavailTag(' '),
      _atwInd(' '),
      _applInd(' '),
      _inhibit(' '),
      _highRTInd(' ')
  {
  }

  virtual ~RoundTripRuleItem() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  int& textTblItemNo() { return _textTblItemNo; }
  const int& textTblItemNo() const { return _textTblItemNo; }

  int& overrideDateTblItemNo() { return _overrideDateTblItemNo; }
  const int& overrideDateTblItemNo() const { return _overrideDateTblItemNo; }

  Indicator& sameCarrierInd() { return _sameCarrierInd; }
  const Indicator& sameCarrierInd() const { return _sameCarrierInd; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  Indicator& atwInd() { return _atwInd; }
  const Indicator& atwInd() const { return _atwInd; }

  Indicator& applInd() { return _applInd; }
  const Indicator& applInd() const { return _applInd; }

  Indicator& highrtInd() { return _highRTInd; }
  const Indicator& highrtInd() const { return _highRTInd; }

  const Indicator inhibit() const { return _inhibit; }
  Indicator& inhibit() { return _inhibit; }

  virtual bool operator==(const RoundTripRuleItem& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_vendor == rhs._vendor) &&
            (_itemNo == rhs._itemNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_textTblItemNo == rhs._textTblItemNo) &&
            (_overrideDateTblItemNo == rhs._overrideDateTblItemNo) &&
            (_sameCarrierInd == rhs._sameCarrierInd) && (_unavailTag == rhs._unavailTag) &&
            (_atwInd == rhs._atwInd) && (_applInd == rhs._applInd) &&
            (_highRTInd == rhs._highRTInd) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(RoundTripRuleItem& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._textTblItemNo = 2;
    obj._overrideDateTblItemNo = 3;
    obj._sameCarrierInd = 'E';
    obj._unavailTag = 'F';
    obj._atwInd = 'G';
    obj._applInd = 'H';
    obj._highRTInd = 'X';
    obj._inhibit = 'I';
  }

private:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  int _textTblItemNo;
  int _overrideDateTblItemNo;
  Indicator _sameCarrierInd;
  Indicator _unavailTag;
  Indicator _atwInd;
  Indicator _applInd;
  Indicator _inhibit; // Inhibit now checked at App Level
  Indicator _highRTInd; /// 'X': apply highest round trip fare

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _overrideDateTblItemNo);
    FLATTENIZE(archive, _sameCarrierInd);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _atwInd);
    FLATTENIZE(archive, _applInd);
    FLATTENIZE(archive, _highRTInd);
    FLATTENIZE(archive, _inhibit);
  }
};
}
