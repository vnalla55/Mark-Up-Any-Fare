//-------------------------------------------------------------------
//
//  File:        DifferentilaValidator.h
//  Created:     May 27, 2004
//  Authors:     Sergey Romanchenko, Alexander Zagrebin
//
//  Description:
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/PaxTypeFare.h"

#include <map>
#include <string>
#include <vector>

namespace tse
{
using TravelSegPtrVec = std::vector<tse::TravelSeg*>;
using TravelSegPtrVecI = std::vector<tse::TravelSeg*>::iterator;
using TravelSegPtrVecIC = std::vector<tse::TravelSeg*>::const_iterator;

using SegmentStatusVec = std::vector<PaxTypeFare::SegmentStatus>;
using SegmentStatusVecI = std::vector<PaxTypeFare::SegmentStatus>::iterator;
using SegmentStatusVecCI = std::vector<PaxTypeFare::SegmentStatus>::const_iterator;

using PaxTypeFarePtrVec = std::vector<PaxTypeFare*>;
using PaxTypeFarePtrVecI = std::vector<PaxTypeFare*>::iterator;
using PaxTypeFarePtrVecIC = std::vector<PaxTypeFare*>::const_iterator;

using DifferentialsPtrList = std::vector<Differentials*>;
using DifferentialsPtrListI = std::vector<Differentials*>::iterator;
using DifferentialsPtrListIC = std::vector<Differentials*>::const_iterator;

using DiffDataFMI = std::vector<FareMarket*>::iterator;

using DiffDataCxrI = std::vector<CarrierCode>::iterator;

// ClassOfService vector of ClassOfServices vectors for each TravelSeg in the FareMarket

using ClassOfServiceV = std::vector<ClassOfService*>;
using ClassOfServicePV = std::vector<ClassOfService*>*;
using ClassOfServicePrtVec = std::vector<std::vector<ClassOfService*>*>;
using ClassOfServicePrtVecI = std::vector<std::vector<ClassOfService*>*>::iterator;
using ClassOfServicePrtVecIC = std::vector<std::vector<ClassOfService*>*>::const_iterator;

using COSInnerPtrVecIC = std::vector<ClassOfService*>::const_iterator;

using DifferentialDataPtrVec = std::vector<DifferentialData*>;
using DifferentialDataPtrVecI = std::vector<DifferentialData*>::iterator;
using DifferentialDataPtrVecIC = std::vector<DifferentialData*>::const_iterator;

using PaxTypeBucketVec = std::vector<PaxTypeBucket>;
using PaxTypeBucketVecI = PaxTypeBucketVec::iterator;

using DiagParamMapVecI = std::map<std::string, std::string>::iterator;
using DiagParamMapVecIC = std::map<std::string, std::string>::const_iterator;

// FORWARD DECLARATIONS
class PricingTrx;
class FareBookingCodeValidator;
class FareMarket;
class PaxTypeFare;
class PaxType;
class AirSeg;
class Itin;
class LocKey;
class Loc;
class Differentials;
class RepricingTrx;
class FarePath;
class PricingUnit;
class FareUsage;

class DifferentialValidator
{
  class FareSelectionData
  {
  protected:
    FareClassCode _fareClassHigh;
    MoneyAmount _amountFareClassHigh = 0;
    FareClassCode _fareClassLow;
    MoneyAmount _amountFareClassLow = 0;
    PaxTypeFare* _fareHigh = nullptr;
    PaxTypeFare* _fareLow = nullptr;
    FareClassCode _fareClassHighOW;
    MoneyAmount _amountFareClassHighOW = 0;
    FareClassCode _fareClassLowOW;
    MoneyAmount _amountFareClassLowOW = 0;
    PaxTypeFare* _fareHighOW = nullptr;
    PaxTypeFare* _fareLowOW = nullptr;
    bool _foundHigh = false;
    bool _foundLow = false;
    CarrierCode _carrier;

  public:
    PaxTypeFare*& fareLow(void) { return _fareLow; }
    const PaxTypeFare* fareLow(void) const { return _fareLow; }

    PaxTypeFare*& fareHigh(void) { return _fareHigh; }
    const PaxTypeFare* fareHigh(void) const { return _fareHigh; }

    FareClassCode& fareClassHigh() { return _fareClassHigh; }
    const FareClassCode& fareClassHigh() const { return _fareClassHigh; }

    MoneyAmount& amountFareClassHigh() { return _amountFareClassHigh; }
    const MoneyAmount& amountFareClassHigh() const { return _amountFareClassHigh; }

    FareClassCode& fareClassLow() { return _fareClassLow; }
    const FareClassCode& fareClassLow() const { return _fareClassLow; }

    MoneyAmount& amountFareClassLow() { return _amountFareClassLow; }
    const MoneyAmount& amountFareClassLow() const { return _amountFareClassLow; }

    PaxTypeFare*& fareLowOW(void) { return _fareLowOW; }
    const PaxTypeFare* fareLowOW(void) const { return _fareLowOW; }

    PaxTypeFare*& fareHighOW(void) { return _fareHighOW; }
    const PaxTypeFare* fareHighOW(void) const { return _fareHighOW; }

    FareClassCode& fareClassHighOW() { return _fareClassHighOW; }
    const FareClassCode& fareClassHighOW() const { return _fareClassHighOW; }

    MoneyAmount& amountFareClassHighOW() { return _amountFareClassHighOW; }
    const MoneyAmount& amountFareClassHighOW() const { return _amountFareClassHighOW; }

    FareClassCode& fareClassLowOW() { return _fareClassLowOW; }
    const FareClassCode& fareClassLowOW() const { return _fareClassLowOW; }

    MoneyAmount& amountFareClassLowOW() { return _amountFareClassLowOW; }
    const MoneyAmount& amountFareClassLowOW() const { return _amountFareClassLowOW; }

    bool& foundHigh() { return _foundHigh; }
    const bool& foundHigh() const { return _foundHigh; }

    bool& foundLow() { return _foundLow; }
    const bool& foundLow() const { return _foundLow; }

    CarrierCode& diffCxr() { return _carrier; }
    const CarrierCode& diffCxr() const { return _carrier; }
  };

  class FareSelectionDataHelper
  {
  private:
    std::map<size_t, FareSelectionData*> _fareSelectionMap;
    CabinType _failedFareCabin;
    CabinType _throughFTD;
    CabinType _throughRealFTD;
    GlobalDirection _globalDirection = GlobalDirection::NO_DIR;
    bool _slideAllowed = true;
    bool _ilow = false;
    Indicator _prPricePr;
    CarrierCode _govCXR;
    PaxTypeFarePtrVec _paxTypeFareVec;
    DifferentialData& _diffDP;

  public:
    FareSelectionDataHelper(PricingTrx& trx, DifferentialData& diffDP);

    FareSelectionDataHelper(const FareSelectionDataHelper&) = delete;
    FareSelectionDataHelper& operator=(const FareSelectionDataHelper&) = delete;

    FareSelectionData* operator[](const CabinType& c);

    void getValue(FareSelectionData& fsd) const;
    void saveTempFSData(FareSelectionData& fsd) const;
    bool foundLow() const;
    bool foundHigh() const;
    bool checkLowHigh(const size_t& posL_or_HF, const size_t& posThrough) const;
    bool matchSlide(const CabinType& c) const;

    bool& slideAllowed() { return _slideAllowed; }
    const bool& slideAllowed() const { return _slideAllowed; }

    CabinType& failedFareCabin() { return _failedFareCabin; }
    const CabinType& failedFareCabin() const { return _failedFareCabin; }

    CabinType& throughFTD() { return _throughFTD; }
    const CabinType& throughFTD() const { return _throughFTD; }

    CabinType& throughRealFTD() { return _throughRealFTD; }
    const CabinType& throughRealFTD() const { return _throughRealFTD; }

    GlobalDirection& globalDirection() { return _globalDirection; }
    const GlobalDirection& globalDirection() const { return _globalDirection; }

    Indicator& prPricePr() { return _prPricePr; }
    const Indicator& prPricePr() const { return _prPricePr; }

    bool& ilow() { return _ilow; }
    const bool& ilow() const { return _ilow; }

    CarrierCode& govCXR() { return _govCXR; }
    const CarrierCode& govCXR() const { return _govCXR; }

    PaxTypeFarePtrVec& paxTypeFareVec() { return _paxTypeFareVec; }
    const PaxTypeFarePtrVec& paxTypeFareVec() const { return _paxTypeFareVec; }

    DifferentialData& diffDP() { return _diffDP; }
    const DifferentialData& diffDP() const { return _diffDP; }

    std::map<size_t, FareSelectionData*>& fareSelectionMap() { return _fareSelectionMap; }
  };

  class LowerSegmentNumber
  {
  protected:
    const Itin* _itin = nullptr;

  public:
    LowerSegmentNumber(const Itin* itin) : _itin(itin) {}

    bool operator()(const DifferentialData* item1, const DifferentialData* item2);
  };

  friend class DifferentialValidatorTest;

public:
  DifferentialValidator() = default;
  virtual ~DifferentialValidator() = default;

  DifferentialValidator(const DifferentialValidator&) = delete;
  DifferentialValidator& operator=(const DifferentialValidator&) = delete;

  std::vector<DifferentialData*>& differential(); // throw (std::string &);

  enum DifferFailBkgCodeEnum
  { CABIN_LOWER = 1,
    CABIN_EQUAL,
    CABIN_HIGHER };

  enum TypeOfCabin
  { CABIN_PREMIUM_FIRST,
    CABIN_FIRST,
    CABIN_PREMIUM_BUSINESS,
    CABIN_BUSINESS,
    CABIN_PREMIUM_ECONOMY,
    CABIN_ECONOMY,
    CABIN_FAMILY_FIRST,
    CABIN_FAMILY_BUSINESS,
    CABIN_FAMILY_ECONOMY };

  enum StatusReturnType
  { PASS = 1,
    FAIL };

  enum Direction
  {
    DF_FROM = 'F',
    DF_BETWEEN = 'B',
    DF_WITHIN = 'W',
    DF_ORIGIN = 'O',
  };

  enum FareDest
  { LOW,
    HIGH };

  enum DifferentialTypes
  { SAME_TYPE = 'S',
    LOW_TYPE = 'A', // used to be an 'L'
    HIGH_TYPE = 'H',
    NOT_FOUND_IN_TABLE = ' ',
    NOT_PERMITTED = 'N',
    CAT23_BYTE10_VALUE_R = 'R',
    CAT23_RESTRICTED = 'C'};

  enum TripTypes
  {
    ROUND_TRIP = 'R',
    ONE_WAY_TRIP = 'O',
  };

  enum PrimePricing
  {
    LOWEST_FARE = 'L',
    PREFER_CXR_FARE = 'C',
  };

  enum Sectors
  { REGULAR_SECTOR = 3, // These numbers shouldn't be changed, since they also indicate
    CONSEC_SECTOR = 4, // what is the length of the corresponding tags...
  };

  enum ReturnValues
  {
    FAILED,
    SUCCEEDED,
    FATAL_ERROR,
    STOP_PROCESSING,
    END_OF_SEQUENCE,
  };

  enum DifferentialTag
  { NUMBER = '1',
    START_LETTER = 'A',
    DEFAULT_CABIN = 'Z',
    BLANK = ' ' };

  enum FlightApplication
  { NONSTOP = 'N',
    SAMECARRIER = 'S', // it's referenced in the FRD as "A"
    LAST_FIRST = 'L' };

  enum AdjustSectors
  {
    PREVIOUS = -1,
    NEXT = 1,
  };

  bool validate(PricingTrx& trx,
                FarePath& fp,
                PricingUnit& pu,
                FareUsage& fu,
                const Itin& itin,
                const PaxType& reqPaxType,
                FareBookingCodeValidator& fbcv);

  bool validateDiff();

  StatusReturnType CheckBkgCodeSegmentStatus(void) const;
  virtual bool validateMiscFareTag(const PaxTypeFare& paxTfare, bool throughFare = false) const;

  PricingTrx* diffTrx() const { return _diffTrx; }
  PricingTrx*& diffTrx() { return _diffTrx; }

protected:
  void adjustRexPricingDates();
  void restorePricingDates();

  // Determine Fail RBD Booking Code in the FareMarket and analyze its cabin
  // compare to through fare booking code
  DifferFailBkgCodeEnum
  analyzeFailBkgCode(const int travelSegNumber, DifferentialData::FareTypeDesignators& cabin) const;

  // Differential sector validation
  virtual bool
  diffSectorValidation(DifferentialData* diffData, const std::vector<Differentials*>& differList);

  bool validateThruFare(Differentials& di) const;

  // Check to see if this sector fails in RB validation of all local fares that
  // are applicable to the Business or Economy fare cabin booked
  bool intermediateParamMatch(const Differentials& diffIter, DifferentialData& diffSector) const;

  bool validateIntermediates(const Differentials& diffTable, DifferentialData& diffSector) const;

  bool
  finalyzeDiffSectorValidation(DifferentialData& diffSector, const Differentials& diffTable) const;

  // Check to see if this sector fails in RB validation of all local fares that
  // are applicable to the Business or Economy fare cabin booked
  // bool currentSectorLocalFareRBDValidation( const AirSeg& airSeg );
  bool currentSectorLocalFareRBDValidation(int travelSegNumber);

  //**********************************************************************************
  // Determination of the adjacent sectors on the Fare Market,
  // Include the First, Business and Economy logics to proceed
  //*1.2.1.or 1.3.1.++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Check to see if there is an adjacent failed RB sector that is in the higher
  // fare cabin than the through fare component being validated
  // in the previous & next travel sector
  //**********************************************************************************
  bool adjacentSectorDetermination(DifferentialData& diffData,
                                   int travelSegNumber,
                                   Indicator& diffNumber,
                                   const Indicator& diffLetter,
                                   uint16_t& thruNumber);

  bool adjacentSectorDiff(DifferentialData& diffData,
                          int travelSegNumber,
                          const int8_t& increment,
                          const Indicator& diffNumber,
                          const Indicator& diffLetter,
                          uint16_t& thruNumber);

  // Validate Directionality of the through fare

  bool thruFareDirectionality(const Differentials& diffIter) const;

  // Match/Nomatch the geographic board or off point of the through fare component
  // Loc1Type & Loc1; Loc2Type & Loc2; ViaLoc Type & ViaLoc;
  bool matchLocation(const LocKey& loc1k,
                     const LocCode& loc1c,
                     const LocKey& loc2k,
                     const LocCode& loc2c,
                     const FareMarket& mkt,
                     const Indicator& direction) const;

  bool matchGeo(const LocKey& loc1k,
                const LocCode& loc1c,
                const LocKey& loc2k,
                const LocCode& loc2c,
                const Loc& origin,
                const Loc& destination,
                const Indicator& direction) const;

  bool matchGeo(const LocKey& loc1k, const LocCode& loc1c, const Loc& origin) const;

  // Match/Nomatch the geographic board point of the Itinenary
  bool matchLocation(const LocKey& loc1k,
                     const LocCode& loc1c,
                     const LocKey& loc2k,
                     const LocCode& loc2c,
                     const Indicator& direction) const;

  // Match/Nomatch the via point of the FareMarket
  bool matchLocation(const LocKey& loc1k, const LocCode& loc1c) const;

  bool countStopsCarriers(DifferentialData& diffSeg, const Differentials& diffTable) const;

  // Match/Nomatch the geographic board/off points for the Intermediate Loc's

  bool validateFlightAppL(DifferentialData& diffSeg, // FlightAppl = 'L'
                          const Differentials& diffTable) const;

  bool matchLocation(const LocKey& loc1k,
                     const LocCode& loc1c,
                     const LocKey& loc2k,
                     const LocCode& loc2c,
                     const DifferentialData& data,
                     const Indicator& direction) const;

  bool matchLoc1A2A1B2B(const LocKey& loc1k,
                        const LocCode& loc1c,
                        const LocKey& loc2k,
                        const LocCode& loc2c,
                        DifferentialData& diffSeg,
                        const Differentials& diffTable) const;

  //*************************************************************************************
  // Match FareType Designator and Carrier for the Intermediate ( Digfferential sectors)
  // For the Flight Appl has 'L' param sets. And, Loc1A/2A and/or Loc1B/2B are set.
  //*************************************************************************************

  bool matchIntermTypeCxrBkg(DifferentialData& diffSector,
                             std::vector<CarrierCode>& carrier,
                             const BookingCode bkg,
                             const Indicator cabin,
                             const Differentials& diffTable,
                             bool donoterase = false) const;

  // Match Through BookingCode in the Fare Market
  bool matchThroughBkgCode(const BookingCode& bkg) const;

  // Fare retrieval for the Differential sectors based on the Calculation Indicator
  bool fareSelection(void);
  bool fareSelection(DifferentialData& diffData);

  // Consolidate differential sectors
  bool consolidate(const std::vector<Differentials*>& differList);

  bool validateFareTypeDesignators(DifferentialData& diffDP,
                                   std::vector<FareMarket*>& fareMarket,
                                   std::vector<CarrierCode>& carrier,
                                   bool gov = true) const;

  void wpncFindFareForDiffSector(DifferentialData& diffDP, const int8_t increment = 0) const;

  bool wpncFindHighFareForDiffSector(DifferentialData& diffDP,
                                     std::vector<FareMarket*>& fareMarket,
                                     std::vector<CarrierCode>& carrier,
                                     const int8_t increment = 0,
                                     bool gov = true) const;

  void setDiffLowHigh(FareSelectionData& diffDP, PaxTypeFare& df, enum FareDest) const;

  virtual bool checkDifferentialFare(const PaxTypeFare& df, DifferentialData& diffDP) const;

  bool validateConsecutiveSectors();

  bool validateConsecutiveSectorsForThroughFareType(
      const DifferentialDataPtrVec& diffVforOneTripType) const;

  bool compareCabins(const FareMarket& mkt, const TypeOfCabin& compCabin) const;

  DifferentialData* compareFareMarkets(const FareMarket& mkt) const;

  DifferentialData* compareFareMarkets(const FareMarket& mkt,
                                       const DifferentialDataPtrVec& diffVec,
                                       const bool) const;

  bool checkFareType(const FareType& throughFareType,
                     const PaxTypeFare& diffFare,
                     bool ignoreFTCheck) const;

  virtual const std::vector<const IndustryPricingAppl*>&
  getIndustryPricingAppl(const CarrierCode& carrier,
                         const GlobalDirection& globalDir,
                         const DateTime& date) const;
  bool consolidateDifferentials(std::vector<DifferentialData*>& diffLforOneTripType,
                                const std::vector<Differentials*>& differList);

  bool consecutiveSectorsFoundInSequence(const std::vector<DifferentialData*>& diffLforOneTripType,
                                         unsigned int& diffStart,
                                         bool& consecFound) const;

  ReturnValues
  consolidateTwoDifferentialSectors(const std::vector<DifferentialData*>& diffLforOneTripType,
                                    const std::vector<Differentials*>& differList,
                                    unsigned int& jmLeft,
                                    unsigned int& jmRight,
                                    DifferentialData*& diffItem,
                                    const bool& theSameSequence,
                                    const bool& consecFound = false);

  unsigned int
  findEndElementInNextSequence(unsigned int& jmBegin,
                               const std::vector<DifferentialData*>& diffLforOneTripType) const;

  bool checkTagFareType(const Indicator& tagType) const;

  DifferentialData::FareTypeDesignators
  findCombinedFareType(const DifferentialData::FareTypeDesignators& tagL,
                       const DifferentialData::FareTypeDesignators& tagR,
                       const bool& consecFound) const;

  bool getPaxTypeFareVec(PaxTypeFarePtrVec& paxTypeFareVec, const FareMarket& mkt) const;

  virtual bool getHip(DifferentialData& diffItem) const throw(std::string&);

  const bool isBkgCodeStatusPass(PaxTypeFare& paxTypeFare, bool a = false) const;

  bool validateRules(const PaxTypeFare& paxTF) const;
  virtual bool validatePuRules(const PaxTypeFare& paxTF, std::vector<uint16_t> vec, bool a = false) const;

  void calculateDPercentage(DifferentialData& diffData) const;

  void defineLowFareCabin(DifferentialData& diffItem) const;
  void defineHighFareCabin(DifferentialData& diffItem) const;

  CarrierCode getCarrierCode(const AirSeg& airSeg, std::vector<TravelSeg*>* tv = nullptr) const;

  PaxTypeFare* throughFare() const { return _paxTfare; }
  PaxTypeFare*& throughFare() { return _paxTfare; }

  FarePath* farePath() const { return _farePath; }
  FarePath*& farePath() { return _farePath; }

  PricingUnit* pricingUnit() const { return _pricingUnit; }
  PricingUnit*& pricingUnit() { return _pricingUnit; }

  FareUsage* fareUsage() const { return _fareUsage; }
  FareUsage*& fareUsage() { return _fareUsage; }

  const Itin* itinerary() const { return _itin; }
  const Itin*& itinerary() { return _itin; }

  const PaxType* requestedPaxType() const { return _reqPaxType; }
  const PaxType*& requestedPaxType() { return _reqPaxType; }

  FareBookingCodeValidator* fBCVal() const { return _fBCValidator; }
  FareBookingCodeValidator*& fBCVal() { return _fBCValidator; }

  bool getBookingCode(const TravelSeg* ts, const AirSeg& airSeg, BookingCode& bc) const;

  // looking for diff sectors number and accumulate them in i[]
  // to see if all sectors are passed in through fare component

  void findNumbers(uint16_t i[], uint16_t arrayCount, DifferentialData& dI) const;

  void collectThruNumbers(uint16_t i[], uint16_t& index, DifferentialData& dd) const;
  uint16_t countThruNumbers(DifferentialData& dd) const;

  bool wpncMatchBkgCodes(PaxTypeFare& df, DifferentialData& diffDP) const;

  void initialize(DiagCollector* diag,
                  PricingTrx* trx,
                  FareMarket* mkt,
                  PaxTypeFare* paxTfare,
                  FarePath* farePath,
                  PricingUnit* pricingUnit,
                  FareUsage* fareUsage,
                  const Itin* itin,
                  const PaxType* reqPaxType,
                  FareBookingCodeValidator* fbcv);

  bool highAndLowFound();
  void createDiag();
  void doDiag();
  void endDiag();
  void copyDifferentialData();
  bool checkDifferentialVector();
  void processDiag();

  void moveCarrierAndMarket(DifferentialData* DidiffItem,
                            AirSeg& airSeg,
                            std::vector<TravelSeg*>& segOrderVec);

  std::vector<ClassOfService*>* getAvailability(const int travelSegNumber) const;

  std::vector<ClassOfService*>* getCOS(const int tSN, std::vector<TravelSeg*>& segOrderVec) const;

  void
  cabinTypeFromFTD(const DifferentialData::FareTypeDesignators& cabin, CabinType& cabinType) const;

  DifferentialData::FareTypeDesignators cabinTypeToFTD(const CabinType& cabinType) const;

  void cabinTypeFromHip(const DifferentialData::HipRelated hip, CabinType& cabin) const;

  DifferentialData::HipRelated cabinTypeToHip(const CabinType& cabin) const;

  virtual bool applyPremEconCabinDiffCalc(const CarrierCode& cxr) const;
  virtual bool applyPremBusCabinDiffCalc(const CarrierCode& cxr) const;
  virtual bool allowPremiumCabinSlide(const CarrierCode& cxr) const;

  bool getIndustryPrimePricingPrecedence(DifferentialData& diffDP,
                                         const CarrierCode& govCXR,
                                         bool gOver,
                                         const DiffDataFMI& fmi,
                                         FareMarket*& fmiDiff,
                                         Indicator& prPricePr) const;

  bool getLowerCabin(CabinType& cabin) const;

  bool isFareValidForFareSelection(PaxTypeFarePtrVecIC& ptFare,
                                   DifferentialData& diffDP,
                                   Indicator prPricePr,
                                   const CarrierCode& govCXR) const;

  bool processNormalFareSelection(FareSelectionDataHelper& fsdm) const;
  bool processWPNCFareSelection(FareSelectionDataHelper& fsdm) const;

  virtual RepricingTrx* getRepricingTrx(const std::vector<TravelSeg*>& tvlSeg,
                                        FMDirection fmDirectionOverride) const;
  const FareMarket* getFareMarket(const std::vector<TravelSeg*>& segOrderVec,
                                  const CarrierCode* carrier,
                                  const CarrierCode* altCarrier = nullptr,
                                  bool* gOver = nullptr) const;

  bool  isPremiumEconomyCabin(const FareType& intermFareType) const;
  bool  validateHighFareForPremiumEconomyCabin(const DifferentialData& diffDP,
                                               const PaxTypeFare& df) const;

private:
  DiagCollector* _diag = nullptr; // Pointer to a Diagnostic object
  PricingTrx* _diffTrx = nullptr;
  FareMarket* _mkt = nullptr;
  PaxTypeFare* _paxTfare = nullptr;
  FarePath* _farePath = nullptr;
  PricingUnit* _pricingUnit = nullptr;
  FareUsage* _fareUsage = nullptr;
  const Itin* _itin = nullptr;
  const PaxType* _reqPaxType = nullptr;
  FareBookingCodeValidator* _fBCValidator = nullptr;
  DateTime _travelDate;
};
} // tse
