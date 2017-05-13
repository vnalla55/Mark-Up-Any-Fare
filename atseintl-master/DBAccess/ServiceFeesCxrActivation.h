//----------------------------------------------------------------------------
//   2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once
#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class ServiceFeesCxrActivation
{
public:
  ServiceFeesCxrActivation() {}
  virtual ~ServiceFeesCxrActivation() {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const ServiceFeesCxrActivation& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate));
  }

  static void dummyData(ServiceFeesCxrActivation& obj)
  {
    obj._carrier = "UA";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }

protected:
  CarrierCode _carrier;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;

private:
  ServiceFeesCxrActivation(const ServiceFeesCxrActivation& rhs);
  ServiceFeesCxrActivation& operator=(const ServiceFeesCxrActivation& rhs);
};
}
