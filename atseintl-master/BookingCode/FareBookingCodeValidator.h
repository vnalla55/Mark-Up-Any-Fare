//----------------------------------------------------------------------------
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

#include "BookingCode/BCETuning.h"
#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "BookingCode/RebookedCOSChecker.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{

class PricingTrx;
class PaxTypeFareRuleData;
class ClassOfService;
class FBRPaxTypeFareRuleData;
class FareMarket;
class AirSeg;
class TravelSeg;
class FarePath;
class FareUsage;
class Itin;
class CarrierApplicationInfo;
class DiagCollector;

/**
 *      This is a base class for the Booking Code validation process.
 *      And, a start point of RBD validation for each FareMarket. It will loop through
 *      all fares for all travel segments in the FareMarket. Depending on the type of fare
 *      the corresponding method will be called to start process the RBD validation.
 *      Currently the FBR fare RBD validation process is considering to be done in the
 *      derived class.
 */

class FareBookingCodeValidator : public FBCVAvailabilityGetter
{

  friend class FareBookingCodeValidatorTest;
  friend class AdvResTktTest;

public:
  FareBookingCodeValidator(PricingTrx& trx, FareMarket& mkt, Itin* itin);

  FareBookingCodeValidator(const FareBookingCodeValidator&) = delete;
  FareBookingCodeValidator& operator=(const FareBookingCodeValidator&) = delete;

  using COSPtrVec = std::vector<std::vector<ClassOfService*>*>;
  using COSPtrVecI = std::vector<std::vector<ClassOfService*>*>::iterator;
  using COSPtrVecIC = std::vector<std::vector<ClassOfService*>*>::const_iterator;

  using COSInnerPtrVecIC = std::vector<ClassOfService*>::const_iterator;

  enum ValidateReturnType
  { RET_PASS = 1,
    RET_FAIL,
    RET_MIXED,
    RET_NO_MATCH,
    RET_CONTINUE,
    RET_PRIME };

  /*---------------------------------------------------------------------------
   * Return Types for the Travel Segment after Booking code validated
   *-------------------------------------------------------------------------*/
  enum StatusReturnType
  { PASS = 1,
    FAIL,
    MIXED,
    SOME_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED,
    NONE_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED,
    SOME_PASS_NONE_FAIL_NO_MATCH_NOT_PROCESSED,
    NO_MATCH_NOT_PROCESSED,
    NEED_REVAL };

  /*---------------------------------------------------------------------------
   * Return Types for the Travel Segment after Booking code validated
   *-------------------------------------------------------------------------*/
  enum setStatus
  { INIT = '0',
    REC1 = '1',
    CONV_2 = '2' };

  //  The Reservation Booking Designation (RBD) validation process invokes by this method.
  //  This is the general RBD validation method.

  bool validate();

  bool validate(FareUsage& fu, const FarePath* farePath = nullptr); // for particular fare
  bool validate(PaxTypeFare& pTfare, const FarePath* farePath = nullptr)
  {
    return validate(pTfare, nullptr, farePath);
  }

  // Shopping
  bool
  validate(const uint32_t& bitIndex, PaxTypeFare* curFare, std::vector<BCETuning>& bceTuningData);

private:
  enum StatusType
  { STATUS_RULE1 = 1,
    STATUS_RULE2,
    STATUS_FLOW_FOR_LOCAL_JRNY_CXR };

  bool validate(PaxTypeFare& ptf, FareUsage* fu, const FarePath* farePath);

protected:
  // The RBD validation process for all Fares, except Industry.

  ValidateReturnType validateFareBkgCode(PaxTypeFare& paxTfare);

  // The RBD determination for Industry (YY) Fares.

  ValidateReturnType validateIndustryFareBkgCode(PaxTypeFare& paxTfare);

  void handleAsBookedClones(bool noRBDChk, bool& passAnyFares);
  virtual void
  validateAndSetUp(std::vector<PaxTypeFare*>& ptcFares, bool noRBDChk, bool& passAnyFares);
  bool validateFare(PaxTypeFare& paxTfare, const FarePath* farePath = nullptr);

  // YY Fares, find carrier item in Carrier Application Table (Table990)
  // for positive, negative ot any ($$) status.

  bool findCXR(const CarrierCode& cxr, const std::vector<CarrierApplicationInfo*>& carrierApplList);

  // Table 999 item number exists in the FareClassAppSegInfo (Record 1B)
  // validate it

  // Get Prime RBD from the Pax Type Fare
  void getPrimeR1RBDs(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes);

  ValidateReturnType validateBookingCodeTblItemNo(PaxTypeFare& paxTfare);

  // Validate RBD for the fare

  ValidateReturnType fareRBDValidation(PaxTypeFare& paxTfare, bool fbrActive);

  // Validate Base Fare Prime RBD Rec 1
  ValidateReturnType baseFarePrimeRBDRec1(PaxTypeFare& basePaxTfare, PaxTypeFare& paxTfare);

  // Validate Prime RBD for Domestic trip

  ValidateReturnType validateDomestic(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes);

  // Validate Booking code in the all travel segments against Prime RBD in the Record1.
  // Loop over the valid booking codes in the Record 1 and determine if the flight class of
  // service matches one of them.

  bool validatePrimeRBDRecord1Domestic(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes);

  bool isCOSValidAfterDomesticMixedClass(PaxTypeFare& paxTfare);

  bool prevalidateRecord1BPrimeBkgCode(PaxTypeFare& paxTfare);

  // Validate Booking code in the particular travel segment against Prime RBD in the Record1.
  // Loop over the valid booking codes in the Record 1 and determine if the flight class of
  // service matches one of them.

  bool validateSectorPrimeRBDRec1(AirSeg& airSeg, // WPNC & WPNCS
                                  std::vector<BookingCode>& bkgCodes,
                                  const std::vector<ClassOfService*>* cosVec,
                                  PaxTypeFare& ptf,
                                  PaxTypeFare::SegmentStatus& segStat,
                                  bool dispRec1 = true);

  bool validateRBDRec1PriceByCabin(AirSeg& travelSeg,  // WPNCS
                                   std::vector<BookingCode>& bkgCodes,
                                   PaxTypeFare& paxTfare,
                                   PaxTypeFare::SegmentStatus& segStat);
  void getRBDRec1Cabin(AirSeg& travelSeg,
                       std::vector<BookingCode>& bkgCodes,
                       std::vector<ClassOfService*>* rbdRec1CabinVec);

  void setFinalStatusForPriceByCabin(PaxTypeFare& paxTfare);

  void validatePrimeRBDRec1AndUpdateStatus(AirSeg& airSeg, // WPNC & WPNCS
                                           std::vector<BookingCode>& bkgCodes,
                                           const std::vector<ClassOfService*>* cosVec,
                                           PaxTypeFare& ptf,
                                           PaxTypeFare::SegmentStatus& segStat,
                                           bool dispRec1 = true);

  // FBR RBD - do last step Prime RBD validation for the secondary sectors
  bool primeRBDRec3CAT25Mandates(PaxTypeFare& paxTfare);

  // Retrieve the ExceptT999 object from the objectManager and invoke the object's validate method.

  ValidateReturnType validateT999(PaxTypeFare& paxTfare);

  // Validate FBR Fare

  ValidateReturnType validateFBRFare(PaxTypeFare& paxTfare, bool indicator);

  //  Validate Prime RBD Rec 1, Rec6 Convention 1 for the FBR fares

  ValidateReturnType validateFBR_RBDInternational(PaxTypeFare& paxTfare, bool indicator);

  //  Validate Prime RBD Rec 1, Rec6 Convention 1 for the FBR fares
  //  after Rec6 Convention2 is done

  ValidateReturnType
  finalValidateFBR_RBDInternational(PaxTypeFare& paxTfare, bool ruleZeroProcessed);

  // Validate Prime RBD Rec 1, Rec6 Convention 1 & LocalMarket for the Published,
  // Constructed Fares

  ValidateReturnType validateRBDInternational(PaxTypeFare& paxTfare);

  ValidateReturnType baseFareT990Rec1(PaxTypeFare& basePaxTfare, PaxTypeFare& paxTfare);

  // FareByRule Fare Record 6 Convention 1 validation
  //  Record6 Convention1, calls LocalMarket
  void validateRecord6Conv1(PaxTypeFare& paxTfare,
                            AirSeg* airSeg,
                            const std::vector<ClassOfService*>* cosVec,
                            PaxTypeFare::SegmentStatus& segStat,
                            std::vector<BookingCode>& bkgCodes,
                            TravelSeg* seg,
                            const FBRPaxTypeFareRuleData* fbrPTFBaseFare);

  // Retrieve the Convention object and invoke the Record6 Convention 2 or 1 validate method.

  ValidateReturnType
  validateConvention1(PaxTypeFare& paxTfare, const VendorCode& vendor, AirSeg* airSeg);

  // Industry YY fare validation require check for carrier code on all sectors of fare component
  // and if so, try to apply the Prime RBD.
  ValidateReturnType checkAllSectorsForOpen(AirSeg* airSegL,
                                            const std::vector<ClassOfService*>* cosVec,
                                            PaxTypeFare::SegmentStatus& segStat,
                                            std::vector<BookingCode>& bkgCodes,
                                            PaxTypeFare& paxTfare);

  // Retrieve the Convention object and invoke the Record6 Convention 2 method for the FBR fare.

  ValidateReturnType validateConvention2(PaxTypeFare& paxTfare,
                                         const PaxTypeFare& basePaxTfare,
                                         AirSeg* airSeg,
                                         bool& isRuleZero,
                                         bool a = false,
                                         bool needRuleZero = false);

  // Retrieve the LocalMarket object from the objectManager and invoke the object's validate method.

  bool validateLocalMarket(PaxTypeFare& paxTfare,
                           AirSeg& airSeg,
                           TravelSeg* seg,
                           const std::vector<ClassOfService*>* cosVec,
                           PaxTypeFare::SegmentStatus& segStat);

  bool validatePTFs(const std::vector<PaxTypeFare*>& fares,
                    const PaxTypeFare& paxTfare,
                    AirSeg& airSeg,
                    const std::vector<ClassOfService*>* cosVec,
                    PaxTypeFare::SegmentStatus& segStat);

  // Mixed class Booking Code validation on the US/CA fare component.

  bool validateDomesticMixedClass(PaxTypeFare& paxTfare, BookingCode& bkg);

  // Analyze Statuses for each  Travel Segment in the FareMarket after T999 validation and return
  //  condition which one helps to make a decision (Pass/Fail/Continue) for the PaxTypeFare

  StatusReturnType CheckBkgCodeSegmentStatus(PaxTypeFare& paxTfare);
  StatusReturnType CheckBkgCodeSegmentStatus_NEW(PaxTypeFare& paxTfare);

  // Analyze status for the travel segment that we sent for T999 validation.
  // Note that in case of Record 6 Convention 1 we send only one airseg at a time to T999
  // validation.

  ValidateReturnType CheckBkgCodeSegmentStatusConv1(PaxTypeFare& paxTfare, AirSeg* pAirSegConv1);
  ValidateReturnType
  CheckBkgCodeSegmentStatusConv1_deprecated(PaxTypeFare& paxTfare, AirSeg* pAirSegConv1);

  // Set up the BookingCode Statuses for each Pass/Fail Travel Segment in the FareMarket after T999
  // validation
  //  It'll be done for Rec1T999 and Conv2T999 validation, so far

  void SetBkgSegmStatus(PaxTypeFare& paxTfare, int itor);

  //
  void setFailureReasonInSegmentAndFare(BookingCodeValidationStatus ibfStatus,
                                        PaxTypeFare& paxTFare,
                                        PaxTypeFare::SegmentStatus& segStat,
                                        bool priceByCabin = false);
  //
  ValidateReturnType setPaxTypeFareBkgCodeStatus(PaxTypeFare& paxTfare);

  // Set up PaxTypeFare status

  ValidateReturnType setUpPaxTFareStatusAfterT999(PaxTypeFare& paxTfare);

  // Turn off the NOMATCH & NOT_YET_PROCESSED in the Booking Code status
  // in the PaxTypeFare

  void turnOffNoMatchNotYetProcessedStatus(PaxTypeFare::SegmentStatus& segStat);

  // Prime RBD does not exist in the Cat25 Fare Resulting Fare
  // Process if it's Discounted FBR

  ValidateReturnType validateRBDforDiscountFBRFare(PaxTypeFare& paxTfare,
                                                   std::vector<BookingCode>& bookingCodeVec,
                                                   PaxTypeFare::SegmentStatus& segStat,
                                                   bool& rec1);

  void setFailPrimeRBDInternationalStatus(PaxTypeFare::SegmentStatus& segStat, const int number);

  void
  setBkgCodePrimeRBDDomesticSegmentStatus(PaxTypeFare::SegmentStatus& segStat, const bool bkgFound);

  void setFailPrimeRBDDomesticStatus(PaxTypeFare::SegmentStatus& segStat, const int number);
  void setPassBkgCodeSegmentStatus(PaxTypeFare::SegmentStatus& segStat, const int number);

  bool isItInfantFare(PaxTypeFare& paxTfare, TravelSeg* seg);

  bool setIndforWPBkgStatus(PaxTypeFare& paxTfare);

  void setFinalBkgStatus(PaxTypeFare& paxTfare);

  ValidateReturnType setPTFBkgStatus(PaxTypeFare& paxTfare, StatusReturnType checkRetType);

  void createDiagnostic();

  bool isDiag400Enabled() const;
  bool isDiag411Enabled() const;

  void printDiag400InvalidFM();

  void printDiagHeader();
  void printDiag400Header();
  void printDiag411Header();

  void printDiag411PaxTypeFare(PaxTypeFare* paxTFare);
  void printDiag400PaxTypeFare(PaxTypeFare* paxTFare);

  void printDiag411FaretypeFamily(PaxTypeFare& paxTFare);

  void printSkipOneLine();

  void printSkipTwoLines();

  void printTable999ItemNumber(const int& itemNumber);

  void printVendorCarrier(const VendorCode& vendor, const CarrierCode& carrier);
  void printVendorCxrRule(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const RuleNumber ruleNumber);

  void printDiag411BkgCodeStatus(PaxTypeFare& paxTfare);

  void printDiag411SegmentStatus(PaxTypeFare::SegmentStatus& segStat);

  void printDiag411PrimeRBD(std::vector<BookingCode>& bkgCodes);

  void printDiag411CAT25PrimeRBD(std::vector<BookingCode>& bkgCodes);

  void printDiag411TravelSegInfo(const TravelSeg* tvl);

  void printDiag411Message(const int number, char = 0);

  void printDiag411FareClass(PaxTypeFare& paxTFare);

  void printDiag411InfantMsg();

  void printDiag411BaseFareMsg();

  void printDiag411FareType(PaxTypeFare& paxTFare);

  void printDiag411RebookBkg(const BookingCode& bkg);

  void printDiag411GovernCXR(const CarrierCode& cxr);

  void printDiag411ChangeYYBkgCode(PaxTypeFare& paxTfare, bool ind);

  void printDiag411BlankLine();

  void printDiag411Count(const int& number);

  void finishDiag();

  bool foundFareForDiagnostic(PaxTypeFare& paxTFare);

  void setFinalPTFBkgStatus(PaxTypeFare& paxTfare);

  bool failedSegInEconomy(PaxTypeFare& paxTfare);

  bool tryRule2(PaxTypeFare& paxTfare);

  void t999Rule2Local(PaxTypeFare& paxTfare);

  int fltIndex(const PaxTypeFare& paxTfare, const AirSeg* airSeg) const;

  bool flowJourneyCarrier(TravelSeg* tvlSeg);

  std::vector<ClassOfService*>*
  flowMarketAvail(TravelSeg* tvlSeg, PaxTypeFare::SegmentStatus& segStat, const uint16_t iCOS);

  std::vector<ClassOfService*>* pricingAvail(TravelSeg* tvlSeg);

  std::vector<ClassOfService*>*
  shoppingAvail(TravelSeg* tvlSeg, PaxTypeFare::SegmentStatus& segStat, const uint16_t iCOS);

  bool tryLocalWithFlowAvail(const PaxTypeFare& paxTypeFare) const;

  bool localJourneyCarrier(const TravelSeg* tvlSeg) const;

  void setAvailBreaks(PaxTypeFare& paxTypeFare) const;

  void diag411T999Rec1(PaxTypeFare& paxTFare);

  void diag411SecondarySectorMsg();

  void diag411PrimarySectorMsg();

  void diag411R6C2Status(PaxTypeFare& paxTfare);

  void diagT999Tuning();

  BCETuning& t999Tuning(const BookingCodeExceptionSequenceList& bceList,
                        const PaxTypeFare& paxTfare,
                        AirSeg* airSeg,
                        Indicator convNo);
  void diagJourney(PaxTypeFare& paxTfare);

  void atleastOneJourneyCxr();

  bool journeyExistInItin() const;

  bool partOfJourney(TravelSeg* tvlSeg);

  void wpaHigherCabin(PaxTypeFare& paxTfare);

  const std::vector<ClassOfService*>* getCosFromFlowCarrierJourneySegment(TravelSeg* tvlSeg);

  const std::vector<ClassOfService*>* getAvailability(TravelSeg* travelSeg,
                                                      PaxTypeFare::SegmentStatus& segStat,
                                                      PaxTypeFare& paxTypeFare,
                                                      const uint16_t iCOS) override;

  bool validateWP(const AirSeg& airSeg, const std::vector<BookingCode>& bkgCodes);

  bool checkBookedClassAvail(const AirSeg& airSeg);

  bool processFare(const PaxTypeFare& paxTypeFare);

  void setCat31RbdInfo();

  void setBkgStatus(PaxTypeFare& ptf, Cat31FareBookingCodeValidator::Result resultCat31) const;

  void initFuSegStatus();

  const BookingCodeExceptionSequenceList&
  getBookingCodeExceptionSequence(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  Indicator conventionNo,
                                  bool& isRuleZero) const;

  void setIgnoreCat31KeepFareFail(PaxTypeFare::BookingCodeStatus& bookingCodeStatus,
                                  PaxTypeFare& paxTypeFare);

  RepricingTrx* getRepricingTrx(PricingTrx& trx,
                                std::vector<TravelSeg*>& tvlSeg,
                                FMDirection fmDirectionOverride);

  bool skipFlownSegCat31() const { return _resultCat31 == Cat31FareBookingCodeValidator::PASSED; }

  void setJumpedDownCabinAllowedStatus(PaxTypeFare& paxTfare);

  // Attributes:

  PricingTrx& _trx;
  FareMarket* _mkt = nullptr;
  Itin* _itin = nullptr;
  DiagCollector* _diag = nullptr;
  bool _processedLocalMkt = true;
  StatusType _statusType = StatusType::STATUS_RULE1;
  bool _t999TuningDiag = false;
  bool _journeyDiag = false;
  std::vector<BCETuning> _bceTuningData;
  bool _partOfLocalJny = false;
  FareUsage* _fu = nullptr;
  bool _wpaNoMatchEntry = false;
  const FareCalcConfig* _fcConfig = nullptr;
  PaxTypeFare* _newPaxTypeFare = nullptr; // if need to be cloned
  Cat31FareBookingCodeValidator::Result _resultCat31 = Cat31FareBookingCodeValidator::SKIPPED;
  bool _rexLocalMarketWPvalidation = false;
  bool _prevalidateRecord1BPrimeBkgCode = false;
  bool _useBKGExceptionIndex = false;
  const Cat31FareBookingCodeValidator* _cat31Validator = nullptr;
};

using CarrierApplicationInfoListPtr = std::vector<CarrierApplicationInfo*>;
using CarrierApplicationInfoListPtrI = std::vector<CarrierApplicationInfo*>::iterator;
using CarrierApplicationInfoListPtrIC = std::vector<CarrierApplicationInfo*>::const_iterator;
using TravelSegPtrVecIC = std::vector<tse::TravelSeg*>::const_iterator;
} // end tse namespace
