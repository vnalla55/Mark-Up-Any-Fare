//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "Common/OcTypes.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "ServiceFees/MerchCarrierStrategy.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
class PricingTrx;
class FarePath;
class SubCodeInfo;
class OptionalServicesInfo;
class ServiceFeeUtil;
class TravelSeg;
class SvcFeesAccountCodeValidator;
class SvcFeesCurrencyInfo;
class OCFees;
class LocKey;
class Loc;
class SvcFeesTktDesigValidator;
class SvcFeesResBkgDesigInfo;
class Diag877Collector;
class CarrierFlightSeg;

struct OcValidationContext
{
  OcValidationContext(PricingTrx& trx, Itin& itin, PaxType& pt, FarePath* fp = nullptr)
    : trx(trx), itin(itin), paxType(pt), fp(fp)
  {}

  PricingTrx& trx;
  Itin& itin;
  PaxType& paxType;
  FarePath* fp;
};

class OptionalServicesValidator
{
  friend class OptionalServicesValidatorTest;

public:
  typedef std::set<PaxTypeFare*>::const_iterator FareIterator;

  static constexpr Indicator T198_MKT_OPER_IND_OPER = 'O';

  OptionalServicesValidator(
      const OcValidationContext& ctx,
      const std::vector<TravelSeg*>::const_iterator segI,
      const std::vector<TravelSeg*>::const_iterator segIE,
      const std::vector<TravelSeg*>::const_iterator endOfJourney,
      const Ts2ss& ts2ss,
      bool isInternational,
      bool isOneCarrier,
      bool isMarketingCxr,
      Diag877Collector* diag);
  OptionalServicesValidator(const OptionalServicesValidator&) = delete;
  void operator=(const OptionalServicesValidator&) = delete;
  virtual ~OptionalServicesValidator() = default;

  virtual bool validate(OCFees& ocFees, bool stopMatch = false) const;
  void setMerchStrategy(MerchCarrierStrategy* merchCrxStrategy)
  {
    _merchCrxStrategy = merchCrxStrategy;
  }

protected:
  static constexpr Indicator T183_SCURITY_PRIVATE = 'P';
  static constexpr Indicator T198_MKT_OPER_IND_MKT = 'M';
  static constexpr Indicator SEC_POR_IND_SECTOR = 'S';
  static constexpr Indicator SEC_POR_IND_PORTION = 'P';
  static constexpr Indicator SEC_POR_IND_JOURNEY = 'J';
  static constexpr Indicator CHAR_BLANK = ' ';
  static constexpr Indicator CHAR_BLANK2 = '\0';
  static constexpr Indicator FTW_FROM = '1';
  static constexpr Indicator FTW_TO = '2';
  static constexpr Indicator FTW_WITHIN = 'W';
  static constexpr Indicator FTW_RULE_BUSTER_A = 'A';
  static constexpr Indicator FTW_RULE_BUSTER_B = 'B';
  static constexpr Indicator SERVICE_NOT_AVAILABLE = 'X';
  static constexpr Indicator SERVICE_FREE_NO_EMD_ISSUED = 'F';
  static constexpr Indicator SERVICE_FREE_EMD_ISSUED = 'E';
  static constexpr Indicator SERVICE_FREE_NO_BOOK_NO_EMD = 'G';
  static constexpr Indicator SERVICE_FREE_NO_BOOK_EMD_ISSUED = 'H';
  static constexpr Indicator SCD_CNX_POINT = 'C';
  static constexpr Indicator SCD_NO_FARE_BREAK = 'F';
  static constexpr Indicator SCD_NO_FARE_BREAK_OR_STOPOVER = 'P';
  static constexpr Indicator SCD_STOPOVER = 'S';
  static constexpr Indicator SCD_STOPOVER_WITH_GEO = 'T';
  static constexpr Indicator FARE_IND_19_22 = '1';
  static constexpr Indicator FARE_IND_25 = '2';
  static constexpr Indicator FARE_IND_35 = '3';
  static const ServiceRuleTariffInd RULE_TARIFF_IND_PUBLIC;
  static const ServiceRuleTariffInd RULE_TARIFF_IND_PRIVATE;

  // Overrides to database or to other services
  virtual const std::vector<OptionalServicesInfo*>&
  getOptionalServicesInfo(const SubCodeInfo& subCode) const;
  virtual bool
  svcFeesAccountCodeValidate(const SvcFeesAccountCodeValidator& validator, int itemNo) const;
  virtual bool
  svcFeesTktDesignatorValidate(const SvcFeesTktDesigValidator& validator, int itemNo) const;
  virtual bool
  inputPtcValidate(const ServiceFeeUtil& util, const OptionalServicesInfo& optSrvInfo) const;
  virtual bool isInLoc(const VendorCode& vendor,
                       const LocKey& locKey,
                       const Loc& loc,
                       CarrierCode carrier = EMPTY_STRING()) const;
  virtual bool isInZone(const VendorCode& vendor,
                        const LocCode& zone,
                        const Loc& loc,
                        CarrierCode carrier) const;
  virtual const std::vector<SvcFeesCurrencyInfo*>&
  getSvcFeesCurrency(const OptionalServicesInfo& optSrvInfo) const;
  virtual const std::vector<SvcFeesResBkgDesigInfo*>&
  getRBDInfo(const VendorCode& vendor, int itemNo) const;
  virtual const std::vector<SvcFeesCxrResultingFCLInfo*>&
  getResFCInfo(const VendorCode& vendor, int itemNo) const;
  virtual const TaxCarrierFlightInfo*
  getTaxCarrierFlight(const VendorCode& vendor, uint32_t itemNo) const;
  virtual bool getFeeRounding_old(const CurrencyCode& currencyCode,
                              RoundingFactor& roundingFactor,
                              CurrencyNoDec& roundingNoDec,
                              RoundingRule& roundingRule) const;

  virtual StatusS7Validation validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  bool checkTravelDate(const OptionalServicesInfo& optSrvInfo) const;
  bool checkInputTravelDate(const OptionalServicesInfo& optSrvInfo) const;
  bool checkAccountCodes(uint32_t itemNo) const;
  bool checkInputTicketDesignator(uint32_t itemNo) const;
  virtual bool checkOutputTicketDesignator(const OptionalServicesInfo& optSrvInfo) const;

  bool checkOutputTicketDesignatorForSegmentsRange(
      const OptionalServicesInfo& optSrvInfo,
      const std::vector<TravelSeg*>::const_iterator segBegin,
      const std::vector<TravelSeg*>::const_iterator segEnd) const;

  bool checkInputPtc(const OptionalServicesInfo& optSrvInfo) const;
  bool checkGeoFtwInd(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual bool
  checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual bool checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                                      std::vector<TravelSeg*>& passedLoc3Dest,
                                      OCFees& ocFees) const;
  virtual bool checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                   const std::vector<TravelSeg*>& passedLoc3Dest,
                                   OCFees& ocFees) const;
  virtual bool retrieveSpecifiedFee(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  bool checkFeeApplication(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual bool checkInterlineIndicator(const OptionalServicesInfo& optSrvInfo) const;
  bool validateSfcLocation(const VendorCode& vendor, const LocKey& locKey, const Loc& loc) const;
  virtual const Loc& getLocForSfcValidation() const;
  virtual bool validateLocation(const VendorCode& vendor,
                                const LocKey& locKey,
                                const Loc& loc,
                                const LocCode& zone,
                                bool emptyRet,
                                CarrierCode carrier,
                                LocCode* matchLoc = nullptr) const;
  bool validateFrom(const OptionalServicesInfo& optSrvInfo,
                    const Loc& orig,
                    const Loc& dest,
                    LocCode* loc1 = nullptr,
                    LocCode* loc2 = nullptr) const;
  bool validateBetween(const OptionalServicesInfo& optSrvInfo,
                       const Loc& orig,
                       const Loc& dest,
                       LocCode* loc1 = nullptr,
                       LocCode* loc2 = nullptr) const;
  bool validateWithin(const OptionalServicesInfo& optSrvInfo,
                      LocCode* loc1 = nullptr,
                      LocCode* loc2 = nullptr) const;
  bool validateViaWithLoc2(const OptionalServicesInfo& optSrvInfo,
                           std::vector<TravelSeg*>& passedLoc3Dest) const;
  bool validateViaWithStopover(const OptionalServicesInfo& optSrvInfo) const;
  bool validateCnxOrStopover(const OptionalServicesInfo& optSrvInfo,
                             const std::vector<TravelSeg*>& passedLoc3Dest,
                             bool validateCnx) const;
  bool validateNoFareBreak(const OptionalServicesInfo& optSrvInfo,
                           const std::vector<TravelSeg*>& passedLoc3Dest) const;
  bool findSegmentInVector(const std::vector<TravelSeg*>& vec, const TravelSeg* segment) const;
  void getFareBreaks(std::vector<TravelSeg*>& fareBreaks) const;
  virtual bool isStopover(const OptionalServicesInfo& optSrvInfo,
                          const TravelSeg* seg,
                          const TravelSeg* next) const;
  virtual std::vector<TravelSeg*>::const_iterator
  findFirstStopover(const OptionalServicesInfo& optSrvInfo) const;
  void setOCFees(const OptionalServicesInfo& optSrvInfo,
                 OCFees& ocFees,
                 SvcFeesCurrencyInfo& svcInfo) const;
  void setSrvInfo(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  void checkDiagS7ForDetail(const OptionalServicesInfo* optSrvInfo) const;
  void printDiagS7Info(const OptionalServicesInfo* optSrvInfo,
                       const OCFees& ocFees,
                       const StatusS7Validation& rc,
                       const bool markAsSelected = false) const;
  void printStopAtFirstMatchMsg() const;
  void printDiagS7NotFound(const SubCodeInfo& subcode) const;
  virtual void printT183TableBufferedData() const;
  bool isDiagSequenceMatch(const OptionalServicesInfo& info) const;
  bool isDDInfo() const;
  bool isDDPass() const;
  virtual bool checkSecurity(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  void displaySvcFeeCurInfoHeader(const int itemNo) const;
  void displaySvcFeeCurInfoDetail(SvcFeesCurrencyInfo& svcInfo, bool status) const;
  void displaySvcFeeCurInfoHeaderAndDetail(SvcFeesCurrencyInfo& svcInfo,
                                           bool status,
                                           bool isNullHeader) const;
  virtual bool skipUpgradeCheck(const OptionalServicesInfo& optSrvInfo, const OCFees& ocFees) const;
  bool skipUpgradeCheckCommon(const OptionalServicesInfo& optSrvInfo, const OCFees& ocFees) const;
  bool skipUpgradeForUpGroupCode(const SubCodeInfo* s5) const;
  bool checkUpgradeT198(const OptionalServicesInfo& optSrvInfo, const OCFees& ocFees) const;
  bool checkSectorPortionInd(const Indicator sectorPortionInd) const;
  bool checkSectorPortionIndForSectorOnP(const Indicator sectorPortionInd) const;
  bool checkSectorPortionIndForBG(const OptionalServicesInfo& optSrvInfo) const;
  bool checkAllGeoForBlank(const OptionalServicesInfo& optSrvInfo) const;
  virtual bool isRBDValid(AirSeg* seg,
                          const std::vector<SvcFeesResBkgDesigInfo*>& validRBDInfoForSeg,
                          OCFees& ocFees) const;
  bool checkRBD(const VendorCode& vendor,
                const uint32_t serviceFeesResBkgDesigTblItemNo,
                OCFees& ocFees) const;
  CabinType mapS7CabinType(const Indicator cabin) const;
  virtual bool checkCabinData(AirSeg& seg,
                              const CabinType& cabin,
                              const CarrierCode& carrier,
                              OCFees& ocFees) const;
  bool checkCabin(const Indicator cabin, const CarrierCode& carrier, OCFees& ocFees) const;
  bool checkTourCode(const OptionalServicesInfo& optSrvInfo) const;
  bool checkStartStopTime(const OptionalServicesInfo& optSrvInfo,
                          bool& skipDOWCheck,
                          OCFees& ocFees) const;
  uint16_t convertMinutesSinceMidnightToActualTime(const std::string& strTimeVal) const;
  virtual bool
  validateDOWAndStartAndStopTime(const OptionalServicesInfo& info, OCFees& ocFees) const;
  virtual bool validateStartAndStopTime(const OptionalServicesInfo& info, OCFees& ocFees) const;
  virtual bool validateStartAndStopTime(OCFees& ocFees) const;
  virtual bool checkDOWRangeAndTime(const OptionalServicesInfo& info, OCFees& ocFees) const;
  virtual bool checkDOWRange(const OptionalServicesInfo& info) const;
  virtual bool checkDOW(const OptionalServicesInfo& optSrvInfo) const;
  void displayStartStopTimeOrDOWFailDetail(bool isErrorFromDOW) const;
  virtual bool checkEquipmentType(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual bool checkAdvPur(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  bool isAdvPurUnitNHDM(const OptionalServicesInfo& info) const;
  std::string getAdvPurPeriod(const OptionalServicesInfo& info,
                              DateTime& calcTime,
                              uint32_t& days,
                              int& advPinDays) const;
  int getDayOfWeek(const std::string& dayOfWeekName) const;
  virtual void getLocalTime(DateTime& calcTime) const;
  virtual short getTimeDiff(const DateTime& time) const;
  virtual bool getUTCOffsetDifference(const Loc& loc1,
                                      const Loc& loc2,
                                      short& utcoffset,
                                      const DateTime& time) const;
  virtual const Loc* getLocation(const LocCode& locCode, const DateTime& refTime) const;
  virtual bool
  setPBDate(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees, const DateTime& pbDate) const;
  StatusS7Validation
  checkServiceNotAvailNoCharge(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual StatusT171 isValidResultingFareClass(const PaxTypeFare* ptf,
                                               SvcFeesCxrResultingFCLInfo& fclInfo,
                                               OCFees& ocFees) const;
  virtual StatusT171 isValidFareClassFareType(const PaxTypeFare* ptf,
                                              SvcFeesCxrResultingFCLInfo& fclInfo,
                                              OCFees& ocFees) const;
  bool isValidFareClass(const PaxTypeFare* ptf,
                        SvcFeesCxrResultingFCLInfo& fclInfo) const;
  bool isValidFareType(const PaxTypeFare* ptf,
                       SvcFeesCxrResultingFCLInfo& fclInfo) const;
  bool checkResultingFareClass(const VendorCode& vendor,
                               const uint32_t serviceFeesCxrResultingFclTblItemNo,
                               OCFees& ocFees) const;
  bool checkMileageFee(const OptionalServicesInfo& info) const;
  virtual bool isValidRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd,
                                    TariffCategory tariffCategory) const;
  bool checkRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd) const;
  virtual bool
  matchRuleTariff(const uint16_t& ruleTariff, const PaxTypeFare& ptf, OCFees& ocFees) const;
  virtual bool checkRuleTariff(const uint16_t& ruleTariff, OCFees& ocFees) const;
  virtual bool checkRule(const RuleNumber& rule, OCFees& ocFees) const;
  virtual bool checkFareInd(Indicator fareInd) const;
  virtual bool checkFareInd(Indicator fareInd, const PaxTypeFare& ptf) const;
  virtual bool
  checkCarrierFlightApplT186(const VendorCode& vendor, const uint32_t itemNo, OCFees& ocFees) const;
  virtual StatusT186 isValidCarrierFlight(const AirSeg& air, CarrierFlightSeg& t186) const;
  virtual StatusT186 isValidCarrierFlightNumber(const AirSeg& air, CarrierFlightSeg& t186) const;
  bool checkCollectSubtractFee(const OptionalServicesInfo& info) const;
  bool checkNetSellFeeAmount(const OptionalServicesInfo& info) const;
  MoneyAmount halfOfAmount_old(const OCFees& ocFees) const; // To remove with ocFeesAmountRoundingRefactoring
  MoneyAmount halfOfAmount(const OCFees& ocFees) const;
  bool isDOWstringValid(const std::string& dow) const;
  virtual bool checkAdvPurchaseTktInd(const OptionalServicesInfo& optSrvInfo) const;
  void setPortionBG(bool i = false) const;
  const bool getPortionBG() const { return _portionBG; }
  virtual void setCurrencyNoDec(OCFees& fee) const;
  virtual bool shouldProcessAdvPur(const OptionalServicesInfo& info) const;
  void addPadisCodesToOCFees(OCFees& ocFees, const OptionalServicesInfo& s7) const;
  virtual void printResultingFareClassInfo(const PaxTypeFare& ptf,
                                           const SvcFeesCxrResultingFCLInfo& info,
                                           StatusT171 status) const;

  bool isValidFareClassCarrier(const PaxTypeFare* ptf,
                               const SvcFeesCxrResultingFCLInfo& fclInfo) const;

protected:
  PricingTrx& _trx;
  Itin& _itin;
  PaxType& _paxType;
  FarePath* _farePath;
  std::vector<TravelSeg*>::const_iterator _segI;
  std::vector<TravelSeg*>::const_iterator _segIE;
  const std::vector<TravelSeg*>::const_iterator _endOfJourney;
  const Ts2ss& _ts2ss;
  std::set<PaxTypeFare*> _processedFares;
  bool _isIntrnl;
  bool _isOneCarrier;
  bool _isMarketingCxr;
  mutable Diag877Collector* _diag;
  mutable bool _portionBG;
  std::vector<AirSeg*> _airSegs;
  MerchCarrierStrategy* _merchCrxStrategy;
  MultipleSegmentStrategy _multipleStrategy;
  GeoTravelType _geoTravelType;
};

struct PadisComparator
    : std::binary_function<const SvcFeesResBkgDesigInfo*, const SvcFeesResBkgDesigInfo*, bool>
{
  bool
  operator()(const SvcFeesResBkgDesigInfo* padis_1, const SvcFeesResBkgDesigInfo* padis_2) const;

private:
  void fill(std::set<BookingCode>& bookingCode, const SvcFeesResBkgDesigInfo* padis) const;
};
} // tse namespace

