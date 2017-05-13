// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <set>

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FareUsage.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"

namespace tax
{
class GeoPath;
class GeoPathMapping;
class SectorDetail;
class SectorDetailEntry;

class SectorDetailMatcher
{
  friend class SectorDetailTest;
public:
  SectorDetailMatcher(const std::vector<FlightUsage>& flightUsages,
                      const std::vector<Flight>& flights,
                      const std::vector<Fare>& fares,
                      const std::vector<FareUsage>& fareUsages,
                      GeoPathMapping const& geoPathMapping)
  : _flightUsages(flightUsages),
    _flights(flights),
    _fares(fares),
    _fareUsages(fareUsages),
    _geoPathMapping(geoPathMapping)
  {
  }

  ~SectorDetailMatcher() {}

  bool matchSectorDetails(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchSectorDetails(const SectorDetailEntry& entry,
                          const type::Index& flightId,
                          const type::Index& fareIdx) const;

private:
  bool matchEquipmentCode(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchFareTypeCode(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchCarrierCode(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchCabinCode(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchFareRuleCode(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool
  matchReservationDesignator(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchFareTariff(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool
  matchFareBasisTicketDesignator(const SectorDetailEntry& entry, const type::Index& flightId) const;
  bool matchTicketCode(const SectorDetailEntry& entry, const type::Index& flightId) const;

  std::string preparePattern(const type::FareBasisCode& fareBasisCode) const;
  bool matchFareBasisPart(const type::FareBasisCode& pattern,
                          const type::FareBasisCode& fareBasis) const;
  bool matchTicketDesignatorPart(const type::FareBasisCode& pattern,
                                 const type::FareBasisCode& ticketDesignator) const;
  bool checkFareBasisTD(const type::FareBasisCode& entryPattern,
                        const type::FareBasisCode& fareBasisCode) const;
  type::Index getFareIndexForFlight(const type::Index& flightId) const;
  bool
  checkTicketCode(const type::TicketCode ticketCode, const type::FareBasisCode fareBasisCode) const;
  bool isValidTicketCode(const type::TicketCode ticketCode) const;

  const std::vector<FlightUsage>& _flightUsages;
  const std::vector<Flight>& _flights;
  const std::vector<Fare>& _fares;
  const std::vector<FareUsage>& _fareUsages;
  GeoPathMapping const& _geoPathMapping;

  static const std::set<std::string> VALID_TICKET_CODES;
};

} // namespace tax
