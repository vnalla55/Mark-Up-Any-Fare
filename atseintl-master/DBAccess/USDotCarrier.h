//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{
class USDotCarrier
{
public:
  USDotCarrier() {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  bool operator==(const USDotCarrier& second) const
  {
    return (_carrier == second._carrier) && (_createDate == second._createDate) &&
           (_expireDate == second._expireDate);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
  }

  static void dummyData(USDotCarrier& obj)
  {
    obj._carrier = "AB";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
  }

private:
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _expireDate;

  friend class SerializationTestBase;
};
} // tse
