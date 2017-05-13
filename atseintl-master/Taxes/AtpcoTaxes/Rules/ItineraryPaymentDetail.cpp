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

#include "BusinessRule.h"
#include "Common/OCUtil.h"
#include "ItineraryPaymentDetail.h"

namespace tax
{
ItineraryPaymentDetail::ItineraryPaymentDetail(const PaymentRuleData& paymentRuleData,
                                               const Geo& taxPointBegin,
                                               const Geo& taxPointEnd,
                                               const TaxName& taxName,
                                               const type::CarrierCode& marketingCarrier)
  : PaymentDetailBase(paymentRuleData, taxPointBegin, taxPointEnd, taxName, marketingCarrier),
    _onItinerary(_taxableData)
{
}

bool
ItineraryPaymentDetail::isFailed() const
{
  return _isOmitted ||
         (getItineraryDetail().isFailedRule() && getYqYrDetails().areAllFailed() &&
          areAllOptionalServicesFailed() && (changeFeeAmount() == 0) && !isValidForTicketingFee());
}

bool
ItineraryPaymentDetail::isValidForGroup(const type::ProcessingGroup& processingGroup) const
{
  if (UNLIKELY(_isOmitted))
  {
    return false;
  }

  switch (processingGroup)
  {
  case type::ProcessingGroup::OC:
  case type::ProcessingGroup::Baggage:
    return isValidForOptionalService(processingGroup);

  case type::ProcessingGroup::OB:
    return isValidForTicketingFee();

  case type::ProcessingGroup::ChangeFee:
    return (changeFeeAmount() != 0);

  case type::ProcessingGroup::Itinerary:
    return !getItineraryDetail().isFailedRule() || !getYqYrDetails().areAllFailed();

  default:
    return false;
  }

  return false;
}

bool
ItineraryPaymentDetail::isValidForOptionalService(const type::ProcessingGroup& processingGroup)
    const
{
  if (UNLIKELY(_isOmitted))
  {
    return false;
  }

  for (const OptionalService& os : optionalServiceItems())
  {
    if (OCUtil::isOCValidForGroup(os.type(), processingGroup) && !os.isFailed())
    {
      return true;
    }
  }
  return false;
}

bool
ItineraryPaymentDetail::isValidForTicketingFee() const
{
  if (_isOmitted)
  {
    return false;
  }

  for (const TicketingFee& each : ticketingFees())
  {
    if (each.taxAmount() > 0)
    {
      return true;
    }
  }
  return false;
}

bool
ItineraryPaymentDetail::isValidForItinerary() const
{
  return !_isOmitted && !_onItinerary.isFailedRule();
}
bool
ItineraryPaymentDetail::areAllOptionalServicesFailed() const
{
  for (type::Index i = 0; i < optionalServiceItems().size(); ++i)
  {
    if (!optionalServiceItems()[i].isFailed())
    {
      return false;
    }
  }

  return true;
}

void
ItineraryPaymentDetail::failItinerary(const BusinessRule& rule)
{
  if (!getItineraryDetail().isFailedRule())
    getMutableItineraryDetail().setFailedRule(&rule);
}

void
ItineraryPaymentDetail::failOptionalServices(const BusinessRule& rule)
{
  for (OptionalService& oc : optionalServiceItems())
  {
    if (!oc.isFailed())
      oc.setFailedRule(&rule);
  }
}

void
ItineraryPaymentDetail::failYqYrs(const BusinessRule& rule)
{
  TaxableYqYrs& taxableYqYrs = getMutableYqYrDetails();
  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (!taxableYqYrs.isFailedRule(i))
      taxableYqYrs.setFailedRule(i, rule);
  }
}

void
ItineraryPaymentDetail::failAll(const BusinessRule& rule)
{
  failItinerary(rule);
  failOptionalServices(rule);
  failYqYrs(rule);
}

std::string
ItineraryPaymentDetail::getFailureReason(Services& services) const
{
  if (_onItinerary.isFailedRule())
    return _onItinerary.getFailedRule()->getDescription(services);
  else if (_isOmitted)
    return "OVERLAPPING";
  else
    return "";
}
}
