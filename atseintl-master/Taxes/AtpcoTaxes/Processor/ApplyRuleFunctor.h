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
#pragma once

#include "DomainDataObjects/Request.h"
#include "Rules/PaymentDetail.h"

#include "Rules/ApplicationLimitApplicator.h"
#include "Rules/AlternateRefAkHiFactorsApplicator.h"
#include "Rules/ApplicationTag01Applicator.h"
#include "Rules/BlankLimitApplicator.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/CarrierFlightApplicator.h"
#include "Rules/ConnectionsTagsApplicator.h"
#include "Rules/ContinuousJourneyApplicator.h"
#include "Rules/ContinuousJourneyLimitApplicator.h"
#include "Rules/CurrencyOfSaleApplicator.h"
#include "Rules/CustomerRestrictionApplicator.h"
#include "Rules/DummyApplicator.h"
#include "Rules/ExemptTagApplicator.h"
#include "Rules/FillTimeStopoversApplicatorFacade.h"
#include "Rules/FlatTaxApplicator.h"
#include "Rules/JourneyIncludesApplicator.h"
#include "Rules/JourneyLoc1AsOriginApplicator.h"
#include "Rules/JourneyLoc2DestinationTurnAroundApplicator.h"
#include "Rules/OptionalServicePointOfDeliveryApplicator.h"
#include "Rules/OptionalServiceTagsApplicator.h"
#include "Rules/PointOverlappingForItineraryApplicator.h"
#include "Rules/PointOverlappingForYqYrApplicator.h"
#include "Rules/PassengerTypeCodeApplicatorFacade.h"
#include "Rules/PercentageTaxApplicator.h"
#include "Rules/PointOfSaleApplicator.h"
#include "Rules/PointOfTicketingApplicator.h"
#include "Rules/ReportingRecordApplicator.h"
#include "Rules/ReturnToOriginApplicator.h"
#include "Rules/SaleDateApplicator.h"
#include "Rules/SectorDetailApplicator.h"
#include "Rules/ServiceBaggageApplicator.h"
#include "Rules/ServiceFeeSecurityApplicatorFacade.h"
#include "Rules/SingleJourneyLimitApplicator.h"
#include "Rules/SpecialTaxProcessingApplicator.h"
#include "Rules/TaxCodeConversionApplicator.h"
#include "Rules/TaxMatchingApplTagApplicator.h"
#include "Rules/TaxMinMaxValueApplicator.h"
#include "Rules/TaxOnChangeFeeApplicator.h"
#include "Rules/TaxOnFareApplicator.h"
#include "Rules/TaxOnOptionalServiceApplicator.h"
#include "Rules/TaxOnTaxApplicator.h"
#include "Rules/TaxOnTicketingFeeApplicator.h"
#include "Rules/TaxOnYqYrApplicator.h"
#include "Rules/TaxPointLoc1Applicator.h"
#include "Rules/TaxPointLoc1InternationalDomesticApplicator.h"
#include "Rules/TaxPointLoc1StopoverTagApplicator.h"
#include "Rules/TaxPointLoc1TransferTypeApplicator.h"
#include "Rules/TaxPointLoc2Applicator.h"
#include "Rules/TaxPointLoc2CompareApplicator.h"
#include "Rules/TaxPointLoc2InternationalDomesticApplicator.h"
#include "Rules/TaxPointLoc2StopoverTagApplicator.h"
#include "Rules/TaxPointLoc3AsNextStopoverApplicator.h"
#include "Rules/TaxPointLoc3AsPreviousPointApplicator.h"
#include "Rules/TaxRoundingApplicator.h"
#include "Rules/TaxRoundingOCApplicator.h"
#include "Rules/ThirdPartyTagApplicator.h"
#include "Rules/TicketedPointApplicator.h"
#include "Rules/TicketMinMaxValueApplicator.h"
#include "Rules/TicketMinMaxValueOCApplicator.h"
#include "Rules/TravelDatesApplicator.h"
#include "Rules/TravelWhollyWithinApplicator.h"
#include "Rules/ValidatingCarrierApplicator.h"
#include "Rules/YqYrAmountApplicator.h"

#include "ServiceInterfaces/Services.h"

namespace tax
{
namespace
{
bool
isRuleExempted(const std::vector<ExemptedRule>& exemptedRules, const BusinessRule& rule)
{
  if (LIKELY(exemptedRules.size() == 0))
    return false;

  for (const ExemptedRule& exemptedRule : exemptedRules)
  {
    if (exemptedRule.ruleId() == rule.getId())
    {
      return true;
    }
  }

  return false;
}
}

template <class Rule>
class ApplyRuleFunctor
{
public:
  static bool apply(const Rule& rule,
                    const type::Index& itinIndex,
                    const Request& request,
                    Services& services,
                    RawPayments& itinPayments,
                    PaymentDetail& paymentDetail)
  {
    if (UNLIKELY(isRuleExempted(request.processing().exemptedRules(), rule)))
      return true;

    const typename Rule::ApplicatorType& applicator =
        ApplicatorFactory::create(rule, itinIndex, request, services, itinPayments);

    if (!applicator.apply(paymentDetail))
    {
      paymentDetail.failAll(rule);
      return false;
    }
    return true;
  }
};
}
