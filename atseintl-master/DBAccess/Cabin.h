//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/CabinType.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class Cabin
{
public:
  Cabin() : _premimumClassOfService(' ') {}

  virtual ~Cabin() {}

  virtual bool operator==(const Cabin& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_cabin == rhs._cabin) &&
            (_classOfService == rhs._classOfService) &&
            (_premimumClassOfService == rhs._premimumClassOfService));
  }

  static void dummyData(Cabin& obj)
  {
    obj._carrier = "ABC";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);

    CabinType::dummyData(obj._cabin);

    obj._classOfService = "EF";
    obj._premimumClassOfService = 'G';
  }

private:
  CarrierCode _carrier;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  CabinType _cabin;
  BookingCode _classOfService;
  Indicator _premimumClassOfService;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _cabin);
    FLATTENIZE(archive, _classOfService);
    FLATTENIZE(archive, _premimumClassOfService);
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  CabinType& cabin() { return _cabin; }
  const CabinType& cabin() const { return _cabin; }

  BookingCode& classOfService() { return _classOfService; }
  const BookingCode& classOfService() const { return _classOfService; }
};
}

