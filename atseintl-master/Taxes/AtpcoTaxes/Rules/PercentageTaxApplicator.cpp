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

#include "Common/CalcDetails.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/Mapping.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/NoMatchingTutMessage.h"
#include "Rules/PercentageTaxApplicator.h"
#include "Rules/PercentageTaxRule.h"
#include "Rules/PaymentDetail.h"

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(ATPCO_TAX_useIATARulesRoundingFix)
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
PercentageTaxApplicator::PercentageTaxApplicator(PercentageTaxRule const& parent,
                                                 const Services& services,
                                                 const type::CurrencyCode& paymentCurrency)
  : BusinessRuleApplicator(&parent),
    _percentageTaxRule(parent),
    _services(services),
    _paymentCurrency(paymentCurrency)
{
}

PercentageTaxApplicator::~PercentageTaxApplicator()
{
}

bool
PercentageTaxApplicator::apply(PaymentDetail& paymentDetail) const
{
  const TaxableUnitTagSet& taxableUnitSet = _percentageTaxRule.taxableUnitSet();
  if (taxableUnitSet.isEmpty())
    return false;

  bool applyTax = false;

  const bool unfilteredTaxableAmount(
      _percentageTaxRule.serviceBaggageApplTag() == type::ServiceBaggageApplTag::E ||
      (_percentageTaxRule.serviceBaggageApplTag() == type::ServiceBaggageApplTag::Blank &&
       _percentageTaxRule.serviceBaggageItemNo() == 0));

  type::MoneyAmount totalTaxableAmount(0);
  type::MoneyAmount totalTaxableWithMarkupAmount(0);

  if (unfilteredTaxableAmount && taxableUnitSet.hasTag(type::TaxableUnit::YqYr))
  {
    totalTaxableAmount += paymentDetail.totalYqYrAmount();
    totalTaxableWithMarkupAmount += paymentDetail.totalYqYrAmount();
    applyTax |= (paymentDetail.totalYqYrAmount() > 0);
  }

  if (unfilteredTaxableAmount && taxableUnitSet.hasTag(type::TaxableUnit::TaxOnTax) &&
      !paymentDetail.getItineraryDetail().isFailedRule())
  {
    totalTaxableAmount += paymentDetail.totalTaxOnTaxAmount();
    totalTaxableWithMarkupAmount += paymentDetail.totalTaxOnTaxWithMarkupAmount();
    applyTax |= (paymentDetail.totalTaxOnTaxAmount() > 0);
  }

  if (paymentDetail.totalTaxOnTaxAmount() != 0)
  {
    paymentDetail.exchangeDetails().isMixedTax = true;
  }


  if (taxableUnitSet.hasTag(type::TaxableUnit::Itinerary) && paymentDetail.getTotalFareAmount() &&
      !paymentDetail.getItineraryDetail().isFailedRule())
  {
    totalTaxableAmount += *paymentDetail.getTotalFareAmount();
    if(paymentDetail.getTotalFareWithMarkupAmount().is_initialized())
    {
      totalTaxableWithMarkupAmount += *paymentDetail.getTotalFareWithMarkupAmount();
    }
    applyTax = true;
  }

  if (paymentDetail.taxAmt() == type::MoneyAmount(0))
    paymentDetail.taxAmt() = _percentageTaxRule.taxPercentage();

  if (!applyTax && taxableUnitSet.hasItineraryTags())
  {
    paymentDetail.getMutableItineraryDetail().setFailedRule(&_percentageTaxRule);
  }

  bool doTruncation = false;
  if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_useIATARulesRoundingFix))
    doTruncation = true;

  for (OptionalService& oc : paymentDetail.optionalServiceItems())
  {
    if (!oc.isFailed())
    {
      oc.taxAmount() = oc.amount() * paymentDetail.taxAmt();
      if (oc.taxInclInd())
      {
        oc.taxAndRounding().setValue(paymentDetail.taxAmt());
        oc.taxAndRounding().setIncludedTaxType(PERCENTAGE);
        oc.taxAndRounding().setTruncation(doTruncation);
      }
    }
  }
  applyTax |= !paymentDetail.areAllOptionalServicesFailed();

  if (taxableUnitSet.hasChangeFeeTags() || taxableUnitSet.hasTicketingFeeTags())
    applyTax = true;

  if (!_percentageTaxRule.taxCurrency().empty())
    paymentDetail.taxCurrency() = _percentageTaxRule.taxCurrency();

  paymentDetail.totalTaxableAmount() += totalTaxableAmount;

  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    paymentDetail.totalTaxableWithMarkupAmount() += totalTaxableWithMarkupAmount;
  }

  if (!paymentDetail.isExempt())
  {
    paymentDetail.taxEquivalentAmount() =
        paymentDetail.totalTaxableAmount() * paymentDetail.taxAmt();

    if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
    {
      paymentDetail.taxEquivalentWithMarkupAmount() =
          paymentDetail.totalTaxableWithMarkupAmount() * paymentDetail.taxAmt();
    }
  }
  paymentDetail.taxEquivalentCurrency() = _paymentCurrency;

  if (!applyTax)
    paymentDetail.applicatorFailMessage() += noMatchingTutMessage(taxableUnitSet);

  return applyTax;
}
}
