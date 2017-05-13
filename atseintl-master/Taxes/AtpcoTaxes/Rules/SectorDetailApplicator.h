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
#include "DataModel/Services/SectorDetail.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FareUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/SectorDetailMatcher.h"
#include "Rules/SectorDetailRule.h"

namespace tax
{

class BusinessRule;
class Fare;
class FareUsage;
class GeoPath;
class GeoPathMapping;
class PaymentDetail;
class SectorDetail;

class SectorDetailApplicator : public BusinessRuleApplicator
{
public:
  SectorDetailApplicator(const BusinessRule* businessRule,
                         std::shared_ptr<const SectorDetail> sectorDetail,
                         const type::SectorDetailApplTag& sectorDetailApplTag,
                         const std::vector<FlightUsage>& flightUsages,
                         const std::vector<Flight>& flights,
                         const std::vector<Fare>& fares,
                         const std::vector<FareUsage>& fareUsages,
                         const GeoPathMapping& geoPathMapping,
                         const GeoPath& geoPath);

  ~SectorDetailApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool applyAnySectorLoc1Loc2(PaymentDetail& paymentDetail) const;
  bool applyAnySectorOrginDestination() const;
  bool applyAnySector(type::Index flightBeginId, type::Index flightEndId) const;
  bool applyFirstInternationalSector() const;
  bool applyEverySectorInItin() const;
  bool applyAllSectorLoc1Loc2(PaymentDetail& paymentDetail) const;
  bool applyEverySector(type::Index flightBeginId, type::Index flightEndId) const;

  std::shared_ptr<const SectorDetail> _sectorDetail;
  const type::SectorDetailApplTag& _sectorDetailApplTag;
  const std::vector<FlightUsage>& _flightUsages;
  const GeoPath& _geoPath;
  const SectorDetailMatcher _matcher;
};

} // namespace tax
