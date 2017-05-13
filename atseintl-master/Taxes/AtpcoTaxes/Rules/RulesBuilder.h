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
#include <utility>

#include "Common/Consts.h"
#include "DataModel/Services/RulesRecord.h"
#include "Rules/GroupId.h"
#include "Rules/RulesGroups.h"

namespace tax
{
class BusinessRule;
class RulesRecord;
class TaxableUnitTagSet;

typedef std::pair<GroupId, BusinessRule*> GroupIdAndBusinessRule;

class RulesBuilder
{
public:
  RulesBuilder(const RulesRecord& rulesRecord, const TaxableUnitTagSet& taxableUnits);

  virtual ~RulesBuilder() {}

  virtual void addValidators() = 0;
  void addLimiters();
  virtual void addCalculators() = 0;

  static void buildRules(const RulesRecord& rulesRecord,
                         const TaxableUnitTagSet& taxableUnits,
                         ValidatorsGroups& validatorsGroups,
                         LimitGroup& limitGroup,
                         CalculatorsGroups& calculatorsGroups);

protected:
  const RulesRecord& _rulesRecord;
  const TaxableUnitTagSet& _taxableUnits;

  template <typename Builder>
  static void buildRules(const RulesRecord& rulesRecord,
                         const TaxableUnitTagSet& taxableUnits,
                         ValidatorsGroups& validatorsGroups,
                         LimitGroup& limitGroup,
                         CalculatorsGroups& calculatorsGroups)
  {
    Builder builder(rulesRecord, taxableUnits);
    builder.addValidators();
    builder.addLimiters();
    builder.addCalculators();
    validatorsGroups = builder._validatorsGroups;
    limitGroup = builder._limitGroup;
    calculatorsGroups = builder._calculatorsGroups;
  }

  // validators
  void buildApplicationLimitRule();
  void buildSpecialTaxProcessingRule();
  void buildOutputTypeIndicatorRule();
  void buildPointOverlappingRule();
  void buildCurrencyOfSaleRule();
  void buildThirdPartyTagRule();
  void buildPointOfTicketingRule();
  void buildSaleDateRule();
  void buildServiceFeeSecurityRule();
  void buildPointOfSaleRule();
  void buildFillTimeStopoversRule();
  void buildTicketedPointRule();
  void buildConnectionsTagsRule();
  void buildJourneyLoc1AsOriginRule();
  void buildJourneyLoc1AsOriginExRule();
  void buildApplicationTagRule();
  void buildJourneyLoc2DestinationTurnAroundRule();
  void buildTravelWhollyWithinRule();
  void buildJourneyIncludes();
  void buildReturnToOriginRule();
  void buildContinuousJourneyRule();
  void buildOptionalServiceTagsRule();
  void buildOptionalServicePointOfDeliveryRule();
  void buildTaxPointLoc1Rule();
  void buildTaxPointLoc1TransferTypeRule();
  void buildTaxPointLoc1StopoverTagRule();
  void buildTaxPointLoc1InternationalDomesticRule();
  void buildTaxPointLoc2StopoverTagRule();
  void buildTaxPointLoc2Rule();
  void buildTaxPointLoc2InternationalDomesticRule();
  void buildTaxLoc2CompareRule();
  void buildTaxPointLoc3GeoRule();
  void buildTravelDatesRule();
  void buildValidatingCarrierRule();
  void buildCarrierFlightRule();
  void buildPassengerTypeCodeRule();
  void buildSectorDetailRule();
  void buildCustomerRestrictionRule();
  void buildPreviousTicketRule();

  // calculators
  void buildYqYrAmountRule();
  void buildServiceBaggageRule();
  void buildTicketMinMaxValueRule();
  void buildTicketMinMaxValueOCRule();
  void buildTaxOnTaxRule();
  void buildTaxOnYqYrRule();
  void buildTaxOnOptionalServiceRule();
  void buildFlatTaxRule();
  void buildAlternateRefAkHiFactorsRule();
  void buildTaxOnFareRule();
  void buildPercentageTaxRule();
  void buildTaxMinMaxValueRule();
  void buildTaxApplicationLimitRule();
  void buildTaxMatchingApplTagRule();
  void buildTaxOnChangeFeeRule();
  void buildTaxOnTicketingFeeRule();
  void buildTaxRoundingRule();
  void buildTaxRoundingOCRule();
  void buildReportingRecordRule();
  void buildExemptTagRule();
  void buildTaxCodeConversionRule();

  // Rules Groups
  ValidatorsGroups _validatorsGroups;
  LimitGroup _limitGroup;
  CalculatorsGroups _calculatorsGroups;
};

} /* namespace tax */
