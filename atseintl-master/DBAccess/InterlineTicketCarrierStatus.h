//----------------------------------------------------------------------------
//          File:           InterlineTicketCarrierStatus.h
//          Description:    InterlineTicketCarrierStatus
//          Created:        02/3/2012
//          Authors:        M Dantas
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class InterlineTicketCarrierStatus
{
public:
  InterlineTicketCarrierStatus() : _status(' ') {}
  virtual ~InterlineTicketCarrierStatus() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& lastModDate() { return _lastModDate; }
  const DateTime& lastModDate() const { return _lastModDate; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  CarrierCode& crsCode() { return _crsCode; }
  const CarrierCode& crsCode() const { return _crsCode; }

  Indicator& status() { return _status; }
  const Indicator& status() const { return _status; }

  virtual bool operator==(const InterlineTicketCarrierStatus& rhs) const
  {
    return ((_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_lastModDate == rhs._lastModDate) && (_carrier == rhs._carrier) &&
            (_crsCode == rhs._crsCode) && (_status == rhs._status));
  }

  static void dummyData(InterlineTicketCarrierStatus& obj)
  {
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._lastModDate = time(nullptr);
    obj._carrier = "ABC";
    obj._crsCode = "DEF";
    obj._status = 'G';
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _lastModDate;
  CarrierCode _carrier;
  CarrierCode _crsCode;
  Indicator _status;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _crsCode);
    FLATTENIZE(archive, _status);
  }

};
}
