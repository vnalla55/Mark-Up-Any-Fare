//-------------------------------------------------------------------
//
//  File:        PricingTrx.h
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: Transaction's root object.
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/15/04 - Mike Carroll - added options
//          06/02/04 - Mark Kasprowicz - rename from Trx to PricingTrx
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

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include "AddonConstruction/SpecifiedFareCache.h"
#endif
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareComponentShoppingContext.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/ItinHelperStructs.h"
#include "DataModel/PosPaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RepriceCache.h"
#include "DataModel/Trx.h"
#include "DataModel/ValidatingCxrGSAData.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/PriceDeviator.h"
#include "Taxes/AtpcoTaxes/Common/AtpcoTaxesActivationStatus.h"
#include "DataModel/PreviousTicketTaxInfo.h"
#include "Xray/JsonMessage.h"

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace tse
{
class ActivationResult;
class AmVatTaxRatesOnCharges;
class BaggagePolicy;
class Billing;
class ClassOfService;
class DiscountAmount;
class FareCalcCollector;
class FareCompInfo;
class FareCalcConfig;
class FareMarket;
class Itin;
class Loc;
class MarketResponse;
class PaxType;
class PricingOptions;
class PricingUnit;
class TaxResponse;
class TravelSeg;
class Service;
class SOLItinGroups;
class SpecifiedRoutingCache;

namespace skipper
{
class TrxGeometryCalculator;
}

using MultiPaxFCMapping = std::unordered_map<const PaxType*, std::vector<FareCompInfo*>>;

class PricingTrx : public Trx
{
  friend class PricingTrxTest;

public:
  // booking class availability storage
  using ClassOfServiceKey = std::vector<TravelSeg*>;

  using ThruFareAvailabilityMap = std::map<TravelSeg*, ClassOfServiceList>;
  using PnrSegmentCollocation = std::map<int16_t, std::set<int16_t>>;
  using GlobalDirectionMap = std::map<std::pair<const Loc*, const Loc*>, GlobalDirection>;

  struct AltDateInfo
  {
    Itin* journeyItin = nullptr;
    std::vector<Itin*> taxItin;
    CarrierCode taxCxr;
    MoneyAmount totalPlusTax = 0;
    int differentCxrCnt = 0;
    int sameCxrCnt = 0;

    uint32_t numOfSolutionNeeded = 0;
    int calendarGroupNumber = 0;
    bool goodItinForDatePairFound = false;
    bool pendingSolution = false;
  };

  using AltDatePairs = std::map<DatePair, AltDateInfo*>;
  using DurationAltDatePairs = std::map<uint64_t, PricingTrx::AltDatePairs>;

  enum TrxType : uint8_t
  { PRICING_TRX,
    MIP_TRX,
    IS_TRX,
    REPRICING_TRX,
    FAREDISPLAY_TRX,
    ESV_TRX,
    FF_TRX,
    RESHOP_TRX };

  enum AltTrxType
  { WP,
    WP_NOMATCH,
    WPA,
    WPA_NOMATCH,
    WP_WITH_RO };

  enum ExcTrxType
  { NOT_EXC_TRX,
    AR_EXC_TRX,
    PORT_EXC_TRX,
    NEW_WITHIN_ME,
    EXC1_WITHIN_ME,
    EXC2_WITHIN_ME,
    ME_DIAG_TRX,
    CSO_EXC_TRX,
    EXC_IS_TRX,
    AF_EXC_TRX,
    TAX_INFO_TRX };

  enum FLPIndexType
  { PERCENTAGE_DIFF_FOR_FAMILY_SPLIT = 0,
    NUM_FAMILY_AVAIL_SPLITS,
    NUM_FAMILY_BOOKED_SPLITS,
    NUM_FAMILY_MOTHER_BOOKED_SPLITS,
    TOTAL_FLP_INDEX };

  enum FLPDefaultValueType
  { PERCENTAGE_DIFF_FOR_FAMILY_SPLIT_VAL = 10,
    NUM_FAMILY_AVAIL_SPLITS_VAL = 1,
    NUM_FAMILY_BOOKED_SPLITS_VAL = 0,
    NUM_FAMILY_MOTHER_BOOKED_SPLITS_VAL = 0,
    TOTAL_FLP_VALS = TOTAL_FLP_INDEX };

  enum BFErrorCode
  { NO_ERROR,
    CARRIER_NOT_ACTIVE,
    BG_ERROR,
    BS_ERROR };

  enum ProjectActivationState
  {
    FareSelection_Active = 0x00000001,
    Answer_Table_Active = 0x00000002
  };

  struct OriginDestination
  {
    LocCode boardMultiCity;
    LocCode offMultiCity;
    DateTime travelDate;
    DateTime travelEndDate;
    uint32_t legID = 0;
    MoneyAmount minBudgetAmount = -1;
    MoneyAmount maxBudgetAmount = -1;
    CurrencyCode currencyCode;
    uint32_t calDaysBefore = 0;
    uint32_t calDaysAfter = 0;
    uint16_t skippedOND = 0;
  };
  std::vector<OriginDestination> orgDest;

  struct SubItinValue
  {
    Itin* firstCxrItin = nullptr;
    Itin* secondCxrItin = nullptr;
    Itin* outboundItin = nullptr;
    Itin* inboundItin = nullptr;
  };

  struct ItinMetrics
  {
    uint16_t numNonStopItins = 0;
    uint16_t numOneConnectItins = 0;
    uint16_t numTwoConnectItins = 0;
    uint16_t numThreeConnectItins = 0;
    uint16_t numOnlineItins = 0;
    uint16_t numInterlineItins = 0;
  };

  class ActivationFlags
  {
    bool _isAB240 = false;
    bool _emdForCharges = false;
    bool _emdForFlightRelatedServiceAndPrepaidBaggage = false;
    bool _isSearchForBrandsPricing = false;
    bool _newRBDforM70 = false;
    bool _monetaryDiscount = false;

  public:
    void setAB240(bool isAB240) { _isAB240 = isAB240; }
    bool isAB240() const { return _isAB240; }

    void setEmdForCharges(bool emdForCharges) { _emdForCharges = emdForCharges; }
    bool isEmdForCharges() const { return _emdForCharges; }

    void setEmdForFlightRelatedServiceAndPrepaidBaggage(bool emdForFlightRelatedServiceAndPrepaidBaggage)
    {
      _emdForFlightRelatedServiceAndPrepaidBaggage = emdForFlightRelatedServiceAndPrepaidBaggage;
    }

    bool isEmdForFlightRelatedServiceAndPrepaidBaggage() const
    {
      return _emdForFlightRelatedServiceAndPrepaidBaggage;
    }

    void setSearchForBrandsPricing(bool isSearchForBrandsPricing) { _isSearchForBrandsPricing = isSearchForBrandsPricing; }
    bool isSearchForBrandsPricing() const { return _isSearchForBrandsPricing; }

    bool isNewRBDbyCabinForM70() const { return _newRBDforM70; }
    void setNewRBDbyCabinForM70(bool newRBDforM70) { _newRBDforM70 = newRBDforM70; }

    bool isMonetaryDiscount() const { return _monetaryDiscount; }
    void setMonetaryDiscount(bool monetaryDiscount) { _monetaryDiscount = monetaryDiscount; }
  };

  // SOL Carnival project
  using SOLItinGroupsMap = std::map<Itin*, SOLItinGroups*>;

protected:
  static const DateTime FAR_FUTURE;

  PricingRequest* _request = nullptr;

  xray::JsonMessagePtr _xrayMessage;

  std::vector<Itin*> _itin;
  std::vector<TravelSeg*> _travelSeg;

  std::vector<FareMarket*> _fareMarket;

  std::vector<PaxType*> _paxType;

  std::vector<TaxResponse*> _taxResponse;
  std::vector<TaxResponse*> _excItinTaxResponse;
  std::vector<TaxResponse*> _taxInfoResponse;

  std::vector<FareCalcCollector*> _fareCalcCollector;

  boost::mutex _mutexRoutingCache;

  std::map<const Itin*, FareCalcCollector*> _fareCalcCollectorMap;
  std::map<std::string, ActivationResult*> _projCACMapData;

  boost::mutex _mutexTaxResponse;
  PricingOptions* _options = nullptr;
  Billing* _billing = nullptr;
  SpecifiedRoutingCache* _specifiedRoutingCache = nullptr;

  std::string _status;

  bool _displayOnly = false;
  // bound fare
  bool _collectBoundFares = false;
  bool _validateUsingBindings = false;
  bool _validateBookingCodeBF = false;
  bool _createFareClassAppInfoBF = false;
  bool _useWebFareFlagBF = false;
  bool _useBoundRoutings = false;

  TravelSeg* _firstUnflownAirSeg = nullptr;

  DateTime _travelDate;
  DateTime _bookingDate;

  std::vector<PosPaxTypePerPax> _posPaxType; // posPaxType per pax type

  AvailabilityMap _availabilityMap;
  ThruFareAvailabilityMap _maxThruFareAvailabilityMap;

  ExcTrxType _excTrxType = ExcTrxType::NOT_EXC_TRX;

  std::vector<std::map<uint32_t, uint32_t> > _schedulingOptionIndices;
  std::vector<std::map<uint32_t, uint32_t> > _indicesToSchedulingOption;

  FareCalcConfig* _fareCalcConfig = nullptr;
  bool _budgetShopping = false; // budget shopping entry
  bool _soldOut = false; // calendar sold out condition check required
  CabinType _calendarRequestedCabin; // clendar requested cabin type from request
  DateTime _outboundDepartureDate;
  DateTime _inboundDepartureDate;
  CurrencyCode _calendarCurrencyCode;
  bool _awardRequest = false; // Award entry
  PnrSegmentCollocation _pnrSegmentCollocation;
  mutable GlobalDirectionMap _gdMap;

  std::multimap<std::string, DatePair> _validCalendarBrandIdMap;
  std::vector<bool> _validBrandIdVec;

  size_t _referenceRSS = 0;
  size_t _referenceVM = 0;

  // ******************************** WN SNAP  ******************************** //
  bool _snapRequest = false; // Q6C

  std::map<Itin*, SubItinValue> _primeSubItinMap;
  std::vector<Itin*> _subItinVecFirstCxr;
  std::vector<Itin*> _subItinVecSecondCxr;
  std::vector<Itin*> _subItinVecOutbound;
  std::vector<Itin*> _subItinVecInbound;
  // ******************************** WN SNAP  ******************************** //

  // SOL Carnival project
  SOLItinGroupsMap _solItinGroupsMap;

  uint32_t _startShortCutPricingItin = 0;
  bool _footNotePrevalidationEnabled = false;
  bool _footNotePrevalidationAllowed = false;

public:
  PricingTrx();

  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  void assignXrayJsonMessage(xray::JsonMessagePtr&& jsonMessage);
  xray::JsonMessage* getXrayJsonMessage() { return _xrayMessage.get(); }
  void pushXrayJsonMessageToContainer();

  void setRequest(PricingRequest* request) { _request = request; }
  const PricingRequest* getRequest() const { return _request; }
  PricingRequest* getRequest() { return _request; }

  virtual std::vector<Itin*>& itin() { return _itin; }
  virtual const std::vector<Itin*>& itin() const { return _itin; }

  virtual std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }
  virtual const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }

  std::vector<FareMarket*>& fareMarket() { return _fareMarket; }
  const std::vector<FareMarket*>& fareMarket() const { return _fareMarket; }

  virtual std::vector<PaxType*>& paxType() { return _paxType; }
  virtual const std::vector<PaxType*>& paxType() const { return _paxType; }

  std::vector<TaxResponse*>& taxResponse() { return _taxResponse; }
  const std::vector<TaxResponse*>& taxResponse() const { return _taxResponse; }

  void addToExcItinTaxResponse(std::vector<TaxResponse*>::const_iterator begin,
      std::vector<TaxResponse*>::const_iterator end)
  {
    _excItinTaxResponse.insert(_excItinTaxResponse.end(), begin, end);
  }

  const std::vector<TaxResponse*>& getExcItinTaxResponse() const { return _excItinTaxResponse; }

  const std::vector<TaxResponse*>& getTaxInfoResponse() const { return _taxInfoResponse; }
  void addToTaxInfoResponse(TaxResponse* res) { _taxInfoResponse.push_back(res); }

  std::vector<FareCalcCollector*>& fareCalcCollector() { return _fareCalcCollector; }
  const std::vector<FareCalcCollector*>& fareCalcCollector() const { return _fareCalcCollector; }

  boost::mutex& mutexTaxResponse() { return _mutexTaxResponse; }
  const boost::mutex& mutexTaxResponse() const { return _mutexTaxResponse; }

  boost::mutex& mutexRoutingCache() { return _mutexRoutingCache; }
  const boost::mutex& mutexRoutingCache() const { return _mutexRoutingCache; }

  void setOptions(PricingOptions* options) { _options = options; }
  const PricingOptions* getOptions() const { return _options; }
  PricingOptions* getOptions() { return _options; }

  Billing*& billing() { return _billing; }
  const Billing* billing() const override { return _billing; }

  SpecifiedRoutingCache*& specifiedRoutingCache() { return _specifiedRoutingCache; }
  const SpecifiedRoutingCache* specifiedRoutingCache() const { return _specifiedRoutingCache; }

  std::string& status() { return _status; }
  const std::string& status() const { return _status; }

  std::vector<uint16_t>& flpVector() { return _flpVector; }
  const std::vector<uint16_t>& flpVector() const { return _flpVector; }

  RepriceCache& repriceCache() { return _repriceCache; }

  bool& displayOnly() { return _displayOnly; }
  bool displayOnly() const { return _displayOnly; }

  bool& noPNRPricing() { return _noPNRPricing; }
  bool noPNRPricing() const { return _noPNRPricing; }

  std::vector<CabinType>& legPreferredCabinClass() { return _legPreferredCabinClass; }
  const std::vector<CabinType>& legPreferredCabinClass() const { return _legPreferredCabinClass; }

  // bound fare
  bool getCollectBoundFares() const { return _collectBoundFares; }
  void setCollectBoundFares(bool collectBoundFares) { _collectBoundFares = collectBoundFares; }

  int getValidForPOFaresCount() const;

  bool getValidateUsingBindings() const { return _validateUsingBindings; }
  void setValidateUsingBindings(bool validateUsingBindings)
  {
    _validateUsingBindings = validateUsingBindings;
  }

  bool getValidateBookingCodeBF() const { return _validateBookingCodeBF; }
  void setValidateBookingCodeBF(bool validateBookingCodeBF)
  {
    _validateBookingCodeBF = validateBookingCodeBF;
  }

  bool getCreateFareClassAppInfoBF() const { return _createFareClassAppInfoBF; }
  void setCreateFareClassAppInfoBF(bool createFareClassAppInfoBF)
  {
    _createFareClassAppInfoBF = createFareClassAppInfoBF;
  }

  bool getUseWebFareFlagBF() const { return _useWebFareFlagBF; }
  void setUseWebFareFlagBF(bool useWebFareFlagBF) { _useWebFareFlagBF = useWebFareFlagBF; }

  bool useBoundRoutings() const { return _useBoundRoutings; }
  void setUseBoundRoutings(bool useBoundRoutings) { _useBoundRoutings = useBoundRoutings; }

  TravelSeg*& firstUnflownAirSeg() { return _firstUnflownAirSeg; }
  TravelSeg* const firstUnflownAirSeg() const { return _firstUnflownAirSeg; }

  void setTravelDate(const DateTime& travelDate) { _travelDate = travelDate; }
  virtual const DateTime& travelDate() const { return _travelDate; }

  DateTime& bookingDate() { return _bookingDate; }
  const DateTime& bookingDate() const { return _bookingDate; }

  virtual DateTime& ticketingDate()
  {
    if (UNLIKELY(nullptr == _request))
      throw std::runtime_error("_request is NULL");
    return _request->ticketingDT();
  }

  virtual const DateTime& ticketingDate() const override
  {
    if (UNLIKELY(nullptr == _request))
      throw std::runtime_error("_request is NULL");
    return _request->ticketingDT();
  }

  std::vector<PosPaxTypePerPax>& posPaxType() { return _posPaxType; }
  const std::vector<PosPaxTypePerPax>& posPaxType() const { return _posPaxType; }

  AvailabilityMap& availabilityMap() { return _availabilityMap; }
  const AvailabilityMap& availabilityMap() const { return _availabilityMap; }

  virtual MultiPaxFCMapping* getMultiPassengerFCMapping() const { return nullptr; }
  virtual bool isMultiPassengerSFRRequestType() const { return false; }
  virtual void setMultiPassengerSFRRequestType() { /* Supported by SFR trx */}

  /** Get map of merged thru availability for segments.
 * This map stores merged (using max) thru availability for a segment. If
 * a segment has multiple thru availabilities, e.g. it is shared between
 * several itineraries, this availability will contain maximum availability
 * for each booking code.
 *
 * This is used to optimize validation, before checking the actual
 * availability.
 *
 * If you want the real thru availability of a segment, use
 * \ref availabilityMap.
 */
  ThruFareAvailabilityMap& maxThruFareAvailabilityMap() { return _maxThruFareAvailabilityMap; }
  const ThruFareAvailabilityMap& maxThruFareAvailabilityMap() const
  {
    return _maxThruFareAvailabilityMap;
  }

  TrxType getTrxType() const { return _trxType; }
  void setTrxType(TrxType value) { _trxType = value; }

  bool isMip() const { return TrxType::MIP_TRX == _trxType; }
  bool isShopping() const
  {
    return (1u << _trxType) & ((1u << IS_TRX) | (1u << ESV_TRX) | (1u << FF_TRX));
  }

  std::vector<std::map<uint32_t, uint32_t> >& schedulingOptionIndices()
  {
    return _schedulingOptionIndices;
  }
  const std::vector<std::map<uint32_t, uint32_t> >& schedulingOptionIndices() const
  {
    return _schedulingOptionIndices;
  }

  std::vector<std::map<uint32_t, uint32_t> >& indicesToSchedulingOption()
  {
    return _indicesToSchedulingOption;
  }
  const std::vector<std::map<uint32_t, uint32_t> >& indicesToSchedulingOption() const
  {
    return _indicesToSchedulingOption;
  }

  std::map<const Itin*, FareCalcCollector*>& fareCalcCollectorMap()
  {
    return _fareCalcCollectorMap;
  }
  const std::map<const Itin*, FareCalcCollector*>& fareCalcCollectorMap() const
  {
    return _fareCalcCollectorMap;
  }

  std::map<std::string, ActivationResult*>& projCACMapData() { return _projCACMapData; }
  const std::map<std::string, ActivationResult*>& projCACMapData() const { return _projCACMapData; }

  FareCalcConfig*& fareCalcConfig() { return _fareCalcConfig; }
  const FareCalcConfig* fareCalcConfig() const { return _fareCalcConfig; }

  AltTrxType& altTrxType() { return _altTrxType; }
  AltTrxType altTrxType() const { return _altTrxType; }

  ExcTrxType excTrxType() const { return _excTrxType; }
  void setExcTrxType(const ExcTrxType& excTrxType) { _excTrxType = excTrxType; }

  bool isNotExchangeTrx() const
  {
    return _excTrxType == NOT_EXC_TRX || _excTrxType == CSO_EXC_TRX;
  }

  bool isExchangeTrx() const { return !isNotExchangeTrx(); }

  bool isRexBaseTrx() const { return _excTrxType == AR_EXC_TRX || _excTrxType == AF_EXC_TRX; }

  bool& fxCnException() { return _fxCnException; }
  bool fxCnException() const { return _fxCnException; }

  ItinBoolMap& fxCnExceptionPerItin() { return _fxCnExceptionPerItin; }
  const ItinBoolMap& fxCnExceptionPerItin() const { return _fxCnExceptionPerItin; }

  AltDatePairs& altDatePairs() { return _altDatePairs; }
  const AltDatePairs& altDatePairs() const { return _altDatePairs; }

  DurationAltDatePairs& durationAltDatePairs() { return _durationAltDatePairs; }
  const DurationAltDatePairs& durationAltDatePairs() const { return _durationAltDatePairs; }

  void setAltDates(bool altDates) { _altDates = altDates; }
  const bool isAltDates() const { return _altDates; }
  const bool& altDates() const { return _altDates; } // left just for TestFactory

  bool& budgetShopping() { return _budgetShopping; }
  const bool& budgetShopping() const { return _budgetShopping; }

  bool& calendarSoldOut() { return _soldOut; }
  const bool& calendarSoldOut() const { return _soldOut; }

  CabinType& calendarRequestedCabin() { return _calendarRequestedCabin; }
  const CabinType& calendarRequestedCabin() const { return _calendarRequestedCabin; }

  DateTime& outboundDepartureDate() { return _outboundDepartureDate; }
  const DateTime& outboundDepartureDate() const { return _outboundDepartureDate; }

  DateTime& inboundDepartureDate() { return _inboundDepartureDate; }
  const DateTime& inboundDepartureDate() const { return _inboundDepartureDate; }

  CurrencyCode& calendarCurrencyCode() { return _calendarCurrencyCode; }
  const CurrencyCode& calendarCurrencyCode() const { return _calendarCurrencyCode; }

  uint64_t& mainDuration() { return _mainDuration; }
  const uint64_t& mainDuration() const { return _mainDuration; }

  MoneyAmount& altDateCutOffNucThreshold() { return _altDateCutOffNucThreshold; }

  const MoneyAmount& altDateCutOffNucThreshold() const { return _altDateCutOffNucThreshold; }

  const bool cutOffReached() const
  {
    return _cutOffReached.load(std::memory_order::memory_order_acquire);
  }
  void setCutOffReached() { _cutOffReached.store(true, std::memory_order::memory_order_release); }

  std::vector<OriginDestination>& originDest() { return orgDest; }
  const std::vector<OriginDestination>& originDest() const { return orgDest; }

  virtual bool matchFareRetrievalDate(const FareMarket&) { return true; }
  virtual bool matchFareRetrievalDate(const PaxTypeFare&) { return true; }

  virtual void setActionCode() {}

  virtual const DateTime& adjustedTravelDate(const DateTime& travelDate) const
  {
    return travelDate;
  } // overriden in RexPricingTrx

  bool& awardRequest() { return _awardRequest; }
  bool awardRequest() const { return _awardRequest; }

  std::multimap<std::string, DatePair>& validCalendarBrandIdMap()
  {
    return _validCalendarBrandIdMap;
  }

  const std::vector<bool>& validBrandIdVec() const { return _validBrandIdVec; }

  std::mutex& validatingCxrMutex() const { return _valCxrMutex; }

  ValidatingCxrGSADataHashMap& validatingCxrHashMap() { return _valCxrGsaDataHashMap; }
  const ValidatingCxrGSADataHashMap& validatingCxrHashMap() const { return _valCxrGsaDataHashMap; }

  HashSpValidatingCxrGSADataMap& hashSpValidatingCxrGsaDataMap() { return _valCxrGsaDataPerSpHashMap; }
  const HashSpValidatingCxrGSADataMap& hashSpValidatingCxrGsaDataMap() const { return _valCxrGsaDataPerSpHashMap; }
  ValidatingCxrGSAData*
  getValCxrGsaData(const std::string& hashString, bool lockingNeeded) const;
  SpValidatingCxrGSADataMap*
  getSpValCxrGsaDataMap(const std::string& hashString, bool lockingNeeded) const;

  CountrySettlementPlanInfo*& countrySettlementPlanInfo() { return _countrySettlementPlanInfo; }
  const CountrySettlementPlanInfo* countrySettlementPlanInfo() const
  { return _countrySettlementPlanInfo; }

  const std::vector<CountrySettlementPlanInfo*>& countrySettlementPlanInfos() const
  { return _countrySettlementPlanInfos; }
  void addCountrySettlementPlanInfo(CountrySettlementPlanInfo* countrySettlementPlanInfo);

  std::vector<CountrySettlementPlanInfo*> getCopyOfCountrySettlementPlanInfos() const;

  // ******************************** WN SNAP  ******************************** //
  bool& snapRequest() { return _snapRequest; }
  const bool& snapRequest() const { return _snapRequest; }

  std::map<Itin*, SubItinValue>& primeSubItinMap() { return _primeSubItinMap; }
  const std::map<Itin*, SubItinValue>& primeSubItinMap() const { return _primeSubItinMap; }

  std::vector<Itin*>& subItinVecFirstCxr() { return _subItinVecFirstCxr; }
  const std::vector<Itin*>& subItinVecFirstCxr() const { return _subItinVecFirstCxr; }

  std::vector<Itin*>& subItinVecSecondCxr() { return _subItinVecSecondCxr; }
  const std::vector<Itin*>& subItinVecSecondCxr() const { return _subItinVecSecondCxr; }

  std::vector<Itin*>& subItinVecOutbound() { return _subItinVecOutbound; }
  const std::vector<Itin*>& subItinVecOutbound() const { return _subItinVecOutbound; }

  std::vector<Itin*>& subItinVecInbound() { return _subItinVecInbound; }
  const std::vector<Itin*>& subItinVecInbound() const { return _subItinVecInbound; }

  void setMipDomestic(bool mipDomestic = true) { _mipDomestic = mipDomestic; }
  bool isMipDomestic() const { return _mipDomestic; }
  void setVisMip(bool visMip = true) { _visMip = visMip; }
  bool isVisMip() const { return _visMip; }

  // ******************************** WN SNAP  ******************************** //

  SOLItinGroupsMap& solItinGroupsMap() { return _solItinGroupsMap; }
  const SOLItinGroupsMap& solItinGroupsMap() const { return _solItinGroupsMap; }

  std::string& token() { return _token; }
  const std::string& token() const { return _token; }

  void setPnrSegmentCollocation(int16_t itinPnrSegment, int16_t similarItinPnrSegment);
  bool checkPnrSegmentCollocation(int16_t itinPnrSegment, int16_t tpdMatchedViaLoc);
  PnrSegmentCollocation& getPnrSegmentCollocation() { return _pnrSegmentCollocation; }

  void setGlobalDirectionToMap(const Loc *loc1, const Loc* loc2, GlobalDirection gd) const;
  bool getGlobalDirectionFromMap(const Loc *loc1, const Loc *loc2, GlobalDirection &gd) const;

  ItinMetrics getItinMetrics() const;

  void updateMemUsage();
  bool memGrowthExceeded() const;

  bool isCustomerActivatedByDateAndFlag(const std::string& projCode, const DateTime& tktDate);
  bool isCustomerActivatedByDate(const std::string& projCode);
  bool isCustomerActivatedByFlag(const std::string& projCode, const CarrierCode* cxr = nullptr);

  void setItinsTimeSpan();
  const DateTime& getEarliestItinDate() const { return *_itinsTimeSpan.first; }
  const DateTime& getLatestItinDate() const { return *_itinsTimeSpan.second; }

  bool& showBaggageTravelIndex() { return _showBaggageTravelIndex; }
  const bool& showBaggageTravelIndex() const { return _showBaggageTravelIndex; }

  bool& delayXpn() { return _delayXpn; }
  const bool& delayXpn() const { return _delayXpn; }

  void setFlexFarePhase1(const bool& val) { _isFlexFarePhase1 = val; }
  const bool& isFlexFarePhase1() const { return _isFlexFarePhase1; }

  void setFlexFare(const bool& val) { _isFlexFare = val; }
  const bool& isFlexFare() const { return _isFlexFare; }

  void setMainFareFFGGroupIDZero(const bool& val) { _isMainFare = val; }
  const bool& isMainFareFFGGroupIDZero() const { return _isMainFare; }

  void setProjectActivationFlag(const uint16_t& val) { _projectActivationFlag = val; }
  const uint16_t& getProjectActivationFlag() const { return  _projectActivationFlag; }

  uint32_t advanceTrxTaskHighLevelSeqNum() { return _nextTrxTaskHighLevelSeqNum++; }

  void startShortCutPricingItin(uint32_t startShortCutPricingItin)
  {
    _startShortCutPricingItin = startShortCutPricingItin;
  }

  const uint32_t& startShortCutPricingItin() const { return _startShortCutPricingItin; }

  typedef std::map<Itin*, std::vector<Itin*> > MultiTicketMap;
  MultiTicketMap& multiTicketMap() { return _multiTicketMap; }
  const MultiTicketMap& multiTicketMap() const { return _multiTicketMap; }

  typedef std::map<int, std::vector<MarketResponse*> > BrandedMarketMap;

  BrandedMarketMap& brandedMarketMap() { return _brandedMarketMap; }
  const BrandedMarketMap& brandedMarketMap() const { return _brandedMarketMap; }

  std::vector<QualifiedBrand>& brandProgramVec() { return _brandProgramVec; }
  const std::vector<QualifiedBrand>& brandProgramVec() const { return _brandProgramVec; }

  // Be sure to check the pointer for validity: zero will be returned
  // if not yet initialized.
  skipper::TrxGeometryCalculator* getTrxGeometryCalculator() { return _trxGeometryCalculator; }
  // This shall be called exactly once.
  void setTrxGeometryCalculator(skipper::TrxGeometryCalculator* calculator);

  void setPbbRequest(PbbProcessing pbbRequest) { _pbbRequest = pbbRequest; }
  PbbProcessing isPbbRequest() const { return _pbbRequest; }

  // ************************************************************************
  // TN Shopping has two branding modes. Single and multiple brands.
  // In single brand mode ( @BFS ) cheapest solution is decorated with brands
  // in multiple brands mode ( @BFA - BRALL ) each itin is priced in multiple combinations of brands
  void setTnShoppingBrandingMode(TnShoppingBrandingMode mode) { _tnShoppingBrandingMode = mode; }
  TnShoppingBrandingMode getTnShoppingBrandingMode() const { return _tnShoppingBrandingMode; }
  // Check if Brands for TN Shopping project is enabled
  bool isBrandsForTnShopping() const
  {
    return _tnShoppingBrandingMode != TnShoppingBrandingMode::NO_BRANDS;
  }
  // Check if pricing in multiple combinations of brands is enabled ( BRALL flag in Green Screen )
  bool isBRAll() const
  {
    return _tnShoppingBrandingMode == TnShoppingBrandingMode::MULTIPLE_BRANDS;
  }
  // Set maximum number of solutions to generate for MULTIPLE_BRANDS; 0 = unlimited
  void setNumberOfBrands(size_t brands) { _tnShoppingNumberOfBrands = brands; }
  // Get maximum number of solutions to generate for MULTIPLE_BRANDS; 0 = unlimited
  size_t getNumberOfBrands() const { return _tnShoppingNumberOfBrands; }
  // Check if soft passes are disabled (BRALL in shopping and TN PBB/EXPEDIA in pricing)
  bool isSoftPassDisabled() const { return _isSoftPassDisabled; }
  void setSoftPassDisabled(bool disabled) { _isSoftPassDisabled = disabled; }
  // ************************************************************************
  // Setting cabin by which brands should be filtered on a given leg
  void setCabinForLeg(size_t legId, const CabinType& cabin)
  {
    _cabinsForLegs[legId] = cabin;
  }
  // returns cabin by which brand should be filtered on a given leg.
  // Returns default CabinType() which has value of CabinType::UNDEFINED_CLASS if not found
  const CabinType getCabinForLeg(size_t legId) const
  {
    try { return _cabinsForLegs.at(legId); }
    catch (std::out_of_range& ex)
    {
      return CabinType();
    }
  }
  // returns true if filtering by cabin should be applied at all.
  // it is only the case when at least on one leg a cabin higher than economy is requested
  bool isCabinHigherThanEconomyRequested() const;
  // returns generalIndex of cabin if the same cabin is requested for the whole
  // trip, or unknown cabin index (std::string::npos) otherwise.
  size_t getWholeTripCabinIndex() const;
  // ************************************************************************
  const skipper::FareComponentShoppingContextsForSegments& getFareComponentShoppingContexts() const
  {
    return _fareComponentShoppingContexts;
  }
  skipper::FareComponentShoppingContextsForSegments& getMutableFareComponentShoppingContexts()
  {
    return _fareComponentShoppingContexts;
  }
  virtual bool isContextShopping() const
  {
    return getRequest()->isContextShoppingRequest();
  }

  const std::vector<bool>& getFixedLegs() const { return _fixedLegs; }
  std::vector<bool>& getMutableFixedLegs() { return _fixedLegs; }

  const BrandCodeSet& getBrandsFilterForIS() const { return _brandsFilterForIS; }
  BrandCodeSet& getMutableBrandsFilterForIS() { return _brandsFilterForIS; }

  ActivationFlags & modifiableActivationFlags() { return _activationFlags; }
  const ActivationFlags & activationFlags() const { return _activationFlags; }

  bool isCommandPricingRq() const;

  std::vector<BrandCode>& validBrands() { return _validBrands; }
  const std::vector<BrandCode>& validBrands() const { return _validBrands; }

  // in half RT Pricing artificial legs are created and then, later
  // multiple fares per itin are rejoined together based on brandIndex
  // in IBF we use brandCode instead of brandIndex so here,
  // an artificial brandIndex has to be used.
  const std::map<BrandCode, uint16_t>& getBrandOrder() { return _brandOrder; }
  void computeBrandOrder();

  BFErrorCode& bfErrorCode() { return _bfErrorCode; }
  const BFErrorCode bfErrorCode() const { return _bfErrorCode; }

  BaggagePolicy& mutableBaggagePolicy() { return *_baggagePolicy; }
  const BaggagePolicy& getBaggagePolicy() const { return *_baggagePolicy; }

  const flexFares::TotalAttrs& getFlexFaresTotalAttrs() const { return _flexFaresTotalAttrs; }
  flexFares::TotalAttrs& getMutableFlexFaresTotalAttrs() { return _flexFaresTotalAttrs; }

  void setValidatingCxrGsaApplicable(bool validatingCxrGsaApplicable)
  {
    _validatingCxrGsaApplicable = validatingCxrGsaApplicable;
  }

  bool isValidatingCxrGsaApplicable() const { return _validatingCxrGsaApplicable; }

  void setTraditionalValidatingCxr(bool traditionalValidatingCxr)
  {
    _traditionalValidatingCxr = traditionalValidatingCxr;
  }

  bool useTraditionalValidatingCxr() const { return _traditionalValidatingCxr; }

  void setSkipGsa(bool skipGsa) { _skipGsa = skipGsa; }

  bool isSkipGsa() const { return _skipGsa; }

  void setSkipNeutral(bool skipNeutral) { _skipNeutral = skipNeutral; }

  bool isSkipNeutral() const { return _skipNeutral; }

  void setOnlyCheckAgreementExistence(bool onlyCheckAgreementExistence)
  {
    _onlyCheckAgreementExistence = onlyCheckAgreementExistence;
  }

  bool onlyCheckAgreementExistence() const { return _onlyCheckAgreementExistence; }

  void setCosExceptionFixEnabled(bool v) { _cosExceptionFixEnabled = v; }
  bool isCosExceptionFixEnabled() const { return _cosExceptionFixEnabled; }

  void setValCxrCode(CarrierCode valCxrCode) { _currentValCxrCode = valCxrCode; }
  CarrierCode validatingCxr() const { return _currentValCxrCode; }

  tax::AtpcoTaxesActivationStatus& atpcoTaxesActivationStatus() { return _atpcoTaxesActivationStatus; }
  const tax::AtpcoTaxesActivationStatus& atpcoTaxesActivationStatus() const { return _atpcoTaxesActivationStatus; }

  void setIataFareSelectionApplicable(bool isIataFareSelectionApplicable)
  {
    _iataFareSelectionApplicable = isIataFareSelectionApplicable;
  }

  bool isIataFareSelectionApplicable() const
    { return _iataFareSelectionApplicable && (TrxType::IS_TRX != _trxType); }

  bool isWpncCloningEnabled() const;
  bool isProcess2CC() const;
  bool isBaggageRequest();

  const std::string& taxRequestToBeReturnedAsResponse() const { return _taxRequestToBeReturned; }
  std::string& taxRequestToBeReturnedAsResponse() { return _taxRequestToBeReturned; }

  bool isObFeeApplied() const;
  bool isAnyNonCreditCardFOP() const;
  virtual bool isSingleMatch() const { return false; }

  const AmVatTaxRatesOnCharges* getAmVatTaxRatesOnCharges() const
  { return _amVatTaxRatesOnCharges; }

  void loadAmVatTaxRatesOnCharges();

  void setFootNotePrevalidationEnabled(bool value);
  bool isFootNotePrevalidationEnabled() const { return _footNotePrevalidationEnabled; }
  virtual void setupFootNotePrevalidation();
  bool isFootNotePrevalidationAllowed() const { return _footNotePrevalidationAllowed; }

  bool isRequestedFareBasisInfoUseful() const;
  bool areFareMarketsFailingOnRules() const;
  void checkFareMarketsCoverTravelSegs();

  bool isRfbListOfSolutionEnabled() const { return _isRfbListOfSolutionEnabled; }
  void setRfblistOfSolution(const bool rfbList = true) { _isRfbListOfSolutionEnabled = rfbList; }
  bool hasNoEmptyRfbOrAdditionalAttrOnEachSeg() const;

  bool isLowestFareOverride() const;

  bool isAssignLegsRequired() const { return _assignLegsRequired; }
  void setAssignLegsRequired(bool assignLegsRequired) { _assignLegsRequired = assignLegsRequired; }
  void assignPriceDeviator(PriceDeviatorPtr deviator);
  void configurePriceDeviator() const;
  const Percent* getDiscountPercentageNew(const FareMarket& fareMarket) const;
  const DiscountAmount* getDiscountAmountNew(const FareMarket& fareMarket) const;
  const Percent* getDiscountPercentageNew(const PricingUnit& pricingUnit) const;
  const DiscountAmount* getDiscountAmountNew(const PricingUnit& pricingUnit) const;
  bool hasPriceDynamicallyDeviated() const;

  void addParentTaxes(const std::set<PreviousTicketTaxInfo>& newParentTaxes)
  {
    _parentTaxes.insert(newParentTaxes.begin(), newParentTaxes.end());
  }

  const std::set<PreviousTicketTaxInfo>& getParentTaxes() const
  {
    return _parentTaxes;
  }

  bool isLockingNeededInShoppingPQ() const { return isOnlineShoppingPQProcessing(); }
  bool isOnlineShoppingPQProcessing() const { return _isOnlineShoppingPQProcessing; }
  void setOnlineShoppingPQProcessing(bool isOnlineProcessing = true)
  {
    _isOnlineShoppingPQProcessing = isOnlineProcessing;
  }

  bool isGNRAllowed();

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  SpecifiedFareCache* specifiedFareCache() const { return _specFareCache.get(); }

#endif

  const StateCode& residencyState() const { return _residencyState; }
  StateCode& residencyState() { return _residencyState; }

protected:
  BaggagePolicy* _baggagePolicy;
  skipper::FareComponentShoppingContextsForSegments _fareComponentShoppingContexts;
  std::vector<bool> _fixedLegs;

private:
  bool isNormalWpPricing() const;

  AltDatePairs _altDatePairs;
  DurationAltDatePairs _durationAltDatePairs;
  MoneyAmount _altDateCutOffNucThreshold = -1.0;
  bool _altDates = false; // alternate date entry
  bool _isOnlineShoppingPQProcessing = false;
  bool _mipDomestic = false;
  bool _visMip = false;
  bool _validatingCxrGsaApplicable = false;
  bool _traditionalValidatingCxr = false;
  bool _skipGsa = false;
  bool _skipNeutral = false;
  bool _GNRAllowed = false;
  std::once_flag _GNRFlag;

  uint64_t _mainDuration = 0;
  bool _onlyCheckAgreementExistence = false;
  bool _cosExceptionFixEnabled = false;
  bool _iataFareSelectionApplicable = false;
  bool _isRfbListOfSolutionEnabled = false;

  RepriceCache _repriceCache;

  boost::mutex _custActvCntrlmutex;

  TrxType _trxType = TrxType::PRICING_TRX;
  AltTrxType _altTrxType = AltTrxType::WP;

  bool _fxCnException = false; // whether FareX Chinese Exception applies - set in FVO
  ItinBoolMap _fxCnExceptionPerItin; // Extended version of the above for the SOLO Carnival project.
  bool _noPNRPricing = false;

  std::vector<CabinType> _legPreferredCabinClass;

  std::atomic_bool _cutOffReached{false};

  std::string _token;

  std::pair<const DateTime*, const DateTime*> _itinsTimeSpan;
  bool _showBaggageTravelIndex = false;
  bool _delayXpn = false;
  bool _isFlexFarePhase1 = false;
  bool _isFlexFare = false;
  bool _isMainFare = false;
  uint16_t _projectActivationFlag = 1;

  uint32_t _nextTrxTaskHighLevelSeqNum = 0; // is related to TseCallableTrxTask

  MultiTicketMap _multiTicketMap;

  BrandedMarketMap _brandedMarketMap;
  std::vector<QualifiedBrand> _brandProgramVec;
  // convenience object for TN Shopping calculations on _brandProgramVec
  skipper::TrxGeometryCalculator* _trxGeometryCalculator = nullptr;
  PbbProcessing _pbbRequest = PbbProcessing::NOT_PBB_RQ;
  CarrierCode _currentValCxrCode;
  std::vector<BrandCode> _validBrands;
  std::map<BrandCode, uint16_t> _brandOrder;
  std::vector<uint16_t> _flpVector; // Contains Family Logic Parameter (FLP) values
  BFErrorCode _bfErrorCode = BFErrorCode::NO_ERROR;

  CountrySettlementPlanInfo* _countrySettlementPlanInfo = nullptr;
  std::vector<CountrySettlementPlanInfo*> _countrySettlementPlanInfos;
  mutable std::mutex _valCxrMutex;
  ValidatingCxrGSADataHashMap _valCxrGsaDataHashMap;
  HashSpValidatingCxrGSADataMap _valCxrGsaDataPerSpHashMap;

  flexFares::TotalAttrs _flexFaresTotalAttrs;
  TnShoppingBrandingMode _tnShoppingBrandingMode = TnShoppingBrandingMode::NO_BRANDS;
  size_t _tnShoppingNumberOfBrands = 0;
  std::map<size_t, CabinType> _cabinsForLegs;

  BrandCodeSet _brandsFilterForIS;

  ActivationFlags _activationFlags;

  tax::AtpcoTaxesActivationStatus _atpcoTaxesActivationStatus;
  std::string _taxRequestToBeReturned;
  AmVatTaxRatesOnCharges* _amVatTaxRatesOnCharges = nullptr;

  bool _isSoftPassDisabled = false;
  bool _assignLegsRequired = false;

  PriceDeviatorWrapper _priceDeviator{_request};

  std::set<PreviousTicketTaxInfo> _parentTaxes;
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  std::unique_ptr<SpecifiedFareCache> _specFareCache;
#endif

  // Spanish fields
  StateCode _residencyState;
};
} // tse namespace
