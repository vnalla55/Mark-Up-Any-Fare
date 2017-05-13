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

#include "Rules/OptionalServiceRulesBuilder.h"
#include "Rules/TaxCodeConversionRule.h"

namespace tax
{
OptionalServiceRulesBuilder::OptionalServiceRulesBuilder(const RulesRecord& rulesRecord,
                                                         const TaxableUnitTagSet& taxableUnits)
  : RulesBuilder(rulesRecord, taxableUnits)
{
}

void
OptionalServiceRulesBuilder::addValidators()
{
  buildOutputTypeIndicatorRule();
  buildSpecialTaxProcessingRule();
  buildPointOverlappingRule();

  // Ticket Group (6)
  buildCurrencyOfSaleRule();
  buildThirdPartyTagRule();
  buildPointOfTicketingRule();
  buildSaleDateRule();
  buildServiceFeeSecurityRule();
  buildPointOfSaleRule();

  // Filler Group (3)
  buildFillTimeStopoversRule();
  buildTicketedPointRule();
  buildConnectionsTagsRule();

  // GeoPath Group (7)
  buildJourneyLoc1AsOriginRule();
  buildApplicationTagRule();
  buildJourneyLoc2DestinationTurnAroundRule();
  buildTravelWhollyWithinRule();
  buildJourneyIncludes();
  buildReturnToOriginRule();
  buildContinuousJourneyRule();

  // OptionalService Group(2)
  buildOptionalServiceTagsRule();
  buildOptionalServicePointOfDeliveryRule();

  // TaxPointBegin Group (4)
  buildTaxPointLoc1Rule();
  buildTaxPointLoc1TransferTypeRule();
  buildTaxPointLoc1StopoverTagRule();
  buildTaxPointLoc1InternationalDomesticRule();

  // TaxPointEnd Group (4)
  buildTaxPointLoc2StopoverTagRule();
  buildTaxPointLoc2Rule();
  buildTaxPointLoc2InternationalDomesticRule();
  buildTaxLoc2CompareRule();
  buildTaxPointLoc3GeoRule();

  // Itin Group (5)
  buildTravelDatesRule();
  buildValidatingCarrierRule();
  buildCarrierFlightRule();
  buildPassengerTypeCodeRule();
  buildSectorDetailRule();

  buildTaxOnOptionalServiceRule();
}

void
OptionalServiceRulesBuilder::addCalculators()
{
  // Apply Group (13)
  buildYqYrAmountRule();
  buildServiceBaggageRule();
  buildTicketMinMaxValueOCRule();
  buildTaxOnTaxRule();
  buildTaxOnYqYrRule();
  buildFlatTaxRule();
  buildAlternateRefAkHiFactorsRule();
  buildTaxOnFareRule();
  buildPercentageTaxRule();
  buildTaxMinMaxValueRule();
  buildTaxApplicationLimitRule();
  buildTaxMatchingApplTagRule();
  buildTaxOnChangeFeeRule();
  buildTaxOnTicketingFeeRule();
  buildTaxRoundingOCRule();

  // Final Group (3)
  buildReportingRecordRule();
  buildExemptTagRule();
  buildTaxCodeConversionRule();
}

} /* namespace tax */
