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

#include "OBRulesBuilder.h"
#include "Rules/TaxCodeConversionRule.h"

namespace tax
{
OBRulesBuilder::OBRulesBuilder(const RulesRecord& rulesRecord,
                               const TaxableUnitTagSet& taxableUnits)
  : RulesBuilder(rulesRecord, taxableUnits)
{
}

void
OBRulesBuilder::addValidators()
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

  // Itin Group (5)
  buildTravelDatesRule();
  buildValidatingCarrierRule();
  buildPassengerTypeCodeRule();
  buildSectorDetailRule();
}

void
OBRulesBuilder::addCalculators()
{
  // Apply Group (13)
  buildYqYrAmountRule();
  buildServiceBaggageRule();
  buildTicketMinMaxValueRule();
  buildTaxOnTaxRule();
  buildTaxOnYqYrRule();
  buildTaxOnOptionalServiceRule();
  buildFlatTaxRule();
  buildAlternateRefAkHiFactorsRule();
  buildTaxOnFareRule();
  buildPercentageTaxRule();
  buildTaxMinMaxValueRule();
  buildTaxApplicationLimitRule();
  buildTaxMatchingApplTagRule();
  buildTaxOnChangeFeeRule();
  buildTaxOnTicketingFeeRule();
  buildTaxRoundingRule();

  // Final Group (3)
  buildReportingRecordRule();
  buildExemptTagRule();
  buildTaxCodeConversionRule();
}

} /* namespace tax */
