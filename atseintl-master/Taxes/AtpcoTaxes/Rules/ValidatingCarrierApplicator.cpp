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

#include <sstream>

#include "DataModel/Services/CarrierApplication.h"
#include "Rules/PaymentDetail.h"
#include "Rules/ValidatingCarrierApplicator.h"
#include "Rules/ValidatingCarrierRule.h"

namespace tax
{
ValidatingCarrierApplicator::ValidatingCarrierApplicator(
    BusinessRule const* rule,
    type::CarrierCode const& validatingCarrier,
    std::shared_ptr<CarrierApplication const> carrierApplication)
  : BusinessRuleApplicator(rule),
    _validatingCarrier(validatingCarrier),
    _carrierApplication(carrierApplication)
{
}

ValidatingCarrierApplicator::~ValidatingCarrierApplicator()
{
}

bool
ValidatingCarrierApplicator::apply(PaymentDetail& paymentDetail) const
{
  bool optionalServiceMatched = false;
  for (OptionalService& service : paymentDetail.optionalServiceItems())
  {
    if (service.isFailed())
      continue;
    if (!hasMatchingEntry(getValidatingCarrier(service)))
      service.setFailedRule(getBusinessRule());
    else
      optionalServiceMatched = true;
  }

  if (!hasMatchingEntry(_validatingCarrier))
  {
    paymentDetail.getMutableItineraryDetail().setFailedRule(getBusinessRule());
    return optionalServiceMatched;
  }

  return true;
}

bool
ValidatingCarrierApplicator::hasMatchingEntry(type::CarrierCode const& validatingCarrier) const
{
  if (_carrierApplication == nullptr)
    return false;

  bool isMatching = false;

  for (const CarrierApplicationEntry& entry : _carrierApplication->entries)
  {
    if (entry.applind == type::CarrierApplicationIndicator::Positive &&
        (entry.carrier == "$$" || entry.carrier == validatingCarrier))
    {
      isMatching = true;
      if (entry.carrier != "$$")
        break;
    }

    if (entry.applind == type::CarrierApplicationIndicator::Negative &&
        entry.carrier == validatingCarrier)
    {
      isMatching = false;
      break;
    }
  }

  return isMatching;
}

type::CarrierCode const&
ValidatingCarrierApplicator::getValidatingCarrier(OptionalService const& optionalService) const
{
  if (optionalService.type() == type::OptionalServiceTag::FareRelated)
    return _validatingCarrier;

  return optionalService.ownerCarrier();
}

} // namespace tax
