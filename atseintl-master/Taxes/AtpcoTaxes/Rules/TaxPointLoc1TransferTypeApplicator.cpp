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

#include "Common/OCUtil.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/Geo.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointLoc1TransferTypeApplicator.h"
#include "Rules/TaxPointLoc1TransferTypeRule.h"
#include "Util/BranchPrediction.h"

namespace tax
{
TaxPointLoc1TransferTypeApplicator::TaxPointLoc1TransferTypeApplicator(
    const TaxPointLoc1TransferTypeRule& businessRule,
    const std::vector<FlightUsage>& flightUsages,
    const std::vector<Flight>& flights)
  : BusinessRuleApplicator(&businessRule),
    _flightUsages(flightUsages),
    _flights(flights),
    _taxPointLoc1TransferTypeRule(businessRule)
{
}

TaxPointLoc1TransferTypeApplicator::~TaxPointLoc1TransferTypeApplicator() {}

bool
TaxPointLoc1TransferTypeApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (LIKELY(_taxPointLoc1TransferTypeRule.getTransferTypeTag() == type::TransferTypeTag::Blank))
  {
    return true;
  }

  const type::Index& taxPointId = paymentDetail.getTaxPointBegin().id();
  const int taxPointsCount = int((_flightUsages.size() * 2) - 1);

  if ((taxPointId <= type::Index(0)) || (type::Index(taxPointsCount) <= taxPointId))
  {
    // First/last Geo point. There is no prev/next flight.
    return failSubjects(paymentDetail);
  }

  const type::Index flightId = (taxPointId / 2);
  const type::Index flightIdToCheck = ((taxPointId % 2) == 0) ? (flightId - 1) : (flightId + 1);

  bool result = true;
  switch (_taxPointLoc1TransferTypeRule.getTransferTypeTag())
  {
  case type::TransferTypeTag::Interline:
    result = applyInterline(flightId, flightIdToCheck);
    break;
  case type::TransferTypeTag::OnlineWithChangeOfFlightNumber:
    result = applyOnlineWithChangeOfFlightNumber(flightId, flightIdToCheck);
    break;
  case type::TransferTypeTag::OnlineWithChangeOfGauge:
    result = applyOnlineWithChangeOfGauge(flightId, flightIdToCheck);
    break;
  case type::TransferTypeTag::OnlineWithNoChangeOfGauge:
    result = applyOnlineWithNoChangeOfGauge(flightId, flightIdToCheck);
    break;
  case type::TransferTypeTag::Blank:
    assert (false);
  }

  if (!result)
    return failSubjects(paymentDetail);

  return true;
}


bool
TaxPointLoc1TransferTypeApplicator::failSubjects(PaymentDetail& paymentDetail) const
{
  for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
  {
    if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
    {
      optionalService.setFailedRule(getBusinessRule());
    }
  }

  TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
      continue;

    taxableYqYrs.setFailedRule(i, *getBusinessRule());
  }

  paymentDetail.setFailedRule(getBusinessRule());
  return !paymentDetail.areAllOptionalServicesFailed() || !taxableYqYrs.areAllFailed();
}

bool
TaxPointLoc1TransferTypeApplicator::applyInterline(const type::Index flightA,
                                                   const type::Index flightB) const
{
  type::CarrierCode carrierA =
      _flights.at(_flightUsages.at(flightA).flightRefId()).marketingCarrier();

  type::CarrierCode carrierB =
      _flights.at(_flightUsages.at(flightB).flightRefId()).marketingCarrier();

  if ((carrierA != carrierB) || carrierA.empty() || carrierB.empty())
  {
    return true;
  }

  return false;
}

bool
TaxPointLoc1TransferTypeApplicator::applyOnlineWithChangeOfFlightNumber(const type::Index flightA,
                                                                        const type::Index flightB)
    const
{
  type::FlightNumber flightNumberA =
      _flights.at(_flightUsages.at(flightA).flightRefId()).marketingCarrierFlightNumber();

  type::FlightNumber flightNumberB =
      _flights.at(_flightUsages.at(flightB).flightRefId()).marketingCarrierFlightNumber();

  const type::FlightNumber blank = 0;
  const bool isDifferentFlightNumber =
      ((flightNumberA != flightNumberB) || (flightNumberA == blank) || (flightNumberB == blank));

  if (isSameCarrierOrBlank(flightA, flightB) && isDifferentFlightNumber)
  {
    return true;
  }

  return false;
}

bool
TaxPointLoc1TransferTypeApplicator::applyOnlineWithChangeOfGauge(const type::Index flightA,
                                                                 const type::Index flightB) const
{
  type::EquipmentCode equipmentA = _flights.at(_flightUsages.at(flightA).flightRefId()).equipment();

  type::EquipmentCode equipmentB = _flights.at(_flightUsages.at(flightB).flightRefId()).equipment();

  const bool areBothCHG = ((equipmentA == TaxPointLoc1TransferTypeRule::ChangeOfGauge) &&
                           (equipmentB == TaxPointLoc1TransferTypeRule::ChangeOfGauge));

  const bool isCHGDifferentOrCHG =
      (equipmentA != equipmentB) || areBothCHG || equipmentA.empty() || equipmentB.empty();

  if (isSameCarrierOrBlank(flightA, flightB) && isSameFlightNumberOrBlank(flightA, flightB) &&
      isCHGDifferentOrCHG)
  {
    return true;
  }

  return false;
}

bool
TaxPointLoc1TransferTypeApplicator::applyOnlineWithNoChangeOfGauge(const type::Index flightA,
                                                                   const type::Index flightB) const
{
  type::EquipmentCode equipmentA = _flights.at(_flightUsages.at(flightA).flightRefId()).equipment();

  type::EquipmentCode equipmentB = _flights.at(_flightUsages.at(flightB).flightRefId()).equipment();

  const bool areBothCHG = (equipmentA == TaxPointLoc1TransferTypeRule::ChangeOfGauge) &&
                          (equipmentB == TaxPointLoc1TransferTypeRule::ChangeOfGauge);

  const bool isCHGSameAndNotCHG =
      ((equipmentA == equipmentB) && !areBothCHG) || equipmentA.empty() || equipmentB.empty();

  if (isSameCarrierOrBlank(flightA, flightB) && isSameFlightNumberOrBlank(flightA, flightB) &&
      isCHGSameAndNotCHG)
  {
    return true;
  }

  return false;
}

bool
TaxPointLoc1TransferTypeApplicator::isSameCarrierOrBlank(const type::Index& flightA,
                                                         const type::Index& flightB) const
{
  type::CarrierCode carrierA =
      _flights.at(_flightUsages.at(flightA).flightRefId()).marketingCarrier();

  type::CarrierCode carrierB =
      _flights.at(_flightUsages.at(flightB).flightRefId()).marketingCarrier();

  return ((carrierA == carrierB) || carrierA.empty() || carrierB.empty());
}

bool
TaxPointLoc1TransferTypeApplicator::isSameFlightNumberOrBlank(const type::Index& flightA,
                                                              const type::Index& flightB) const
{
  type::FlightNumber flightNumberA =
      _flights.at(_flightUsages.at(flightA).flightRefId()).marketingCarrierFlightNumber();

  type::FlightNumber flightNumberB =
      _flights.at(_flightUsages.at(flightB).flightRefId()).marketingCarrierFlightNumber();

  const type::FlightNumber blank = 0;
  return ((flightNumberA == flightNumberB) || (flightNumberA == blank) || (flightNumberB == blank));
}

} // namespace tax
