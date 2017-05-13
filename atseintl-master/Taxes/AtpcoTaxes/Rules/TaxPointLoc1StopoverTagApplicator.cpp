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
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointLoc1StopoverTagApplicator.h"
#include "Rules/TaxPointLoc1StopoverTagRule.h"

namespace tax
{

namespace
{

bool isLoc1Connection(const PaymentDetail& paymentDetail)
{
  return !paymentDetail.isLoc1Stopover();
}

bool
matchesTicketedPointTag(Geo const& geo, type::TicketedPointTag const& ticketedPointTag)
{
  return (ticketedPointTag == type::TicketedPointTag::MatchTicketedAndUnticketedPoints ||
          geo.unticketedTransfer() == type::UnticketedTransfer::No);
}

type::StopoverTag matchedStopoverReason(PaymentDetail& paymentDetail,
                                        type::StopoverTag tag,
                                        type::TicketedPointTag const& ticketedPointTag,
                                        bool fareBreakMustAlsoBeStopover)
{
  Geo const& geo1 = paymentDetail.getTaxPointBegin();

  if (tag == type::StopoverTag::Connection)
  {
    if (isLoc1Connection(paymentDetail) && matchesTicketedPointTag(geo1, ticketedPointTag))
      return type::StopoverTag::Connection;
  }
  else if (tag == type::StopoverTag::Stopover)
  {
    if (paymentDetail.isLoc1Stopover())
      return type::StopoverTag::Stopover;
  }
  else if (tag == type::StopoverTag::FareBreak)
  {
    if (paymentDetail.isLoc1FareBreak() &&
        (!fareBreakMustAlsoBeStopover || paymentDetail.isLoc1Stopover()))
      return type::StopoverTag::FareBreak;
  }
  else if (tag == type::StopoverTag::NotFareBreak)
  {
    if (!paymentDetail.isLoc1FareBreak() && matchesTicketedPointTag(geo1, ticketedPointTag))
      return type::StopoverTag::NotFareBreak;
  }

  return type::StopoverTag::Blank;
}

} // anonymous namespace

TaxPointLoc1StopoverTagApplicator::TaxPointLoc1StopoverTagApplicator(
    TaxPointLoc1StopoverTagRule const& rule,
    type::StopoverTag const& stopoverTag,
    type::TicketedPointTag const& ticketedPointTag,
    bool fareBreakMustAlsoBeStopover)
  : BusinessRuleApplicator(&rule),
    _stopoverTag(stopoverTag),
    _ticketedPointTag(ticketedPointTag),
    _fareBreakMustAlsoBeStopover(fareBreakMustAlsoBeStopover)
{
}

TaxPointLoc1StopoverTagApplicator::~TaxPointLoc1StopoverTagApplicator() {}

bool
TaxPointLoc1StopoverTagApplicator::apply(PaymentDetail& paymentDetail) const
{
  paymentDetail.loc1StopoverTag() = matchedStopoverReason(paymentDetail,
                                                          _stopoverTag,
                                                          _ticketedPointTag,
                                                          _fareBreakMustAlsoBeStopover);
  const bool matchedLoc1 = (paymentDetail.loc1StopoverTag() != type::StopoverTag::Blank);

  if (!matchedLoc1)
  {
    for (OptionalService & optionalService: paymentDetail.optionalServiceItems())
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

  return true;
}

} // namespace tax
