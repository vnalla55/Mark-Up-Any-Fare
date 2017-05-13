//-------------------------------------------------------------------
//
//  File:        FareDisplayOptions.h
//  Created:     February 15, 2005
//  Authors:
//
//  Description: Fare Display Processing options
//
//  Updates:
//          02/15/05 - Mike Carroll - file created.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with

//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/FareDisplayUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AddOnAttributes.h"
#include "DataModel/ConstructedAttributes.h"
#include "DataModel/NonPublishedValues.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareDisplayPref.h"

namespace tse
{
class ERDFareComp;

class FareDisplayOptions : public PricingOptions
{
public:
  std::vector<Indicator> _surchargeTypes;

private:
  FareDisplayOptions(const FareDisplayOptions&) = delete;
  FareDisplayOptions& operator=(const FareDisplayOptions&) = delete;

  Indicator _oneWayFare = 0; // P04
  Indicator _roundTripFare = 0; // P05
  Indicator _halfRoundTripFare = 0; // P03
  Indicator _sortAscending = 0; // P88
  Indicator _sortDescending = 0; // P89
  Indicator _privateFares = 0; // P1Z
  Indicator _publicFares = 0; // P1Y
  Indicator _excludePenaltyFares = 0; // P84
  Indicator _excludeAdvPurchaseFares = 0; // P85
  Indicator _excludeRestrictedFares = 0; // P86
  Indicator _validateRules = 'T'; // P80
  Indicator _reverseSort = 0; // P81
  Indicator _displayBaseTaxTotalAmounts = 'F'; // P82
  Indicator _excludeMinMaxStayFares = 0; // P83
  Indicator _allCarriers = 0; // P87
  Indicator _adultFares = 0; // P90
  Indicator _childFares = 0; // P91
  Indicator _infantFares = 0; // P92
  Indicator _uniqueAccountCode = 0; // P93
  Indicator _uniqueCorporateID = 0; // P94
  Indicator _viewPrivateFares = ' '; // PBN
  Indicator _sellingCurrency = ' '; // PBO
  Indicator _excludeDayTimeValidation = 'F'; // PCC
  Indicator _brandGroupingOptOut = 'F'; // PCF
  int32_t _excludePercentagePenaltyFares = 0; // Q16
  int32_t _templateOverride = 0; // Q40
  bool _bsrReciprocal = false; // PBR
  Indicator _ruleMenuDisplay = ' '; // PAM
  Indicator _routingDisplay = ' '; // PAN
  Indicator _headerDisplay = ' '; // PAO
  Indicator _IntlConstructionDisplay = ' '; // PAP
  Indicator _retailerDisplay = ' ';
  Indicator _FBDisplay = ' '; // PAQ
  Indicator _combScoreboardDisplay = ' '; // PBA
  std::vector<CatNumber> _ruleCategories; // Q3W
  std::vector<AlphaCode> _alphaCodes; // S59
  std::vector<CombinabilityCode> _combinabilityCodes; // S60
  int32_t _lineNumber = 0; // Q46

  // CAT25 match information for RD
  NonPublishedValues _cat25Values;

  // CAT35 match information for RD
  NonPublishedValues _cat35Values;

  // Discounted fare match information for RD
  NonPublishedValues _discountedValues;

  // Short RD specific
  CurrencyCode _baseFareCurrency; // C46
  CarrierCode _carrierCode; // B00
  FareClassCode _fareClass; // BJ0
  TariffNumber _fareTariff = 0; // Q3W
  RoutingNumber _routing; // S49  Routing Sequence
  RoutingNumber _routingNumber; // S86  Routing Number
  VendorCode _vendorCode; // S37
  LinkNumber _linkNumber = 0; // Q46
  SequenceNumber _sequenceNumber = 0; // Q1K
  DateTime _createDate = DateTime::emptyDate(); // D12
  std::string _createTime; // D55
  Indicator _templateType = 'S'; // NOP
  Indicator _FDcat35Type = '\0'; // N1P
  FareType _fareType; // S53
  RuleNumber _ruleNumber; // S90
  std::string _privateIndicator; // PCG

  // Short MP specific
  Indicator _oneWay = ' '; // PBP
  Indicator _halfRoundTrip = ' '; // PBQ

  // ERD specific
  std::vector<ERDFareComp*> _erdFareComps;
  std::vector<uint16_t> _requestedSegments; // PRO:Q0C
  std::vector<uint16_t> _surfaceSegments; // PRO:N0F
  PaxTypeCode _requestedPaxTypeCode; // PRO:B70
  int16_t _requestedPaxTypeNumber = -1; // PRO:B71
  int16_t _requestedLineNumber = -1; // PRO:Q46
  Indicator _errorFromPSS = ' '; // PRO:ERR
  bool _isWqrd = false; // DTS:C20
  bool _swappedDirectionality = false; // C25/C35/DFI:S70
  bool _isErdAfterCmdPricing = false; // ERD:CP0

  // FareDisplayPref attributes
  Indicator _displayHalfRoundTrip = ' ';

  // FareDisplayPrefSeg attributes
  Indicator _applySurcharges = NO;

  // Constructed fare items
  ConstructedAttributes _constructedAttributes;
  AddOnAttributes _origAttributes;
  AddOnAttributes _destAttributes;

  FareDisplayPref* _fareDisplayPref = nullptr;

  std::string _lineEntry;

  PseudoCityCode _sourcePCCForCat35;
  PseudoCityCode _sourcePCCForASL;

  TariffNumber _frrTariffNumber = 0;         // fares with specified tariff number
  RuleNumber   _frrRuleNumber;               // fares with specified rule number
  std::string  _frrFareTypeCode;             // fares with specified fare type code
  Indicator    _frrDisplayType = ' ';        // fares with specified display type
  Indicator    _frrPrivateIndicator = ' ';   // fares with specified private indicator
  bool         _frrSelectCat35Fares = false; // select negotiated fares
  bool         _frrSelectCat25Fares = false; // select fare by rule fares
  bool         _frrSelectCat15Fares = false; // select Sales Restriction fares

public:
  FareDisplayOptions();

  AddOnAttributes& origAttributes() { return _origAttributes; }
  const AddOnAttributes& origAttributes() const { return _origAttributes; }

  AddOnAttributes& destAttributes() { return _destAttributes; }
  const AddOnAttributes& destAttributes() const { return _destAttributes; }

  ConstructedAttributes& constructedAttributes() { return _constructedAttributes; }
  const ConstructedAttributes& constructedAttributes() const { return _constructedAttributes; }

  NonPublishedValues& cat25Values() { return _cat25Values; }
  const NonPublishedValues& cat25Values() const { return _cat25Values; }

  NonPublishedValues& cat35Values() { return _cat35Values; }
  const NonPublishedValues& cat35Values() const { return _cat35Values; }

  NonPublishedValues& discountedValues() { return _discountedValues; }
  const NonPublishedValues& discountedValues() const { return _discountedValues; }

  char& oneWayFare() { return _oneWayFare; }
  const char& oneWayFare() const { return _oneWayFare; }
  const bool isOneWayFare() const { return TypeConvert::pssCharToBool(_oneWayFare); }

  char& roundTripFare() { return _roundTripFare; }
  const char& roundTripFare() const { return _roundTripFare; }
  const bool isRoundTripFare() const { return TypeConvert::pssCharToBool(_roundTripFare); }

  char& halfRoundTripFare() { return _halfRoundTripFare; }
  const char& halfRoundTripFare() const { return _halfRoundTripFare; }
  const bool isHalfRoundTripFare() const { return TypeConvert::pssCharToBool(_halfRoundTripFare); }

  char& sortAscending() { return _sortAscending; }
  const char& sortAscending() const { return _sortAscending; }
  const bool isSortAscending() const { return TypeConvert::pssCharToBool(_sortAscending); }

  char& sortDescending() { return _sortDescending; }
  const char& sortDescending() const { return _sortDescending; }
  const bool isSortDescending() const { return TypeConvert::pssCharToBool(_sortDescending); }

  char& privateFares() { return _privateFares; }
  const char& privateFares() const { return _privateFares; }
  const bool isPrivateFares() const { return TypeConvert::pssCharToBool(_privateFares); }

  char& publicFares() { return _publicFares; }
  const char& publicFares() const { return _publicFares; }
  const bool isPublicFares() const { return TypeConvert::pssCharToBool(_publicFares); }

  char& excludePenaltyFares() { return _excludePenaltyFares; }
  const char& excludePenaltyFares() const { return _excludePenaltyFares; }
  const bool isExcludePenaltyFares() const
  {
    return TypeConvert::pssCharToBool(_excludePenaltyFares);
  }

  char& excludeAdvPurchaseFares() { return _excludeAdvPurchaseFares; }
  const char& excludeAdvPurchaseFares() const { return _excludeAdvPurchaseFares; }
  const bool isExcludeAdvPurchaseFares() const
  {
    return TypeConvert::pssCharToBool(_excludeAdvPurchaseFares);
  }

  char& excludeRestrictedFares() { return _excludeRestrictedFares; }
  const char& excludeRestrictedFares() const { return _excludeRestrictedFares; }
  const bool isExcludeRestrictedFares() const
  {
    return TypeConvert::pssCharToBool(_excludeRestrictedFares);
  }

  int32_t& excludePercentagePenaltyFares() { return _excludePercentagePenaltyFares; }
  const int32_t& excludePercentagePenaltyFares() const { return _excludePercentagePenaltyFares; }

  char& validateRules() { return _validateRules; }
  const char& validateRules() const { return _validateRules; }
  const bool isValidateRules() const { return TypeConvert::pssCharToBool(_validateRules); }

  char& reverseSort() { return _reverseSort; }
  const char& reverseSort() const { return _reverseSort; }

  char& displayBaseTaxTotalAmounts() { return _displayBaseTaxTotalAmounts; }
  const char& displayBaseTaxTotalAmounts() const { return _displayBaseTaxTotalAmounts; }

  char& excludeMinMaxStayFares() { return _excludeMinMaxStayFares; }
  const char& excludeMinMaxStayFares() const { return _excludeMinMaxStayFares; }
  const bool isExcludeMinMaxStayFares() const
  {
    return TypeConvert::pssCharToBool(_excludeMinMaxStayFares);
  }

  char& allCarriers() { return _allCarriers; }
  const char& allCarriers() const { return _allCarriers; }
  const bool isAllCarriers() const { return TypeConvert::pssCharToBool(_allCarriers); }

  char& adultFares() { return _adultFares; }
  const char& adultFares() const { return _adultFares; }
  const bool isAdultFares() const { return TypeConvert::pssCharToBool(_adultFares); }

  char& childFares() { return _childFares; }
  const char& childFares() const { return _childFares; }
  const bool isChildFares() const { return TypeConvert::pssCharToBool(_childFares); }

  char& infantFares() { return _infantFares; }
  const char& infantFares() const { return _infantFares; }
  const bool isInfantFares() const { return TypeConvert::pssCharToBool(_infantFares); }

  char& uniqueAccountCode() { return _uniqueAccountCode; }
  const char& uniqueAccountCode() const { return _uniqueAccountCode; }
  const bool isUniqueAccountCode() const { return TypeConvert::pssCharToBool(_uniqueAccountCode); }

  char& uniqueCorporateID() { return _uniqueCorporateID; }
  const char& uniqueCorporateID() const { return _uniqueCorporateID; }
  const bool isUniqueCorporateID() const { return TypeConvert::pssCharToBool(_uniqueCorporateID); }

  Indicator& viewPrivateFares() { return _viewPrivateFares; }
  const Indicator& viewPrivateFares() const { return _viewPrivateFares; }
  const bool isViewPrivateFares() const { return TypeConvert::pssCharToBool(_viewPrivateFares); }

  Indicator& sellingCurrency() { return _sellingCurrency; }
  const Indicator& sellingCurrency() const { return _sellingCurrency; }
  const bool isSellingCurrency() const { return TypeConvert::pssCharToBool(_sellingCurrency); }

  char& excludeDayTimeValidation() { return _excludeDayTimeValidation; }
  const char& excludeDayTimeValidation() const { return _excludeDayTimeValidation; }
  const bool isExcludeDayTimeValidation() const
  {
    return TypeConvert::pssCharToBool(_excludeDayTimeValidation);
  }

  char& brandGroupingOptOut() { return _brandGroupingOptOut; }
  const char& brandGroupingOptOut() const { return _brandGroupingOptOut; }
  const bool isBrandGroupingOptOut() const
  {
    return TypeConvert::pssCharToBool(_brandGroupingOptOut);
  }

  int32_t& templateOverride() { return _templateOverride; }
  const int32_t& templateOverride() const { return _templateOverride; }

  Indicator& ruleMenuDisplay() { return _ruleMenuDisplay; }
  const Indicator& ruleMenuDisplay() const { return _ruleMenuDisplay; }

  Indicator& routingDisplay() { return _routingDisplay; }
  const Indicator& routingDisplay() const { return _routingDisplay; }
  const bool isRoutingDisplay() const { return TypeConvert::pssCharToBool(_routingDisplay); }

  Indicator& headerDisplay() { return _headerDisplay; }
  const Indicator& headerDisplay() const { return _headerDisplay; }

  Indicator& IntlConstructionDisplay() { return _IntlConstructionDisplay; }
  const Indicator& IntlConstructionDisplay() const { return _IntlConstructionDisplay; }

  Indicator& retailerDisplay() { return _retailerDisplay; }
  const Indicator& retailerDisplay() const { return _retailerDisplay; }

  Indicator& FBDisplay() { return _FBDisplay; }
  const Indicator& FBDisplay() const { return _FBDisplay; }

  Indicator& combScoreboardDisplay() { return _combScoreboardDisplay; }
  const Indicator& combScoreboardDisplay() const { return _combScoreboardDisplay; }
  const bool isCombScoreboardDisplay() const
  {
    return TypeConvert::pssCharToBool(_combScoreboardDisplay);
  }

  std::vector<CatNumber>& ruleCategories() { return _ruleCategories; }
  const std::vector<CatNumber>& ruleCategories() const { return _ruleCategories; }

  std::vector<AlphaCode>& alphaCodes() { return _alphaCodes; }
  const std::vector<AlphaCode>& alphaCodes() const { return _alphaCodes; }

  std::vector<CombinabilityCode>& combinabilityCodes() { return _combinabilityCodes; }
  const std::vector<CombinabilityCode>& combinabilityCodes() const { return _combinabilityCodes; }

  int32_t& lineNumber() { return _lineNumber; }
  const int32_t& lineNumber() const { return _lineNumber; }

  CurrencyCode& baseFareCurrency() { return _baseFareCurrency; }
  const CurrencyCode& baseFareCurrency() const { return _baseFareCurrency; }

  CarrierCode& carrierCode() { return _carrierCode; }
  const CarrierCode& carrierCode() const { return _carrierCode; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  RoutingNumber& routingNumber() { return _routingNumber; }
  const RoutingNumber& routingNumber() const { return _routingNumber; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

  LinkNumber& linkNumber() { return _linkNumber; }
  const LinkNumber& linkNumber() const { return _linkNumber; }

  SequenceNumber& sequenceNumber() { return _sequenceNumber; }
  const SequenceNumber& sequenceNumber() const { return _sequenceNumber; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  std::string& createTime() { return _createTime; }
  const std::string& createTime() const { return _createTime; }

  Indicator& oneWay() { return _oneWay; }
  const Indicator& oneWay() const { return _oneWay; }
  const bool isOneWay() const { return TypeConvert::pssCharToBool(_oneWay); }

  Indicator& halfRoundTrip() { return _halfRoundTrip; }
  const Indicator& halfRoundTrip() const { return _halfRoundTrip; }
  const bool isHalfRoundTrip() const { return TypeConvert::pssCharToBool(_halfRoundTrip); }

  // Accessors for attributes mapped from FareDisplayPref table

  TemplateID& singleCxrTemplateId() { return _fareDisplayPref->singleCxrTemplateId(); }
  const TemplateID& singleCxrTemplateId() const { return _fareDisplayPref->singleCxrTemplateId(); }

  TemplateID& multiCxrTemplateId() { return _fareDisplayPref->multiCxrTemplateId(); }
  const TemplateID& multiCxrTemplateId() const { return _fareDisplayPref->multiCxrTemplateId(); }

  Indicator& showRoutings() { return _fareDisplayPref->showRoutings(); }
  const Indicator& showRoutings() const { return _fareDisplayPref->showRoutings(); }

  Indicator& doubleForRoundTrip() { return _fareDisplayPref->doubleForRoundTrip(); }
  const Indicator& doubleForRoundTrip() const { return _fareDisplayPref->doubleForRoundTrip(); }

  Indicator& displayHalfRoundTrip() { return _fareDisplayPref->displayHalfRoundTrip(); }
  const Indicator& displayHalfRoundTrip() const { return _displayHalfRoundTrip; }

  Indicator& sameFareBasisSameLine() { return _fareDisplayPref->sameFareBasisSameLine(); }
  const Indicator& sameFareBasisSameLine() const
  {
    return _fareDisplayPref->sameFareBasisSameLine();
  }

  Indicator& returnDateValidation() { return _fareDisplayPref->returnDateValidation(); }
  const Indicator& returnDateValidation() const { return _fareDisplayPref->returnDateValidation(); }

  Indicator& noFutureSalesDate() { return _fareDisplayPref->noFutureSalesDate(); }
  const Indicator& noFutureSalesDate() const { return _fareDisplayPref->noFutureSalesDate(); }

  Indicator& singleCarrierSvcSched() { return _fareDisplayPref->singleCarrierSvcSched(); }
  const Indicator& singleCarrierSvcSched() const
  {
    return _fareDisplayPref->singleCarrierSvcSched();
  }

  Indicator& multiCarrierSvcSched() { return _fareDisplayPref->multiCarrierSvcSched(); }
  const Indicator& multiCarrierSvcSched() const { return _fareDisplayPref->multiCarrierSvcSched(); }

  TemplateID& taxTemplateId() { return _fareDisplayPref->taxTemplateId(); }
  const TemplateID& taxTemplateId() const { return _fareDisplayPref->taxTemplateId(); }

  TemplateID& addOnTemplateId() { return _fareDisplayPref->addOnTemplateId(); }
  const TemplateID& addOnTemplateId() const { return _fareDisplayPref->addOnTemplateId(); }

  Indicator& journeyInd() { return _fareDisplayPref->journeyInd(); }
  const Indicator& journeyInd() const { return _fareDisplayPref->journeyInd(); }

  Indicator& applyDOWvalidationToOWFares()
  {
    return _fareDisplayPref->applyDOWvalidationToOWFares();
  }
  const Indicator& applyDOWvalidationToOWFares() const
  {
    return _fareDisplayPref->applyDOWvalidationToOWFares();
  }

  Indicator& applySurcharges() { return _applySurcharges; }
  const Indicator& applySurcharges() const { return _applySurcharges; }

  std::vector<Indicator>& surchargeTypes() { return _surchargeTypes; }
  const std::vector<Indicator>& surchargeTypes() const { return _surchargeTypes; }

  bool& bsrReciprocal() { return _bsrReciprocal; }
  const bool& bsrReciprocal() const { return _bsrReciprocal; }

  FareDisplayPref*& fareDisplayPref() { return _fareDisplayPref; }
  const FareDisplayPref* fareDisplayPref() const { return _fareDisplayPref; }

  Indicator& templateType() { return _templateType; }
  const Indicator& templateType() const { return _templateType; }

  virtual std::string& lineEntry() override { return _lineEntry; }
  virtual const std::string& lineEntry() const override { return _lineEntry; }

  Indicator& FDcat35Type() { return _FDcat35Type; }
  const Indicator& FDcat35Type() const { return _FDcat35Type; }

  bool isPublishedFare();

  Indicator& validateLocaleForPublFares() { return _fareDisplayPref->validateLocaleForPublFares(); }
  const Indicator& validateLocaleForPublFares() const
  {
    return _fareDisplayPref->validateLocaleForPublFares();
  }

  RuleNumber& ruleNumber() { return _ruleNumber; }
  const RuleNumber& ruleNumber() const { return _ruleNumber; }

  std::string& privateIndicator() { return _privateIndicator; }
  const std::string& privateIndicator() const { return _privateIndicator; }

  const std::vector<ERDFareComp*>& erdFareComps() const { return _erdFareComps; }
  std::vector<ERDFareComp*>& erdFareComps() { return _erdFareComps; }

  std::vector<uint16_t>& requestedSegments() { return _requestedSegments; }
  const std::vector<uint16_t>& requestedSegments() const { return _requestedSegments; }

  std::vector<uint16_t>& surfaceSegments() { return _surfaceSegments; }
  const std::vector<uint16_t>& surfaceSegments() const { return _surfaceSegments; }

  PaxTypeCode& requestedPaxTypeCode() { return _requestedPaxTypeCode; }
  const PaxTypeCode& requestedPaxTypeCode() const { return _requestedPaxTypeCode; }

  int16_t& requestedPaxTypeNumber() { return _requestedPaxTypeNumber; }
  const int16_t& requestedPaxTypeNumber() const { return _requestedPaxTypeNumber; }

  int16_t& requestedLineNumber() { return _requestedLineNumber; }
  const int16_t& requestedLineNumber() const { return _requestedLineNumber; }

  Indicator& errorFromPSS() { return _errorFromPSS; }
  const Indicator& errorFromPSS() const { return _errorFromPSS; }

  bool& isWqrd() { return _isWqrd; }
  const bool& isWqrd() const { return _isWqrd; }

  bool& swappedDirectionality() { return _swappedDirectionality; }
  const bool& swappedDirectionality() const { return _swappedDirectionality; }

  bool& isErdAfterCmdPricing() { return _isErdAfterCmdPricing; }
  const bool& isErdAfterCmdPricing() const { return _isErdAfterCmdPricing; }

  void setSourcePCCForCat35(const PseudoCityCode& pcc) { _sourcePCCForCat35 = pcc; }
  const PseudoCityCode& getSourcePCCForCat35() const { return _sourcePCCForCat35; }

  void setSourcePCCForASL(const PseudoCityCode& pcc) { _sourcePCCForASL = pcc; }
  const PseudoCityCode& getSourcePCCForASL() const { return _sourcePCCForASL; }

  TariffNumber& frrTariffNumber() { return _frrTariffNumber; }
  const TariffNumber& frrTariffNumber() const { return _frrTariffNumber; }

  RuleNumber& frrRuleNumber() { return _frrRuleNumber; }
  const RuleNumber& frrRuleNumber() const { return _frrRuleNumber; }

  std::string& frrFareTypeCode() { return _frrFareTypeCode; }
  const std::string& frrFareTypeCode() const { return _frrFareTypeCode; }

  Indicator& frrDisplayType() { return _frrDisplayType; }
  const Indicator& frrDisplayType() const { return _frrDisplayType; }

  Indicator& frrPrivateIndicator() { return _frrPrivateIndicator; }
  const Indicator& frrPrivateIndicator() const { return _frrPrivateIndicator; }

  bool& frrSelectCat35Fares() { return _frrSelectCat35Fares; }
  const bool& frrSelectCat35Fares() const { return _frrSelectCat35Fares; }

  bool& frrSelectCat25Fares() { return _frrSelectCat25Fares; }
  const bool& frrSelectCat25Fares() const { return _frrSelectCat25Fares; }

  bool& frrSelectCat15Fares() { return _frrSelectCat15Fares; }
  const bool& frrSelectCat15Fares() const { return _frrSelectCat15Fares; }
};

} // tse namespace

