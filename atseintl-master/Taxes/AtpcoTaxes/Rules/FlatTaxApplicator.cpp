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

#include "Common/CalcDetails.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/LoggerService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/FlatTaxApplicator.h"
#include "Rules/NoMatchingTutMessage.h"
#include "Rules/PaymentDetail.h"

namespace tse
{
ATPCO_FALLBACK_DECL(ATPCO_TAX_OcCurrencyConversionFix)
ATPCO_FALLBACK_DECL(monetaryDiscountFlatTaxesApplication)
ATPCO_FALLBACK_DECL(ATPCO_TAX_useIATARulesRoundingFix)
}

namespace tax
{

FlatTaxApplicator::FlatTaxApplicator(const BusinessRule* parent,
                                     const Services& services,
                                     const TaxableUnitTagSet& taxableUnitSet,
                                     const type::CurrencyCode& paymentCurrency)
  : BusinessRuleApplicator(parent),
    _services(services),
    _taxableUnitSet(taxableUnitSet),
    _paymentCurrency(paymentCurrency)
{
}

FlatTaxApplicator::~FlatTaxApplicator() {}

bool
FlatTaxApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (_taxableUnitSet.isEmpty())
  {
    paymentDetail.applicatorFailMessage() += "NO TAXABLE UNIT TAGS MARKED";
    return false;
  }

  bool applyTax = false;
  type::Index applyCount = 0;

  if (!paymentDetail.getItineraryDetail().isFailedRule() &&
      _taxableUnitSet.hasTag(type::TaxableUnit::Itinerary))
  {
    ++applyCount;
    applyTax = true;
  }

  if (_taxableUnitSet.hasTag(type::TaxableUnit::YqYr))
  {
    applyCount += paymentDetail.getYqYrDetails().getValidCount();
    applyTax |= (paymentDetail.getYqYrDetails().getValidCount() > 0);
  }

  if (_taxableUnitSet.hasTag(type::TaxableUnit::TaxOnTax))
  {
    applyCount += paymentDetail.taxOnTaxItems().size();
    applyTax |= (paymentDetail.taxOnTaxItems().size() > 0);
  }

  if (_taxableUnitSet.hasChangeFeeTags())
  {
    applyCount = 1;
    applyTax = true;
  }

  bool doTruncation = false;
  if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_useIATARulesRoundingFix))
    doTruncation = true;

  const type::Money publishedAmount = paymentDetail.getPublishedAmount();
  { // no tag check here; if we have OC => some OC TUT is on
    if (!_services.fallbackService().isSet(tse::fallback::monetaryDiscountFlatTaxesApplication))
    {
      for (OptionalService& oc : paymentDetail.optionalServiceItems())
      {
        if (!oc.isFailed())
        {
          const type::Money actualAmount{publishedAmount._amount * oc.getQuantity(), publishedAmount._currency};

          if (oc.taxInclInd())
          {
            oc.taxAndRounding().setValue(actualAmount._amount);
            oc.taxAndRounding().setIncludedTaxType(FLAT);
            oc.taxAndRounding().setTruncation(doTruncation);
          }

          oc.taxAmount() = actualAmount._amount;
          if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_OcCurrencyConversionFix))
          {
            setOcTaxEquivalentAmount(actualAmount, oc);
          }
        }
      }
    }
    else
    {
      for (OptionalService& oc : paymentDetail.optionalServiceItems())
      {
        if (!oc.isFailed())
        {
          if (oc.taxInclInd())
          {
            oc.taxAndRounding().setValue(publishedAmount._amount);
            oc.taxAndRounding().setIncludedTaxType(FLAT);
            oc.taxAndRounding().setTruncation(doTruncation);
          }

          oc.taxAmount() = publishedAmount._amount;
          if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_OcCurrencyConversionFix))
          {
            setOcTaxEquivalentAmount(publishedAmount, oc);
          }
        }
      }
    }
    applyTax |= !paymentDetail.areAllOptionalServicesFailed();
  }

  type::MoneyAmount taxAmount = paymentDetail.isExempt() ? 0 : publishedAmount._amount * applyCount;

  try
  {
    type::Money taxMoney = { taxAmount, publishedAmount._currency };
    paymentDetail.taxEquivalentAmount() = _services.currencyService().convertTo(_paymentCurrency, taxMoney, &paymentDetail.calcDetails());
    paymentDetail.taxEquivalentCurrency() = _paymentCurrency;
  }
  catch (const std::runtime_error&)
  {
    std::string err = std::string("FlatTaxApplicator - conversion failed ") +
        publishedAmount._currency.asString() + _paymentCurrency.asString();
    _services.loggerService().log_ERROR(err.c_str());
    paymentDetail.taxEquivalentAmount() = 0;
    paymentDetail.taxEquivalentCurrency() = publishedAmount._currency;
  }

  if (!applyTax)
  {
    paymentDetail.applicatorFailMessage() += noMatchingTutMessage(_taxableUnitSet);
  }

  return applyTax;
}

void FlatTaxApplicator::setOcTaxEquivalentAmount(const type::Money& publishedAmount,
                                                 OptionalService& oc) const
{
  if (_paymentCurrency != publishedAmount._currency)
  {
    try
    {
      oc.setTaxEquivalentAmount(
          _services.currencyService().convertTo(_paymentCurrency, publishedAmount));
    }
    catch (const std::runtime_error&)
    {
      std::string err = std::string("FlatTaxApplicator - conversion failed ") +
          publishedAmount._currency.asString() + _paymentCurrency.asString();
      _services.loggerService().log_ERROR(err.c_str());
    }
  }
}

} // namespace tax
