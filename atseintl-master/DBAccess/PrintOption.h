//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class PrintOption
{
public:
  PrintOption() : _printOption(' ') {}

  virtual ~PrintOption() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  std::string& fareSource() { return _fareSource; }
  const std::string& fareSource() const { return _fareSource; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& printOption() { return _printOption; }
  const Indicator& printOption() const { return _printOption; }

  bool operator==(const PrintOption& second) const
  {
    return (_vendor == second._vendor) && (_fareSource == second._fareSource) &&
           (_createDate == second._createDate) && (_expireDate == second._expireDate) &&
           (_effDate == second._effDate) && (_discDate == second._discDate) &&
           (_printOption == second._printOption);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _fareSource);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _printOption);
  }

  static void dummyData(PrintOption& obj)
  {
    obj._vendor = "ABCD";
    obj._fareSource = "EFGHIJKL";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._printOption = 'M';
  }

private:
  VendorCode _vendor;
  std::string _fareSource;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _printOption;

  friend class SerializationTestBase;
};
} // tse
