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

#include "Processor/TaxValidator.h"

#include "Processor/RawSubjectsCollector.h"


namespace tax
{
TaxValidator::TaxValidator(BusinessRulesProcessor& processor,
                           Services& services,
                           const TaxValue& tax,
                           const Geo& taxPoint,
                           const Geo& nextPrevTaxPoint,
                           const type::Index itinId,
                           const GeoPathProperties& geoPathProperties,
                           const Request& request,
                           RawPayments& itinRawPayments,
                           std::vector<PaymentWithRules>& paymentsToCalculate,
                           type::CarrierCode& marketingCarrier,
                           const type::ProcessingGroup& processingGroup)
    : _processor(processor),
      _services(services),
      _tax(tax),
      _taxPoint(taxPoint),
      _nextPrevTaxPoint(nextPrevTaxPoint),
      _itinId(itinId),
      _geoPathProperties(geoPathProperties),
      _request(request),
      _itinRawPayments(itinRawPayments),
      _paymentsToCalculate(paymentsToCalculate),
      _marketingCarrier(marketingCarrier),
      _subjectsCollector(processingGroup, request, itinId, taxPoint, nextPrevTaxPoint),
      _subjects(_subjectsCollector.getSubjects()),
      _validationProgress(_subjects)
{
}

PaymentDetail&
TaxValidator::addPaymentDetails(const BusinessRulesContainer& rulesContainer,
                                const RawSubjects& subjects)
{
  // only basic exception safety
  PaymentDetail& detail = _itinRawPayments.emplace_back(rulesContainer.getPaymentRuleData(),
                            _taxPoint,
                            _nextPrevTaxPoint,
                            _tax->getTaxName(),
                            _marketingCarrier);
  detail.getMutableItineraryDetail()._subject = subjects._itinerary;

  if (UNLIKELY(rulesContainer.taxableUnits().hasTag(type::TaxableUnit::YqYr)))
    detail.getMutableYqYrDetails() = subjects._yqyrs;

  detail.setTaxPointsProperties(_geoPathProperties.taxPointsProperties);
  detail.roundTripOrOpenJaw() = _geoPathProperties.roundTripOrOpenJaw;
  detail.setLimitGroup(&rulesContainer.getLimitGroup());

  return detail;
}

bool
TaxValidator::validate(const BusinessRulesContainer& rulesContainer)
{
  if (UNLIKELY(!_processor.matchesFilter(_tax->getTaxName(), rulesContainer)))
    return false;

  PaymentDetail& detail = addPaymentDetails(rulesContainer, _subjects);

  if (markDuplicatedOptionalServices(detail) &&
      rulesContainer.getValidatorsGroups().foreach<ApplyRuleFunctor>(_validationProgress.foundItinApplication(),
                                                                     _validationProgress.foundYqYrApplication(),
                                                                     _itinId,
                                                                     _request,
                                                                     _services,
                                                                     _itinRawPayments,
                                                                     detail))
  {
    detail.setValidated();
    _paymentsToCalculate.push_back(PaymentWithRules(detail, &rulesContainer.getCalculatorsGroups()));
    _validationProgress.update(detail, rulesContainer);
  }

  return _validationProgress.isFinished();
}

bool
TaxValidator::markDuplicatedOptionalServices(PaymentDetail& paymentDetail)
{
  boost::ptr_vector<OptionalService>& optionalServices = paymentDetail.optionalServiceItems();
  if (optionalServices.size() == 0)
    return true;

  bool hasValidOptionalServices = false;
  for (type::Index i = 0; i < optionalServices.size(); ++i)
  {
    if (_validationProgress.ocProgressMonitor().matchedOc()[i])
    {
      optionalServices[i].setDuplicated();
    }
    else if (!optionalServices[i].isFailed())
    {
      hasValidOptionalServices = true;
    }
  }

  return hasValidOptionalServices;
}

} // end of tax namespace
