//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class FareProperties
{
public:
  FareProperties() : _ruleTariff(0), _excludeQSurchargeInd(' ') {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::string& fareSource() { return _fareSource; }
  const std::string& fareSource() const { return _fareSource; }

  std::string& valueCodeAlgorithmName() { return _valueCodeAlgorithmName; }
  const std::string& valueCodeAlgorithmName() const { return _valueCodeAlgorithmName; }

  Indicator& excludeQSurchargeInd() { return _excludeQSurchargeInd; }
  const Indicator& excludeQSurchargeInd() const { return _excludeQSurchargeInd; }

  bool operator==(const FareProperties& second) const
  {
    return (_vendor == second._vendor) && (_carrier == second._carrier) &&
           (_ruleTariff == second._ruleTariff) && (_rule == second._rule) &&
           (_createDate == second._createDate) && (_expireDate == second._expireDate) &&
           (_fareSource == second._fareSource) &&
           (_valueCodeAlgorithmName == second._valueCodeAlgorithmName) &&
           (_excludeQSurchargeInd == second._excludeQSurchargeInd);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _fareSource);
    FLATTENIZE(archive, _valueCodeAlgorithmName);
    FLATTENIZE(archive, _excludeQSurchargeInd);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(FareProperties& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EF";
    obj._ruleTariff = 1;
    obj._rule = "GHIJ";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._fareSource = "KLMNOPQR";
    obj._valueCodeAlgorithmName = "STUVXYZABC";
    obj._excludeQSurchargeInd = 'D';
  }

private:
  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_ruleTariff
           & ptr->_rule
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_fareSource
           & ptr->_valueCodeAlgorithmName
           & ptr->_excludeQSurchargeInd;
  }

  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  DateTime _createDate;
  DateTime _expireDate;
  std::string _fareSource;
  std::string _valueCodeAlgorithmName;
  Indicator _excludeQSurchargeInd;

  friend class SerializationTestBase;
};
} // tse
