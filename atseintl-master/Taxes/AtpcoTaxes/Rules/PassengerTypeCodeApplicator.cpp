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

#include "Common/PassengerUtil.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Rules/PassengerTypeCodeApplicator.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/PassengerMapper.h"

namespace tax
{
PassengerTypeCodeApplicator::PassengerTypeCodeApplicator(
    const PassengerTypeCodeRule& rule,
    const Itin& itin,
    type::Date referenceDate,
    const PassengerMapper& paxMapper,
    std::shared_ptr<const PassengerTypeCodeItems> passengerTypeCodeItems,
    const LocService& locService,
    const type::AirportCode& pointOfSale)
  : BusinessRuleApplicator(&rule),
    _itin(itin),
    _referenceDate(referenceDate),
    _birthDate(itin.passenger()->_birthDate),
    _paxMapper(paxMapper),
    _passengerTypeCodeItems(passengerTypeCodeItems),
    _prescannedPassengerTypeCodeItems(),
    _parentRule(rule),
    _locService(locService),
    _pointOfSale(pointOfSale)
{
  const Passenger& passenger = *_itin.passenger();

  // These records will be matched multiple times, but the only thing changing will be
  // the passenger code. We can prevalidate them to rule out the ones which won't ever match.
  for (type::Index index = 0; index < _passengerTypeCodeItems->size(); ++index)
  {
    const PassengerTypeCodeItem& item = (*_passengerTypeCodeItems)[index];
    if (!locationMatches(passenger, item))
      continue;

    if (!ageMatches(_referenceDate, _birthDate, item.minimumAge, item.maximumAge))
      continue;

    _prescannedPassengerTypeCodeItems.push_back(&item);
  }
}

const type::PassengerCode findOutputPtc(type::TaxPointTag taxPointTag,
                                        type::Index loc1Id,
                                        const Itin& itin)
{
  if (itin.farePath()->fareUsages().empty())
    return itin.farePath()->outputPtc();

  if (taxPointTag == type::TaxPointTag::Sale || taxPointTag == type::TaxPointTag::Delivery)
    return itin.farePath()->fareUsages().front().fare()->outputPtc();

  // Departure or Arrival, need to find which fare is in range of taxation
  auto mappingIt = itin.geoPathMapping()->mappings().begin();
  auto fareUsageIt = itin.farePath()->fareUsages().begin();
  const auto mappingsEnd = itin.geoPathMapping()->mappings().end();
  const auto fareUsagesEnd = itin.farePath()->fareUsages().end();

  while (fareUsageIt != fareUsagesEnd && mappingIt != mappingsEnd)
  {
    if (mappingIt->maps().front().index() <= loc1Id &&
        mappingIt->maps().back().index() >= loc1Id)
      return fareUsageIt->fare()->outputPtc();
    ++fareUsageIt;
    ++mappingIt;
  }

  return itin.farePath()->outputPtc();
}

bool
PassengerTypeCodeApplicator::apply(PaymentDetail& paymentDetail) const
{
  const type::CarrierCode validatingCarrier = _itin.farePath()->validatingCarrier();

  const type::PassengerCode& outputPtc = findOutputPtc(paymentDetail.taxName().taxPointTag(),
                                                       paymentDetail.getTaxPointBegin().id(),
                                                       _itin);

  if (!passengerMatches(_itin.passenger()->_code, outputPtc, validatingCarrier))
  {
    paymentDetail.failItinerary(_parentRule);
    paymentDetail.failYqYrs(_parentRule);
  }

  for (OptionalService& ocItem: paymentDetail.optionalServiceItems())
  {
    if (ocItem.isFailed())
      continue;

    if (ocItem.type() == type::OptionalServiceTag::FareRelated)
      continue;

    const type::CarrierCode validatingCarrier = ocItem.ownerCarrier();

    if (!passengerMatches(_itin.passenger()->_code, ocItem.outputPtc(), validatingCarrier))
      ocItem.setFailedRule(&_parentRule);
  }
  return !paymentDetail.isFailed();
}

bool
PassengerTypeCodeApplicator::passengerMatches(const type::PassengerCode& inputCode,
                                              const type::PassengerCode& outputCode,
                                              const type::CarrierCode& validatingCarrier) const
{
  for (std::size_t index = 0; index < _prescannedPassengerTypeCodeItems.size(); ++index)
  {
    const PassengerTypeCodeItem* item = _prescannedPassengerTypeCodeItems[index];

    const type::PassengerCode* code = &outputCode;
    if (item->matchIndicator == type::PtcMatchIndicator::Input && !inputCode.empty())
      code = &inputCode;
    if (outputCode == "ALL")
      code = &inputCode;
    if (_paxMapper.matches(_parentRule.vendor(), validatingCarrier, *code, item->passengerType))
    {
      return (item->applTag == type::PtcApplTag::Permitted);
    }
  }
  return false;
}

bool
PassengerTypeCodeApplicator::ageMatches(const type::Date& ourDate,
                                        const type::Date& birthDate,
                                        int16_t minAge,
                                        int16_t maxAge)
{
  if (minAge <= 0 && maxAge < 0)
    return true;
  if (birthDate.is_blank_date())
    return false;
  if (minAge < 0)
    minAge = 0;
  if (maxAge < 0)
    maxAge = 200;
  if (birthDate.is_special())
    return false;

  int passengerAge = ourDate.year() - birthDate.year();
  if (birthDate.month() > ourDate.month() || (birthDate.month() == ourDate.month() && birthDate.day() > ourDate.day()))
    --passengerAge;

  return (minAge <= passengerAge) && (passengerAge <= maxAge);
}

bool
PassengerTypeCodeApplicator::locationMatches(const Passenger& passenger,
                                             const PassengerTypeCodeItem& item) const
{
  if (item.statusTag == type::PassengerStatusTag::Blank)
    return true;

  PassengerUtil passengerUtil(passenger);

  tax::type::LocCode passengerLocation = "";
  if (!passengerUtil.getStateCode().empty())
    passengerLocation = "US" + passengerUtil.getStateCode();
  else if (item.statusTag == type::PassengerStatusTag::Resident)
    passengerLocation = passengerUtil.getLocZoneText().asString();
  else
    passengerLocation = passengerUtil.getNation(item.statusTag).asString();

  if (passengerLocation.empty())
    passengerLocation = _pointOfSale.asString();

  return _locService.matchPassengerLocation(
      passengerLocation, item.location, _parentRule.vendor());
}

} // namespace tax
