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
#include <cassert>

#include "DataModel/Common/CodeOps.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "Rules/TaxOnTaxApplicator.h"
#include "Rules/TaxOnTaxRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointUtils.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
TaxOnTaxApplicator::TaxOnTaxApplicator(TaxOnTaxRule const& rule,
                                       const Services& services,
                                       std::shared_ptr<ServiceBaggage const> serviceBaggage,
                                       bool restrictServiceBaggage,
                                       RawPayments& rawPayments)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _services(services),
    _serviceBaggage(serviceBaggage),
    _restrictServiceBaggage(restrictServiceBaggage),
    _rawPayments(rawPayments)
{
}

TaxOnTaxApplicator::~TaxOnTaxApplicator() {}

bool
TaxOnTaxApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (!_serviceBaggage && _restrictServiceBaggage)
  {
    return false;
  }

  if (paymentDetail.getItineraryDetail().isFailedRule())
    return true;

  for(RawPayments::value_type& rawPayment : _rawPayments)
  {
    assert(rawPayment.taxName);
    bool ignore = rawPayment.detail.getItineraryDetail().isFailedRule()
               || rawPayment.detail.isExempt()
               || !isTaxNameOnList(*rawPayment.taxName)
               || !isInRange(paymentDetail, rawPayment)
               || *rawPayment.taxName == paymentDetail.taxName();

    if (ignore)
      continue;

    paymentDetail.totalTaxOnTaxAmount() +=
        rawPayment.detail.isCommandExempt() ? 0 : rawPayment.detail.taxEquivalentAmount();

    if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
    {
      paymentDetail.totalTaxOnTaxWithMarkupAmount() +=
        rawPayment.detail.isCommandExempt() ? 0 : rawPayment.detail.taxEquivalentWithMarkupAmount();
    }

    paymentDetail.taxOnTaxItems().push_back(&rawPayment.detail);
  }

  return true;
}

bool
TaxOnTaxApplicator::isTaxNameOnList(TaxName const& taxName) const
{
  if (!_serviceBaggage && (0 == _rule.itemNo() || !_restrictServiceBaggage))
  {
    return true;
  }

  if (_serviceBaggage->entries.size() == 0)
  {
    return true;
  }

  for(ServiceBaggageEntry const & entry : _serviceBaggage->entries)
  {
    if (taxName.taxCode() == entry.taxCode)
    {
      if (matches(entry.taxTypeSubcode, taxName.taxType()))
      {
        if (entry.applTag == type::ServiceBaggageAppl::Positive)
        {
          return true;
        }
        else
        {
          return false;
        }
      }
    }
  }

  return false;
}

bool
TaxOnTaxApplicator::isInRange(PaymentDetail& paymentDetail, RawPayment const& rawPayment) const
{
  if (paymentDetail.taxName().taxPointTag() == type::TaxPointTag::Sale ||
      paymentDetail.taxAppliesToTagInd() == type::TaxAppliesToTagInd::AllBaseFare)
  {
    return true;
  }

  type::Index detailBegin = paymentDetail.getTaxPointBegin().id();
  type::Index detailEnd = paymentDetail.getTaxPointEnd().id();
  if (detailBegin >= detailEnd)
    std::swap(detailBegin, detailEnd);

  type::Index rawPaymentBegin = rawPayment.detail.getTaxPointBegin().id();
  type::Index rawPaymentEnd = rawPayment.detail.getTaxPointEnd().id();
  if (rawPaymentBegin >= rawPaymentEnd)
    std::swap(rawPaymentBegin, rawPaymentEnd);

  return (detailBegin <= rawPaymentBegin && detailEnd >= rawPaymentEnd);
}

} // namespace tax
