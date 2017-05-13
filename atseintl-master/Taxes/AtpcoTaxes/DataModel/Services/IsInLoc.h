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

#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"

#include <vector>

namespace tax
{

struct LocUsage
{
public:
  const type::Index& locRefId() const { return _locRefId; }
  type::Index& locRefId() { return _locRefId; }

  const type::AirportOrCityCode& airportCode() const { return _airportCode; }
  type::AirportOrCityCode& airportCode() { return _airportCode; }

  const bool& included() const { return _included; }
  bool& included() { return _included; }

private:
  type::Index _locRefId {0}; // locRefId to Services/Loc section
  type::AirportOrCityCode _airportCode;
  bool _included {true};
};

struct PaxLocUsage
{
  const type::LocCode& locCode() const { return _locCode; }
  type::LocCode& locCode() { return _locCode; }

  const bool& included() const { return _included; }
  bool& included() { return _included; }

private:
  type::LocCode _locCode {};
  bool _included {true};
};

class IsInLoc
{
public:
  type::LocType& type() { return _loc.type(); }
  const type::LocType& type() const { return _loc.type(); }

  type::LocZoneText& code() { return _loc.code(); }
  const type::LocZoneText& code() const { return _loc.code(); }

  type::Vendor& vendor() { return _vendor; }
  const type::Vendor& vendor() const { return _vendor; }

  const LocZone& loc() const { return _loc; }

  const bool& isPartial() const { return _isPartial; }
  bool& isPartial() { return _isPartial; }

  std::vector<LocUsage>& locUsages() { return _locUsages; }
  const std::vector<LocUsage>& locUsages() const { return _locUsages; }

  std::vector<PaxLocUsage>& paxLocUsages() { return _paxLocUsages; }
  const std::vector<PaxLocUsage>& paxLocUsages() const { return _paxLocUsages; }

private:
  LocZone _loc;
  type::Vendor _vendor;
  std::vector<LocUsage> _locUsages;
  std::vector<PaxLocUsage> _paxLocUsages;
  bool _isPartial = false;
};

}

