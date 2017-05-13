// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Rules/test/RawPaymentsHelper.h"
#include "Rules/RawPayments.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/PaymentRuleData.h"

namespace tax
{

PaymentDetail&
RawPaymentsHelper::emplace(RawPayments& rawPayments, const type::Index id, TaxName& taxName,
                           type::TicketedPointTag ticketedPointTag)
{
  assert (_geoPath);
  Geo& taxPoint1 = _geoPath->geos()[id];
  Geo  taxPoint2;

  PaymentDetail& detail = rawPayments.emplace_back(
    PaymentRuleData(type::SeqNo(), ticketedPointTag, TaxableUnitTagSet::none(), 0,
                    type::CurrencyCode(UninitializedCode), type::TaxAppliesToTagInd::Blank),
    taxPoint1,
    taxPoint2,
    taxName);

  detail.getMutableTaxPointsProperties().resize(_geoPath->geos().size());
  return detail;
}

PaymentDetail&
RawPaymentsHelper::emplace(RawPayments& rawPayments, const type::Index idB, const type::Index idE, TaxName& taxName,
                           type::TicketedPointTag ticketedPointTag)
{
  assert (_geoPath);
  Geo& taxPoint1 = _geoPath->geos()[idB];
  Geo& taxPoint2 = _geoPath->geos()[idE];

  PaymentDetail& detail = rawPayments.emplace_back(
    PaymentRuleData(type::SeqNo(), ticketedPointTag, TaxableUnitTagSet::none(), 0,
                    type::CurrencyCode(UninitializedCode), type::TaxAppliesToTagInd::Blank),
    taxPoint1,
    taxPoint2,
    taxName);

  return detail;
}

} // namespace tax

