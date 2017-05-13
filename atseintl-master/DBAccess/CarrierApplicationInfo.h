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

class CarrierApplicationInfo : public RuleItemInfo
{
public:
  CarrierApplicationInfo() : _orderNo(0), _applInd(' '), _inhibit(' ') {}

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& applInd() { return _applInd; }
  const Indicator& applInd() const { return _applInd; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const CarrierApplicationInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_orderNo == rhs._orderNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_applInd == rhs._applInd) && (_carrier == rhs._carrier) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(CarrierApplicationInfo& obj)
  {
    obj._orderNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._applInd = ' ';
    obj._carrier = "BCD";
    obj._inhibit = 'E';
  }

private:
  int _orderNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _applInd;
  CarrierCode _carrier;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _applInd);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _inhibit);
  }

};
}
