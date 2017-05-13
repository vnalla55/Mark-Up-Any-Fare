//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CorpId
{
public:
  CorpId() : _ruleTariff(0) {}

  std::string& corpId() { return _corpId; }
  const std::string& corpId() const { return _corpId; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  std::string& accountCode() { return _accountCode; }
  const std::string& accountCode() const { return _accountCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const CorpId& rhs) const
  {
    return ((_corpId == rhs._corpId) && (_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_ruleTariff == rhs._ruleTariff) && (_rule == rhs._rule) &&
            (_accountCode == rhs._accountCode) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate));
  }

  static void dummyData(CorpId& obj)
  {
    obj._corpId = "aaaaaaaa";
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._ruleTariff = 1;
    obj._rule = "HIJK";
    obj._accountCode = "bbbbbbbb";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

private:
  std::string _corpId;
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  std::string _accountCode;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _corpId);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _accountCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }
};

} // namespace tse

