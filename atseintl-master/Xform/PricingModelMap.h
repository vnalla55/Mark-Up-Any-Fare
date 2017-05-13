//----------------------------------------------------------------------------
//
//      File: PricingModelMap.h
//      Description: Create and interpret Data Model mappings for a Pricing
//                   request.
//      Created: January 29, 2005
//      Authors: Mike Carroll
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/TseConsts.h"
#include "Common/VCTR.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/Trx.h"
#include "Xform/DataModelMap.h"
#include "Xform/RequestXmlValidator.h"

#include <boost/optional.hpp>
#include <xercesc/sax2/Attributes.hpp>

#include <map>
#include <stack>
#include <string>

namespace tse
{
class Agent;
class AirSeg;
class BaseExchangeTrx;
class ConfigMan;
class ExcItin;
class FareCompInfo;
class Itin;
class MileageTypeData;
class MinimumFareOverride;
class MultiExchangeTrx;
class PlusUpOverride;
class PricingTrx;
class RefundPricingTrx;
class RexBaseTrx;
class RexBaseRequest;
class RexPricingTrx;
class StopoverOverride;
class SurchargeOverride;
class XMLChString;

namespace xray
{
class JsonMessage;
}

class PricingModelMap : public DataModelMap
{
  friend class PricingModelMapTest;

public:
  PricingModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~PricingModelMap();

  void reset();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function PricingModelMap::classMapEntry
  //
  // Description: Multi purpose method.
  //     1.  tagName can be a class tag name
  //     2.  tagName can be a member tag name
  //     3.  tagName can be a data value
  //
  // @param tagName - value to be evaluated
  // @param atts - vector of attributes
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool classMapEntry(std::string& tagName, const xercesc::Attributes& atts) override;

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - value to be evaluated
  //--------------------------------------------------------------------------
  void saveMapEntry(std::string& tagName) override;

private:
  void checkTrxRequiredInfo();
  void checkRexPricingTrxRequiredInfo(RexPricingTrx& rexTrx);
  void checkRegularPricingTrxReqInfo(PricingTrx& princingTrx);
  std::string checkCommonTrxReqInfo(PricingTrx& pricingTrx);
  void checkRexBaseTrxRequiredInfo(RexBaseTrx& rexTrx);

  bool hasFullVCTR() const;
  virtual FareCompInfo* getFareComponent(Itin* itin, uint16_t fareCompNum) const;
  FareCompInfo* getPassengerFareComponent(PaxType* paxType, uint16_t fareCompNum) const;
  bool shouldSaveFareComponentFromFLI() const;
  void checkSideTrip();
  virtual bool corpIdExist() const;

  void tryExtractVersion(const XMLChString& attribName, const XMLChString& attribValue);

  struct Mapping
  {
    void (PricingModelMap::*func)(const xercesc::Attributes&); // store function
    void (PricingModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  PricingTrx* _pricingTrx = nullptr;

  AirSeg* _airSeg = nullptr;
  ArunkSeg* _arunkSeg = nullptr;
  Billing* _billing = nullptr;
  Itin* _itin = nullptr;
  PricingOptions* _options = nullptr;
  PaxType* _paxType = nullptr;
  TravelSeg* _currentTvlSeg = nullptr;
  bool _intlItin = false;
  ExcItin* _excItin = nullptr;

  // Local use
  int16_t _pnrSegmentNumber = 0;
  int16_t _segmentNumber = 0;
  Percent _discountPercentage = -1.0;
  Percent _discountPercentageNew = 0.0;
  MoneyAmount _discountAmount = -1.0;
  MoneyAmount _discountAmountNew = 0.0;
  size_t _discountGroupNum = 0;
  CurrencyCode _discountCurrencyCode;
  std::string _currentCRC;
  std::string _fareBasisCode;
  bool _isSpecificFareBasis = false;
  std::vector<TravelSeg::RequestedFareBasis> _specificFbcList;
  char _fbcUsage = COMMAND_PRICE_FBC;
  std::string _specifiedFbc;
  TktDesignator _tktDesignator;
  TktDesignator _specifiedTktDesignator;
  char _forcedConx = ' ';
  char _forcedStopOver = ' ';
  char _forcedSideTrip = ' ';
  char _forcedFareBreak = ' ';
  char _forcedNoFareBreak = ' ';
  CarrierCode _cxrOverride;
  std::string _globalDirectionOverride;
  int16_t _inPROCount = 0;

  // Tax exempt amount and codes
  std::vector<std::string> _taxCodeOverrides;
  std::vector<double> _overrideTaxAmounts;

  std::vector<std::string> _taxRequestedInfo;

  // Order passenger input
  uint16_t _paxInputOrder = 0;

  // for Exchange Itin
  bool _isRexTrx = false;
  bool _isExchangeItin = false;
  bool _processExchangePricingTrx = false;
  MultiExchangeTrx* _multiExcTrx = nullptr;
  CurrencyCode _fareCalcCurrency;
  CurrencyCode _newItinFareCalcCurrency;
  CurrencyCode _excItinFareCalcCurrency;
  uint16_t _fareCompNum = 0;
  int _mileageSurchargePctg = -1;
  LocCode _mileageSurchargeCity;
  LocCode _mileageTktCity;
  bool _segFlown = false;
  MoneyAmount _fareCompAmount = 0.0;
  std::string _fareCompAmountStr;
  std::vector<SurchargeOverride*> _surchargeOverride;
  std::vector<StopoverOverride*> _stopoverOverride;
  std::vector<PlusUpOverride*> _plusUpOverride;
  std::vector<MileageTypeData*> _mileDataOverride;

  enum VCTRState
  {
    VCTR_Vendor = 0x1,
    VCTR_Carrier = 0x2,
    VCTR_Tariff = 0x4,
    VCTR_Rule = 0x8,
    VCTR_AllSet = 0xF
  };
  SmallBitSet<unsigned char, VCTRState> _hasVCTR;
  VCTR _VCTR;

  // Map for datetime in airports
  std::map<LocCode, DateTime> _airportTimes;

  // for Consolidator Plus Up (Plus Up Pricing)
  MoneyAmount _consolidatorPlusUpAmount = 0.0;
  CurrencyCode _consolidatorPlusUpCurrency;
  TktDesignator _consolidatorPlusUpTktDesignator;

  std::string _xrayMessageId;
  std::string _xrayConversationId;

  uint16_t _sideTripNumber = 0;
  bool _sideTripStart = false;
  bool _sideTripEnd = false;
  bool _insideSideTrip = false;
  std::stack<FareCompInfo*> _fareCompInfoStack;
  FareCompInfo* _prevFareCompInfo = nullptr;

  bool _restrictCmdPricing = false;
  bool _isEPRKeyword = false;
  bool _isWsUser = false;
  bool _requestFromPO = false;
  RequestXmlValidator _reqXmlValidator;
  char _primaryProcessType = ' ';
  char _secondaryProcessType = ' ';
  bool _isMissingArunkForPO = false;
  std::string _considerOnlyCabin;
  bool _ticketingDTadjustedWithTime = false;
  DateTime _originalTicketingDT;
  BrandCode _brandCode;
  int16_t _legId = -1;

  std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*> _ffData;

  boost::optional<smp::Mode> _mpo;
  boost::optional<MaxPenaltyInfo::Filter> _mpChangeFilter;
  boost::optional<MaxPenaltyInfo::Filter> _mpRefundFilter;

  // Placed here so they won't be called
  PricingModelMap(const PricingModelMap& pricingModelMap);
  PricingModelMap& operator=(const PricingModelMap& pricingModelMap);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storePricingInformation
  //
  // Description: This is the equivalent of a document start for a Pricing
  //              request xml document.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePricingInformation(const xercesc::Attributes& attrs);

  void storeAltPricingInformation(const xercesc::Attributes& attrs);
  void storeRexPricingInformation(const xercesc::Attributes& attrs);

  void storeStructuredRuleInformation(const xercesc::Attributes& attrs);
  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeNoPNRPricingInformation
  //
  // Description: This is the equivalent of a document end for a No PNR Pricing
  //              request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void storeNoPNRPricingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::savePricingInformation
  //
  // Description: This is the equivalent of a document end for a Pricing
  //              request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void savePricingInformation();

  void storeXrayInformation(const xercesc::Attributes& attrs);
  void saveXrayInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeAgentInformation
  //
  // Description: This is the equivalent of a start element for a Pricing
  //              AGI element.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeAgentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveAgentInformation
  //
  // Description: This is the equivalent of a element end for a Pricing
  //              AGI element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveAgentInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::updateAgentInformation
  //
  // Description: Netherlands Antilles Country Code Changes.
  //              Correct agent location for historical pricing.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void updateAgentInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::updatePrevTicketIssueAgent
  //
  // Description: Netherlands Antilles Country Code Changes.
  //              Correct prev agent location for exchange/reissue/refund.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void updatePrevTicketIssueAgent();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeBillingInformation
  //
  // Description: This is the equivalent of a start element for a Pricing
  //              BIL element.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveBillingInformation
  //
  // Description: This is the equivalent of a element end for a Pricing
  //              BIL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  //--------------------------------------------------------------------------
  // Description: Convenience method for mapping RFB sent XML requested fare
  //              basis into the Data Model.  Must have a valid
  //              mapping scheme.
  //--------------------------------------------------------------------------
  void storeRequestedFareBasisInformation(const xercesc::Attributes& attrs);
  void saveRequestedFareBasisInformation() {}
  bool checkRequestedFareBasisInformation();
  void saveRequestedFareBasisInSegmentInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeFlightInformation
  //
  // Description: Convenience method for mapping sent XML flight
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeFlightInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveFlightInformation
  //
  // Description: Convenience method for saving a populated travel
  //              segment to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFlightInformation();

  void saveFlightInformationForDiscountAndPlusUp(PricingRequest *request);
  void saveFlightInformationForDiscountAndPlusUp(RexBaseRequest& request);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeProcOptsInformation
  //
  // Description: Convenience method for mapping PCC sent XML options
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeProcOptsInformation(const xercesc::Attributes& attrs);
  void storeExcItinProcOptsInformation(const xercesc::Attributes& attrs);
  void storeExcItinProcOptsInformationNonCat31(const xercesc::Attributes& attrs);

  void storeCountryAndStateRegionCodes(const NationCode& countryCode,
                                       const NationCode& stateRegionCode,
                                       NationCode& nationality,
                                       NationCode& employment,
                                       LocCode& residency);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveProcOptsInformation
  //
  // Description: Convenience method for saving populated option
  //              information to the request object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveProcOptsInformation();
  void checkAndAdjustPurchaseDTwithTimeForExchange();
  void setRestrictCmdPricing(const char* str);
  void setEPRKeywordFlag(const char* token);
  void restrictCmdPricing();

  void restrictExcludeFareFocusRule();
  void checkEprForPDOorXRS();
  void checkEprForPDR();
  //--------------------------------------------------------------------------
  // @function PricingModelMap::storePassengerInformation
  //
  // Description: Convenience method for mapping PCC sent XML passenger
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePassengerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::savePassengerInformation
  //
  // Description: Convenience method for saving a populated travel
  //              segment to the trx vector.
  //
  // @return void
  //--------------------------------------------------------------------------
  void savePassengerInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storePenaltyInformation
  //
  // Description: Convenience method for mapping PCC sent XML penalty
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePenaltyInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::savePenaltyInformation
  //
  // Description: Convenience method for saving a populated penalty
  //              information.
  //
  // @return void
  //--------------------------------------------------------------------------
  void savePenaltyInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeSegmentInformation
  //
  // Description: Convenience method for mapping PCC sent XML segment
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSegmentInformation(const xercesc::Attributes& attrs);

  void storePassengerTypeFlightInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveSegmentInformation
  //
  // Description: Convenience method for saving a populated SGI
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSegmentInformation();

  void savePassengerTypeFlightInformation();
  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeDiagInformation
  //
  // Description: Convenience method for mapping PCC sent XML diagnostic
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeDiagInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveDiagInformation
  //
  // Description: Convenience method for saving a populated DIG
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveDiagInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeReservationInformation
  //
  // Description: Convenience method for mapping PCC sent XML Reservation Data
  //              into the Data Model.  Must have a valid mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeReservationInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveReservationInformation
  //
  // Description: Convenience method for saving a populated Reservation Data.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveReservationInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeItinSegmentInformation
  //
  // Description: Convenience method for mapping PCC sent XML Reservation itinerary
  //              segment into the Data Model.  Must have a valid mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeItinSegmentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveItinSegmentInformation
  //
  // Description: Convenience method for saving a populated Reservation
  //              Itinerary Segment information.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveItinSegmentInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeRecordLocatorInformation
  //
  // Description: Convenience method for mapping PCC sent XML Reservation Record
  //              Locator information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeRecordLocatorInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveRecordLocatorInformation
  //
  // Description: Convenience method for saving a populated Reservation Record
  //              Locator information.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveRecordLocatorInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeReservPassengerInformation
  //
  // Description: Convenience method for mapping PCC sent XML Reservation Passenger
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void
  storeReservPassengerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveReservPassengerInformation
  //
  // Description: Convenience method for saving a populated Reservation Passenger
  //              Information.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveReservPassengerInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeCorpFreqFlyerInformation
  //
  // Description: Convenience method for mapping PCC sent XML Corporate Frequent
  //              Flyer information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeCorpFreqFlyerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveCorpFreqFlyerInformation
  //
  // Description: Convenience method for saving a populated Corporate Frequent
  //              Flyer information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveCorpFreqFlyerInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeFreqFlyerPartner
  //
  // Description: Convenience method for mapping PCC sent XML Frequent
  //              Flyer Partner information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void
  storeFreqFlyerPartnerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveCorpFreqFlyerInformation
  //
  // Description: Convenience method for saving a populated Frequent
  //              Flyer partner information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFreqFlyerPartnerInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeJPSInformation
  //
  // Description: Convenience method for mapping PCC sent XML JPS Data
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeJPSInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveJPSInformation
  //
  // Description: Convenience method for saving a populated JPS information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveJPSInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::inputPaxTypeOrder
  //
  // Description: Convenience method for calculating order number for PaxType
  // in input entry
  // @param input - input entry
  // @return      - input order number for specific PaxType
  //--------------------------------------------------------------------------

  uint16_t inputPaxTypeOrder(const std::string& inputEntry);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeAvailabilityInformation
  //
  // Description: Convenience method for Availability information
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeAvailabilityInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveAvailabilityInformation
  //
  // Description: Convenience method for saving Availability information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveAvailabilityInformation();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeFlightAvailInformation
  //
  // Description: Convenience method for Flight Availability information
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeFlightAvailInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveFlightAvailInformation
  //
  // Description: Convenience method for saving Flight Availability information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFlightAvailInformation();

  // Information for Exchange Itin
  void storeExchangeItinInformation(const xercesc::Attributes& attrs);
  void saveExchangeItinInformation();
  void checkRtwSegment(const TravelSeg& ts);

  void storeExcItinInfo(MultiExchangeTrx& multiExcTrx,
                        const xercesc::Attributes& attrs);
  void saveExcItinInfo(const MultiExchangeTrx& multiExcTrx);

  void
  storeAccompanyPassengerInformation(const xercesc::Attributes& attrs);
  void saveAccompanyPassengerInformation();

  void parsePlusUpInformation(const xercesc::Attributes& attrs,
                              MinimumFareOverride& minFareOverride,
                              std::string& moduleStr);

  int16_t parseLegId(const std::string& value) const;

  void storeNonCat31PlusUpInformation(MinimumFareOverride& minFareOverride, std::string& moduleStr);

  void storeRexPlusUpInformation(MinimumFareOverride* minFareOverride);

  void storePlusUpInformation(const xercesc::Attributes& attrs);
  void savePlusUpInformation();

  void storeTaxInformation(const xercesc::Attributes& attrs);
  void saveTaxInformation();

  bool setPlusUpType(MinimumFareModule& moduleName, const MinimumFareOverride* plusUpItem);

  bool storePlusUpOverride(MinimumFareModule& moduleName, MinimumFareOverride* plusUpItem);

  void storeDifferentialInformation(const xercesc::Attributes& attrs);
  void saveDifferentialInformation();

  void storeSurchargeInformation(const xercesc::Attributes& attrs);
  void saveSurchargeInformation();

  void storeMileageInformation(const xercesc::Attributes& attrs);
  void saveMileageInformation();

  void
  saveMileDataOverideInformation(Indicator mileageSurchargeType, LocCode& mileageSurchargeCity);

  void storeStopoverInformation(const xercesc::Attributes& attrs);
  void saveStopoverInformation();

  // set TravelSeg*, and copy from PricingModelMap to ExchangePricingTrx
  void saveStopoverOverride(BaseExchangeTrx& excTrx);

  void savePlusUpOverride(BaseExchangeTrx& excTrx);

  void saveMileDataOverride(BaseExchangeTrx& excTrx);

  void checkFlightContinuity(const LocCode& airport, const DateTime& time);

  void setBoardCity(AirSeg& airSeg);
  void setOffCity(AirSeg& airSeg);
  void setIntlBoardOffCity(Itin& itin);

  void removeArunkInSameCity(std::vector<TravelSeg*>& tvlSegs);
  bool createItin();
  void checkNoMatchItin(NoPNRPricingTrx& trx);
  void saveItin();
  void saveExchangeOverrides();
  void saveFareComponent();

  Agent* createAgent();
  Agent* getAgent(bool forPrevTicketIssueAgent);

  RexPricingTrx* getRexPricingTrx();
  RefundPricingTrx* getRefundPricingTrx();
  RexBaseTrx* getRexBaseTrx();
  ExchangePricingTrx* getExchangePricingTrx();

  RexBaseTrx::RequestTypes getRefundTypes(const xercesc::Attributes& attrs);

  void prepareRexPricingTrx(const RexBaseTrx::RequestTypes& refundTypes);
  void prepareRefundPricingTrx(const RexBaseTrx::RequestTypes& refundTypes);
  void prepareRexBaseTrx(RexBaseTrx& rexBaseTrx, const RexBaseTrx::RequestTypes& refundTypes);

  void prepareExchangePricingTrx(const std::string& refundType);

  void prepareMultiExchangePricingTrx();

  FareCompInfo*
  findFareCompInfo(const std::vector<FareCompInfo*>& itinFc, uint16_t fareCompNum) const;

  void adjustFlownSegArrivalTime(Itin* itin, TravelSeg* tvlSeg, int index);

  bool shouldStoreOverrides();
  bool exchangeTypeNotCE(BaseExchangeTrx* baseExcTrx);

  bool isFreqFlyerStatusCorrect(int ffStatus) const;
  void storeFreqFlyerStatus(const xercesc::Attributes& attrs);
  void setFreqFlyerStatus();
  void storeRequestedGroupCodes(const xercesc::Attributes& attrs);
  void checkCat35NetSellOption(PricingTrx* trx);

  void storePocInformation(const xercesc::Attributes& attrs);
  void savePocInformation();

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);
  // get cabin from the RBDByCabin table
  void getCabin(AirSeg& airSeg);

  bool isValidAmount(std::string& s, uint16_t currencyNoDec);
  void processManualAdjustmentAmount(const XMLChString& value);
  void storeB50Data(const char* xmlValue);

  void checkParsingRetailerCodeError(bool isPRM, uint8_t rcqCount);
  void parseRetailerCode(const XMLChString& xmlValue, PricingRequest* request, uint8_t& rcqCount);

  void checkArunkSegmentForSfrReq() const;
};

} // End namespace tse

