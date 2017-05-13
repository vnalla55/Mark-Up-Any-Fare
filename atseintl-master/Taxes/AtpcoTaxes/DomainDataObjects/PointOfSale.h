// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DataModel/Common/Types.h"

namespace tax
{

class PointOfSale
{
public:
  PointOfSale();
  virtual ~PointOfSale();

  type::AirportCode const& loc() const
  {
    return _loc;
  };

  type::AirportCode& loc()
  {
    return _loc;
  };

  type::Index const& id() const
  {
    return _id;
  };

  type::Index& id()
  {
    return _id;
  };

  type::PseudoCityCode const& agentPcc() const { return _agentPcc; }

  type::PseudoCityCode& agentPcc() { return _agentPcc; }

  type::CarrierGdsCode const& vendorCrsCode() const { return _vendorCrsCode; }

  type::CarrierGdsCode& vendorCrsCode() { return _vendorCrsCode; }

  type::CarrierCode const& carrierCode() const { return _carrierCode; }

  type::CarrierCode& carrierCode() { return _carrierCode; }

  type::DutyFunctionCode const& agentDuty() const { return _agentDuty; }

  type::DutyFunctionCode& agentDuty() { return _agentDuty; }

  type::DutyFunctionCode const& agentFunction() const { return _agentFunction; }

  type::DutyFunctionCode& agentFunction() { return _agentFunction; }

  type::CityCode const& agentCity() const { return _agentCity; }

  type::CityCode& agentCity() { return _agentCity; }

  std::string const& iataNumber() const { return _iataNumber; }

  std::string& iataNumber() { return _iataNumber; }

  std::string const& ersp() const { return _ersp; }

  std::string& ersp() { return _ersp; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

  std::string& agentAirlineDept() { return _agentAirlineDept; }
  const std::string& agentAirlineDept() const { return _agentAirlineDept; }

  std::string& agentOfficeDesignator() { return _agentOfficeDesignator; }
  const std::string& agentOfficeDesignator() const { return _agentOfficeDesignator; }

private:
  type::AirportCode _loc;
  type::Index _id;
  type::PseudoCityCode _agentPcc;
  type::CarrierGdsCode _vendorCrsCode;
  type::CarrierCode _carrierCode;
  type::DutyFunctionCode _agentDuty;
  type::DutyFunctionCode _agentFunction;
  type::CityCode _agentCity;
  std::string _iataNumber;
  std::string _ersp;
  std::string _agentAirlineDept;
  std::string _agentOfficeDesignator;
};
} // namespace tax

