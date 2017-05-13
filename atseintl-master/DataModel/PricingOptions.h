//-------------------------------------------------------------------
//
//  File:        PricingOptions.h
//  Created:     March 31, 2004
//  Authors:
//
//  Description: Processing options
//
//  Updates:
//          03/31/04 - VN - file created.
//          04/13/04 - Mike Carroll - added alternateCurrency
//          04/14/05 - Quan Ta - renamed from "Options" to PricingOptions
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

#include "Common/CabinType.h"
#include "Common/DateTime.h"
#include "Common/SpanishLargeFamilyEnum.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TypeConvert.h"
#include "DataModel/OcFeeGroupConfig.h"

#include <set>
#include <vector>

namespace tse
{

enum class AncRequestPath : unsigned
{
  AncReservationPath,
  AncCheckInPath
};


class RequestedOcFeeGroup
{
public:
  RequestedOcFeeGroup() : _groupCode(""), _numberOfItems(-1), _reqInformation(NoData) {}

  ServiceGroup& groupCode() { return _groupCode; }
  const ServiceGroup& groupCode() const { return _groupCode; }

  int& numberOfItems() { return _numberOfItems; }
  const int& numberOfItems() const { return _numberOfItems; }

  bool isAncillaryServiceType(const Indicator& ancillaryServiceType) const;
  void addAncillaryServiceType(const Indicator& ancillaryServiceType);
  void emptyAncillaryServiceType();
  bool isEmptyAncillaryServiceType() const;

  enum RequestedInformation
  {
    NoData,
    DisclosureData,
    CatalogData,
    AncillaryData
  };

   RequestedInformation getRequestedInformation() const { return _reqInformation; }
   void setRequestedInformation(const RequestedInformation& reqInf ) { _reqInformation = reqInf; }

protected:
  ServiceGroup _groupCode;
  int _numberOfItems;
  std::set<Indicator> _ancillaryServiceTypes;
  RequestedInformation _reqInformation;
};


class PricingOptions
{
  friend class PricingOptionsTest;
protected:
  // These weren't defined in the original class, make them explicit.
  PricingOptions(const PricingOptions&) = delete;

  int16_t _requestedNumberOfSolutions = 0; // NTP     SHOPPING
  char _resTicketRestr = 0; // RTK
  char _privateFares = 0; // PVL
  char _publishedFares = 0; // PUB
  char _xoFares = 'F'; // PXO
  char _mainGroupXOFares = 'F'; // PXO backup for main<PRO>
  char _onlineFares = 0; // ONL
  char _iataFares = 0; // PYY
  char _firstFlightRepeat = 0; // FRP
  CurrencyCode _currencyOverride; // CU3
  CurrencyCode _baseFareCurrencyOverride;
  int16_t _ticketStock = 0; // TKK
  int16_t _ticketTimeOverride = 0; // TKT
  char _returnAllData = 0; // GDS
  char _noPenalties = 0; // ITB
  char _noAdvPurchRestr = 0; // ITC
  char _noMinMaxStayRestr = 0; // ITD
  char _normalFare = 0; // ITE
  char _cat35NotAllowed = 0; // ITN
  char _cat35FareShouldBeIgnored = 0; // IT3
  char _returnFareCalc = 0; // ITS    SHOPPING
  char _fareByRuleEntry = 0; // FBI
  char _web = 0; // WEB
  NationCode _fareByRuleShipRegistry; // SRT
  std::vector<std::string> _excludeCarrier; // NCE
  CurrencyCode _alternateCurrency; // DUC
  char _fareCalculationDisplay = 0; // FCD
  std::string _pnr; // PNR
  std::string _lineEntry; // LNC
  std::set<std::string> _eprKeywords; // EPR
  char _mOverride = 0; // MCC
  char _cat35Sell = 0; // SEL
  char _cat35Net = 0; // NET
  bool _cat35NetSell = false; // NETSELL
  char _recordQuote = 0; // RQE
  char _thruFares = 0; // THR
  std::string _aaBasicEBC; // EBC

  // Per request
  NationCode _nationality;
  LocCode _residency;
  NationCode _employment;

  // DC options are living here for the time being
  double _amount = 0; // AMT
  NationCode _baseCountry; // CCB
  std::string _commandText; // CUR
  CurrencyCode _baseCurrency; // CUT
  DateTime _baseDT = DateTime::localTime(); // CDT
  std::string _reciprocal; // RCP

  // Used by Fare Calc
  char _sortTaxByOrigCity = ' ';

  // Q<Fare Basis Code> entry
  bool _fbcSelected = false;
  bool _journeyActivatedForPricing = false;
  bool _applyJourneyLogic = false;
  char _jpsEntered = ' '; // 'Y'=JPS-ON, 'N'=JPS-OFF, ' '=JPS not entered
  bool _journeyActivatedForShopping = false;
  bool _soloActiveForPricing = false;
  bool _allowLATAMDualRBD = false;

  // NF
  bool _ignoreFuelSurcharge = false; // PBH

  // Q-B<Booking Code>
  bool _bookingCodeOverride = false; // PBJ
  // AFBA Subscriber
  bool _freeBaggage = false; // PBK
  // PMB               MIP : callToAvailability
  char _callToAvailability = ' '; // PMB
  // CS0               MIP : use cache proxy for avai
  bool _cacheProxyMIP = false; // CS0
  // Fare Type Pricing - FareFamily in the request
  char _fareFamilyType = ' '; // N1R

  bool _fareX = false; // P4A
  int16_t _puShortCKTTimeout = -1; // Q4U
  int16_t _altDateMIPCutOffRequest = -1; // P2F

  bool _isTicketingDateOverrideEntry = false; // PBZ
  bool _isOCHistorical = false; // will be calculated by OC Service
  bool _fCorpFares = false; // PXC    Force Corporate Fares
  bool _isInitialForceCorpFares = false; // PXC    Force Corporate Fares original value
  bool _inhibitSplitPNR = false; // PXE     inhibit Split PNR

  bool _newIndicatorToRemoveCat5 = false;

  char _AdvancePurchaseOption = ' '; // PCG
  bool _validateTicketingAgreement = false; // VTI Interline Ticketing Agreement Indicator
  bool _MIPWithoutPreviousIS = false; // MWI MIP Without Previous IS Processing
  bool _isCarnivalSumOfLocal = false; // SLC
  std::string _solGroupGenerationConfig;

  bool _isMslRequest = false;

  int16_t _additionalItinsRequestedForOWFares = 0; // QD1 (IS only)
  int16_t _maxAllowedUsesOfFareCombination = 0; // QD2 (IS only)

  // Minimum connect time in seconds (domestic)
  int64_t _minConnectionTimeDomestic = -1;

  // Minimum connect time in seconds (international)
  int64_t _minConnectionTimeInternational = -1;
  CabinType _cabin; // BI0

  // ******************************** OC FEES  ******************************** //
  bool _processAllGroups = false;
  bool _ticketingInd = false;
  bool _wpWithOutAE = false;
  bool _netRemitFareRestricted = false;
  std::vector<RequestedOcFeeGroup> _serviceGroupsVec;
  int16_t _maxNumberOfOcFeesForItin = -1;
  bool _isSummaryRequest = false;
  std::string _groupsSummaryConfig;
  std::vector<OcFeeGroupConfig> _groupsSummaryConfigVec;
  bool _serviceFeesTemplateRequested = false;
  // ******************************** OC FEES  ******************************** //

  // BEGIN: ---------- Calendar Shopping for Interlines --------------
  bool _enableCalendarForInterlines = false;
  bool _splitTaxesByLeg = false;
  bool _splitTaxesByFareComponent = false;
  bool _forceFareBreaksAtLegPoints = false;
  // END: ------------ Calendar Shopping for Interlines --------------

  SLFUtil::DiscountLevel _spanishLargeFamilyDiscountLevel =
      SLFUtil::DiscountLevel::NO_DISCOUNT; // DFN
  bool _calcPfc = true; // PFC
  bool _zeroFareLogic = false; // PZF
  bool _rtw = false; // RTW
  bool _excludeFareFocusRule = false; // XFF
  bool _fRRPDOOption = false; // PDO
  bool _fRRPDROption = false; // PDR
  bool _fRRXRSOption = false; // XRS
  uint32_t _numFaresForCat12Estimation = 0;

  AncRequestPath _ancRequestPath = AncRequestPath::AncCheckInPath;

  bool _matchAndNoMatchRequested = false;

public:
  PricingOptions() = default;

  virtual ~PricingOptions() = default;

  void assign(PricingOptions& src) { *this = src; }

  //-------------------------------------------------------------------------
  // Accessors
  //-------------------------------------------------------------------------

  void setRequestedNumberOfSolutions(int16_t value) { _requestedNumberOfSolutions = value; }
  int16_t getRequestedNumberOfSolutions() const { return _requestedNumberOfSolutions; }

  char& resTicketRestr() { return _resTicketRestr; }
  const char& resTicketRestr() const { return _resTicketRestr; }
  const bool isResTicketRestr() const { return TypeConvert::pssCharToBool(_resTicketRestr); }

  char& privateFares() { return _privateFares; }
  const char& privateFares() const { return _privateFares; }
  const bool isPrivateFares() const { return TypeConvert::pssCharToBool(_privateFares); }

  char& publishedFares() { return _publishedFares; }
  const char& publishedFares() const { return _publishedFares; }
  const bool isPublishedFares() const { return TypeConvert::pssCharToBool(_publishedFares); }

  char& xoFares() { return _xoFares; }
  const char& xoFares() const { return _xoFares; }
  const bool isXoFares() const { return TypeConvert::pssCharToBool(_xoFares); }

  char& mainGroupXOFares() { return _mainGroupXOFares; }
  const char& mainGroupXOFares() const { return _mainGroupXOFares; }

  char& onlineFares() { return _onlineFares; }
  const char& onlineFares() const { return _onlineFares; }
  const bool isOnlineFares() const { return TypeConvert::pssCharToBool(_onlineFares); }

  char& iataFares() { return _iataFares; }
  const char& iataFares() const { return _iataFares; }
  const bool isIataFares() const { return TypeConvert::pssCharToBool(_iataFares); }

  char& firstFlightRepeat() { return _firstFlightRepeat; }
  const char& firstFlightRepeat() const { return _firstFlightRepeat; }

  CurrencyCode& currencyOverride() { return _currencyOverride; }
  const CurrencyCode& currencyOverride() const { return _currencyOverride; }

  CurrencyCode& baseFareCurrencyOverride() { return _baseFareCurrencyOverride; }
  const CurrencyCode& baseFareCurrencyOverride() const { return _baseFareCurrencyOverride; }

  int16_t& ticketStock() { return _ticketStock; }
  const int16_t& ticketStock() const { return _ticketStock; }

  int16_t& ticketTimeOverride() { return _ticketTimeOverride; }
  const int16_t& ticketTimeOverride() const { return _ticketTimeOverride; }

  char& returnAllData() { return _returnAllData; }
  const char& returnAllData() const { return _returnAllData; }

  char& noPenalties() { return _noPenalties; }
  const char& noPenalties() const { return _noPenalties; }
  const bool isNoPenalties() const { return TypeConvert::pssCharToBool(_noPenalties); }

  char& noAdvPurchRestr() { return _noAdvPurchRestr; }
  const char& noAdvPurchRestr() const { return _noAdvPurchRestr; }
  const bool isNoAdvPurchRestr() const { return TypeConvert::pssCharToBool(_noAdvPurchRestr); }

  char& noMinMaxStayRestr() { return _noMinMaxStayRestr; }
  const char& noMinMaxStayRestr() const { return _noMinMaxStayRestr; }
  const bool isNoMinMaxStayRestr() const { return TypeConvert::pssCharToBool(_noMinMaxStayRestr); }

  char& normalFare() { return _normalFare; }
  const char& normalFare() const { return _normalFare; }
  const bool isNormalFare() const { return TypeConvert::pssCharToBool(_normalFare); }

  char& cat35NotAllowed() { return _cat35NotAllowed; }
  const char& cat35NotAllowed() const { return _cat35NotAllowed; }

  char& cat35FareShouldBeIgnored() { return _cat35FareShouldBeIgnored; }
  const char& cat35FareShouldBeIgnored() const { return _cat35FareShouldBeIgnored; }

  char& returnFareCalc() { return _returnFareCalc; }
  const char& returnFareCalc() const { return _returnFareCalc; }

  char& fareByRuleEntry() { return _fareByRuleEntry; }
  const char& fareByRuleEntry() const { return _fareByRuleEntry; }

  char& thruFares() { return _thruFares; }
  const char& thruFares() const { return _thruFares; }
  const bool isThruFares() const { return TypeConvert::pssCharToBool(_thruFares); }

  char& web() { return _web; }
  const char& web() const { return _web; }
  const bool isWeb() const { return TypeConvert::pssCharToBool(_web); }

  NationCode& fareByRuleShipRegistry() { return _fareByRuleShipRegistry; }
  const NationCode& fareByRuleShipRegistry() const { return _fareByRuleShipRegistry; }

  std::vector<std::string>& excludeCarrier() { return _excludeCarrier; }
  const std::vector<std::string>& excludeCarrier() const { return _excludeCarrier; }

  CurrencyCode& alternateCurrency() { return _alternateCurrency; }
  const CurrencyCode& alternateCurrency() const { return _alternateCurrency; }

  char& fareCalculationDisplay() { return _fareCalculationDisplay; }
  const char& fareCalculationDisplay() const { return _fareCalculationDisplay; }

  std::string& pnr() { return _pnr; }
  const std::string& pnr() const { return _pnr; }

  virtual std::string& lineEntry() { return _lineEntry; }
  virtual const std::string& lineEntry() const { return _lineEntry; }

  std::set<std::string>& eprKeywords() { return _eprKeywords; }
  const std::set<std::string>& eprKeywords() const { return _eprKeywords; }
  bool isKeywordPresent(std::string& value)
  {
    return _eprKeywords.find(value) != _eprKeywords.end();
  }

  char& mOverride() { return _mOverride; }
  const char& mOverride() const { return _mOverride; }
  const bool isMOverride() const { return TypeConvert::pssCharToBool(_mOverride); }

  char& cat35Sell() { return _cat35Sell; }
  const char& cat35Sell() const { return _cat35Sell; }
  const bool isCat35Sell() const { return TypeConvert::pssCharToBool(_cat35Sell); }

  char& cat35Net() { return _cat35Net; }
  const char& cat35Net() const { return _cat35Net; }
  const bool isCat35Net() const { return TypeConvert::pssCharToBool(_cat35Net); }

  bool& cat35NetSell() { return _cat35NetSell; }
  const bool cat35NetSell() const { return _cat35NetSell; }

  char& recordQuote() { return _recordQuote; }
  const char& recordQuote() const { return _recordQuote; }
  const bool isRecordQuote() const { return TypeConvert::pssCharToBool(_recordQuote); }

  const bool isFareCalculationDisplay() const
  {
    return TypeConvert::pssCharToBool(_fareCalculationDisplay);
  }

  virtual NationCode& nationality() { return _nationality; }
  virtual const NationCode& nationality() const { return _nationality; }

  virtual LocCode& residency() { return _residency; }
  virtual const LocCode& residency() const { return _residency; }

  virtual NationCode& employment() { return _employment; }
  virtual const NationCode& employment() const { return _employment; }

  double& amount() { return _amount; }
  const double& amount() const { return _amount; }

  NationCode& baseCountry() { return _baseCountry; }
  const NationCode& baseCountry() const { return _baseCountry; }

  std::string& commandText() { return _commandText; }
  const std::string& commandText() const { return _commandText; }

  CurrencyCode& baseCurrency() { return _baseCurrency; }
  const CurrencyCode& baseCurrency() const { return _baseCurrency; }

  DateTime& baseDT() { return _baseDT; }
  const DateTime& baseDT() const { return _baseDT; }

  std::string& reciprocal() { return _reciprocal; }
  const std::string& reciprocal() const { return _reciprocal; }

  char& sortTaxByOrigCity() { return _sortTaxByOrigCity; }
  const char& sortTaxByOrigCity() const { return _sortTaxByOrigCity; }
  const char& isSortTaxByOrigCity() const { return _sortTaxByOrigCity; }

  bool& fbcSelected() { return _fbcSelected; }
  const bool fbcSelected() const { return _fbcSelected; }

  bool& journeyActivatedForPricing() { return _journeyActivatedForPricing; }
  const bool journeyActivatedForPricing() const { return _journeyActivatedForPricing; }

  bool& applyJourneyLogic() { return _applyJourneyLogic; }
  const bool applyJourneyLogic() const { return _applyJourneyLogic; }

  char& jpsEntered() { return _jpsEntered; }
  const char& jpsEntered() const { return _jpsEntered; }

  bool& journeyActivatedForShopping() { return _journeyActivatedForShopping; }
  const bool journeyActivatedForShopping() const { return _journeyActivatedForShopping; }

  bool& soloActiveForPricing() { return _soloActiveForPricing; }
  const bool soloActiveForPricing() const { return _soloActiveForPricing; }

  bool& allowLATAMDualRBD() { return _allowLATAMDualRBD; }
  const bool allowLATAMDualRBD() const { return _allowLATAMDualRBD; }

  bool& ignoreFuelSurcharge() { return _ignoreFuelSurcharge; }
  const bool ignoreFuelSurcharge() const { return _ignoreFuelSurcharge; }

  bool& bookingCodeOverride() { return _bookingCodeOverride; }
  const bool& bookingCodeOverride() const { return _bookingCodeOverride; }

  bool& freeBaggage() { return _freeBaggage; }
  const bool& freeBaggage() const { return _freeBaggage; }

  char& callToAvailability() { return _callToAvailability; }
  const char& callToAvailability() const { return _callToAvailability; }

  bool& cacheProxyMIP() { return _cacheProxyMIP; }
  const bool& cacheProxyMIP() const { return _cacheProxyMIP; }

  virtual char& fareFamilyType() { return _fareFamilyType; }
  virtual const char& fareFamilyType() const { return _fareFamilyType; }
  const bool isFareFamilyType() const
  {
    return (normalFareType() || specialFareType() || incTourFareType());
  }

  const bool normalFareType() const { return fareFamilyType() == 'N'; } // WPT/NL

  const bool specialFareType() const { return fareFamilyType() == 'S'; } // WPT/EX

  const bool incTourFareType() const { return fareFamilyType() == 'T'; } // WPT/IT

  // this will provide parallel testing for specified option (BET-OP?) ?=A-Z,1-9
  static constexpr char CAT9_BETA = '9'; // Cat 9 parallel testing

  bool& fareX() { return _fareX; }
  const bool fareX() const { return _fareX; }

  int16_t& puShortCKTTimeout() { return _puShortCKTTimeout; }
  const int16_t& puShortCKTTimeout() const { return _puShortCKTTimeout; }

  int16_t& altDateMIPCutOffRequest() { return _altDateMIPCutOffRequest; }
  const int16_t& altDateMIPCutOffRequest() const { return _altDateMIPCutOffRequest; }

  bool& isTicketingDateOverrideEntry() { return _isTicketingDateOverrideEntry; }
  const bool& isTicketingDateOverrideEntry() const { return _isTicketingDateOverrideEntry; }

  bool& isOCHistorical() { return _isOCHistorical; }
  const bool& isOCHistorical() const { return _isOCHistorical; }

  bool& forceCorpFares() { return _fCorpFares; }
  const bool& forceCorpFares() const { return _fCorpFares; }

  bool& initialForceCorpFares() { return _isInitialForceCorpFares; }
  const bool& initialForceCorpFares() const { return _isInitialForceCorpFares; }

  bool& inhibitSplitPNR() { return _inhibitSplitPNR; }
  const bool& inhibitSplitPNR() const { return _inhibitSplitPNR; }

  bool& newIndicatorToRemoveCat5() { return _newIndicatorToRemoveCat5; }
  const bool& newIndicatorToRemoveCat5() const { return _newIndicatorToRemoveCat5; }

  char& AdvancePurchaseOption() { return _AdvancePurchaseOption; }
  const char& AdvancePurchaseOption() const { return _AdvancePurchaseOption; }

  bool& validateTicketingAgreement() { return _validateTicketingAgreement; }
  const bool& validateTicketingAgreement() const { return _validateTicketingAgreement; }

  bool& MIPWithoutPreviousIS() { return _MIPWithoutPreviousIS; }
  const bool& MIPWithoutPreviousIS() const { return _MIPWithoutPreviousIS; }

  bool isCarnivalSumOfLocal() const { return _isCarnivalSumOfLocal; }
  void setCarnivalSumOfLocal(bool sol) { _isCarnivalSumOfLocal = sol; }

  bool isMslRequest() const { return _isMslRequest; }
  void setMslRequest() { _isMslRequest = true; }

  std::string& setSolGroupGenerationConfig() { return _solGroupGenerationConfig; }
  const std::string& getSolGroupGenerationConfig() const { return _solGroupGenerationConfig; }

  int16_t getAdditionalItinsRequestedForOWFares() const
  {
    return _additionalItinsRequestedForOWFares;
  }
  void setAdditionalItinsRequestedForOWFares(int16_t v) { _additionalItinsRequestedForOWFares = v; }

  int16_t getMaxAllowedUsesOfFareCombination() const { return _maxAllowedUsesOfFareCombination; }
  void setMaxAllowedUsesOfFareCombination(int16_t v) { _maxAllowedUsesOfFareCombination = v; }

  int64_t getMinConnectionTimeDomestic() const { return _minConnectionTimeDomestic; }
  void setMinConnectionTimeDomestic(int64_t v) { _minConnectionTimeDomestic = v; }

  int64_t getMinConnectionTimeInternational() const { return _minConnectionTimeInternational; }
  void setMinConnectionTimeInternational(int64_t v) { _minConnectionTimeInternational = v; }

  // ******************************** OC FEES  ******************************** //
  bool& isProcessAllGroups() { return _processAllGroups; }
  const bool& isProcessAllGroups() const { return _processAllGroups; }

  bool& isTicketingInd() { return _ticketingInd; }
  const bool& isTicketingInd() const { return _ticketingInd; }

  std::vector<RequestedOcFeeGroup>& serviceGroupsVec() { return _serviceGroupsVec; }
  const std::vector<RequestedOcFeeGroup>& serviceGroupsVec() const { return _serviceGroupsVec; }

  bool& isWPwithOutAE() { return _wpWithOutAE; }
  const bool& isWPwithOutAE() const { return _wpWithOutAE; }

  bool isNetRemitFareRestricted() { return _netRemitFareRestricted; }
  void setNetRemitFareRestricted(bool b) { _netRemitFareRestricted = b; }

  int16_t& maxNumberOfOcFeesForItin() { return _maxNumberOfOcFeesForItin; }
  const int16_t& maxNumberOfOcFeesForItin() const { return _maxNumberOfOcFeesForItin; }

  bool& isSummaryRequest() { return _isSummaryRequest; }
  const bool& isSummaryRequest() const { return _isSummaryRequest; }

  std::string& groupsSummaryConfig() { return _groupsSummaryConfig; }
  const std::string& groupsSummaryConfig() const { return _groupsSummaryConfig; }

  std::vector<OcFeeGroupConfig>& groupsSummaryConfigVec() { return _groupsSummaryConfigVec; }
  const std::vector<OcFeeGroupConfig>& groupsSummaryConfigVec() const
  {
    return _groupsSummaryConfigVec;
  }

  void setServiceFeesTemplateRequested() {_serviceFeesTemplateRequested = true;}
  bool isServiceFeesTemplateRequested() const {return _serviceFeesTemplateRequested;}

  void getGroupCodes(const std::vector<RequestedOcFeeGroup>& requestedGroups,
                     std::vector<ServiceGroup>& groupCodes) const;

  bool isOcOrBaggageDataRequested(RequestedOcFeeGroup::RequestedInformation ri) const;

  bool isGroupRequested(RequestedOcFeeGroup::RequestedInformation ri) const;

  bool isServiceTypeRequested(const Indicator& ancillaryServiceType) const;

  // ******************************** OC FEES  ******************************** //

  // BEGIN: ---------- Calendar Shopping for Interlines --------------
  void setEnableCalendarForInterlines(bool value) { _enableCalendarForInterlines = value; }
  bool isEnableCalendarForInterlines() const { return _enableCalendarForInterlines; }

  void setSplitTaxesByLeg(bool value) { _splitTaxesByLeg = value; }
  bool isSplitTaxesByLeg() const { return _splitTaxesByLeg; }

  void setSplitTaxesByFareComponent(bool value) { _splitTaxesByFareComponent = value; }
  bool isSplitTaxesByFareComponent() const { return _splitTaxesByFareComponent; }

  void setForceFareBreaksAtLegPoints(bool value) { _forceFareBreaksAtLegPoints = value; }
  bool isForceFareBreaksAtLegPoints() const { return _forceFareBreaksAtLegPoints; }
  // END: ------------ Calendar Shopping for Interlines --------------

  virtual void setSpanishLargeFamilyDiscountLevel(const SLFUtil::DiscountLevel level)
  {
    _spanishLargeFamilyDiscountLevel = level;
  }
  virtual SLFUtil::DiscountLevel getSpanishLargeFamilyDiscountLevel() const
  {
    return _spanishLargeFamilyDiscountLevel;
  }

  bool getCalcPfc() const { return _calcPfc; }
  void setCalcPfc(const bool calcPfc) { _calcPfc = calcPfc; }

  bool isZeroFareLogic() const { return _zeroFareLogic; }
  void setZeroFareLogic(bool value) { _zeroFareLogic = value; }

  virtual bool isRtw() const { return _rtw; }
  void setRtw(bool value) { _rtw = value; }

  virtual bool isExcludeFareFocusRule() const { return _excludeFareFocusRule; }
  void setExcludeFareFocusRule(bool value) { _excludeFareFocusRule = value; }

  virtual bool isPDOForFRRule() const { return _fRRPDOOption; }
  void setPDOForFRRule(bool value) { _fRRPDOOption = value; }

  virtual bool isPDRForFRRule() const { return _fRRPDROption; }
  void setPDRForFRRule(bool value) { _fRRPDROption = value; }

  virtual bool isXRSForFRRule() const { return _fRRXRSOption; }
  void setXRSForFRRule(bool value) { _fRRXRSOption = value; }

  void setNumFaresForCat12Estimation(uint32_t val) { _numFaresForCat12Estimation = val; }
  uint32_t getNumFaresForCat12Estimation() const { return _numFaresForCat12Estimation; }

  const AncRequestPath getAncRequestPath() const { return _ancRequestPath; }
  void setAncRequestPath(AncRequestPath ancRequestPath) { _ancRequestPath = ancRequestPath; }

  bool isMatchAndNoMatchRequested() const { return _matchAndNoMatchRequested; }
  void setMatchAndNoMatchRequested(bool val) { _matchAndNoMatchRequested = val; }

  CabinType& cabin() { return _cabin; }
  const CabinType& cabin() const { return _cabin; }

  std::string& aaBasicEBC() { return _aaBasicEBC; }
  const std::string& aaBasicEBC() const { return _aaBasicEBC; }
};

} // tse namespace
