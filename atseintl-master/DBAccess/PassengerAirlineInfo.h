//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class PassengerAirlineInfo
{
public:
  PassengerAirlineInfo();
  virtual ~PassengerAirlineInfo() {}

  const CarrierCode& getAirlineCode() const { return _airlineCode; }
  void setAirlineCode(const CarrierCode& code) { _airlineCode = code; }

  const std::string& getAirlineName() const { return _airlineName; }
  void setAirlineName(const std::string& name) { _airlineName = name; }

  const DateTime& effDate() const { return _effDate; }
  void setEffDate(const DateTime& dt) { _effDate = dt; }

  const DateTime& discDate() const { return _discDate; }
  void setDiscDate(const DateTime& dt) { _discDate = dt; }

  const DateTime& createDate() const { return _createDate; }
  void setCreateDate(const DateTime& dt) { _createDate = dt; }

  const DateTime& expireDate() const { return _expireDate; }
  void setExpireDate(const DateTime& dt) { _expireDate = dt; }

  virtual void flattenize(Flattenizable::Archive&);
  static void dummyData(PassengerAirlineInfo&);

  virtual bool operator==(const PassengerAirlineInfo&) const;

protected:
  CarrierCode _airlineCode;
  std::string _airlineName;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  DateTime _expireDate;

private:
  PassengerAirlineInfo(const PassengerAirlineInfo&);
  PassengerAirlineInfo& operator=(const PassengerAirlineInfo&);
};
}

