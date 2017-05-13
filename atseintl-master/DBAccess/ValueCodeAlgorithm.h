//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{
class ValueCodeAlgorithm
{
public:
  static const size_t NUMBER_OF_DIGITS = 10;

  ValueCodeAlgorithm() : _decimalChar(' ')
  {
    for (auto& elem : _digitToChar)
      elem = ' ';
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  std::string& algorithmName() { return _algorithmName; }
  const std::string& algorithmName() const { return _algorithmName; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::string& prefix() { return _prefix; }
  const std::string& prefix() const { return _prefix; }

  std::string& suffix() { return _suffix; }
  const std::string& suffix() const { return _suffix; }

  Indicator* digitToChar() { return _digitToChar; }
  const Indicator* digitToChar() const { return _digitToChar; }

  Indicator& decimalChar() { return _decimalChar; }
  const Indicator& decimalChar() const { return _decimalChar; }

  bool operator==(const ValueCodeAlgorithm& second) const
  {
    return (_vendor == second._vendor) && (_carrier == second._carrier) &&
           (_algorithmName == second._algorithmName) && (_createDate == second._createDate) &&
           (_expireDate == second._expireDate) && (_effDate == second._effDate) &&
           (_discDate == second._discDate) && (_prefix == second._prefix) &&
           (_suffix == second._suffix) &&
           (memcmp(_digitToChar, second._digitToChar, sizeof(_digitToChar)) == 0) &&
           (_decimalChar == second._decimalChar);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _algorithmName);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _prefix);
    FLATTENIZE(archive, _suffix);
    FLATTENIZE(archive, _digitToChar);
    FLATTENIZE(archive, _decimalChar);
  }

  static void dummyData(ValueCodeAlgorithm& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EF";
    obj._algorithmName = "GHIJKLMNOP";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._prefix = "QRST";
    obj._suffix = "UVXY";

    Indicator ch = '0';

    for (size_t cnt = 0; cnt < NUMBER_OF_DIGITS; cnt++)
      obj._digitToChar[cnt] = ch++;

    obj._decimalChar = 'Z';
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  std::string _algorithmName;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _prefix;
  std::string _suffix;
  Indicator _digitToChar[NUMBER_OF_DIGITS];
  Indicator _decimalChar;

  friend class SerializationTestBase;
};
} // tse
