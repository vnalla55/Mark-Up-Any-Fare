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
#include "BookingCode/FareBookingCodeValidator.h"

#include "BookingCode/BookingCodeExceptionValidator.h"
#include "BookingCode/DomesticMixedClass.h"
#include "Common/BookingCodeUtil.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/Swapper.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IbfAvailabilityTools.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag400Collector.h"
#include "Diagnostic/Diag411Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Fares/BooleanFlagResetter.h"
#include "Fares/Record1Resolver.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackCheckBckCodeSegStatusFix);
FALLBACK_DECL(fallbackAPM837);
FALLBACK_DECL(fallbackPriceByCabinActivation)
FALLBACK_DECL(fallbackAAExcludedBookingCode);
FALLBACK_DECL(fallbackFlexFareGroupNewJumpCabinLogic);
/**
 *  This is a base class for the Booking Code validation process.
 *  And, a start point of RBD validation for each FareMarket. It will loop through
 *  all fares for all travel segments in the FareMarket. Depending on the type of fare
 *  the corresponding method will be called to start process the RBD validation.



 *  Currently the FBR fare RBD validation process is considering to be done in the
 *  derived class.
 */

namespace
{
Logger
logger("atseintl.BookingCode.FareBookingCodeValidator");
ConfigurableValue<bool>
prevalidateRecord18PrimeBkgCode("SHOPPING_OPT", "PREVALIDATE_RECORD1B_PRIME_BKG_CODE", false);
ConfigurableValue<bool>
useBookingCodeExceptionIndex("FARESV_SVC", "USE_BOOKINGCODEEXCEPTION_INDEX", false);

}

FareBookingCodeValidator::FareBookingCodeValidator(PricingTrx& trx, FareMarket& mkt, Itin* itin)
  : _trx(trx), _mkt(&mkt), _itin(itin)
{
  AltPricingTrx* altTrx = dynamic_cast<AltPricingTrx*>(&_trx);
  if (UNLIKELY(altTrx && _trx.getRequest()->isLowFareRequested() &&
               (altTrx->altTrxType() == AltPricingTrx::WPA ||
                altTrx->altTrxType() == AltPricingTrx::WP_NOMATCH)))
  {
    _wpaNoMatchEntry = true;
    _fcConfig = FareCalcUtil::getFareCalcConfig(_trx);
  }

  if (PricingTrx::IS_TRX == _trx.getTrxType())
    _prevalidateRecord1BPrimeBkgCode = prevalidateRecord18PrimeBkgCode.getValue();

  _useBKGExceptionIndex = useBookingCodeExceptionIndex.getValue();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  The Reservation Booking Designation (RBD) validation process invokes by this method.
//  This is the general RBD validation method.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool
FareBookingCodeValidator::validate()
{
  bool noRBDChk = false;

  if (_trx.getOptions()->isCarnivalSumOfLocal())
  {
    noRBDChk = (_trx.getOptions()->fareX() || _trx.fxCnExceptionPerItin().getValForKey(_itin));
  }
  else
  {
    noRBDChk = (_trx.getOptions()->fareX() || _trx.fxCnException());
  }

  LOG4CXX_DEBUG(logger, "Entered FareBookingCodeValidator::validate()");

  bool passAnyFares = false;
  createDiagnostic();
  atleastOneJourneyCxr();
  setCat31RbdInfo();
  printDiagHeader();

  validateAndSetUp(_mkt->allPaxTypeFare(), noRBDChk, passAnyFares);
  if (UNLIKELY(_trx.getRequest()->isLowFareRequested() && _trx.isWpncCloningEnabled()))
    handleAsBookedClones(noRBDChk, passAnyFares);

  finishDiag();

  LOG4CXX_DEBUG(
      logger,
      "Leaving FareBookingCodeValidator::validate(): " << (passAnyFares ? "true" : "false"));

  return passAnyFares; // before it was - hard TRUE
}

void
FareBookingCodeValidator::handleAsBookedClones(bool noRBDChk, bool& passAnyFares)
{
  using PtfVec = std::vector<PaxTypeFare*>;

  if (_mkt->allPaxTypeFare().empty())
    return;

  for (PaxTypeBucket& ptc : _mkt->paxTypeCortege())
  {
    PtfVec ptcWithAsBooked;
    ptcWithAsBooked.reserve(ptc.paxTypeFare().size() * 2u);

    for (PaxTypeFare* ptf : ptc.paxTypeFare())
    {
      ptf->cloneMeForAsBooked(_trx);
      ptcWithAsBooked.push_back(ptf);
      ptcWithAsBooked.push_back(ptf->getAsBookedClone());
    }

    ptc.paxTypeFare().swap(ptcWithAsBooked);
  }

  PtfVec allAsBookedClones;
  PtfVec allWithAsBooked;
  allAsBookedClones.reserve(_mkt->allPaxTypeFare().size());
  allWithAsBooked.reserve(_mkt->allPaxTypeFare().size() * 2u);

  for (PaxTypeFare* ptf : _mkt->allPaxTypeFare())
  {
    ptf->cloneMeForAsBooked(_trx);
    allAsBookedClones.push_back(ptf->getAsBookedClone());
    allWithAsBooked.push_back(ptf);
    allWithAsBooked.push_back(ptf->getAsBookedClone());
  }

  RaiiWithMemory<char> raiiWithMemory(_trx.getRequest()->lowFareRequested(), 'N');
  validateAndSetUp(allAsBookedClones, noRBDChk, passAnyFares);

  _mkt->allPaxTypeFare().swap(allWithAsBooked);
}

void
FareBookingCodeValidator::validateAndSetUp(std::vector<PaxTypeFare*>& ptcFares,
                                           bool noRBDChk,
                                           bool& passAnyFares)
{
  for (const auto ptcFare : ptcFares)
  {
    _newPaxTypeFare = nullptr;
    PaxTypeFare& pTfare = *ptcFare;
    PaxTypeFare::BookingCodeStatus& bks = pTfare.bookingCodeStatus();
    SetBkgSegmStatus(pTfare, INIT); // Initialyze BkgSegmentStatus vector
    if (!fallback::fallbackFlexFareGroupNewJumpCabinLogic(&_trx) && _trx.isFlexFare())
      setJumpedDownCabinAllowedStatus(pTfare);

    if (UNLIKELY(noRBDChk))
    {
      bks.set(PaxTypeFare::BKS_PASS);
      continue;
    }

    if (UNLIKELY(!_trx.matchFareRetrievalDate(pTfare)))
      continue;

    if (_mkt->direction() == FMDirection::OUTBOUND && pTfare.directionality() == TO &&
        _mkt->geoTravelType() == GeoTravelType::International)
      continue;

    if (UNLIKELY(!foundFareForDiagnostic(pTfare)))
      continue;

    if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX && pTfare.retrievalInfo() != nullptr &&
                 pTfare.retrievalInfo()->keep()))
    {
      if (!pTfare.isValidAsKeepFare(_trx))
        continue;
    }
    else
    {
      if (!pTfare.isValidForPricing())
        continue;
    }

    if (UNLIKELY(bks.isSet(PaxTypeFare::BKS_FAIL)))
      continue;

    if (UNLIKELY(bks.isSet(PaxTypeFare::BKS_PASS)))
      continue;

    printDiag411PaxTypeFare(&pTfare);
    setAvailBreaks(pTfare);
    bks.set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);

    if (UNLIKELY(!processFare(pTfare)))
    {
      printDiag411Message(39); // "Fare type is not Published/YY or FBR "
      continue;
    }

    if (validateFare(pTfare))
      passAnyFares = true;

    wpaHigherCabin(pTfare);
    setIgnoreCat31KeepFareFail(bks, pTfare);

    if (UNLIKELY(!fallback::fallbackPriceByCabinActivation(&_trx) &&
                 _mkt->isFMInvalidByRbdByCabin()))
    {
      bks.set(PaxTypeFare::BKS_PASS, false);
      bks.set(PaxTypeFare::BKS_MIXED, false);
      bks.set(PaxTypeFare::BKS_FAIL);
    }

    printDiag400PaxTypeFare(&pTfare);
    diagJourney(pTfare);
  }
  if (UNLIKELY(!fallback::fallbackPriceByCabinActivation(&_trx) && _mkt->isFMInvalidByRbdByCabin()))
  {
    printDiag400InvalidFM();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  The Reservation Booking Designation (RBD) validation process invokes by this method
//  for ONE fare only. This fare has not been validated before. Called from
//  DifferentialValidator object.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validate(FareUsage& fu, const FarePath* farePath)
{
  _mkt = fu.paxTypeFare()->fareMarket();
  return validate(*fu.paxTypeFare(), &fu, farePath);
}

bool
FareBookingCodeValidator::validate(PaxTypeFare& ptf, FareUsage* fu, const FarePath* farePath)
{
  if (_trx.getOptions()->fareX() || _trx.fxCnException())
    return true;

  _fu = fu;

  if (LIKELY(foundFareForDiagnostic(ptf)))
    createDiagnostic();

  if (fu)
  {
    initFuSegStatus();
  }
  else
  {
    atleastOneJourneyCxr();
    SetBkgSegmStatus(ptf, INIT);
  }

  setCat31RbdInfo();
  printDiagHeader();
  printDiag411PaxTypeFare(&ptf);
  setAvailBreaks(ptf);

  if (fu)
    fu->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);

  if (!processFare(ptf))
  {
    printDiag411Message(39); // "Fare type is not Published/YY or FBR "
    finishDiag();
    return false;
  }

  validateFare(ptf, farePath);

  wpaHigherCabin(ptf);

  setIgnoreCat31KeepFareFail(fu ? fu->bookingCodeStatus() : ptf.bookingCodeStatus(), ptf);

  printDiag400PaxTypeFare(&ptf);
  diagJourney(ptf);
  finishDiag();

  return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validateFare(PaxTypeFare& pTfare, const FarePath* farePath)
{
  ValidateReturnType bkgReturn = RET_PASS;
  _statusType = STATUS_RULE1;
  _resultCat31 = Cat31FareBookingCodeValidator::SKIPPED;

  bool processedFare = false;

  if (LIKELY(_cat31Validator))
  {
    _resultCat31 = _cat31Validator->validateCat31(pTfare, farePath, _diag);

    if (UNLIKELY(!_cat31Validator->validateCat33(pTfare, _diag)))
    {
      PaxTypeFare::BookingCodeStatus& bookingCodeStatus =
          _fu ? _fu->bookingCodeStatus() : pTfare.bookingCodeStatus();
      bookingCodeStatus.set(PaxTypeFare::BKS_SAME_FAIL, true);
    }

    if (UNLIKELY(_resultCat31 == Cat31FareBookingCodeValidator::FAILED ||
                 _resultCat31 == Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2 ||
                 (_resultCat31 == Cat31FareBookingCodeValidator::PASSED && _mkt->isFlown())))
    {
      setBkgStatus(pTfare, _resultCat31);
      return true;
    }
  }

  if (pTfare.fare()->isIndustry())
  {
    bkgReturn = validateIndustryFareBkgCode(pTfare);
    if (UNLIKELY((bkgReturn != RET_PASS) && tryRule2(pTfare)))
    {
      printDiag411Message(42);
      _statusType = STATUS_RULE2;
      bkgReturn = validateIndustryFareBkgCode(pTfare);
    }
    if (tryLocalWithFlowAvail(pTfare))
    {
      printDiag411BlankLine();
      printDiag411Message(49); // Diagnostic  FLOW FOR LOCAL JOURNEY
      printDiag411BlankLine();
      _statusType = STATUS_FLOW_FOR_LOCAL_JRNY_CXR;
      bkgReturn = validateIndustryFareBkgCode(pTfare);
    }
  }
  else if (LIKELY(pTfare.fare()->isPublished() || pTfare.fare()->isConstructed() ||
                  pTfare.isFareByRule() || pTfare.isDiscounted()))
  {
    bkgReturn = validateFareBkgCode(pTfare);
    if (UNLIKELY((bkgReturn != RET_PASS) && tryRule2(pTfare)))
    {
      printDiag411Message(42);
      _statusType = STATUS_RULE2;
      bkgReturn = validateFareBkgCode(pTfare);
    }
    if (tryLocalWithFlowAvail(pTfare))
    {
      printDiag411BlankLine();
      printDiag411Message(49); // Diagnostic  FLOW FOR LOCAL JOURNEY
      printDiag411BlankLine();
      _statusType = STATUS_FLOW_FOR_LOCAL_JRNY_CXR;
      bkgReturn = validateFareBkgCode(pTfare);
    }
  }
  else
  {
    return false;
  }

  printSkipTwoLines();

  if ((bkgReturn == RET_PASS) || ((bkgReturn == RET_MIXED || bkgReturn == RET_FAIL) &&
                                  (!_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))))
  {
    processedFare = true;
    const PaxTypeFare::BookingCodeStatus& bookingCodeStatus =
        _fu ? _fu->bookingCodeStatus() : pTfare.bookingCodeStatus();

    if (bookingCodeStatus.isSet(PaxTypeFare::BKS_FAIL) && !tryLocalWithFlowAvail(pTfare))
    {
      setFinalPTFBkgStatus(pTfare);
    }
  }

  if (bkgReturn == RET_PASS)
  {
    //"F" is set when booking code validation passes
    // it means that if itin is not priced then it's not due to availability
    //(hence no fares found is set as a potential reason in case of failure )
    PaxTypeFare::BookingCodeStatus& ptfStat =
        (_fu) ? _fu->bookingCodeStatus() : pTfare.bookingCodeStatus();

    if (!(tryLocalWithFlowAvail(pTfare) && ptfStat.isSet(PaxTypeFare::BKS_MIXED)))
    {
      IbfAvailabilityTools::setBrandsStatus(
          IbfErrorMessage::IBF_EM_NO_FARE_FOUND, pTfare, _trx, __PRETTY_FUNCTION__);
    }
  }

  if (!fallback::fallbackPriceByCabinActivation(&_trx) &&
      !_trx.getOptions()->cabin().isUndefinedClass())
  {
    setFinalStatusForPriceByCabin(pTfare);
  }
  return processedFare;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  The Reservation Booking Designation (RBD) validation process invokes by this method.
//  This is the general RBD validation method for shopping.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validate(const uint32_t& bitIndex,
                                   PaxTypeFare* curFare,
                                   std::vector<BCETuning>& bceTuning)
{
  LOG4CXX_DEBUG(logger, "Entered FareBookingCodeValidator::validate(uint32_t,PaxTypeFare*&)");
  bool farePassed = false;
  // check if it is a bound fare
  const std::vector<BookingCode>* bookingCodesBF =
      curFare->fare()->fareInfo()->getBookingCodes(_trx);
  if (UNLIKELY(bookingCodesBF != nullptr))
  {
    createDiagnostic();
    printDiagHeader();

    // originally set to true
    farePassed = true;
    uint16_t numSeatsFare(PaxTypeUtil::numSeatsForFare(_trx, *curFare));
    PaxTypeFare::SegmentStatusVec segmentStatusVec;
    segmentStatusVec.reserve(_mkt->travelSeg().size());
    curFare->segmentStatusRule2().resize(_mkt->travelSeg().size());

    for (TravelSegPtrVec::const_iterator it(_mkt->travelSeg().begin()),
         itEnd(_mkt->travelSeg().end());
         it != itEnd;
         ++it)
    {
      TravelSeg* travelSeg = *it;
      PaxTypeFare::SegmentStatus segStat;
      bool bSegmentPassed(false);
      const std::vector<ClassOfService*>& classesOfService = travelSeg->classOfService();

      BcvsNotAvailableWins ibfPrioritizer; // IBF Prioritizer that decides on error messages ( not
      // offered vs not available )

      for (std::vector<ClassOfService*>::const_iterator cosIt(classesOfService.begin()),
           cosItEnd(classesOfService.end());
           cosIt != cosItEnd && !bSegmentPassed;
           ++cosIt)
      {
        const ClassOfService& cos = **cosIt;
        // no cabin validation
        // number of seats
        if (cos.numSeats() >= numSeatsFare)
        {
          // bkg codes
          for (const auto& bookingCodeBF : *bookingCodesBF)
          {
            if (bookingCodeBF == cos.bookingCode())
            {
              // update segmentStatus
              segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
              segStat._bkgCodeReBook = bookingCodeBF;
              segStat._reBookCabin = cos.cabin();
              bSegmentPassed = true;
              break;
            }
            else
              ibfPrioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED);
          }
        }
        else
          ibfPrioritizer.updateStatus(BOOKING_CODE_NOT_AVAILABLE);
      }
      segmentStatusVec.push_back(segStat);
      if (!bSegmentPassed)
      {
        setFailureReasonInSegmentAndFare(
            ibfPrioritizer.getStatus(), *curFare, segmentStatusVec.back());
        curFare->setFlightInvalidESV(bitIndex, RuleConst::BOOKINGCODE_FAIL);
        farePassed = false;
        break;
      }
    }

    PaxTypeFare::BookingCodeStatus& bookingCodeStatus =
        curFare->flightBitmapESV()[bitIndex]._bookingCodeStatus;
    bookingCodeStatus.set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);
    bookingCodeStatus.set(farePassed ? PaxTypeFare::BKS_PASS : PaxTypeFare::BKS_FAIL, true);

    if (farePassed)
    {
      curFare->flightBitmapESV()[bitIndex]._segmentStatus.swap(segmentStatusVec);
    }
    printDiag400PaxTypeFare(curFare);
    finishDiag();
    return farePassed;
  }
  ///////
  _processedLocalMkt = false;
  _statusType = STATUS_RULE1;
  Swapper<std::vector<BCETuning>> swapper(_bceTuningData, bceTuning);

  createDiagnostic();
  printDiagHeader();

  PaxTypeFare& pTfare = *curFare;

  if (UNLIKELY(!foundFareForDiagnostic(pTfare)))
    return false;

  if (LIKELY(PricingTrx::ESV_TRX != _trx.getTrxType()))
  {
    if (UNLIKELY((pTfare.isFlightBitmapInvalid()) || (!pTfare.isFlightValid(bitIndex))))
    {
      return false;
    }
  }

  // get new bookingcode status and segment status for each sop.
  PaxTypeFare::BookingCodeStatus bookingCodeStatus;
  std::vector<PaxTypeFare::SegmentStatus> segmentStatusVec;
  pTfare.segmentStatus().swap(segmentStatusVec);
  std::swap(bookingCodeStatus, pTfare.bookingCodeStatus());

  pTfare.segmentStatus().clear();
  SetBkgSegmStatus(pTfare, INIT); // Initialyze BkgSegmentStatus vector

  printDiag411PaxTypeFare(&pTfare);

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // This is the major loop for all PaxTypeFares in the FareMarket
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ValidateReturnType bkgReturn = RET_PASS; // Defult condition

  if (pTfare.fare()->isIndustry())
  {
    bkgReturn = validateIndustryFareBkgCode(pTfare);
  }
  else if (LIKELY(pTfare.fare()->isPublished() || pTfare.fare()->isConstructed() ||
                  pTfare.isFareByRule() || pTfare.isDiscounted()))
  {
    bkgReturn = validateFareBkgCode(pTfare);
  }
  else
  {
    printDiag411Message(39); // "Fare type is not Published/YY or FBR "
    return false;
  }

  printSkipTwoLines();

  if (bkgReturn != RET_PASS)
  {
    if (UNLIKELY(PricingTrx::ESV_TRX == _trx.getTrxType()))
    {
      pTfare.setFlightInvalidESV(bitIndex, RuleConst::BOOKINGCODE_FAIL);
    }
    else
    {
      pTfare.setFlightInvalid(bitIndex, RuleConst::BOOKINGCODE_FAIL);
    }
    // turn off the fail status in case one of the itins is valid.
    pTfare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, false);
  }
  else
  {
    farePassed = true;
  }
  // set the booking code status and segment status for each sop in the flight bit map

  std::swap(bookingCodeStatus, pTfare.bookingCodeStatus());

  bookingCodeStatus.set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // Turn off
  pTfare.segmentStatus().swap(segmentStatusVec);

  if (UNLIKELY(PricingTrx::ESV_TRX == _trx.getTrxType()))
  {
    curFare->flightBitmapESV()[bitIndex]._segmentStatus.swap(segmentStatusVec);
  }
  else
  {
    pTfare.swapInFlightSegmentStatus(bitIndex, segmentStatusVec);
    pTfare.setFlightBookingCodeStatus(bitIndex, bookingCodeStatus);
  }

  printDiag400PaxTypeFare(&pTfare);

  finishDiag();

  LOG4CXX_DEBUG(
      logger, "Leaving FareBookingCodeValidator::validate(): " << (farePassed ? "true" : "false"));

  return farePassed; // before it was - hard TRUE
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// The RBD validation process for the Carrier Specified Fares.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
 * The RBD validation process for the Cariier Specified Fares starts from this point.
 *
 * This method is the general RBD validation method.
 *
 *  @param  paxTfare    Fare being processed
 *
 *  @return true if fare pass the validation, false otherwise
 *
 *  @todo Main Flow of Events
  -#Check the Record 1B for the presence of the RBD Table 999.
    -#RBD Table 999 presents in the Record 1B.
        -#Process the associated Table 999 and apply RBD for the sector being processed.
        -#Booking Code does not pass or fail RBD validation on any Table 999 sequence.
          Apply Booking Code from the Record 1B for the sector being validated.
            a)  Fail RBD validation, if no match.
            b)  RBD has been applied, and then continue process for the next sector.
        -#No match all sequence numbers in Table 999, and no match this RBD level validation,
          continue validation.

    -#RBD Table 999 does not present.
      -#Domestic fare component.
        Apply Booking Code from the Record 1B for the sector being validated.
          a)  Fail RBD validation, if no match.
          b)  RBD has been applied, and then continue process for the next sector.
      -#International fare component.
        -#Retrieve the applicable Record 6 Convention 2 (Booking Code Application)
          from the cache for the fare component being validated. Expected to get the
          Rec6 Conv2 matched to the following key data VCTR on Record 1,

          Travel date (EFF/DISC date) and number 2. If no match found, continue
          processing, attempt to match on Rule 0000)
        -#Record 6 Convention 2 is found.
          Apply RBD validation logic for the sector being processed.
          -#Process the associated Table 999 and apply RBD for the sector being processed.
          -#Booking Code does not pass or fail RBD validation on any Table 999 sequence.
            Apply Booking Code from the Record 1B for the sector being validated.
            -#Fail RBD validation, if no match.
            -#RBD has been applied, and then continue process for the next sector.
          -#No match all sequence numbers in Table 999, and no match this RBD level validation,
            continue validation
          -#Record 6 Convention 2 is not found.
          -#Marketing carrier does match carrier on the Record 1.
            Apply Booking Code from the Record 1 for the sector being validated.
            -#Fail RBD validation, if no match.
            -#RBD has been applied, and then continue process for the next sector
          -#Marketing carrier does not match carrier on the Record 1.
            -#Retrieve the applicable Record 6 Convention 1 (Booking Code Application)
              in the cache for the marketing carrier of the sector being validated.
              Expected to get the Rec6 Conv1 matched to the following key data:
              Vendor on Record 1, Carrier and Travel (EFF/DISC) date on the validated sector.
              Tariff and Rule not used (zeroed).
            -#Record 6 Convention 1 is found.
              -#Apply RBD validation logic for the sector being processed.
                -#Process the associated Table 999 and apply RBD for the sector being processed.
                -#Booking Code does not pass or fail RBD validation on any Table 999 sequence.
                  Apply Booking Code from the Record 1B for the sector being validated.
                  -#Fail RBD validation, if no match.
                  -#RBD has been applied, and then continue process for the next sector.
                -#No match all sequence numbers in Table 999, and no match this RBD
                  level validation, continue validation.
            -#Record 6 Convention 1 is not found.
              Apply Local Market Processing Logic.
  -#Check the status of the fare segment for each flight segment on the fare component.
    Set up the fare segment status to:

    -#Pass, if all flight segments passed RBD validation
    -#Fail, if all flight segments failed RBD validation
    -#Mixed, if pass and fail status are presents.
 */

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  The RBD determination for any fares except an Industry.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateFareBkgCode(PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_PASS; // defult

  long itemNumber = paxTfare.fcasBookingCodeTblItemNo(); // "TABLE 999 ITEM NUMBER - "
  const bool fbrActive = paxTfare.isFareByRule();
  const PaxTypeFareRuleData* fbrPTFare = nullptr;
  if (fbrActive)
  {
    fbrPTFare = paxTfare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);

    if (UNLIKELY(fbrPTFare == nullptr))
    {
      PaxTypeFare::BookingCodeStatus& ptfStat =
          (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();

      ptfStat.set(PaxTypeFare::BKS_FAIL);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      // if command pricing e.g WPQY26 then PASS the fare
      if (paxTfare.validForCmdPricing(false))
      {
        if (!_fu)
          paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
        ptfStat.set(PaxTypeFare::BKS_PASS);
        ptfStat.set(PaxTypeFare::BKS_FAIL, false);
        printDiag411Message(41); // COMMAND PRICING RBD STATUS - PASS
        return RET_PASS;
      }
      printDiag411Message(65);
      return RET_FAIL;
    }
    if (UNLIKELY(paxTfare.vendor() != ATPCO_VENDOR_CODE && paxTfare.vendor() != SITA_VENDOR_CODE &&
                 paxTfare.fcasBookingCodeTblItemNo())) // Ignore CAT25 BCET 999
    { // if vendor is FMS
      itemNumber = 0;
    }
  }

  if (UNLIKELY(_prevalidateRecord1BPrimeBkgCode && prevalidateRecord1BPrimeBkgCode(paxTfare)))
  {
    return setPaxTypeFareBkgCodeStatus(paxTfare);
  }

  // Do check the dummy Rec1B for table 999 item number

  printDiag411Message(6);
  printVendorCarrier(paxTfare.vendor(), paxTfare.carrier());
  printTable999ItemNumber(itemNumber);
  printDiag411BlankLine();

  if (itemNumber != 0)
  {
    validReturnType = validateBookingCodeTblItemNo(paxTfare);

    if (validReturnType == RET_CONTINUE || validReturnType == RET_NO_MATCH)
      validReturnType = fareRBDValidation(paxTfare, fbrActive);

    return validReturnType;

  } // if Table999 item presents in the Record 1B
  else
  {
    // No Rec1 Table999 Item number exists, check for Domestic

    if (_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
    {
      std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
      paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
      validReturnType = validateDomestic(paxTfare, bkgCodes);
      return validReturnType;
    }

    // Validate International Market

    if (fbrPTFare != nullptr) // FBR fare
    {
      validReturnType = validateFBRFare(paxTfare, false);
    }
    else
    {
      // Record 6 Convention 2 validation for the Fare Market
      TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
      AirSeg* airSeg = nullptr;

      for (; iterTvl != _mkt->travelSeg().end(); iterTvl++)
      {
        airSeg = dynamic_cast<AirSeg*>(*iterTvl);
        if (airSeg != nullptr)
          break;
      }
      bool isRuleZero = false;
      if (airSeg == nullptr || ((validReturnType = validateConvention2(
                                     paxTfare, paxTfare, airSeg, isRuleZero)) == RET_NO_MATCH))
      {
        // International RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1 , Local Market
        // validations for the international FareMarket
        // validReturnType = Pass;

        validReturnType = validateRBDInternational(paxTfare);
      } // Validate Conv 2 done
    } //  Else done
  }
  return validReturnType;

} //  We done with the Published/Constructed/FBR/Discounted fare

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   The RBD determination for Industry (YY) Fares.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateIndustryFareBkgCode(PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_PASS; // defult setup

  // preparation to get Booking codes from the Record 1
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  //  paxTfare.getPrimeBookingCode(bkgCodes);              // gets Bkgs Codes from Rec 1B
  getPrimeR1RBDs(paxTfare, bkgCodes);

  // Retrieve Carrier Application Table 990

  int itemNumber = paxTfare.fcasCarrierApplTblItemNo(); // Table 990 item number

  printDiag411Message(36);
  printVendorCarrier(paxTfare.vendor(), paxTfare.carrier());
  printTable999ItemNumber(itemNumber); // display item number
  printDiag411BlankLine();

  const std::vector<CarrierApplicationInfo*>& carrierApplList =
      _trx.dataHandle().getCarrierApplication(paxTfare.vendor(),
                                              paxTfare.fcasCarrierApplTblItemNo());

  if (UNLIKELY(itemNumber && carrierApplList.empty())) // IJUL05
  {
    printDiag411Message(37);
    printDiag411BlankLine();
    return (validReturnType = RET_FAIL);
  }
  // Loop all Travel segments for this PaxTypeFare

  bool setChangeBkg = false;
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();

  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (UNLIKELY(sizeCOS != _mkt->travelSeg().size())) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::validateIndustryFareBkgCode():size problem");
    return (validReturnType = RET_FAIL);
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;

  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    printDiag411TravelSegInfo(*iterTvl);
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (UNLIKELY(!carrierApplList.empty()))
    {
      if (_mkt->governingCarrier() == airSeg->carrier())
      {
        if (findCXR(airSeg->carrier(), carrierApplList))
        {
          if (!setChangeBkg && !bkgCodes.empty())
          {
            // change Bkg Code for the YY Fare Basis Code
            paxTfare.setChangeFareBasisBkgCode(bkgCodes[0]);
            setChangeBkg = true;
          }

          validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat);
          continue;
        }
      }
    }

    if (!paxTfare.getChangeFareBasisBkgCode().empty() && setChangeBkg)
    {
      printDiag411ChangeYYBkgCode(paxTfare, false);
    }

    // Marketing carrier not equal Governing carrier OR
    // Carrier status in Table990 is "negative"
    // Record 6 Convention 1 validation for the Travel Segment
    //
    validateRecord6Conv1(paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, nullptr);
  } // travel segment loop

  if (UNLIKELY(isDiag411Enabled()))
  {
    const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(paxTfare.fare());
    if (indFare != nullptr && indFare->changeFareClass())
    {
      printDiag411ChangeYYBkgCode(paxTfare, true);
    }
  }
  validReturnType = setPaxTypeFareBkgCodeStatus(paxTfare);

  return validReturnType;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Find carrier item in Carrier Application Table (Table990)
// for positive, negative ot any status.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::findCXR(const CarrierCode& cxr,
                                  const std::vector<CarrierApplicationInfo*>& carrierApplList)
{
  CarrierApplicationInfoListPtrIC iterCxr = carrierApplList.begin();

  for (; iterCxr != carrierApplList.end(); iterCxr++)
  {
    if ((cxr == (*iterCxr)->carrier()) && ((*iterCxr)->applInd() != 'X'))
    {
      return true; // proceed to Prime RBD
    }
  } // for loop

  iterCxr = carrierApplList.begin();
  for (; iterCxr != carrierApplList.end(); iterCxr++)
  {
    if ((*iterCxr)->applInd() == 'X') // any "negative" item carrier in the table
    {
      if (cxr == (*iterCxr)->carrier())
        return false; // proceed to Rec6 Conv 1
    }
  } // for loop

  iterCxr = carrierApplList.begin();
  for (; iterCxr != carrierApplList.end(); iterCxr++)
  {
    if ((*iterCxr)->applInd() == '$') // any item carrier in the table
    {
      return true; // proceed to Prime RBD
    }
  }
  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Table 999 item number exists in the FareClassAppSegInfo (Record 1B)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateBookingCodeTblItemNo(PaxTypeFare& paxTfare)
{
  //  NoMatch means - nomatch is found in the DB or/and cache
  ValidateReturnType validReturnType = RET_NO_MATCH; // defult setup

  if (LIKELY((validReturnType = validateT999(paxTfare)) != RET_NO_MATCH))
  {
    return (validReturnType = setUpPaxTFareStatusAfterT999(paxTfare));
  } //   Table  999 data are not-Match or are not foun in the Data Base
  else
  {
    printDiag411Message(38); // TABLE 999 VALIDATION STATUS FOR THE FARE - NOMATCH
  }
  return validReturnType;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   After T999 validated, choose the right way to continue
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::fareRBDValidation(PaxTypeFare& paxTfare, bool fbrActive)
{
  ValidateReturnType validReturnType = RET_PASS; // defult setup

  // Two ways of validation will be applied depends on Domestic or International
  // First, check for Domestic

  if (_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
  {
    std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
    paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
    validReturnType = validateDomestic(paxTfare, bkgCodes);
  }
  else
  {
    // International RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1
    // validations for the international FareMarket

    validReturnType = RET_PASS;

    if (fbrActive) // FBR fare
    {
      validReturnType = validateFBRFare(paxTfare, fbrActive);
    }
    else
    {
      validReturnType = validateRBDInternational(paxTfare);
    }
  }
  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Retrieve the ExceptT999 object from the objectManager and invoke the object's
// validate method.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateT999(PaxTypeFare& paxTfare)
//                                        StatusType statusType)

{
  ValidateReturnType returnType = RET_PASS; // defult setup

  BookingCodeExceptionValidator bce(
      *_itin, _partOfLocalJny, skipFlownSegCat31(), _useBKGExceptionIndex);

  const BookingCodeExceptionSequenceList& bkgExcpSeqList =
      _trx.dataHandle().getBookingCodeExceptionSequence(
          paxTfare.vendor(), paxTfare.fcasBookingCodeTblItemNo(), !_useBKGExceptionIndex);

  if (UNLIKELY(bkgExcpSeqList.empty()))
  {
    printDiag411Message(37);
    printDiag411BlankLine();
    //    return ( returnType = RET_NO_MATCH );    IJUL05
    return (returnType = RET_FAIL);
  }

  BCETuning& bceTuning = t999Tuning(bkgExcpSeqList, paxTfare, nullptr, '0');

  // if statusType == STATUS_RULE2 then there is no need to validate Table999 again
  // we already did for Rule 2 also while doing Rule 1
  if (_statusType == STATUS_RULE1)
  {
    if (UNLIKELY(!bce.validate(
            _trx, bkgExcpSeqList, paxTfare, 0, bceTuning, _fu, paxTfare.isFareByRule())))
      // Record 1 Table 999 validation;
      // The paxTfare.isFareByRule() - added for the RBD T999 Dual Inventory project.
      // The 'B' and 'D' restriction tags not allowed if the T999 is in the Cat25 R3
      returnType = RET_FAIL;
  }
  else if (UNLIKELY(_statusType == STATUS_RULE2))
  {
    // if STATUS_RULE2
    // then we know that we put the status of those carriers in segmentStatusRule2
    // Copy that status to segmentStatus now.
    t999Rule2Local(paxTfare);
  }

  // don't copy for STATUS_FLOW_FOR_LOCAL_JRNY_CXR
  return returnType;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateDomestic(PaxTypeFare& paxTfare,
                                           std::vector<BookingCode>& bkgCodes)
{
  ValidateReturnType validReturnType = RET_PASS; // defult setup

  // Apply Prime RBD from the Rec 1 for the Domestic FareMarket
  PaxTypeFare::BookingCodeStatus& ptfStat =
      (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();

  if (validatePrimeRBDRecord1Domestic(paxTfare, bkgCodes))
  {
    ptfStat.set(PaxTypeFare::BKS_PASS); // BKG CODE VALIDATION STATUS  - PASS
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
      ptfStat.set(PaxTypeFare::BKS_PASS_FLOW_AVAIL);
    else if (LIKELY(_statusType != STATUS_RULE2))
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    }
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    validReturnType = RET_PASS;
  }
  else
  {
    // if command pricing e.g WPQY26 then PASS the fare
    if (UNLIKELY(paxTfare.validForCmdPricing(false)))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS); // BKG CODE VALIDATION STATUS-PASS
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(41); // COMMAND PRICING RBD STATUS - PASS
      validReturnType = RET_PASS;
    }
    else if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR && ptfStat.isSet(PaxTypeFare::BKS_PASS))
    {
      validReturnType = RET_PASS;
    }
    else
    {
      ptfStat.set(PaxTypeFare::BKS_FAIL); // BKG CODE VALIDATION STATUS  - FAIL
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      validReturnType = RET_FAIL;
    }
  }

  printDiag411BkgCodeStatus(paxTfare);

  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixed class Booking Code validation on the US/CA fare component.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validateDomesticMixedClass(PaxTypeFare& paxTfare, BookingCode& bkgCode)
{
  printDiag411Message(24); // "MIXED CLASS VALIDATION: "

  bool returnType = false; // defult setup

  DomesticMixedClass mix; // Domestic Mixed Class object

  returnType = mix.validate(_trx, paxTfare, *_mkt, bkgCode, _diag);

  return returnType;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// International Non-FBR RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1
// validations for the international FareMarket
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateRBDInternational(PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_PASS; // defult

  // preparation to get Booking codes from the Record 1
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B

  // Need to go thru all travel sectors and apply RBD for the sectors
  // with NoMatch and NoYetProcess statuses

  // Get statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (sizeCOS != _mkt->travelSeg().size()) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::validateRBDInternational():size problem");
    return (validReturnType = RET_FAIL);
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;
  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    printDiag411TravelSegInfo(*iterTvl);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      if (airSeg->carrier() == paxTfare.carrier())
      {
        validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat);
        continue;
      } // If carriers are the same

      printDiag411Message(9); //"SKIP PRIME RBD VALIDATION - CARRIERS DO NOT MATCH"

      // Record 6 Convention 1 validation for the Travel Segment
      //

      validateRecord6Conv1(paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, nullptr);

    } // If Segment status is NoMatch or Not Processed
    else
    {
      printDiag411SegmentStatus(segStat); // BOOKING CODE VALIDATION STATUS -
      // PASS/FAIL/MIXED/NOMATCH
    }
  } // For loop Travel Segment
  validReturnType = setPaxTypeFareBkgCodeStatus(paxTfare);

  return validReturnType;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Validate BookingCode for the Particular Travel Sector  for the WPNC or WPNCS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validateSectorPrimeRBDRec1(AirSeg& travelSeg,
                                                     std::vector<BookingCode>& bkgCodes,
                                                     const std::vector<ClassOfService*>* cosVec,
                                                     PaxTypeFare& paxTfare,
                                                     PaxTypeFare::SegmentStatus& segStat,
                                                     bool dispRec1)
{
  BcvsNotAvailableWins prioritizer;
  uint16_t b = 0;
  if (UNLIKELY(bkgCodes.empty())) // No book. codes in Rec 1
  {
    IbfAvailabilityTools::setBrandsStatus(
        IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
    return false;
  }
  if (dispRec1)
    printDiag411PrimeRBD(bkgCodes);

  if (validateWP(travelSeg, bkgCodes))
    return true;

  std::vector<BookingCode> vTempBkgCodes;
  // validate WPNCS
  if (_trx.getRequest()->isLowFareNoAvailability())
  {
    if (!BookingCodeUtil::validateExcludedBookingCodes(_trx, bkgCodes, segStat, vTempBkgCodes, _diag))
      return false;

    if (!fallback::fallbackPriceByCabinActivation(&_trx))
    {
      if (vTempBkgCodes.size() > 0)
      {
        std::vector<BookingCode> vLocalTempBkgCodes;
        for(auto tmpCode : vTempBkgCodes)
          vLocalTempBkgCodes.push_back((BookingCode)tmpCode);
        
        return (validateRBDRec1PriceByCabin(travelSeg, 
                                          vLocalTempBkgCodes, 
                                          paxTfare, segStat));
      }
      else
      {
        return (validateRBDRec1PriceByCabin(travelSeg, 
                                          bkgCodes, 
                                          paxTfare, segStat));
     
      }
    }
    else
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
      segStat._bkgCodeReBook = bkgCodes[0];
      segStat._reBookCabin = paxTfare.cabin();
      printDiag411Message(35); // APPLICABLE COS
      printDiag411RebookBkg(segStat._bkgCodeReBook);
      return true;
    }
  }

  bool offered = false;

  // validate WPNC

  if (_trx.getRequest()->isLowFareRequested() && !_rexLocalMarketWPvalidation)
  {
    uint16_t numSeats = PaxTypeUtil::numSeatsForFare(_trx, paxTfare);

    bool wpaNoMatchProcess = false;
    bool cabinCheckOK = false;
    if (UNLIKELY((_wpaNoMatchEntry && _fcConfig != nullptr) && !_trx.noPNRPricing()))
    {
      if (_fcConfig->wpaNoMatchHigherCabinFare() == '1')
        wpaNoMatchProcess = true;
    }
    COSInnerPtrVecIC classOfS = cosVec->begin();
    COSInnerPtrVecIC classOfSEnd = cosVec->end();
    if(vTempBkgCodes.size() <=  0)
    {
      if (!BookingCodeUtil::validateExcludedBookingCodes(_trx, bkgCodes, segStat, vTempBkgCodes, _diag))
        return false;
    }
    bkgCodes = vTempBkgCodes;
    std::vector<ClassOfService*>* rbdRec1CabinVec = nullptr;
    DataHandle& dataHandle = _trx.dataHandle();
    dataHandle.get(rbdRec1CabinVec);
    getRBDRec1Cabin(travelSeg, bkgCodes, rbdRec1CabinVec);

    for (b = 0; b < bkgCodes.size(); ++b)
    {
      classOfS = cosVec->begin();
      for (; classOfS != classOfSEnd; classOfS++)
      {
        ClassOfService& cOs = *(*classOfS);
        if (UNLIKELY(wpaNoMatchProcess || (!fallback::fallbackPriceByCabinActivation(&_trx) &&
                                           !_trx.getRequest()->isjumpUpCabinAllowed())))
        {
          if (cOs.cabin() == travelSeg.bookedCabin())
            cabinCheckOK = true;
          else
            cabinCheckOK = false;
        }
        else
        {
          if (cOs.cabin() <= travelSeg.bookedCabin())
            cabinCheckOK = true;
          else
            cabinCheckOK = false;
        }

        if (bkgCodes[b] == cOs.bookingCode())
        {
          if (!rbdRec1CabinVec->empty())
          {
            if ((*((*rbdRec1CabinVec)[b])).cabin() != cOs.cabin())
              cabinCheckOK = false;
          }

          offered = true;
          if (cabinCheckOK)
          {
            if (cOs.numSeats() >= numSeats)
            {
              if (LIKELY(travelSeg.rbdReplaced() ||
                         travelSeg.getBookingCode() != cOs.bookingCode() ||
                         travelSeg.realResStatus() == QF_RES_STATUS ||
                         (PricingTrx::IS_TRX == _trx.getTrxType()) ||
                         (PricingTrx::FF_TRX == _trx.getTrxType()) ||
                         (PricingTrx::MIP_TRX == _trx.getTrxType() &&
                          _trx.billing()->actionCode() != "WPNI.C" &&
                          _trx.billing()->actionCode() != "WFR.C")))
              {
                segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
                segStat._bkgCodeReBook = cOs.bookingCode();
                segStat._reBookCabin = cOs.cabin();
                printDiag411Message(35); // Rebooked COS
                printDiag411RebookBkg(segStat._bkgCodeReBook);
              }
              return true;
            }
            else
              prioritizer.updateStatus(BOOKING_CODE_NOT_AVAILABLE);
          }
          else
            prioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED);
        }
      }
    }
  }

  if (!offered)
    prioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED);

  bool priceByCabin = false;

  if (!fallback::fallbackPriceByCabinActivation(&_trx) &&
      !_trx.getOptions()->cabin().isUndefinedClass())
    priceByCabin = true;

  setFailureReasonInSegmentAndFare(prioritizer.getStatus(), paxTfare, segStat, priceByCabin);
  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// International FBR RBD validation for the international FareMarket if Booking
// Code Table Item number in Rec1 does not present
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateFBRFare(PaxTypeFare& paxTfare, bool bkgCodeItem)
{
  ValidateReturnType validReturnType = RET_PASS; // defult

  // CAT25 fare, then we need to validate Resulting Fare Prime RBD only, if it's not Match
  // need to validate Rec 6 Convention 2 for the Base Fare, if it's not match need to validate
  // prime RBD from the Base Fare, and the last step process Rec 6 Conv 1

  const FBRPaxTypeFareRuleData* fbrPTFBaseFare = paxTfare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  validReturnType = validateFBR_RBDInternational(paxTfare, bkgCodeItem);

  if (!bkgCodeItem) // BkgCodeTableItem does not present
  {
    if (LIKELY(fbrPTFBaseFare != nullptr))
    {
      if (!fbrPTFBaseFare->isSpecifiedFare()) // FBR calculated
      {
        bool isRuleZero = false;
        if (validReturnType == RET_CONTINUE || // Rec6 Conv2 for the Base Fare
            validReturnType == RET_NO_MATCH) // CXR's not Match,
        {
          bool smfFbrWithPrimeSector = false;
          if (!(paxTfare.vendor() == ATPCO_VENDOR_CODE || paxTfare.vendor() == SITA_VENDOR_CODE))
          {
            const FareByRuleItemInfo* fbrItemInfo =
                dynamic_cast<const FareByRuleItemInfo*>(fbrPTFBaseFare->ruleItemInfo());
            if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
            {
              smfFbrWithPrimeSector = true;
            }
          }
          PaxTypeFare* pBaseF = fbrPTFBaseFare->baseFare();
          if (LIKELY(pBaseF != nullptr)) // Base Fare present
          {
            if (LIKELY(!smfFbrWithPrimeSector))
            {
              printDiag411BaseFareMsg(); // START VALIDATION THE BASE FARE
              PaxTypeFare* fakePtf = pBaseF->clone(_trx.dataHandle(), false);
              fakePtf->bookingCodeStatus() = paxTfare.bookingCodeStatus();
              fakePtf->segmentStatus() = paxTfare.segmentStatus();
              fakePtf->segmentStatusRule2() = paxTfare.segmentStatusRule2();
              long itemNumber = pBaseF->fcasBookingCodeTblItemNo(); // "TABLE 999 ITEM NUMBER - "
              // // default Table999 item
              // Number

              // Do check the dummy Rec1B for table 999 item number

              printDiag411Message(61); // Base Fare R1/T999
              printVendorCarrier(pBaseF->vendor(), pBaseF->carrier());
              printTable999ItemNumber(itemNumber);
              printDiag411BlankLine();

              if (itemNumber != 0)
              {
                // if Table999 item presents in the Base Fare Record 1B
                validReturnType = validateBookingCodeTblItemNo(*fakePtf);
                // restore stauses in original paxtypefare

                paxTfare.bookingCodeStatus() = fakePtf->bookingCodeStatus();
                paxTfare.segmentStatus() = fakePtf->segmentStatus();
                paxTfare.segmentStatusRule2() = fakePtf->segmentStatusRule2();
                if (validReturnType != RET_CONTINUE && validReturnType != RET_NO_MATCH)
                {
                  return validReturnType;
                }
                else
                {
                  if ((validReturnType = baseFarePrimeRBDRec1(*pBaseF, paxTfare)) != RET_NO_MATCH)
                    return validReturnType;
                }
              }
              // Continue validate Base Fare Rec 1 if T999 is not referenced OR marketing cxr
              // does not match Cat25 Rec2 carrier
              // Starts with T990 if Base Fare is Industry fare
              if (pBaseF->carrier() == INDUSTRY_CARRIER)
              {
                return (validReturnType = baseFareT990Rec1(*pBaseF, paxTfare));
              }
            }

            if (UNLIKELY(_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))) // It is
            // Domestic
            // fare
            // component
            {
              std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
              pBaseF->getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B

              validReturnType = validateDomestic(paxTfare, bkgCodes);
              return validReturnType;
            }

            TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
            AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
            for (; iterTvl != _mkt->travelSeg().end(); iterTvl++)
            {
              airSeg = dynamic_cast<AirSeg*>(*iterTvl);
              if (airSeg != nullptr)
                break;
            }
            // Record 6 Convention 2 validation for the Fare Market
            if ((validReturnType = validateConvention2(
                     paxTfare, smfFbrWithPrimeSector ? paxTfare : *pBaseF, airSeg, isRuleZero)) ==
                RET_NO_MATCH)
            {
              // validations for the international FareMarket
              validReturnType = finalValidateFBR_RBDInternational(paxTfare, isRuleZero);
            }
          } // Base Fare present
        } // validReturnType == RET_CONTINUE || RET_NO_MATCH
      }
    }
  }
  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Prime RBD does not exist in the Cat25 Fare Resulting Fare
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateRBDforDiscountFBRFare(PaxTypeFare& paxTfare,
                                                        std::vector<BookingCode>& bookingCodeVec,
                                                        PaxTypeFare::SegmentStatus& segStat,
                                                        bool& dispRec1)
{
  ValidateReturnType validReturnType = RET_PASS; // defult

  // Cat25 & Cat 19

  const FBRPaxTypeFareRuleData* fbrPTFBaseFare = paxTfare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (fbrPTFBaseFare == nullptr)
  {
    return (validReturnType = RET_FAIL);
  }

  // Discounted FBR fare pointer
  const PaxTypeFareRuleData* diskPTfare =
      paxTfare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE); // Cat 19 pointer

  // If it's a CAT25 Fare and CXR's are match, and, no Prime RBD exists

  if (!fbrPTFBaseFare->isSpecifiedFare() && !paxTfare.isDiscounted())
  {
    if (bookingCodeVec.empty())
    {
      fbrPTFBaseFare->getBaseFarePrimeBookingCode(bookingCodeVec); // Base Fare
      printDiag411Message(48); //"GETS PRIME RBD OF BASE FARE "
      printDiag411PrimeRBD(bookingCodeVec);
    }
    dispRec1 = false; // do not display Rec1 RBD in the validateSectorPrimeRBDRec1 method
  }
  else if (fbrPTFBaseFare->isSpecifiedFare() && !paxTfare.isDiscounted())
  {
    if (bookingCodeVec.empty())
    {
      setFailPrimeRBDDomesticStatus(segStat, 2); // PRIME RBD VALIDATION STATUS - FAIL
      IbfAvailabilityTools::setBrandsStatus(
          IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
      validReturnType = RET_CONTINUE;
    }
  }
  else if (paxTfare.isDiscounted())
  {
    if (diskPTfare == nullptr)
    {
      setFailPrimeRBDDomesticStatus(segStat, 31); // PRIME RBD VALIDATION STATUS - FAIL
      validReturnType = RET_CONTINUE;
    }
    else
    {
      const DiscountInfo* discountInfo =
          dynamic_cast<const DiscountInfo*>(diskPTfare->ruleItemInfo());
      if (discountInfo != nullptr)
      {
        if (discountInfo->bookingCode().empty())
        {
          if (fbrPTFBaseFare->isSpecifiedFare())
          {
            setFailPrimeRBDDomesticStatus(segStat, 2); // PRIME RBD VALIDATION STATUS - FAIL
            validReturnType = RET_CONTINUE;
          }
          else
          {
            fbrPTFBaseFare->getBaseFarePrimeBookingCode(bookingCodeVec); // Prime RBD Base Fare
            printDiag411PrimeRBD(bookingCodeVec);
          }
        }
        else
        {
          bookingCodeVec.push_back(discountInfo->bookingCode());
        }
      }
    } // No RBD exists in the Discounted 19-22 Fare
  } // Discounted PaxTypeFare
  return validReturnType;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// International FBR RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1
// validations for the international FareMarket

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::finalValidateFBR_RBDInternational(PaxTypeFare& paxTfare,
                                                            bool ruleZeroProcessed)
{
  ValidateReturnType validReturnType = RET_PASS; // defult

  const FBRPaxTypeFareRuleData* fbrPTFBaseFare = paxTfare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (UNLIKELY(fbrPTFBaseFare == nullptr))
  {
    return (validReturnType = RET_FAIL);
  }
  const CategoryRuleInfo* fbrRuleInfo = fbrPTFBaseFare->categoryRuleInfo(); // Get Rec 2 FBR

  // CAT25 Mandates (next 5 lines)
  const FareByRuleItemInfo* fbrItemInfo = nullptr;
  bool smfFbrWithPrimeSector = false;
  fbrItemInfo = dynamic_cast<const FareByRuleItemInfo*>(fbrPTFBaseFare->ruleItemInfo());
  if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
  {
    if (!fbrPTFBaseFare->isSpecifiedFare() &&
        !(paxTfare.vendor() == ATPCO_VENDOR_CODE || paxTfare.vendor() == SITA_VENDOR_CODE))
    {
      smfFbrWithPrimeSector = true;
    }
  }

  // preparation to get Booking codes from the Record 1
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  fbrPTFBaseFare->getBaseFarePrimeBookingCode(bkgCodes); // gets Bkgs Codes from Base Fare

  //  Need to go thru all travel sectors and apply RBD for the sectors
  //  with NoMatch and NoYetProcess statuses
  //  Gets statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (UNLIKELY(sizeCOS != _mkt->travelSeg().size())) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger,
                  "FareBookingCodeValidator::finalValidateFBR_RBDInternational():size problem");
    return (validReturnType = RET_FAIL);
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;

  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    printDiag411TravelSegInfo(*iterTvl);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      if (airSeg->carrier() == fbrRuleInfo->carrierCode())
      {
        validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat);
        continue;
      } // If carriers are the same

      printDiag411Message(9); //"SKIP PRIME RBD VALIDATION - CARRIERS DO NOT MATCH"

      if (UNLIKELY(smfFbrWithPrimeSector && ruleZeroProcessed &&
                   segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH)))
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
        printDiag411Message(64); // SKIP LOCAL MARKET PROCESSING
        printDiag411BlankLine();
        turnOffNoMatchNotYetProcessedStatus(segStat);
      }
      else
      {
        // Record 6 Convention 1 validation for the Travel Segment
        validateRecord6Conv1(paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, fbrPTFBaseFare);
      }
    } // If Segment status is NoMatch or Not Processed
    else
    {
      printDiag411SegmentStatus(segStat); // BOOKING CODE VALIDATION STATUS -
      // PASS/FAIL/MIXED/NOMATCH
    }

  } // For loop Travel Segment
  validReturnType = setPaxTypeFareBkgCodeStatus(paxTfare);

  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Invoke the Record6 Convention 1 validate method and
// call LocalMarket if it's necessary. Also, do additional
// validation for the airSegment for WPNC/WPNCS entry
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::validateRecord6Conv1(PaxTypeFare& paxTfare,
                                               AirSeg* airSeg,
                                               const std::vector<ClassOfService*>* cosVec,
                                               PaxTypeFare::SegmentStatus& segStat,
                                               std::vector<BookingCode>& bkgCodes,
                                               TravelSeg* seg,
                                               const FBRPaxTypeFareRuleData* fbrPTFBaseFare)
{
  ValidateReturnType validReturnType = RET_PASS;

  bool processSmfRuleZero = false;
  VendorCode vendor = paxTfare.vendor();
  if (paxTfare.vendor() != ATPCO_VENDOR_CODE && paxTfare.vendor() != SITA_VENDOR_CODE)
  {
    if (fbrPTFBaseFare != nullptr && fbrPTFBaseFare->baseFare() != nullptr)
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPTFBaseFare->ruleItemInfo());
      if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
        processSmfRuleZero = true;
      else
        vendor = fbrPTFBaseFare->baseFare()->vendor();
    }
    else
      processSmfRuleZero = true;
  }

  if (processSmfRuleZero)
  {
    bool isRuleZero = false;
    validReturnType =
        validateConvention2(paxTfare, paxTfare, airSeg, isRuleZero, false, processSmfRuleZero);
  }
  else
  {
    validReturnType = validateConvention1(paxTfare, vendor, airSeg);
  }

  if (validReturnType == RET_PRIME && paxTfare.fare()->isIndustry())
  {
    validReturnType = checkAllSectorsForOpen(airSeg, cosVec, segStat, bkgCodes, paxTfare);
  }

  if (validReturnType == RET_NO_MATCH || validReturnType == RET_PRIME)
  {
    if (!processSmfRuleZero)
      printDiag411Message(10); //  "REC 6 CONV 1 VALIDATION STATUS - NOMATCH" ;
  }

  if (validReturnType == RET_PASS)
  {
    if (LIKELY(!processSmfRuleZero))
      setPassBkgCodeSegmentStatus(segStat, 11);
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      printDiag411Message(35); // Rebooked COS
      printDiag411RebookBkg(segStat._bkgCodeReBook);
    }
    turnOffNoMatchNotYetProcessedStatus(segStat);
    return;
  }

  if (validReturnType == RET_FAIL) //  Fail
  {
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_CONV1_T999, true);
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
    if (LIKELY(!processSmfRuleZero))
      printDiag411Message(12); // R6/C1 VALIDATION STATUS - FAIL
    printDiag411BlankLine();
    turnOffNoMatchNotYetProcessedStatus(segStat);
    return;
  }

  if (LIKELY((validReturnType == RET_NO_MATCH || validReturnType == RET_PRIME)))
  {
    if (_trx.dataHandle().getVendorType(paxTfare.vendor()) == 'T')
    {
      // if SMF fare the do not do local market processing
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      IbfAvailabilityTools::setBrandsStatus(
          IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
      printDiag411Message(64); // SKIP LOCAL MARKET PROCESSING
      printDiag411BlankLine();
      turnOffNoMatchNotYetProcessedStatus(segStat);
      return;
    }

    if (_processedLocalMkt)
    {
      if (RexBaseTrx::isRexTrxAndNewItin(_trx))
      {
        if (_diag)
          *_diag << "      REX WP";
        PaxTypeFare::SegmentStatus rexWPsegStat;
        _rexLocalMarketWPvalidation = true;
        bool result = validateLocalMarket(paxTfare, *airSeg, seg, cosVec, rexWPsegStat);
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION, result);
        _rexLocalMarketWPvalidation = false;
        if (_diag)
          *_diag << "      REX WPNC";
      }

      if (validateLocalMarket(paxTfare, *airSeg, seg, cosVec, segStat) && !_newPaxTypeFare)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
      }
      else
      {
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
            segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_LOCALMARKET, true);
      }
      turnOffNoMatchNotYetProcessedStatus(segStat);
    }
    else if (UNLIKELY(_trx.getTrxType() == PricingTrx::FF_TRX))
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_LOCALMARKET, true);
      turnOffNoMatchNotYetProcessedStatus(segStat);
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Retrieve the Convention object and invoke the Record6
// Convention 1 validate method.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateConvention1(PaxTypeFare& paxTfare,
                                              const VendorCode& vendor,
                                              AirSeg* airSeg)
{
  ValidateReturnType returnType = RET_NO_MATCH; // defult setup

  if (UNLIKELY(airSeg == nullptr))
  {
    return returnType;
  }

  printSkipOneLine();
  printDiag411Message(14, REC1); // "REC 6 CONV NUMBER "

  BookingCodeExceptionValidator bce(
      *_itin, _partOfLocalJny, skipFlownSegCat31(), _useBKGExceptionIndex);

  // SITA does not have filed the Rec6 Conv1
  // Need to do it because of Vendor code
  VendorCode vendorA = (vendor == SITA_VENDOR_CODE) ? VendorCode(ATPCO_VENDOR_CODE) : vendor;

  //  Convention 1 does not have the Tariff and Rule as the primary keys.

  TariffNumber ruleTariff = 0;
  RuleNumber rule = "0000";
  bool isRuleZero = false;

  const BookingCodeExceptionSequenceList& bkgExcpSeqList = getBookingCodeExceptionSequence(
      vendorA, airSeg->carrier(), ruleTariff, rule, REC1, isRuleZero);

  if (bkgExcpSeqList.empty())
  {
    printDiag411Message(13); // "TABLE 999 ITEM NUMBER - "

    printVendorCarrier(vendorA, airSeg->carrier());
    printDiag411Count(bkgExcpSeqList.size());
    if (paxTfare.fare()->isIndustry())
    {
      returnType = RET_PRIME;
    }
    printDiag411BlankLine();
    return returnType;
  }

  BCETuning& bceTuning = t999Tuning(bkgExcpSeqList, paxTfare, airSeg, '1');

  printDiag411Message(13); // "  TABLE 999 ITEM NUMBER - "
  printVendorCarrier(vendorA, airSeg->carrier());
  printTable999ItemNumber((*bkgExcpSeqList.begin())->itemNo());
  printDiag411BlankLine();

  // if statusType == STATUS_RULE2 then there is no need to validate Table999 again

  if (_statusType == STATUS_RULE1)
  {
    if (UNLIKELY(!(bce.validate(_trx, bkgExcpSeqList, paxTfare, airSeg, bceTuning, _fu))))
      // Rec6/Convention1 Table 999 validation
      returnType = RET_NO_MATCH;
  }
  else if (_statusType == STATUS_RULE2)
  {
    // if STATUS_RULE2
    // then we know that we put the status of those carriers in segmentStatusRule2
    // Copy that status to segmentStatus now.

    int i = fltIndex(paxTfare, airSeg);
    paxTfare.segmentStatus()[i] = paxTfare.segmentStatusRule2()[i];
  }
  // if status is STATUS_FLOW_FOR_LOCAL_JRNY_CXR then we know that status is in
  // segmentStatusRule2

  if (fallback::fixed::fallbackCheckBckCodeSegStatusFix())
    returnType = CheckBkgCodeSegmentStatusConv1_deprecated(paxTfare, airSeg);
  else
    returnType = CheckBkgCodeSegmentStatusConv1(paxTfare, airSeg); // get segment status

  // SetBkgSegmStatus( paxTfare, REC1 );        // Set fail status - Rec 6 Conv1 Table 999

  return returnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check if the carrier code on all sectors of the FM is YY
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::checkAllSectorsForOpen(AirSeg* airSegL,
                                                 const std::vector<ClassOfService*>* cosVec,
                                                 PaxTypeFare::SegmentStatus& segStat,
                                                 std::vector<BookingCode>& bkgCodes,
                                                 PaxTypeFare& paxTfare)
{
  ValidateReturnType returnType = RET_PRIME; // defult setup

  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t openS = 0;

  for (; iterTvl != iterTvlEnd; iterTvl++)
  {
    AirSeg* airSeg = (*iterTvl)->toAirSeg();
    if (UNLIKELY(airSeg == nullptr))
    {
      openS--;
      continue;
    }

    if (UNLIKELY(airSeg->segmentType() == Open))
    {
      openS++;
      if (airSeg->carrier() != INDUSTRY_CARRIER)
      {
        printDiag411Message(60); // "YY carrier NOT on all sectors
        printDiag411BlankLine();
        return returnType;
      }
    }
  }
  if (UNLIKELY(openS == _mkt->travelSeg().size()))
    printDiag411Message(59); // "YY carrier on all sectors
  if (UNLIKELY(openS == _mkt->travelSeg().size() || _trx.getRequest()->isLowFareNoAvailability()))
  {
    if (validateSectorPrimeRBDRec1(*airSegL, bkgCodes, cosVec, paxTfare, segStat))
    {
      setPassBkgCodeSegmentStatus(segStat, 1);
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      {
        printDiag411Message(35); // Rebooked COS
        printDiag411RebookBkg(segStat._bkgCodeReBook);
      }
    }
    else
    {
      setFailPrimeRBDInternationalStatus(segStat, 2);
    }
  }
  else
    return returnType;

  return returnType = RET_CONTINUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Retrieve the Convention object and invoke the Record6
// Convention 2 validate method for the FBR fare.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateConvention2(PaxTypeFare& paxTfare,
                                              const PaxTypeFare& basePaxTfare,
                                              AirSeg* airSeg,
                                              bool& isRuleZero,
                                              bool cat25PrimeSector,
                                              bool needRuleZero)
{
  ValidateReturnType returnType = RET_NO_MATCH; // defult setup
  StatusReturnType checkRetType = NO_MATCH_NOT_PROCESSED; // assume it ready for processing

  if (UNLIKELY(airSeg == nullptr))
  {
    return returnType;
  }

  printSkipOneLine();
  printDiag411Message(14, CONV_2); // "REC 6 CONV NUMBER "

  BookingCodeExceptionValidator bce(
      *_itin, _partOfLocalJny, skipFlownSegCat31(), _useBKGExceptionIndex);

  const BookingCodeExceptionSequenceList& bkgExcpSeqList =
      getBookingCodeExceptionSequence(basePaxTfare.fare()->vendor(),
                                      basePaxTfare.fare()->carrier(),
                                      basePaxTfare.fare()->tcrRuleTariff(),
                                      (needRuleZero) ? "0000" : basePaxTfare.fare()->ruleNumber(),
                                      CONV_2,
                                      isRuleZero);

  if (bkgExcpSeqList.empty())
  {
    printDiag411Message(13); // "TABLE 999 ITEM NUMBER - "
    printVendorCxrRule(basePaxTfare.fare()->vendor(),
                       basePaxTfare.fare()->carrier(),
                       (isRuleZero || needRuleZero) ? "0000" : basePaxTfare.fare()->ruleNumber());
    printDiag411Count(bkgExcpSeqList.size());
    printDiag411BlankLine();
    if (UNLIKELY(cat25PrimeSector))
      returnType = RET_CONTINUE; // CAT25 Mandates

    return returnType;
  }

  BCETuning& bceTuning = t999Tuning(bkgExcpSeqList, paxTfare, nullptr, '2');

  printDiag411Message(13); // "  TABLE 999 ITEM NUMBER - "
  printVendorCxrRule(basePaxTfare.fare()->vendor(),
                     basePaxTfare.fare()->carrier(),
                     (isRuleZero || needRuleZero) ? "0000" : basePaxTfare.fare()->ruleNumber());
  printDiag411Count((*bkgExcpSeqList.begin())->itemNo());

  printDiag411BlankLine();

  // only process T999 for statusType == STATUS_RULE1

  // during statusType == STATUS_RULE1 we will set the
  // indicators for statusType == STATUS_RULE2 also
  if (_statusType == STATUS_RULE1)
  {
    if (UNLIKELY(!bce.validate(
            _trx, bkgExcpSeqList, paxTfare, nullptr, bceTuning, _fu, false, cat25PrimeSector)))
      // Rec6/Convention2 Table 999 validation
      // The 'false' is for the FareByRule ind. - added for the "RBD T999 Dual Inventory
      // project". This ind is for the R1 T999 Cat25 R3.
      returnType = RET_NO_MATCH;
  }
  else if (UNLIKELY(_statusType == STATUS_RULE2))
  {
    // if STATUS_RULE2
    // then we know that we put the status of those carriers in segmentStatusRule2
    // Copy that status to segmentStatus now.
    t999Rule2Local(paxTfare);
  }

  if (cat25PrimeSector)
    returnType = RET_NO_MATCH; // CAT 25 Mandates need to return to continue processing.

  // don't copy for STATUS_FLOW_FOR_LOCAL_JRNY_CXR

  checkRetType = CheckBkgCodeSegmentStatus(paxTfare); // get segment status

  SetBkgSegmStatus(paxTfare, CONV_2); // Set fail status - Rec 6 Conv2 Table 999

  diag411R6C2Status(paxTfare); // Display statuses for each travel segment

  PaxTypeFare::BookingCodeStatus& ptfStat =
      (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();

  ptfStat.set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);

  switch (checkRetType)
  {
  case PASS:
    ptfStat.set(PaxTypeFare::BKS_PASS); // BKG CODE VALIDATION STATUS  - PASS
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_FLOW_AVAIL);
    }
    else if (LIKELY(_statusType != STATUS_RULE2))
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    }
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    returnType = RET_PASS;
    break;

  case FAIL:
    returnType = RET_FAIL;
    // if command pricing e.g WPQY26 then PASS the fare
    if (UNLIKELY(paxTfare.validForCmdPricing(false)))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS); // BKG CODE VALIDATION STATUS  - PASS
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(41); // COMMAND PRICING RBD STATUS - PASS

      returnType = RET_PASS;
      break;
    }
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR &&
        (ptfStat.isSet(PaxTypeFare::BKS_PASS) || ptfStat.isSet(PaxTypeFare::BKS_MIXED)))
    {
      returnType = RET_PASS;
      break;
    }

    ptfStat.set(PaxTypeFare::BKS_FAIL); // BKG CODE VALIDATION STATUS  - FAIL
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    break;

  case MIXED:
    returnType = RET_MIXED;
    // if command pricing e.g WPQY26 then PASS the fare
    if (UNLIKELY(paxTfare.validForCmdPricing(false)))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS); // BKG CODE VALIDATION STATUS  - PASS
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(43); // COMMAND PRICING RBD STATUS - PASS
      returnType = RET_PASS;
      break;
    }

    if (paxTfare.isNegotiated())
    {
      ptfStat.set(PaxTypeFare::BKS_FAIL);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      returnType = RET_FAIL;
      break;
    }
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      ptfStat.set(PaxTypeFare::BKS_MIXED_FLOW_AVAIL);
      if (ptfStat.isSet(PaxTypeFare::BKS_FAIL))
      {
        ptfStat.set(PaxTypeFare::BKS_PASS);
        ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      }
      returnType = RET_PASS;
      break;
    }

    ptfStat.set(PaxTypeFare::BKS_MIXED); // BKG CODE VALIDATION STATUS  - MIXED
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    if (UNLIKELY(_trx.getOptions() && _trx.getOptions()->isRtw()))
      ptfStat.set(PaxTypeFare::BKS_FAIL);
    else
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    break;

  default:
    returnType = RET_NO_MATCH; // REC 6 CONVENTION 2 VALIDATION STATUS - NOMATCH
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      break;
    }

    ptfStat.set(PaxTypeFare::BKS_NOT_YET_PROCESSED);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    break;
  }

  if ((PricingTrx::IS_TRX == _trx.getTrxType()) &&
      (checkRetType == NONE_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED))
  {
    const CabinType& fareCabin = paxTfare.cabin();
    if (fareCabin.isPremiumEconomyClass() || fareCabin.isEconomyClass())
    {
      ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(&_trx);
      std::vector<ShoppingTrx::Leg>& legs = shoppingTrx->legs();
      ShoppingTrx::Leg currentLeg = legs[(paxTfare.fareMarket())->legIndex()];
      CabinType cab;
      cab.setPremiumEconomyClass();
      if (currentLeg.preferredCabinClass() < cab)
      {
        returnType = RET_FAIL;
      }
    }
  }

  printDiag411BkgCodeStatus(paxTfare);
  printDiag411BlankLine();

  return returnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Retrieve the LocalMarket object from the objectManager and invoke the object's
// validate method.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validateLocalMarket(PaxTypeFare& paxTfare,
                                              AirSeg& airSeg,
                                              TravelSeg* seg,
                                              const std::vector<ClassOfService*>* cosVec,
                                              PaxTypeFare::SegmentStatus& segStat)
{
  printDiag411Message(16); // "LOCAL MARKET VALIDATION FOR"
  printDiag411FaretypeFamily(paxTfare);

  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  std::vector<TravelSeg*> segOrderVec;
  segOrderVec.push_back(seg);

  FareMarket* pFM =
      TrxUtil::getFareMarket(_trx, airSeg.carrier(), segOrderVec, paxTfare.retrievalDate(), _itin);

  if (pFM == nullptr)
  {
    RepricingTrx* reTrx = getRepricingTrx(_trx,
                                          segOrderVec,
                                          paxTfare.directionality() == TO ? FMDirection::INBOUND
                                                                          : FMDirection::UNKNOWN);
    if (!reTrx)
    {
      LOG4CXX_DEBUG(logger, "FareBookingCodeValidator: no repricing trx");
      return false;
    }
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator: repricing trx is ready");

    pFM = TrxUtil::getFareMarket(
        *reTrx, airSeg.carrier(), segOrderVec, paxTfare.retrievalDate(), _itin);
    if (pFM == nullptr)
    {
      printDiag411Message(17); // "STATUS - FAIL, NO PUBLISHED FARES FOR THE TRAVEL SECTOR"
      return false;
    }
  }

  //  Big LOOP
  //  Now it's time to go thru all PaxTypeFares to find the Published Fare
  //  and compare current fare vs published.

  uint16_t numSeats = PaxTypeUtil::totalNumSeats(_trx);
  if (!numSeats || (isItInfantFare(paxTfare, seg) && !_newPaxTypeFare))
  {
    printDiag411Message(20); // "PRIME RBD VALIDATION STATUS - PASS"
    // "LOCAL MARKET VALIDATION STATUS - PASS"
    printDiag411InfantMsg();
    printDiag411BlankLine();
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
    return true;
  }

  if (fallback::fallbackAPM837(&_trx))
  {
    PaxTypeFarePtrVecCI itPTFare = pFM->allPaxTypeFare().begin();

    for (; itPTFare != pFM->allPaxTypeFare().end(); itPTFare++)
    {
      //  Initialize Iterator for the major loop through all Fares for the ONE PaxType
      if ((*itPTFare)->fare()->isPublished() &&
          (*itPTFare)->carrier() == airSeg.carrier()) //    || (*itPTFare)->fare()->isConstructed())
      {
        if ((*itPTFare)->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
          continue;
        // Compare two fares by Fare Type
        if (RuleUtil::matchFareType(paxTfare.fcaFareType(), (*itPTFare)->fcaFareType()))
        {
          printDiag411Message(18);
          printDiag411FareClass(**itPTFare); // "PUBLISHED FARE - "

          (*itPTFare)->getPrimeBookingCode(bkgCodes);

          if (validateSectorPrimeRBDRec1(airSeg, bkgCodes, cosVec, (**itPTFare), segStat))
          {
            printDiag411Message(20); // "PRIME RBD VALIDATION STATUS - PASS"
            // "LOCAL MARKET VALIDATION STATUS - PASS"
            printDiag411Message(19); //   FARE TYPE
            printDiag411FareType(**itPTFare);
            printDiag411BlankLine();
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
            turnOffNoMatchNotYetProcessedStatus(segStat);

            return true;
          }
          printDiag411Message(21); // "PRIME RBD VALIDATION STATUS - FAIL"
        }
      }
    }

    std::ostringstream ss;
    ss << '*' << paxTfare.fcaFareType()[0];
    FareType fareType = ss.str().c_str();

    itPTFare = pFM->allPaxTypeFare().begin();

    int pubFareCount = 0; // Published fares counter
    for (; itPTFare != pFM->allPaxTypeFare().end(); itPTFare++)
    {
      //  Initialize Iterator for the major loop through all Fares for the ONE PaxType
      if ((*itPTFare)->fare()->isPublished() &&
          (*itPTFare)->carrier() == airSeg.carrier()) //    || (*itPTFare)->fare()->isConstructed())
      {
        if ((*itPTFare)->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
          continue;

        pubFareCount++;

        // Compare two fares by FareFamily Type
        if (RuleUtil::matchFareType(fareType, (*itPTFare)->fcaFareType()))
        {
          printDiag411Message(18);
          printDiag411FareClass(**itPTFare); // "PUBLISHED FARE - "

          (*itPTFare)->getPrimeBookingCode(bkgCodes);

          if (validateSectorPrimeRBDRec1(airSeg, bkgCodes, cosVec, (**itPTFare), segStat))
          {
            printDiag411Message(20); // "PRIME RBD VALIDATION STATUS - PASS\n"
            printDiag411Message(22); // "FARE FAMILY - "
            printDiag411BlankLine();
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
            turnOffNoMatchNotYetProcessedStatus(segStat);
            return true;
          }
          printDiag411Message(21); // "PRIME RBD VALIDATION STATUS - FAIL"
          printDiag411BlankLine();
        }
      } //  if Published
    } // for loop for all PaxTypeFare vectors

    printDiag411Message(23); // "LOCAL MARKET VALIDATION STATUS - FAIL\n"
    // " PUBLISHED FARES # - "
    printDiag411Count(pubFareCount);
    printDiag411BlankLine();

    return false;
  }
  else
  {
    // Check all valid PTFs
    if (!validatePTFs(pFM->allPaxTypeFare(), paxTfare, airSeg, cosVec, segStat))
    {
      // Check if we have any failed fares? (e.x failed during footnote prevalidation)
      if (LIKELY(_trx.isFootNotePrevalidationAllowed()) && pFM->footNoteFailedFares().size() > 0)
      {
        // First check cache of failed PTFs
        if (!validatePTFs(pFM->footNoteFailedPaxTypeFares(), paxTfare, airSeg, cosVec, segStat))
        {
          // Last resort is to resolve rest of the invalid Fares to create PTFs
          // and check them also
          Record1Resolver r1resolver(_trx, *_itin, *pFM);
          std::vector<PaxTypeFare*> invalidPTFs;

          for (const auto& fareIter : pFM->footNoteFailedFares())
          {
            if (fareIter.second)
              continue;

            r1resolver.resolveR1s(*(fareIter.first), invalidPTFs);
          }
          return validatePTFs(invalidPTFs, paxTfare, airSeg, cosVec, segStat);
        }
        return true;
      }
      return false;
    }
    return true;
  }
}

bool
FareBookingCodeValidator::validatePTFs(const std::vector<PaxTypeFare*>& fares,
                                       const PaxTypeFare& paxTfare,
                                       AirSeg& airSeg,
                                       const std::vector<ClassOfService*>* cosVec,
                                       PaxTypeFare::SegmentStatus& segStat)
{
  std::vector<BookingCode> bkgCodes;

  for (PaxTypeFare* currentFare : fares)
  {
    //  Initialize Iterator for the major loop through all Fares for the ONE PaxType
    if (currentFare->fare()->isPublished() &&
        currentFare->carrier() == airSeg.carrier()) //    || (*itPTFare)->fare()->isConstructed())
    {
      if (currentFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
        continue;
      // Compare two fares by Fare Type
      if (RuleUtil::matchFareType(paxTfare.fcaFareType(), currentFare->fcaFareType()))
      {
        printDiag411Message(18);
        printDiag411FareClass(*currentFare); // "PUBLISHED FARE - "

        currentFare->getPrimeBookingCode(bkgCodes);

        if (validateSectorPrimeRBDRec1(airSeg, bkgCodes, cosVec, *currentFare, segStat))
        {
          printDiag411Message(20); // "PRIME RBD VALIDATION STATUS - PASS"
          // "LOCAL MARKET VALIDATION STATUS - PASS"
          printDiag411Message(19); //   FARE TYPE
          printDiag411FareType(*currentFare);
          printDiag411BlankLine();
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
          turnOffNoMatchNotYetProcessedStatus(segStat);

          return true;
        }
        printDiag411Message(21); // "PRIME RBD VALIDATION STATUS - FAIL"
      }
    }
  }

  std::ostringstream ss;
  ss << '*' << paxTfare.fcaFareType()[0];
  FareType fareType = ss.str().c_str();

  int pubFareCount = 0; // Published fares counter
  for (PaxTypeFare* currentFare : fares)
  {
    //  Initialize Iterator for the major loop through all Fares for the ONE PaxType
    if (currentFare->fare()->isPublished() &&
        currentFare->carrier() == airSeg.carrier()) //    || (*itPTFare)->fare()->isConstructed())
    {
      if (currentFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
        continue;

      pubFareCount++;

      // Compare two fares by FareFamily Type
      if (RuleUtil::matchFareType(fareType, currentFare->fcaFareType()))
      {
        printDiag411Message(18);
        printDiag411FareClass(*currentFare); // "PUBLISHED FARE - "

        currentFare->getPrimeBookingCode(bkgCodes);

        if (validateSectorPrimeRBDRec1(airSeg, bkgCodes, cosVec, *currentFare, segStat))
        {
          printDiag411Message(20); // "PRIME RBD VALIDATION STATUS - PASS\n"
          printDiag411Message(22); // "FARE FAMILY - "
          printDiag411BlankLine();
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
          turnOffNoMatchNotYetProcessedStatus(segStat);
          return true;
        }
        printDiag411Message(21); // "PRIME RBD VALIDATION STATUS - FAIL"
        printDiag411BlankLine();
      }
    } //  if Published
  } // for loop for all PaxTypeFare vectors

  printDiag411Message(23); // "LOCAL MARKET VALIDATION STATUS - FAIL\n"
  // " PUBLISHED FARES # - "
  printDiag411Count(pubFareCount);
  printDiag411BlankLine();

  return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set up the booking Code status in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::setPaxTypeFareBkgCodeStatus(PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_PASS; // defult setup

  StatusReturnType checkRetType = NO_MATCH_NOT_PROCESSED; // assume it ready for processing

  if (UNLIKELY(_newPaxTypeFare))
  {
    checkRetType = CheckBkgCodeSegmentStatus(*_newPaxTypeFare); // get segment status
    validReturnType = setPTFBkgStatus(*_newPaxTypeFare, checkRetType);
  }
  checkRetType = CheckBkgCodeSegmentStatus(paxTfare); // get segment status
  validReturnType = setPTFBkgStatus(paxTfare, checkRetType);

  return validReturnType;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set up the BookingCode Statuses for each Pass/Fail Travel Segment in the FareMarket
// after T999 validation.
// It'll be done for Rec1T999 and Conv2T999 validation, so far
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::SetBkgSegmStatus(PaxTypeFare& paxTfare, int number)
{
  if (paxTfare.segmentStatus().empty() && number == INIT) // '0')
  {
    // we can predict the final size of these vectors, so reserve space
    paxTfare.segmentStatus().reserve(paxTfare.segmentStatus().size() + _mkt->travelSeg().size());
    paxTfare.segmentStatusRule2().reserve(paxTfare.segmentStatusRule2().size() +
                                          _mkt->travelSeg().size());

    PaxTypeFare::SegmentStatus segStat;

    for (const auto& ts : _mkt->travelSeg())
    {
      segStat._bkgCodeSegStatus.setNull();

      if (ts->isAir())
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }
      paxTfare.segmentStatus().push_back(segStat);
      paxTfare.segmentStatusRule2().push_back(segStat);
    }
  }
  else
  {
    uint16_t i = 0;
    uint16_t j = paxTfare.segmentStatus().size();
    if (_fu)
    {
      j = _fu->segmentStatus().size();
    }

    for (; i < j; i++)
    {
      PaxTypeFare::SegmentStatus& segStat =
          (_fu) ? _fu->segmentStatus()[i] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                 ? paxTfare.segmentStatus()[i]
                                                 : paxTfare.segmentStatusRule2()[i]);

      if (!(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_SURFACE)))
      { // RBD Record 1 Validation
        if (number == REC1 && (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
                               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999)))

        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_T999, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_REC1_T999, true);
        }
        else if (UNLIKELY(number == REC1 &&
                          (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION))))
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NEED_REVALIDATION, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NEED_REVALIDATION_REC1, true);
        }
        else // Convention 2
            if (number == CONV_2 && (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
                                     segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999)))
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_T999, false);

          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_CONV2_T999, true);
        }
        else if (UNLIKELY(number == CONV_2 &&
                          (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION))))

        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NEED_REVALIDATION, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NEED_REVALIDATION_CONV2, true);
        }
        else // Convention 1
            if (UNLIKELY(number == REC1 &&
                         (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
                          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999))))
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_T999, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_CONV1_T999, true);
        } // if/else
        else if (UNLIKELY(number == REC1 &&
                          (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION))))

        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NEED_REVALIDATION, false);
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NEED_REVALIDATION_CONV1, true);
        }
      } // if ! surface
    } // for all segments
  } // else, not initialyze
  return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sets IBF Error Status to both segment and a fare
// This is used to return "not offered/ not available" message
// describing the reason for not returning some options
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::setFailureReasonInSegmentAndFare(BookingCodeValidationStatus ibfStatus,
                                                           PaxTypeFare& paxTfare,
                                                           PaxTypeFare::SegmentStatus& segStat,
                                                           bool priceByCabin)
{
  if (UNLIKELY(!IbfAvailabilityEnumTools::isBcvsError(ibfStatus)))
  {
    if (UNLIKELY(isDiag411Enabled()))
      *(_diag) << "      AVAILABILITY STATUS         - -\n";
    if (_UNLIKELY(priceByCabin))
      printDiag411Message(68); // CABIN NOT MATCH

    return;
  }
  IbfAvailabilityTools::setBrandsStatus(
      IbfAvailabilityEnumTools::toIbfErrorMessage(ibfStatus), paxTfare, _trx, __PRETTY_FUNCTION__);
  if (ibfStatus == BOOKING_CODE_NOT_AVAILABLE)
  {
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_AVAILABILITY);
    if (UNLIKELY(isDiag411Enabled()))
      *(_diag) << "      AVAILABILITY STATUS         - A\n";
  }
  else
  {
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_OFFER);
    if (UNLIKELY(isDiag411Enabled()))
      *(_diag) << "      AVAILABILITY STATUS         - O\n";
  }
  if (_UNLIKELY(priceByCabin))
    printDiag411Message(68); // CABIN NOT MATCH
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Determine the result of T999 validation and Set up PaxTypeFare
// status
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::setUpPaxTFareStatusAfterT999(PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_CONTINUE; // defult

  StatusReturnType checkRetType = NO_MATCH_NOT_PROCESSED; // assume it ready for processing

  // Determine booking Code validation status for each segment in the Fare Market

  checkRetType = CheckBkgCodeSegmentStatus(paxTfare); // get segment status

  // Set - fail Rec1 T999 status
  SetBkgSegmStatus(paxTfare, REC1);
  PaxTypeFare::BookingCodeStatus& ptfStat =
      (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();

  ptfStat.set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);

  diag411T999Rec1(paxTfare);

  switch (checkRetType)
  {
  case PASS:
    printDiag411Message(3); // TABLE 999 VALIDATION STATUS FOR THE FARE - PASS
    ptfStat.set(PaxTypeFare::BKS_PASS);
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_FLOW_AVAIL);
      if (!fallback::fallbackAAExcludedBookingCode(&_trx))
        printDiag411Message(69);
    }
    else if (LIKELY(_statusType != STATUS_RULE2))
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      if (!fallback::fallbackAAExcludedBookingCode(&_trx))
        printDiag411Message(70);
    }
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    return validReturnType = RET_PASS;

  case FAIL:
    // if command pricing e.g WPQY26 then PASS the fare
    if (paxTfare.validForCmdPricing(false))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS);
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(41); // COMMAND PRICING RBD STATUS - PASS
      return validReturnType = RET_PASS;
    }
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR &&
        (ptfStat.isSet(PaxTypeFare::BKS_PASS) || ptfStat.isSet(PaxTypeFare::BKS_MIXED)))
    {
      ptfStat.set(PaxTypeFare::BKS_FAIL_FLOW_AVAIL, true);
      if (!fallback::fallbackAAExcludedBookingCode(&_trx))
        printDiag411Message(71);
      printDiag411Message(3); // TABLE 999 VALIDATION STATUS FOR THE FARE - PASS
      return validReturnType = RET_PASS;
    }
    if (!fallback::fallbackAAExcludedBookingCode(&_trx))
      printDiag411Message(72);
    printDiag411Message(4); // TABLE 999 VALIDATION STATUS FOR THE FARE - FAIL
    ptfStat.set(PaxTypeFare::BKS_FAIL);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);

    return validReturnType = RET_FAIL;

  case MIXED: // segments with PASS & FAIL statuses only
    // if command pricing e.g WPQY26 then PASS the fare
    if (paxTfare.validForCmdPricing(false))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS);
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(43); // COMMAND PRICING RBD STATUS - PASS
      return validReturnType = RET_PASS;
    }

    if (paxTfare.isNegotiated() && !_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
    {
      printDiag411Message(50); // TABLE 999 VALIDATION STATUS FOR NEG FARE - FAIL
      ptfStat.set(PaxTypeFare::BKS_FAIL);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      return validReturnType = RET_FAIL;
    }

    printDiag411Message(5); // TABLE 999 VALIDATION STATUS FOR THE FARE - MIXED
    printDiag411BlankLine();

    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      ptfStat.set(PaxTypeFare::BKS_MIXED_FLOW_AVAIL);
      if (ptfStat.isSet(PaxTypeFare::BKS_FAIL) &&
          !_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
      {
        ptfStat.set(PaxTypeFare::BKS_PASS);
        ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      }
    }
    else
    {
      ptfStat.set(PaxTypeFare::BKS_MIXED);
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
    }
    if (!_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
    {
      return validReturnType = RET_MIXED;
    }
    break;

  default:
    // NOP
    break;
  }
  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Validate all segment's statuses in the FareMarket and set up the status for the
// PaxTypeFare itself

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::StatusReturnType
FareBookingCodeValidator::CheckBkgCodeSegmentStatus(PaxTypeFare& paxTfare)
{
  if (LIKELY(_trx.getRequest() != nullptr))
    return CheckBkgCodeSegmentStatus_NEW(paxTfare);

  bool segFail = false;
  bool segPass = false;
  bool segNoMatchNoProcess = false;

  //  Gets the booking Code statuses for each segment in the Fare Market

  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlE = _mkt->travelSeg().end();

  uint16_t i = 0;
  uint16_t j = paxTfare.segmentStatus().size();

  if (_fu)
    j = _fu->segmentStatus().size();

  if (j != _mkt->travelSeg().size()) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::CheckBkgCodeSegmentStatus():size problem");
    return FAIL;
  }
  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N))
  {
    if (UNLIKELY(isDiag411Enabled()))
    {
      Diag411Collector* diag = dynamic_cast<Diag411Collector*>(_diag);
      if (diag != nullptr)
      {
        diag->displayTagNSegs(paxTfare);
      }
    }
    return FAIL;
  }

  for (; (iterTvl != iterTvlE) && (i < j); iterTvl++, i++)
  {
    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[i] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                               ? paxTfare.segmentStatus()[i]
                                               : paxTfare.segmentStatusRule2()[i]);

    PaxTypeFare::SegmentStatus& segStatn = (_newPaxTypeFare)
                                               ? ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                      ? _newPaxTypeFare->segmentStatus()[i]
                                                      : _newPaxTypeFare->segmentStatusRule2()[i])
                                               : segStat;
    if (_newPaxTypeFare && (_newPaxTypeFare != &paxTfare))
    {
      if (segStatn._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
        segStatn._bkgCodeSegStatus = segStat._bkgCodeSegStatus;
    }

    if (!(*iterTvl)->isAir())
    {
      continue;
    }

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
    {
      segPass = true;
    }
    else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
             segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      segNoMatchNoProcess = true;
    }
    else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION))
    {
      // if any pass already - set fail,    - we need to force it MIXED
      // if any fail already - set pass
      // if no one is set - set both
      if (segPass)
        segFail = true;
      else if (segFail)
        segPass = true;
      else
      {
        segPass = true;
        segFail = true;
      }
    }
    else
      segFail = true;
  }

  // Now it's time to set up the condition of the PaxTypeFare for the future validation or Stop

  if (segPass && segFail && segNoMatchNoProcess)
    return SOME_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED;
  else if (segPass && segFail && !segNoMatchNoProcess)
    return MIXED;
  else if (segPass && !segFail && segNoMatchNoProcess)
    return SOME_PASS_NONE_FAIL_NO_MATCH_NOT_PROCESSED;
  else if (segPass && !segFail && !segNoMatchNoProcess)
    return PASS;
  else if (!segPass && segFail && segNoMatchNoProcess)
    return NONE_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED;
  else if (!segPass && segFail && !segNoMatchNoProcess)
    return FAIL;
  else
    return NO_MATCH_NOT_PROCESSED;
}

FareBookingCodeValidator::StatusReturnType
FareBookingCodeValidator::CheckBkgCodeSegmentStatus_NEW(PaxTypeFare& paxTfare)
{
  bool segFail = false;
  bool segPass = false;
  bool segNoMatchNoProcess = false;

  //  Gets the booking Code statuses for each segment in the Fare Market

  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlE = _mkt->travelSeg().end();

  uint16_t i = 0;
  uint16_t j = paxTfare.segmentStatus().size();

  if (_fu)
    j = _fu->segmentStatus().size();

  if (UNLIKELY(j != _mkt->travelSeg().size())) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::CheckBkgCodeSegmentStatus():size problem");
    return FAIL;
  }
  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N))
  {
    if (UNLIKELY(isDiag411Enabled()))
    {
      Diag411Collector* diag = dynamic_cast<Diag411Collector*>(_diag);
      if (diag != nullptr)
      {
        diag->displayTagNSegs(paxTfare);
      }
    }
    return FAIL;
  }

  BcvsNotOfferedWins prioritizer;
  bool notOfferedOnAllSegments = true;

  for (; (iterTvl != iterTvlE) && (i < j); iterTvl++, i++)
  {
    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[i] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                               ? paxTfare.segmentStatus()[i]
                                               : paxTfare.segmentStatusRule2()[i]);

    PaxTypeFare::SegmentStatus& segStatn = (_newPaxTypeFare)
                                               ? ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                      ? _newPaxTypeFare->segmentStatus()[i]
                                                      : _newPaxTypeFare->segmentStatusRule2()[i])
                                               : segStat;
    if (UNLIKELY(_newPaxTypeFare && (_newPaxTypeFare != &paxTfare)))
    {
      if (segStatn._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
        segStatn._bkgCodeSegStatus = segStat._bkgCodeSegStatus;
    }

    if (!(*iterTvl)->isAir())
    {
      continue;
    }

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_AVAILABILITY))
    {
      prioritizer.updateStatus(BOOKING_CODE_NOT_AVAILABLE);
      notOfferedOnAllSegments = false;
    }
    else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_OFFER))
    {
      prioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED);
    }

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
    {
      segPass = true;
      notOfferedOnAllSegments = false;
    }
    else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
             segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      segNoMatchNoProcess = true;
    }
    else if (UNLIKELY(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION)))
    {
      // if any pass already - set fail,    - we need to force it MIXED
      // if any fail already - set pass
      // if no one is set - set both
      if (segPass)
        segFail = true;
      else if (segFail)
        segPass = true;
      else
      {
        segPass = true;
        segFail = true;
      }
    }
    else
      segFail = true;
  }

  // Now it's time to set up the condition of the PaxTypeFare for the future validation or Stop

  if (segPass && segFail && segNoMatchNoProcess)
    return SOME_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED;

  if (segPass && segFail && !segNoMatchNoProcess)
    return MIXED;

  if (segPass && !segFail && segNoMatchNoProcess)
    return SOME_PASS_NONE_FAIL_NO_MATCH_NOT_PROCESSED;

  if (segPass && !segFail && !segNoMatchNoProcess)
  {
    // We set IbfErrorMessage::IBF_EM_NO_FARE_FOUND for all fares that pass
    // booking code validation. If a fare of such kind
    // participates in an itin which could not be priced,
    // we induce status F for the whole itin. On the other hand,
    // if the itin is successfully priced, this status will
    // be ignored.
    IbfAvailabilityTools::setBrandsStatus(
        IbfErrorMessage::IBF_EM_NO_FARE_FOUND, paxTfare, _trx, __PRETTY_FUNCTION__);
    return PASS;
  }

  if (!segPass && segFail && segNoMatchNoProcess)
    return NONE_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED;

  if (!segPass && segFail && !segNoMatchNoProcess)
  {
    const BookingCodeValidationStatus bcStat = prioritizer.getStatus();
    if (IbfAvailabilityEnumTools::isBcvsError(bcStat))
    {
      IbfAvailabilityTools::setBrandsStatus(
          IbfAvailabilityEnumTools::toIbfErrorMessage(bcStat), paxTfare, _trx, __PRETTY_FUNCTION__);
    }
    if (notOfferedOnAllSegments)
      paxTfare.setNotOfferedOnAllSegments();

    return FAIL;
  }

  return NO_MATCH_NOT_PROCESSED;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check the Booking code segment status for the airSeg that we sent for T999
// validation from Record 6 Convention 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::CheckBkgCodeSegmentStatusConv1(PaxTypeFare& paxTfare,
                                                         AirSeg* pAirSegConv1) // get segment status
{
  ValidateReturnType returnValue = RET_NO_MATCH;
  int i = fltIndex(paxTfare, pAirSegConv1);

  PaxTypeFare::SegmentStatusVec& segStatusVec =
      (_fu) ? _fu->segmentStatus()
            : ((_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR) ? paxTfare.segmentStatusRule2()
                                                               : paxTfare.segmentStatus());

  if (i != -1) // pAirSegConv1 found in paxTfare
  {
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))

    {
      returnValue = RET_PASS;
    }
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
    {
      returnValue = RET_FAIL;
    }
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
    {
      returnValue = RET_NO_MATCH;
    }
  }
  else
  {
    returnValue = RET_NO_MATCH;
  }

  return returnValue;
}

FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::CheckBkgCodeSegmentStatusConv1_deprecated(PaxTypeFare& paxTfare,
                                                                    AirSeg* pAirSegConv1) // get
// segment
// status
{
  ValidateReturnType returnValue = RET_NO_MATCH;
  int i = fltIndex(paxTfare, pAirSegConv1);

  PaxTypeFare::SegmentStatusVec& segStatusVec = (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                    ? paxTfare.segmentStatusRule2()
                                                    : paxTfare.segmentStatus();
  if (_fu)
    segStatusVec = _fu->segmentStatus();

  if (LIKELY(i != -1)) // pAirSegConv1 found in paxTfare
  {
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))

    {
      returnValue = RET_PASS;
    }
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
    {
      returnValue = RET_FAIL;
    }
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
    {
      returnValue = RET_NO_MATCH;
    }
  }
  else
  {
    returnValue = RET_NO_MATCH;
  }

  return returnValue;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Turn off the NOMATCH & NOT_YET_PROCESSED in the Booking Code status
// in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::turnOffNoMatchNotYetProcessedStatus(PaxTypeFare::SegmentStatus& segStat)
{
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, false);
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set Fail RBD Internationalstatus for the Booking code in the
// Travel Segment in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::setFailPrimeRBDInternationalStatus(PaxTypeFare::SegmentStatus& segStat,
                                                             const int number)
{
  turnOffNoMatchNotYetProcessedStatus(segStat);
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL, true);
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
  printDiag411Message(number); // PRIME RBD VALIDATION STATUS - FAIL
  printDiag411BlankLine();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set Prime RBD Domestic status for the Booking Code in the Travel
// Segment in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::setBkgCodePrimeRBDDomesticSegmentStatus(
    PaxTypeFare::SegmentStatus& segStat, const bool bkgFound)
{
  if ((bkgFound) && (!(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
                       segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) ||
                       segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999))))
  {
    setPassBkgCodeSegmentStatus(segStat, 1);
  }
  else if (LIKELY(!bkgFound))
  {
    setFailPrimeRBDDomesticStatus(segStat, 2);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set Fail RBD Domestic status for the Booking Code in the Travel
// Segment in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::setFailPrimeRBDDomesticStatus(PaxTypeFare::SegmentStatus& segStat,
                                                        const int number)
{
  turnOffNoMatchNotYetProcessedStatus(segStat);
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC, true);
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
  printDiag411Message(number); // PRIME RBD VALIDATION STATUS - FAIL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set Pass RBD Domestic status for the Booking Code in the Travel
// Segment in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::setPassBkgCodeSegmentStatus(PaxTypeFare::SegmentStatus& segStat,
                                                      const int number)
{
  turnOffNoMatchNotYetProcessedStatus(segStat);
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
  printDiag411Message(number); // PRIME RBD VALIDATION STATUS - PASS
  printDiag411BlankLine();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printDiagHeader()
{
  if (LIKELY(!isDiag400Enabled() && !isDiag411Enabled()))
    return;

  DiagParamMapVecI endD = _trx.diagnostic().diagParamMap().end();
  DiagParamMapVecI beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);

  std::string specifiedFM("");
  std::string compFM("");

  const LocCode& fmBoardCity =
      FareMarketUtil::getBoardMultiCity(*_mkt, *(_mkt->travelSeg().front()));
  const LocCode& fmOffCity = FareMarketUtil::getOffMultiCity(*_mkt, *(_mkt->travelSeg().back()));
  const LocCode& fmBoardLoc = _mkt->travelSeg().front()->origin()->loc();
  const LocCode& fmOffLoc = _mkt->travelSeg().back()->destination()->loc();

  // procees /FMDFWLON
  if (beginD != endD)
  {
    specifiedFM = (*beginD).second;
    if (!(specifiedFM.empty()))
    {
      const std::string boardCity(specifiedFM.substr(0, 3));
      const std::string offCity(specifiedFM.substr(3, 3));

      if (((boardCity != fmBoardCity) && (boardCity != fmBoardLoc)) ||
          ((offCity != fmOffCity) && (offCity != fmOffLoc)))
        return;
    }
  }

  if (_cat31Validator && _diag)
    _cat31Validator->printDiagHeader(*_diag, _fu);

  printDiag400Header();
  printDiag411Header();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::isDiag400Enabled() const
{
  return (_diag && _diag->diagnosticType() == Diagnostic400);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::isDiag411Enabled() const
{
  return (_diag && _diag->diagnosticType() == Diagnostic411);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printDiag400Header()
{
  if (!isDiag400Enabled())
    return;
  *(_diag) << *_mkt;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printDiag411Header()
{
  if (!isDiag411Enabled())
    return;

  _diag->lineSkip(0);
  *(_diag) << *_mkt;
  _diag->lineSkip(1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printDiag400InvalidFM()
{
  if (!isDiag400Enabled())
    return;
  *(_diag) << "FARE MARKET NOT VALID FOR PRICE BY CABIN - SEE DIAGNOSTIC 185\n";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printDiag411PaxTypeFare(PaxTypeFare* paxTFare)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    std::pair<PricingTrx*, PaxTypeFare*> output(&_trx, paxTFare);
    *(_diag) << output;
    _diag->lineSkip(2); // skip one line =  "\n" ;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printDiag400PaxTypeFare(PaxTypeFare* paxTFare)
{
  if (LIKELY(!isDiag400Enabled()))
    return;

  std::pair<PricingTrx*, PaxTypeFare*> output(&_trx, paxTFare);
  *(_diag) << output;

  if (_cat31Validator)
  {
    if (_cat31Validator->isActive())
    {
      *(_diag) << "              ";
      _cat31Validator->printResultDiag(*_diag, *paxTFare, _resultCat31);
      *(_diag) << " \n";
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printSkipOneLine()
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->lineSkip(1); // skip two lines =  "\n\n" ;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printSkipTwoLines()
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->lineSkip(2); // skip two lines =  "\n\n" ;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::printTable999ItemNumber(const int& itemNumber)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << itemNumber; // "TABLE 999 ITEM NUMBER - "
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printVendorCarrier(const VendorCode& vendor, const CarrierCode& carrier)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << vendor << "/" << carrier << " - ";
  }
}

void
FareBookingCodeValidator::printVendorCxrRule(const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const RuleNumber ruleNumber)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << vendor << "/" << carrier << "/" << ruleNumber << " - ";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411BkgCodeStatus(PaxTypeFare& paxTfare)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    if (_fu == nullptr)
    {
      *(_diag) << paxTfare.bookingCodeStatus();

      if (_trx.getRequest()->isBrandedFaresRequest())
      {
        _diag->printIbfErrorMessage(paxTfare.getIbfErrorMessage());
      }
    }
    else
      *(_diag) << _fu->bookingCodeStatus();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411SegmentStatus(PaxTypeFare::SegmentStatus& segStat)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << segStat;
    printDiag411BlankLine();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411PrimeRBD(std::vector<BookingCode>& bkgCodes)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << bkgCodes;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411CAT25PrimeRBD(std::vector<BookingCode>& bkgCodes)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << "\n        CAT25 RESULTING FARE PRIME RBD BKG CODES - ";

    if (bkgCodes.size() != 0)
    {
      for (const auto& bkgCode : bkgCodes)
      {
        *(_diag) << bkgCode << " ";
      }
    }
    else
      *(_diag) << "N/A";

    *(_diag) << "\n";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411TravelSegInfo(const TravelSeg* tvl)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->lineSkip(1); // skip one line =  "\n" ;
    DiagCollector& dc = (DiagCollector&)*_diag;
    dc << "      TRAVEL SEGMENT - ";
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvl);

    if (airSeg != nullptr)
    {
      dc << airSeg->carrier() << " ";

      dc.unsetf(std::ios::left);
      dc.setf(std::ios::right, std::ios::adjustfield);

      if (airSeg->flightNumber() == 0)
        dc << std::setw(5) << "OPEN";
      else

        dc << std::setw(5) << airSeg->flightNumber();

      dc << airSeg->getBookingCode() << " " << airSeg->origAirport() << "-"
         << airSeg->destAirport();

      if (RexBaseTrx::isRexTrxAndNewItin(_trx) && !airSeg->unflown() && skipFlownSegCat31())
        dc << " FLOWN-SKIP RBD ";
      dc << " \n";
    }
    else
    {
      if (tvl->isAir())
        dc << "- OPEN - \n";
      else
        dc << "- SURFACE - \n";
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411Message(const int number, char ch)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->printMessages(number, ch);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411FareClass(PaxTypeFare& paxTFare)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << paxTFare.fareClass() << "   /FARE TYPE - " << paxTFare.fcaFareType() << "/";
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411InfantMsg()
{
  if (isDiag411Enabled())
  {
    *(_diag) << "\n            INFANT WITHOUT A SEAT PSGR\n";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411BaseFareMsg()
{
  if (UNLIKELY(isDiag411Enabled()))

  {
    *(_diag) << "\n      START VALIDATING THE BASE FARE\n";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::diag411PrimarySectorMsg()
{
  if (isDiag411Enabled())

  {
    *(_diag) << "\n      PRIMARY SECTOR VALIDATION \n";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::diag411SecondarySectorMsg()
{
  if (isDiag411Enabled())

  {
    *(_diag) << "\n      SECONDARY SECTOR VALIDATION";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411FareType(PaxTypeFare& paxTFare)
{
  if (isDiag411Enabled())
  {
    *(_diag) << paxTFare.fcaFareType();
    printSkipOneLine();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411FaretypeFamily(PaxTypeFare& paxTFare)
{
  if (isDiag411Enabled())

  {
    *(_diag) << paxTFare.fcaFareType();
    printDiag411BlankLine();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411Count(const int& number)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << number;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411RebookBkg(const BookingCode& bkg)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << bkg;
    _diag->lineSkip(1);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411GovernCXR(const CarrierCode& cxr)
{
  if (isDiag411Enabled())
  {
    *(_diag) << cxr;
    _diag->lineSkip(1);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411ChangeYYBkgCode(PaxTypeFare& paxTfare, bool ind)
{
  if (isDiag411Enabled())
  {
    if (ind)
    {
      if (!paxTfare.getChangeFareBasisBkgCode().empty())
      {
        printDiag411Message(34); // "SEQUENCE FOR THE YY FARE IS FOUND, CXR - "
        printDiag411GovernCXR(paxTfare.fareMarket()->governingCarrier());
        printDiag411Message(45); // "1ST CHAR OF THE FARE BASIS CHANGED TO - "
        printDiag411RebookBkg(paxTfare.getChangeFareBasisBkgCode());
        printDiag411BlankLine();
      }
      else
      {
        printDiag411Message(34); // "SEQUENCE FOR THE YY FARE IS FOUND, CXR - "
        printDiag411GovernCXR(paxTfare.fareMarket()->governingCarrier());
        printDiag411Message(40); // "CANDIDATE TO CHANGE 1-ST CHAR WAS NOT FOUND"
        printDiag411BlankLine();
      }
    }
    else
    {
      printDiag411Message(47); // "TABLE 990 EXISTS, USE PRIME RBD TO CHANGE"
      printDiag411Message(45); // "1ST CHAR OF THE FARE BASIS CHANGED TO - "
      printDiag411RebookBkg(paxTfare.getChangeFareBasisBkgCode());
      printDiag411BlankLine();
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::printDiag411BlankLine()
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    *(_diag) << "                          \n";
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::finishDiag()
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->flushMsg();
    _diag->disable(Diagnostic411);
    _diag = nullptr;
    return;
  }

  if (UNLIKELY(isDiag400Enabled()))
  {
    if (_t999TuningDiag)
      diagT999Tuning();
    _diag->flushMsg();
    _diag->disable(Diagnostic400);
    _diag = nullptr;
    return;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::foundFareForDiagnostic(PaxTypeFare& paxTFare)
{
  // Try to find the fareClassCode in the allPaxTypeFare vector
  // if it's provided in the _trx.request->fareClassCode()

  if (UNLIKELY(_trx.diagnostic().isActive() &&
               ((_trx.diagnostic().diagnosticType() == Diagnostic400) ||
                (_trx.diagnostic().diagnosticType() == Diagnostic411))))
  {
    DiagParamMapVecI endD = _trx.diagnostic().diagParamMap().end();
    DiagParamMapVecI beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);

    size_t len = 0;
    std::string specifiedFM("");
    std::string compFM("");

    const LocCode& fmBoardCity =
        FareMarketUtil::getBoardMultiCity(*_mkt, *(_mkt->travelSeg().front()));
    const LocCode& fmOffCity = FareMarketUtil::getOffMultiCity(*_mkt, *(_mkt->travelSeg().back()));
    const LocCode& fmBoardLoc = _mkt->travelSeg().front()->origin()->loc();
    const LocCode& fmOffLoc = _mkt->travelSeg().back()->destination()->loc();

    // procees /FMDFWLON
    if (beginD != endD)
    {
      specifiedFM = (*beginD).second;
      if (!(specifiedFM.empty()))
      {
        //        compFM = paxTFare.fareMarket()->boardMultiCity() +
        //                 paxTFare.fareMarket()->offMultiCity();
        const std::string boardCity(specifiedFM.substr(0, 3));
        const std::string offCity(specifiedFM.substr(3, 3));

        if (((boardCity != fmBoardCity) && (boardCity != fmBoardLoc)) ||
            ((offCity != fmOffCity) && (offCity != fmOffLoc)))
          return false;
      }
    }

    // process /FCY26
    beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
    if (beginD != endD)
    {
      len = ((*beginD).second).size();
      if (len != 0)
      {
        if (((*beginD).second).substr(0, len) != paxTFare.fareClass())
          return false;
      }
    }

    // process /FBY26
    beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);

    if (beginD != endD)
    {
      len = ((*beginD).second).size();
      if (len != 0)
      {
        if (((*beginD).second).substr(0, len) != paxTFare.createFareBasis(_trx, false))
          return false;
      }
    }
  }
  return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
FareBookingCodeValidator::createDiagnostic()
{
  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (UNLIKELY((diagType == Diagnostic400) || (diagType == Diagnostic411)))
  {
    _diag = DCFactory::instance()->create(_trx);

    if (_diag == nullptr)
      return;

    _diag->enable(Diagnostic400, Diagnostic411);

    if (diagType == Diagnostic400)
    {
      DiagParamMapVecI endD = _trx.diagnostic().diagParamMap().end();

      DiagParamMapVecI beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
      _t999TuningDiag = false;
      _journeyDiag = false;
      // procees /DDTUNE999 and /DDJOURNEY
      if (beginD != endD)
      {
        if (!((*beginD).second.empty()))
        {
          if ((*beginD).second == "TUNE999")
            _t999TuningDiag = true;
          if ((*beginD).second == "JOURNEY")
            _journeyDiag = true;
        }
      }
      beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
      std::string specifiedFM("");
      std::string compFM("");

      const LocCode& fmBoardCity =
          FareMarketUtil::getBoardMultiCity(*_mkt, *(_mkt->travelSeg().front()));
      const LocCode& fmOffCity =
          FareMarketUtil::getOffMultiCity(*_mkt, *(_mkt->travelSeg().back()));
      const LocCode& fmBoardLoc = _mkt->travelSeg().front()->origin()->loc();
      const LocCode& fmOffLoc = _mkt->travelSeg().back()->destination()->loc();

      // procees /FMDFWLON
      if (beginD != endD)
      {
        specifiedFM = (*beginD).second;
        if (!(specifiedFM.empty()))
        {
          const std::string boardCity(specifiedFM.substr(0, 3));
          const std::string offCity(specifiedFM.substr(3, 3));

          if (((boardCity != fmBoardCity) && (boardCity != fmBoardLoc)) ||
              ((offCity != fmOffCity) && (offCity != fmOffLoc)))
          {
            _t999TuningDiag = false;
            _journeyDiag = false;
          }
        }
      }
    }
  }
}

/*******************************************************************************************/

void
FareBookingCodeValidator::setFinalPTFBkgStatus(PaxTypeFare& paxTfare)
{
  bool ind = false;
  const CabinType& fareCabin = paxTfare.cabin();

  if (_trx.getRequest()->isLowFareRequested()) // WPNC entry
  {
    if (paxTfare.isNormal() && setIndforWPBkgStatus(paxTfare))
    {
      if (!_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
      {
        if ((fareCabin.isEconomyClass() && (!paxTfare.isFareByRule())) ||
            (fareCabin.isFirstClass()))
          ind = true;

        else if (fareCabin.isBusinessClass())
        {
          if (failedSegInEconomy(paxTfare))
            ind = true;
          else if (!paxTfare.isFareByRule())
            ind = true;
        }
      }
    }
    else // Special fare
        if (!paxTfare.isNormal())
    {
      if ((!_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) &&
          (fareCabin.isBusinessClass() || fareCabin.isEconomyClass()))
        ind = true;
    }
    if (ind)
      setFinalBkgStatus(paxTfare);
  }
  else // WP entry
  {
    if (setIndforWPBkgStatus(paxTfare))
      setFinalBkgStatus(paxTfare);

    if (_newPaxTypeFare)
    {
      if (setIndforWPBkgStatus(*_newPaxTypeFare))
        setFinalBkgStatus(paxTfare);
      _newPaxTypeFare = nullptr;
    }
  }
}

/*******************************************************************************************/

bool
FareBookingCodeValidator::failedSegInEconomy(PaxTypeFare& paxTfare)
{
  //  typedef vector<TravelSeg*>::const_iterator      TravelSegPtrVecIC;
  using SegmentStatusVecCI = std::vector<PaxTypeFare::SegmentStatus>::const_iterator;
  TravelSegPtrVecIC iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecIC iterTvlEnd = _mkt->travelSeg().end();
  SegmentStatusVecCI iterSegStat = paxTfare.segmentStatus().begin();
  SegmentStatusVecCI iterSegStatEnd = paxTfare.segmentStatus().end();

  if (_fu)
  {
    iterSegStat = _fu->segmentStatus().begin();
    iterSegStatEnd = _fu->segmentStatus().end();
  }

  for (; (iterTvl != iterTvlEnd) && (iterSegStat != iterSegStatEnd); iterTvl++, iterSegStat++)
  {
    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;
    const AirSeg* pAirSeg = dynamic_cast<const AirSeg*>(*iterTvl);
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_SURFACE) || pAirSeg == nullptr)
    {
      continue;
    }
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
    {
      if (!(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)) &&
          pAirSeg->bookedCabin().isEconomyClass())
      {
        return true;
      }
    }
  }
  return false;
}

/*******************************************************************************************/
bool
FareBookingCodeValidator::tryRule2(PaxTypeFare& paxTfare)
{
  // if its not a WPNC entry - do not try Rule2
  if (!(_trx.getRequest()->isLowFareRequested()))
    return false;

  if (LIKELY(_trx.getTrxType() != PricingTrx::PRICING_TRX))
    return false;

  if (!(_trx.getOptions()->soloActiveForPricing()))
    return false;

  // if only one flight in this fare component then no need to try Rule 2
  uint16_t airSegSize = paxTfare.fareMarket()->travelSeg().size();
  if (airSegSize < 2)
    return false;

  // Only try Rule 2 if :
  // 1. All the flights of the carriers for which Rule2 is activated and
  //    Journey not activated did not PASS
  // 2. All NON Rule2 carrier flights passed

  TravelSegVectorCI tvlI = paxTfare.fareMarket()->travelSeg().begin();
  TravelSegVectorCI tvlE = paxTfare.fareMarket()->travelSeg().end();

  PaxTypeFare::SegmentStatusVec& segStatusVec = paxTfare.segmentStatus();

  if (_newPaxTypeFare)
    segStatusVec = _newPaxTypeFare->segmentStatus();

  if (_fu)
  {
    segStatusVec = _fu->segmentStatus();
  }

  bool allFlightsPassed = true;
  bool nonRule2CarrierFailed = false;
  bool atleastOneSoloCxrPresent = false;
  uint16_t i = 0;
  AirSeg* airSeg = nullptr;
  for (; i < airSegSize && tvlI != tvlE; tvlI++, i++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;
    if ((*tvlI)->carrierPref() == nullptr)
      return false;

    if ((*tvlI)->carrierPref()->availabilityApplyrul2st() == NO && !airSeg->localJourneyCarrier() &&
        !airSeg->flowJourneyCarrier()) // db availabilityIgrul2st
    {
      atleastOneSoloCxrPresent = true;
      if (!(segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS)))
        allFlightsPassed = false;
    }
    else
    {
      // if any non Rule 2 carrier failed then do not try Rule2
      if (!(segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS)))
        nonRule2CarrierFailed = true;
    }
  }

  if (!atleastOneSoloCxrPresent)
    return false;

  if (allFlightsPassed || nonRule2CarrierFailed)
    return false;

  return true;
}

//-------------------------------------------------------------------
void
FareBookingCodeValidator::t999Rule2Local(PaxTypeFare& paxTfare)
{
  if (_fu)
    return;

  uint16_t i = 0;
  for (const auto* tvlSeg : paxTfare.fareMarket()->travelSeg())
  {
    const auto* airSeg = tvlSeg->toAirSeg();

    if (airSeg && tvlSeg->carrierPref() && tvlSeg->carrierPref()->availabilityApplyrul2st() == NO &&
        !airSeg->flowJourneyCarrier() && !airSeg->localJourneyCarrier()) // db availabilityIgrul2st
    {
      paxTfare.segmentStatus()[i] = paxTfare.segmentStatusRule2()[i];
    }
    ++i;
  }
}

//-------------------------------------------------------------------
int
FareBookingCodeValidator::fltIndex(const PaxTypeFare& paxTfare, const AirSeg* airSeg) const
{
  const auto& tvlSegVect = paxTfare.fareMarket()->travelSeg();
  const auto it = std::find(tvlSegVect.cbegin(), tvlSegVect.cend(), airSeg);
  if (it != tvlSegVect.cend())
    return std::distance(tvlSegVect.cbegin(), it);
  return -1;
}

//-------------------------------------------------------------------
bool
FareBookingCodeValidator::flowJourneyCarrier(TravelSeg* tvlSeg)
{
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(_trx.getOptions()->journeyActivatedForPricing()))
      return false;
  }
  else if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(_trx.getOptions()->journeyActivatedForShopping()))
      return false;
  }

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return false;
  // do not do FLOW Journey logic if carrier does not want it
  const AirSeg* airSeg = (tvlSeg)->toAirSeg();
  if (LIKELY(airSeg != nullptr))
  {
    if (airSeg->flowJourneyCarrier())
      return true;
  }

  return false;
}

//-------------------------------------------------------------------

std::vector<ClassOfService*>*
FareBookingCodeValidator::flowMarketAvail(TravelSeg* tvlSeg,
                                          PaxTypeFare::SegmentStatus& segStat,
                                          const uint16_t iCOS)
{
  std::vector<ClassOfService*>* cosVec = nullptr;
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    cosVec = pricingAvail(tvlSeg);
  }
  else if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    cosVec = shoppingAvail(tvlSeg, segStat, iCOS);
  }

  return cosVec;
}

//-------------------------------------------------------------------
std::vector<ClassOfService*>*
FareBookingCodeValidator::pricingAvail(TravelSeg* tvlSeg)
{
  if (_trx.itin().size() > 1)
    return nullptr;

  Itin& itin = *_trx.itin().front();
  return JourneyUtil::availability(tvlSeg, &itin);
}

//-------------------------------------------------------------------
std::vector<ClassOfService*>*
FareBookingCodeValidator::shoppingAvail(TravelSeg* tvlSeg,
                                        PaxTypeFare::SegmentStatus& segStat,
                                        const uint16_t iCOS)
{
  bool useFmAvail = iCOS < _mkt->classOfServiceVec().size();
  std::vector<ClassOfService*>* cosVec = useFmAvail ? _mkt->classOfServiceVec()[iCOS] : nullptr;

  if (flowJourneyCarrier(tvlSeg) && !_mkt->flowMarket())
  {
    if (partOfJourney(tvlSeg))
    {
      cosVec = &ShoppingUtil::getMaxThruClassOfServiceForSeg(_trx, tvlSeg);
      if (LIKELY(cosVec != nullptr))
      {
        if (cosVec->empty())
          cosVec = nullptr;
      }
    }
  }
  else if (localJourneyCarrier(tvlSeg) && !_mkt->flowMarket())
  {
    if (partOfJourney(tvlSeg))
      cosVec = &ShoppingUtil::getMaxThruClassOfServiceForSeg(_trx, tvlSeg);
    if (LIKELY(cosVec != nullptr))
    {
      if (cosVec->empty())
        cosVec = nullptr;
    }
  }

  return cosVec;
}

//-------------------------------------------------------------------
bool
FareBookingCodeValidator::tryLocalWithFlowAvail(const PaxTypeFare& paxTypeFare) const
{
  // do not try local with flowAvail if the customer does not want it
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(_trx.getOptions()->journeyActivatedForPricing()))
      return false;

    if (!(_trx.getOptions()->applyJourneyLogic())) // do not move outside if-else : tuning reason
      return false;

    if (!_partOfLocalJny)
      return false;
  }
  else if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(_trx.getOptions()->journeyActivatedForShopping()))
      return false;

    if (!(_trx.getOptions()->applyJourneyLogic())) // do not move outside if-else : tuning reason
      return false;

    if (!_partOfLocalJny)
      return false;

    if (UNLIKELY(!journeyExistInItin()))
      return false;
  }
  else
  {
    return false;
  }

  return true;
}

//-------------------------------------------------------------------
bool
FareBookingCodeValidator::localJourneyCarrier(const TravelSeg* tvlSeg) const

{
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(_trx.getOptions()->journeyActivatedForPricing()))
      return false;
  }
  else if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(_trx.getOptions()->journeyActivatedForShopping()))
      return false;
  }

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return false;

  if (!_partOfLocalJny)
    return false;

  // do not do LOCAL Journey logic if carrier does not want it
  const AirSeg* airSeg = (tvlSeg)->toAirSeg();
  if (LIKELY(airSeg != nullptr))
  {
    if (airSeg->localJourneyCarrier())
      return true;
  }

  return false;
}

//-------------------------------------------------------------------
void
FareBookingCodeValidator::setAvailBreaks(PaxTypeFare& paxTypeFare) const
{
  // if its not a WPNC entry then no need to do this
  if (!(_trx.getRequest()->isLowFareRequested()))
    return;

  if (paxTypeFare.fareMarket()->availBreaks().size() !=
      paxTypeFare.fareMarket()->travelSeg().size())
    paxTypeFare.fareMarket()->availBreaks().resize(paxTypeFare.fareMarket()->travelSeg().size());
  uint16_t i = 0;
  uint16_t seg = 0;
  uint16_t noTvlSegs = _mkt->travelSeg().size();

  if (_fu)
    seg = _fu->segmentStatus().size();
  else
    seg = paxTypeFare.segmentStatus().size();

  if (UNLIKELY(noTvlSegs != seg))
    return;

  for (; i < noTvlSegs; i++)
  {
    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[i] : paxTypeFare.segmentStatus()[i];

    if (_mkt->availBreaks()[i])
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
  }
}

//-------------------------------------------------------------------
void
FareBookingCodeValidator::diag411T999Rec1(PaxTypeFare& paxTfare)
{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->lineSkip(1); // skip one line =  "\n" ;
    DiagCollector& dc = (DiagCollector&)*_diag;
    dc << "        STATUS AFTER TABLE 999 REC1 PROCESS \n";

    TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
    TravelSegPtrVecI iterTvlE = _mkt->travelSeg().end();

    uint16_t i = 0;
    uint16_t j = paxTfare.segmentStatus().size();

    if (j != _mkt->travelSeg().size()) // prevent segmentation violation
    {
      dc << "        ERROR - TRAVELSEG SIZE NOT EQUALS SEGMENT STATUS SIZE \n";
      return;
    }

    for (; (iterTvl != iterTvlE) && (i < j); iterTvl++, i++)
    {
      PaxTypeFare::SegmentStatus& segStat = (_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                ? paxTfare.segmentStatus()[i]
                                                : paxTfare.segmentStatusRule2()[i];
      if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
        dc << "        PROCESSING LOCAL JRNY CXR WITH FLOW AVAIL \n";

      printDiag411TravelSegInfo(*iterTvl);
      if (_trx.getRequest()->isLowFareRequested() &&
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS) &&
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      {
        dc << "           REBOOK COS  - " << segStat._bkgCodeReBook << " \n";
      }

      if (!(*iterTvl)->isAir())
        continue;
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
        printDiag411Message(52);
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999))
        printDiag411Message(53);
      else
        printDiag411Message(54);
    }
  }
}

//-------------------------------------------------------------------
void
FareBookingCodeValidator::diag411R6C2Status(PaxTypeFare& paxTfare)

{
  if (UNLIKELY(isDiag411Enabled()))
  {
    _diag->lineSkip(1); // skip one line =  "\n" ;
    DiagCollector& dc = (DiagCollector&)*_diag;
    dc << "      STATUS OF R6/CONV2 TABLE 999 PROCESS \n";

    AirSeg* airSeg = nullptr;

    TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
    TravelSegPtrVecI iterTvlE = _mkt->travelSeg().end();

    uint16_t i = 0;
    uint16_t j = paxTfare.segmentStatus().size();

    if (j != _mkt->travelSeg().size()) // prevent segmentation violation
    {
      dc << "      ERROR - TRAVELSEG SIZE NOT EQUALS SEGMENT STATUS SIZE \n";
      return;
    }

    for (; (iterTvl != iterTvlE) && (i < j); iterTvl++, i++)
    {
      PaxTypeFare::SegmentStatus& segStat = (_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                ? paxTfare.segmentStatus()[i]
                                                : paxTfare.segmentStatusRule2()[i];
      if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
        dc << "      PROCESSING LOCAL JRNY CXR WITH FLOW AVAIL \n";

      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      {
        printDiag411Message(56);
      }
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) &&
               !segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL) &&
               !segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC))
        printDiag411Message(57);
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) &&
               (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL) ||
                segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC)))
        printDiag411Message(67);
      else
        printDiag411Message(58);

      airSeg = dynamic_cast<AirSeg*>(*iterTvl);

      if (airSeg != nullptr)
      {
        dc << airSeg->carrier() << " ";

        dc.unsetf(std::ios::left);
        dc.setf(std::ios::right, std::ios::adjustfield);

        if (airSeg->flightNumber() == 0)
          dc << std::setw(5) << "OPEN";
        else
          dc << std::setw(5) << airSeg->flightNumber();

        dc << airSeg->getBookingCode() << " " << airSeg->origAirport() << "-"
           << airSeg->destAirport();

        if (_trx.getRequest()->isLowFareRequested() &&
            segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS) &&
            segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))

        {
          dc << "  REBOOK - " << segStat._bkgCodeReBook;
        }

        if (skipFlownSegCat31() && !airSeg->unflown())
          dc << " FLOWN-SKIP RBD ";
        dc << " \n";
      }
      else
      {
        if ((*iterTvl)->isAir())
          dc << "- OPEN - \n";
        else
          dc << "- SURFACE - \n";
      }
    }
  }
}

//------------------------------------------------------------------
void
FareBookingCodeValidator::diagT999Tuning()
{
  DiagCollector& dc = (DiagCollector&)*_diag;
  dc << " \n";
  dc << "************ TABLE 999 REUSE DATA FOR TUNING ************** \n";
  dc << "FARE MARKET: " << _mkt->boardMultiCity() << "-" << _mkt->offMultiCity() << " \n";
  dc << "ITEM      NUM OF FARES  CONV          FARE              \n";

  uint16_t numItems = _bceTuningData.size();
  if (numItems < 1)
  {
    dc << " \n";
    dc << "*********************************************************** \n";
    return;
  }

  for (uint16_t j = 0; j < numItems; j++)
  {
    dc << _bceTuningData[j]._itemNo << "      " << std::setw(4) << _bceTuningData[j]._numRepeat
       << "        ";
    if (_bceTuningData[j]._convNum == '1')
    {
      dc << "1";
      if (_bceTuningData[j]._airSeg != nullptr)
      {
        dc << " - " << _bceTuningData[j]._airSeg->carrier() << " " << std::setw(4)
           << _bceTuningData[j]._airSeg->flightNumber();
      }
    }
    else if (_bceTuningData[j]._convNum == '2')
    {
      dc << "2          ";
    }
    else
    {
      dc << "REC1       ";
    }
    dc << "    " << _bceTuningData[j]._fareClass;
    dc << " \n";
  }

  dc << "*********************************************************** \n";
}

//------------------------------------------------------------------
BCETuning&
FareBookingCodeValidator::t999Tuning(const BookingCodeExceptionSequenceList& bceList,
                                     const PaxTypeFare& paxTfare,
                                     AirSeg* airSeg,
                                     Indicator convNo)
{
  if (_trx.getTrxType() == PricingTrx::IS_TRX)
  {
    static BCETuning dummy;
    return dummy;
  }
  uint16_t i = 0;
  uint16_t vecSize = 0;
  bool itemFound = false;
  vecSize = _bceTuningData.size();
  for (i = 0; i < vecSize; i++)
  {
    if ((*bceList.begin())->itemNo() != _bceTuningData[i]._itemNo)
      continue;
    if (convNo == '1')
    {
      if (UNLIKELY(_bceTuningData[i]._convNum != '1'))
        continue;
      if (airSeg == _bceTuningData[i]._airSeg)
      {
        itemFound = true;
        break;
      }
    }
    else // either Rec1 or Rec6 convention 1
    {
      itemFound = true;
      break;
    }
  }

  if (itemFound)

  {
    _bceTuningData[i]._numRepeat += 1;
    return _bceTuningData[i];
  }

  BCETuning bceItem;
  _bceTuningData.push_back(bceItem);
  _bceTuningData[vecSize]._itemNo = (*bceList.begin())->itemNo();
  _bceTuningData[vecSize]._convNum = convNo;
  _bceTuningData[vecSize]._airSeg = airSeg;
  _bceTuningData[vecSize]._fareClass = paxTfare.fareClass();
  _bceTuningData[vecSize]._numRepeat = 1;
  _bceTuningData[vecSize]._sequencesProcessed.resize(bceList.size());
  return _bceTuningData[vecSize];
}

//-----------------------------------------------------------------
void
FareBookingCodeValidator::diagJourney(PaxTypeFare& paxTfare)
{
  if (LIKELY(!isDiag400Enabled()))
    return;
  if (_journeyDiag == false)
    return;

  // process /FCY26
  DiagParamMapVecI endD = _trx.diagnostic().diagParamMap().end();
  DiagParamMapVecI beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  size_t len = 0;
  if (beginD != endD)
  {
    len = ((*beginD).second).size();
    if (len != 0)
    {
      if (((*beginD).second).substr(0, len) != paxTfare.fareClass())
        return;
    }
  }

  DiagCollector& dc = (DiagCollector&)*_diag;
  dc << " \n";
  dc << "********************* JOURNEY DIAG ************************ \n";
  dc << "PAXTYPE FARE: " << paxTfare.fareClass() << "    FARE MARKET: " << _mkt->boardMultiCity()
     << "-" << _mkt->offMultiCity() << " \n";
  dc << "BOOKING CODE STATUS: \n";
  // if(paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
  //  dc << "  BKS NOT YET PROCESSED: T";
  // else
  //  dc << "  BKS NOT YET PROCESSED: F";

  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
    dc << "  BKS FAIL: T";
  else
    dc << "  BKS FAIL: F";

  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))

    dc << "  BKS PASS: T";
  else
    dc << "  BKS PASS: F";

  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED))
    dc << "  BKS MIXED: T \n";
  else
    dc << "  BKS MIXED: F \n";

  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS_LOCAL_AVAIL))
    dc << "  BKS PASS LOCAL AVAIL: T";
  else
    dc << "  BKS PASS LOCAL AVAIL: F";

  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS_FLOW_AVAIL))
    dc << "  BKS PASS FLOW AVAIL: T \n";
  else
    dc << "  BKS PASS FLOW AVAIL: F \n";

  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
    dc << "  BKS MIXED FLOW AVAIL: T \n";
  else
    dc << "  BKS MIXED FLOW AVAIL: F \n";

  dc << " \n";
  dc << "SEGMENT STATUS VECTOR: \n";
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t i = 0;
  for (; ((iterTvl != iterTvlEnd) && i < paxTfare.segmentStatus().size()); iterTvl++, i++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat = paxTfare.segmentStatus()[i];
    dc << "  FLIGHT: ";
    if (airSeg)
      dc << airSeg->carrier() << " " << airSeg->flightNumber() << " \n";
    else
      dc << "ARUNK"
         << " \n";
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      dc << "    BKSS PASS: T";
    else
      dc << "    BKSS PASS: F";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
      dc << "  BKSS FAIL: T";
    else
      dc << "  BKSS FAIL: F";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
      dc << "  BKSS NOMATCH: T \n";
    else
      dc << "  BKSS NOMATCH: F \n";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
      dc << "    BKSS AVAIL BREAK: T";
    else
      dc << "    BKSS AVAIL BREAK: F";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      dc << "  BKSS REBOOKED: T - ";
    else
      dc << "  BKSS REBOOKED: F - ";
    dc << segStat._bkgCodeReBook << " \n";
  }

  dc << " \n";
  dc << "SEGMENT STATUS VECTOR2: \n";
  iterTvl = _mkt->travelSeg().begin();
  i = 0;
  for (; ((iterTvl != iterTvlEnd) && i < paxTfare.segmentStatusRule2().size()); iterTvl++, i++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
    PaxTypeFare::SegmentStatus& segStat = paxTfare.segmentStatusRule2()[i];
    dc << "  FLIGHT: ";
    if (airSeg)
      dc << airSeg->carrier() << " " << airSeg->flightNumber() << " \n";
    else
      dc << "ARUNK"
         << " \n";
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      dc << "    BKSS PASS: T";
    else
      dc << "    BKSS PASS: F";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
      dc << "  BKSS FAIL: T";
    else
      dc << "  BKSS FAIL: F";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
      dc << "  BKSS NOMATCH: T \n";
    else
      dc << "  BKSS NOMATCH: F \n";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
      dc << "    BKSS AVAIL BREAK: T";
    else
      dc << "    BKSS AVAIL BREAK: F";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      dc << "  BKSS REBOOKED: T - ";
    else
      dc << "  BKSS REBOOKED: F - ";
    dc << segStat._bkgCodeReBook << " \n";
  }
  dc << "*********************************************************** \n";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FBR RBD validation = apply Base Fare PrimeRBD Rec 1 for the CAT25 owning carrier
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::baseFarePrimeRBDRec1(PaxTypeFare& basePaxTfare, PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_NO_MATCH; // defult

  const PaxTypeFareRuleData* fbrPTFare =
      paxTfare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE); // Cat 25 pointer
  const CategoryRuleInfo* fbrRuleInfo = nullptr; // Initialize Pointer Rec 2

  fbrRuleInfo = fbrPTFare->categoryRuleInfo(); // Retrieve Rec 2 FBR

  // preparation to get Booking codes from the Record 1
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  basePaxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B

  // Need to go thru all travel sectors and apply RBD for the sectors
  // with NoMatch and NoYetProcess statuses

  // Get statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (sizeCOS != _mkt->travelSeg().size()) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::baseFarePrimeRBDRec1():size problem");
    return (validReturnType = RET_FAIL);
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;
  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    printDiag411TravelSegInfo(*iterTvl);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      if (airSeg->carrier() == fbrRuleInfo->carrierCode())
      {
        validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat);
        continue;
      } // If carriers are the same
      printDiag411Message(9); //"SKIP PRIME RBD VALIDATION - CARRIERS DO NOT MATCH"
    } // If Segment status is NoMatch or Not Processed
    else
    {
      printDiag411SegmentStatus(segStat); // BOOKING CODE VALIDATION STATUS -
      // PASS/FAIL/MIXED/NOMATCH
    }
  } // For loop Travel Segment

  StatusReturnType checkRetType = NO_MATCH_NOT_PROCESSED; // assume it ready for processing

  checkRetType = CheckBkgCodeSegmentStatus(paxTfare); // get segment status
  if (checkRetType == PASS || checkRetType == FAIL || checkRetType == MIXED)
  {
    validReturnType = setPaxTypeFareBkgCodeStatus(paxTfare);
  }
  else
  {
    validReturnType = RET_NO_MATCH;
  }

  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Base Fare T990 validation for the CAT25 owning carrier & the Carrier is Gov CXR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::baseFareT990Rec1(PaxTypeFare& basePaxTfare, PaxTypeFare& paxTfare)
{
  ValidateReturnType validReturnType = RET_PASS; // defult setup

  // preparation to get Booking codes from the Record 1
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  basePaxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B

  // Retrieve Carrier Application Table 990

  int itemNumber = basePaxTfare.fcasCarrierApplTblItemNo(); // Table 990 item number

  printDiag411Message(36);
  printVendorCarrier(basePaxTfare.vendor(), basePaxTfare.carrier());
  printTable999ItemNumber(itemNumber); // display item number
  printDiag411BlankLine();

  const std::vector<CarrierApplicationInfo*>& carrierApplList =
      _trx.dataHandle().getCarrierApplication(basePaxTfare.vendor(),
                                              basePaxTfare.fcasCarrierApplTblItemNo());

  if (itemNumber && carrierApplList.empty())
  {
    printDiag411Message(37);
    printDiag411BlankLine();
  }
  // Loop all Travel segments for this PaxTypeFare

  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (sizeCOS != _mkt->travelSeg().size()) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::validateIndustryFareBkgCode():size problem");
    return (validReturnType = RET_FAIL);
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;

  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    printDiag411TravelSegInfo(*iterTvl);
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (!carrierApplList.empty() &&
        (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
         segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED)))
    {
      if (_mkt->governingCarrier() == airSeg->carrier())
      {
        if (findCXR(airSeg->carrier(), carrierApplList))
        {
          validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat);
          continue;
        }
      }
    }
    // Marketing carrier not equal Governing carrier OR
    // Carrier status in Table990 is "negative"
    // Record 6 Convention 1 validation for the Travel Segment
    //
    validateRecord6Conv1(paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, nullptr);
  } // travel segment loop
  validReturnType = setPaxTypeFareBkgCodeStatus(paxTfare);

  return validReturnType;
}

//------------------------------------------------------------------------------
void
FareBookingCodeValidator::atleastOneJourneyCxr()
{
  // Only try local with flowAvail if :
  // Any Carrier in this paxTypeFare want to have local journey functionality
  _partOfLocalJny = false;
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(_trx.getOptions()->journeyActivatedForPricing()))
      return;
  }
  else if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(_trx.getOptions()->journeyActivatedForShopping()))
      return;
  }
  else
  {
    return;
  }

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return;

  TravelSegVectorCI tvlI = _mkt->travelSeg().begin();
  const TravelSegVectorCI tvlE = _mkt->travelSeg().end();
  const AirSeg* airSeg = nullptr;
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;

    if (!airSeg->localJourneyCarrier())
      continue;

    if (partOfJourney(*tvlI))
    {
      _partOfLocalJny = true;
      return;
    }
  }
}

//-------------------------------------------------------------------------------
bool
FareBookingCodeValidator::journeyExistInItin() const
{
  return !_itin->oAndDMarkets().empty();
}

//-------------------------------------------------------------------------------
bool
FareBookingCodeValidator::partOfJourney(TravelSeg* tvlSeg)
{
  return JourneyUtil::checkIfSegInFlowOd(tvlSeg, _itin->segmentOAndDMarket());
}

//-------------------------------------------------------------------------------
void
FareBookingCodeValidator::wpaHigherCabin(PaxTypeFare& paxTfare)
{
  if (LIKELY(!_wpaNoMatchEntry || _fcConfig == nullptr))
    return;

  PaxTypeFare::BookingCodeStatus& bkgStatus =
      (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();

  if (bkgStatus.isSet(PaxTypeFare::BKS_MIXED) || bkgStatus.isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
    return;

  if (!bkgStatus.isSet(PaxTypeFare::BKS_PASS))
    return;

  if (_fcConfig->wpaNoMatchHigherCabinFare() != '2' || _trx.noPNRPricing())
    return;

  if (_itin != nullptr)
  {
    if (!ItinUtil::allFlightsBookedInSameCabin(*_itin))
      return;
  }
  std::vector<TravelSeg*>::iterator tvlI = paxTfare.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlE = paxTfare.fareMarket()->travelSeg().end();
  uint16_t segStatusSize = paxTfare.segmentStatus().size();

  std::vector<CabinType> differentCabins;
  uint16_t sizeDifferentCabins = 0;

  for (uint16_t iSegStatus = 0; ((tvlI != tvlE) && (iSegStatus < segStatusSize));
       iSegStatus++, tvlI++)
  {
    if ((*tvlI) == nullptr)
      continue;

    if (!(*tvlI)->isAir())
      continue;

    const TravelSeg& tvlSeg = *(*tvlI);
    const PaxTypeFare::SegmentStatus& segStat = paxTfare.segmentStatus()[iSegStatus];

    differentCabins.push_back(tvlSeg.bookedCabin());

    if (segStat._bkgCodeReBook.empty())
      continue;

    if (!segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      continue;

    if (tvlSeg.bookedCabin() == segStat._reBookCabin)
      continue;

    sizeDifferentCabins = differentCabins.size();
    if (sizeDifferentCabins < 1)
      differentCabins.push_back(segStat._reBookCabin);
    else
      differentCabins[sizeDifferentCabins - 1] = segStat._reBookCabin;
  }

  sizeDifferentCabins = differentCabins.size();
  if (sizeDifferentCabins < 2)
    return;

  uint16_t iDiffCabin = 0;

  for (; iDiffCabin < sizeDifferentCabins; iDiffCabin++)
  {
    if (differentCabins[iDiffCabin] != differentCabins[0])
    {
      bkgStatus.set(PaxTypeFare::BKS_FAIL, true);
      bkgStatus.set(PaxTypeFare::BKS_PASS, false);
      bkgStatus.set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, false);
      bkgStatus.set(PaxTypeFare::BKS_MIXED, false);
      bkgStatus.set(PaxTypeFare::BKS_PASS_FLOW_AVAIL, false);
      bkgStatus.set(PaxTypeFare::BKS_MIXED_FLOW_AVAIL, false);
      printDiag411Message(63);
      return;
    }
  }
}

//-------------------------------------------------------------------------------
const std::vector<ClassOfService*>*
FareBookingCodeValidator::getCosFromFlowCarrierJourneySegment(TravelSeg* tvlSeg)
{
  const OAndDMarket* od = JourneyUtil::getOAndDMarketFromSegment(tvlSeg, _itin);
  if (od && (od->isFlowCarrierJourney() || od->isJourneyByMarriage()))
    return od->getCosVector(tvlSeg);

  return nullptr;
}

//-------------------------------------------------------------------------------
const std::vector<ClassOfService*>*
FareBookingCodeValidator::getAvailability(TravelSeg* travelSeg,
                                          PaxTypeFare::SegmentStatus& segStat,
                                          PaxTypeFare& paxTypeFare,
                                          const uint16_t iCOS)
{
  const std::vector<ClassOfService*>* cosVec = getCosFromFlowCarrierJourneySegment(travelSeg);
  if (UNLIKELY(cosVec))
    return cosVec;

  if (UNLIKELY(_statusType == STATUS_RULE2))
  {
    if (travelSeg->carrierPref() == nullptr)
      return nullptr;
    if (travelSeg->carrierPref()->availabilityApplyrul2st() == YES)
      return nullptr;
    AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
    if (airSeg != nullptr)
    {
      if (airSeg->flowJourneyCarrier() || airSeg->localJourneyCarrier())
        return nullptr;
    }
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
    return (&(travelSeg->classOfService()));
  }

  if (_statusType == STATUS_RULE1)
  {
    if (flowJourneyCarrier(travelSeg) && !(paxTypeFare.fareMarket()->flowMarket()))
    {
      return (flowMarketAvail(travelSeg, segStat, iCOS));
    }
    else if (localJourneyCarrier(travelSeg))
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
      return (&(travelSeg->classOfService()));
    }
  }
  else if (LIKELY(_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR))
  {
    if (localJourneyCarrier(travelSeg))
    {
      const std::vector<ClassOfService*>* cosVecFlow = flowMarketAvail(travelSeg, segStat, iCOS);
      if (cosVecFlow != nullptr)
        return cosVecFlow;
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
      return (&(travelSeg->classOfService()));
    }
  }

  return nullptr;
}

//-------------------------------------------------------------------------------
bool
FareBookingCodeValidator::validateWP(const AirSeg& airSeg, const std::vector<BookingCode>& bkgCodes)
{
  if (UNLIKELY(RexBaseTrx::isRexTrxAndNewItin(_trx) && !airSeg.unflown() && skipFlownSegCat31()))
  {
    return true;
  }

  if (!fallback::fallbackPriceByCabinActivation(&_trx) &&
      !_trx.getOptions()->cabin().isUndefinedClass())
    return false;

  if (checkBookedClassAvail(airSeg))
    return false;

  PaxTypeFare::SegmentStatus ss;
  std::vector<BookingCode> newBkgCodes;

  if (!BookingCodeUtil::validateExcludedBookingCodes(_trx, bkgCodes, ss, newBkgCodes, _diag))
    return false;

  std::vector<BookingCode> bkgCodesClone = (fallback::fallbackAAExcludedBookingCode(&_trx))?
    bkgCodes : newBkgCodes;

  uint16_t i = 0;
  uint16_t nPrimeBc = bkgCodesClone.size();
  const BookingCode& bkg = airSeg.getBookingCode();

  for (i = 0; i < nPrimeBc; ++i)
  {
    if (bkg.compare(bkgCodesClone[i]) == 0)
      return true;
  }
  return false;
}

//----------------------------------------------------------------------------------
// CAT 25 Mandates
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Validate Booking code in the travel segment against Prime RBD in the Record1.
// Loop over the valid booking codes in the Record 1 and determine
// if the flight class of service matches one of them.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::validatePrimeRBDRecord1Domestic(PaxTypeFare& paxTfare,
                                                          std::vector<BookingCode>& bkgCodes)
{
  const PaxTypeFareRuleData* fbrPTFare =
      paxTfare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE); // Cat 25 pointer
  const FBRPaxTypeFareRuleData* fbrPTFBaseFare = nullptr;
  if (fbrPTFare == nullptr) // If it's not a CAT25 Fare,
  {
    if (UNLIKELY(bkgCodes.empty())) // and, no Prime RBD exists -> return
      return false; // No book codes in Rec 1
  }
  else
    fbrPTFBaseFare = PTFRuleData::toFBRPaxTypeFare(fbrPTFare);

  const CategoryRuleInfo* fbrRuleInfo = nullptr; // Rec 2
  // Discounted FBR fare pointer
  const PaxTypeFareRuleData* diskPTfare =
      paxTfare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE); // Cat 19 pointer

  bool cat25Rec2Carrier = true; // Cat25 Rec2 Cxr does match Marketing Cxr

  bool wpncs = _trx.getRequest()->isLowFareNoAvailability(); // WPNCS entry
  bool wpnc = _trx.getRequest()->isLowFareRequested(); // WPNC entry

  if (fbrPTFare != nullptr) // If it's a CAT25 Fare,
  {
    printDiag411CAT25PrimeRBD(bkgCodes);
  }
  else
    printDiag411PrimeRBD(bkgCodes);

  printDiag411BlankLine();

  BookingCode previousBkgCode;
  BookingCode currentBkgCode;
  BookingCode foundBkgCode;
  BookingCode alreadyPassedBkg;

  bool dispRec1 = true;

  bool oneBkgCodefound = false;
  bool bkgCodeValid = true;
  bool mixedClassDetected = false;

  int flightCount = 0;

  // Loop all Travel segments for this PaxTypeFare

  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (UNLIKELY(sizeCOS != _mkt->travelSeg().size())) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger,
                  "FareBookingCodeValidator::validatePrimeRBDRecord1Domestic():size problem");
    return false;
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlE = _mkt->travelSeg().end();

  for (; (iterTvl != iterTvlE) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    if (!bkgCodes.empty())
      printDiag411TravelSegInfo(*iterTvl);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (UNLIKELY(airSeg == nullptr))
    {
      ++flightCount;
      continue;
    }

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (fbrPTFare != nullptr) // Cat 25 Fare ?
    {
      // If Cat25 fare, Rec 2 Carrier code must match the marketing carrier

      fbrRuleInfo = fbrPTFare->categoryRuleInfo(); // Rec 2 FBR

      if (UNLIKELY(airSeg->carrier() != fbrRuleInfo->carrierCode() &&
                   fbrPTFBaseFare->isSpecifiedFare()))

      {
        cat25Rec2Carrier = false;
        setFailPrimeRBDDomesticStatus(segStat, '2');
        IbfAvailabilityTools::setBrandsStatus(
            IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
        continue;
      }
      // New code below for CAT25 Mandates
      if (paxTfare.isDiscounted())
      {
        if (diskPTfare == nullptr)
        {
          setFailPrimeRBDDomesticStatus(segStat, 2); // PRIME RBD VALIDATION STATUS - FAIL
          IbfAvailabilityTools::setBrandsStatus(
              IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
          continue;
        }
        else
        {
          const RuleItemInfo* ruleItemInfo = diskPTfare->ruleItemInfo();
          const DiscountInfo* discountInfo = dynamic_cast<const DiscountInfo*>(ruleItemInfo);
          if (discountInfo != nullptr)
          {
            if (!discountInfo->bookingCode().empty())
            {
              bkgCodes.clear();
              bkgCodes.push_back(discountInfo->bookingCode());
            }
          }
        } // Discounted 19-22 Fare Exists
      } // CAT19-22 fare
      //--------------------------------------------------------------------

      // Apply Base Fare Rec1 T999 if Resulting fare T999 = 0 & Prime RBD = blank
      // Codes under this if is from updated FRD Cat25 in Oct/05
      if (bkgCodes.empty() && !paxTfare.fcasBookingCodeTblItemNo() &&
          !fbrPTFBaseFare->isSpecifiedFare())
      {
        PaxTypeFare* pBaseF = fbrPTFBaseFare->baseFare();
        PaxTypeFare* fakePtf = pBaseF->clone(_trx.dataHandle(), false);

        if (LIKELY(pBaseF != nullptr)) // Base Fare present
        {
          fakePtf->bookingCodeStatus() = paxTfare.bookingCodeStatus();
          fakePtf->segmentStatus() = paxTfare.segmentStatus();
          fakePtf->segmentStatusRule2() = paxTfare.segmentStatusRule2();

          long itemNumber = pBaseF->fcasBookingCodeTblItemNo(); // "TABLE 999 ITEM NUMBER - "
          // // default Table999 item Number
          // Do check the dummy Rec1B for table 999 item number

          printDiag411Message(61); // Base Fare R1/T999
          printVendorCarrier(pBaseF->vendor(), pBaseF->carrier());
          printTable999ItemNumber(itemNumber);

          printDiag411BlankLine();

          ValidateReturnType validReturnType = RET_PASS;

          if (itemNumber != 0)
          {
            // if Table999 item presents in the Base Fare Record 1B
            validReturnType = validateBookingCodeTblItemNo(*fakePtf);
            // restore statuses in original paxtypefare
            paxTfare.bookingCodeStatus() = fakePtf->bookingCodeStatus();
            paxTfare.segmentStatus() = fakePtf->segmentStatus();
            paxTfare.segmentStatusRule2() = fakePtf->segmentStatusRule2();

            if (validReturnType != RET_CONTINUE && validReturnType != RET_NO_MATCH &&
                validReturnType != RET_MIXED)
            {
              if (validReturnType == RET_PASS)
                return true;
              else if (validReturnType == RET_FAIL)
                return false;
            }
          }
          pBaseF->getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
          if (LIKELY(validReturnType != RET_MIXED))
          {
            // Continue validate Base Fare Rec 1 if T999 is not referenced OR marketing cxr
            // does not match Cat25 Rec2 carrier
            // Starts with T990 if Base Fare is Industry fare
            if (UNLIKELY(pBaseF->carrier() == INDUSTRY_CARRIER))
            {
              validReturnType = baseFareT990Rec1(*pBaseF, *fakePtf);
              // restore statuses in original paxtypefare
              paxTfare.bookingCodeStatus() = fakePtf->bookingCodeStatus();
              paxTfare.segmentStatus() = fakePtf->segmentStatus();
              paxTfare.segmentStatusRule2() = fakePtf->segmentStatusRule2();
              if (validReturnType != RET_CONTINUE && validReturnType != RET_NO_MATCH &&
                  validReturnType != RET_MIXED)
              {
                if (validReturnType == RET_PASS)
                  return true;
                else if (validReturnType == RET_FAIL)
                  return false;
              }
            }
          }
        }
      }
      // If it's a CAT25 Fare and CXR's are match, and, no Prime RBD (booking codes) exists (???)
      else if (UNLIKELY(bkgCodes.empty()))
      {
        if ((validateRBDforDiscountFBRFare(paxTfare, bkgCodes, segStat, dispRec1)) == RET_CONTINUE)
        {
          bkgCodeValid = false;
          continue;
        }
      }
    } // FBR PaxTypeFare

    // Loop over the valid booking codes in record 1 and determine if the flight
    // class of service matches one of them.

    bool bkgFound = false;

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
    {
      // if flight already passed by table 999 then do not check
      // prime RBD status, just check if the booking code matches with prime RBD or not

      alreadyPassedBkg = airSeg->getBookingCode();
      if (wpnc || wpncs)
      {
        if (!(segStat._bkgCodeReBook.empty()) &&
            segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          alreadyPassedBkg = segStat._bkgCodeReBook;
        }
      }

      for (const auto& bkgCode : bkgCodes)
      {
        if (alreadyPassedBkg == bkgCode)
        {
          bkgFound = true;
          oneBkgCodefound = true;
          foundBkgCode = alreadyPassedBkg;
          break;
        }
      }
      printDiag411Message(55);
    }
    else if (!segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) &&
             !segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) &&
             !segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999) &&
             !segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999) &&
             !segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999))
    {
      if (validateSectorPrimeRBDRec1(*airSeg, bkgCodes, cosVec, paxTfare, segStat, false))
      {
        bkgFound = true;
        oneBkgCodefound = true;
        foundBkgCode = airSeg->getBookingCode();
        if (LIKELY(wpnc || wpncs))
        {
          if (!(segStat._bkgCodeReBook.empty()) &&
              segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
          {
            foundBkgCode = segStat._bkgCodeReBook;
          }
        }
      }
      else
      {
        bkgCodeValid = false;
      }
      setBkgCodePrimeRBDDomesticSegmentStatus(segStat, bkgFound);
    }
    else
    {
      bkgCodeValid = false;
      printDiag411SegmentStatus(segStat);
    }
    // For multi-flight segment fares, determine if mixed class checking is necessary

    currentBkgCode = airSeg->getBookingCode();
    if (LIKELY(wpnc || wpncs))
    {
      if (!(segStat._bkgCodeReBook.empty()) &&
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      {
        currentBkgCode = segStat._bkgCodeReBook;
      }
    }

    if (flightCount > 0 && currentBkgCode != previousBkgCode)
    {
      mixedClassDetected = true;
    }

    // save previous Bkg code to check with the next Bkg Code
    previousBkgCode = currentBkgCode;

    ++flightCount;

  } // for loop

  if (UNLIKELY(!cat25Rec2Carrier)) // Cat25 Rec2 Carrier code does not match the Marketing carrier
  {
    printDiag411Message(30); // "CAT 25 REC 2 CXR DOES NOT MATCH - PRIME RBD FAIL ";
    return false;
  }

  if (bkgCodeValid)
  {
    return true; // Pass Prime RBD Rec1
  }

  if (oneBkgCodefound && mixedClassDetected)
  {
    if (UNLIKELY(paxTfare.isNegotiated() &&
                 !_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)))
      return false;

    PaxTypeFare::BookingCodeStatus& ptfStat =
        (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();
    if (validateDomesticMixedClass(paxTfare, foundBkgCode))
    {
      if (!isCOSValidAfterDomesticMixedClass(paxTfare))
      {
        if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
          ptfStat.set(PaxTypeFare::BKS_FAIL_FLOW_AVAIL, true);

        printDiag411Message(8); // "MIXED CLASS VALIDATION STATUS - FAILn";
        return false;
      }

      printDiag411Message(7); // "MIXED CLASS VALIDATION STATUS - PASS";
      return true;
    }
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
      ptfStat.set(PaxTypeFare::BKS_FAIL_FLOW_AVAIL, true);

    printDiag411Message(8); // "MIXED CLASS VALIDATION STATUS - FAILn";
  }

  return false;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Checks rebooked COS availability after domestic mixed class validation.
//
// WP assumes availability was checked at booking time. WPNC and Shopping may
// rebook during domestic mixed class without checking availability. We need to
// check if the resulting COS has enough seats available.
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::isCOSValidAfterDomesticMixedClass(PaxTypeFare& paxTfare)
{
  if (_trx.getRequest()->isLowFareRequested() && !_rexLocalMarketWPvalidation)
  {
    RebookedCOSChecker cosChecker(
        _trx, paxTfare, *_mkt, _fu, *this, (_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR));
    return cosChecker.checkRebookedCOS();
  }
  return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// International RBD validation = apply PrimeRBD Rec 1 only (needs for FBR fares)
// validations for the international FareMarket
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::validateFBR_RBDInternational(PaxTypeFare& paxTfare, bool bkgCodeItem)
{
  ValidateReturnType validReturnType = RET_PASS; // defult

  bool dispRec1 = true; // default, display Rec1 RBD codes in the validateSectorPrimeRBDRec1 method

  // FareByRule PaxTypeFare
  const FBRPaxTypeFareRuleData* fbrPTFBaseFare = paxTfare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (UNLIKELY(fbrPTFBaseFare == nullptr))
  {
    return (validReturnType = RET_FAIL);
  }
  const CategoryRuleInfo* fbrRuleInfo = fbrPTFBaseFare->categoryRuleInfo(); // Retrieve Rec 2 FBR
  // New code for CAT25 Mandates  (next 6 lines)
  const FareByRuleItemInfo* fbrItemInfo =
      dynamic_cast<const FareByRuleItemInfo*>(fbrPTFBaseFare->ruleItemInfo());
  bool primeSector = false;
  bool smfFbrWithPrimeSector = false;

  if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
  {
    primeSector = true;
    if (!(paxTfare.vendor() == ATPCO_VENDOR_CODE || paxTfare.vendor() == SITA_VENDOR_CODE))
    {
      smfFbrWithPrimeSector = true;
    }
  }

  bool primeDone = false;
  bool secondaryDone = false;

  TravelSeg* pTS = paxTfare.fareMarket()->primarySector();

  // Discounted FBR fare pointer
  const PaxTypeFareRuleData* diskPTfare =
      paxTfare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE); // Cat 19 pointer

  // preparation to get Booking codes from the Record 1
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes

  paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B

  // Need to go thru all travel sectors and apply RBD for the sectors
  // with NoMatch and NoYetProcess statuses

  // Get statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (sizeCOS != _mkt->travelSeg().size()) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::validateFBR_RBDInternational():size problem");
    return (validReturnType = RET_FAIL);
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;
  bool isRuleZero = false;

  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    if (primeSector)
    {
      if ((*iterTvl) == pTS) // test Prime RBD for a primary or secondary sector (primeSector=blank)
        diag411PrimarySectorMsg();
      else
        diag411SecondarySectorMsg();
    }
    printDiag411TravelSegInfo(*iterTvl);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      // Cat 25 Rec 2 Carrier code must match the marketing carrier
      if (airSeg->carrier() == fbrRuleInfo->carrierCode())

      {
        if (bkgCodeItem) // BookingCodeTableItemNumber exists (logic)
        {
          if (paxTfare.isDiscounted())
          {
            if (diskPTfare == nullptr)
            {
              setFailPrimeRBDInternationalStatus(segStat, 31); // PRIME RBD VALIDATION STATUS - FAIL
              IbfAvailabilityTools::setBrandsStatus(
                  IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
              continue;
            }
            const RuleItemInfo* ruleItemInfo = diskPTfare->ruleItemInfo();
            const DiscountInfo* discountInfo = dynamic_cast<const DiscountInfo*>(ruleItemInfo);
            if (discountInfo != nullptr)
            {
              if (discountInfo->bookingCode().empty())
              {
                if (bkgCodes.empty()) // CAT25 Resulting Fare Prime RBD
                {
                  if (!fbrPTFBaseFare->isSpecifiedFare())
                  {
                    const PaxTypeFare* pBaseF = fbrPTFBaseFare->baseFare();
                    pBaseF->getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B Base Fare
                  }
                }
              }
              else
              {
                bkgCodes.clear();
                bkgCodes.push_back(discountInfo->bookingCode());
              }
            }
          } // CAT19-22 fare
          else // resulting fare is No CAT19-22 fare
          {
            if (bkgCodes.empty())
            {
              if (bkgCodeItem) // BookingCodeTableItemNumber exists (logic)
              {
                if (!fbrPTFBaseFare->isSpecifiedFare())
                {
                  const PaxTypeFare* pBaseF = fbrPTFBaseFare->baseFare();
                  pBaseF->getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B Base Fare
                }
              }
            } //  No PRIME RBD in CAT 25 Resulting Fare
          } //  collect PRIME RBD
        } //  T999 presents
        //--------------------------------------------------------------------
        else //  CAT25 Resulting fare does not have T999
        {
          if (paxTfare.isDiscounted())
          {
            if (diskPTfare == nullptr)
            {
              setFailPrimeRBDInternationalStatus(segStat, 31); // PRIME RBD VALIDATION STATUS - FAIL
              IbfAvailabilityTools::setBrandsStatus(
                  IbfErrorMessage::IBF_EM_EARLY_DROP, paxTfare, _trx, __PRETTY_FUNCTION__);
              continue;
            }
            const RuleItemInfo* ruleItemInfo = diskPTfare->ruleItemInfo();
            const DiscountInfo* discountInfo = dynamic_cast<const DiscountInfo*>(ruleItemInfo);
            if (LIKELY(discountInfo != nullptr))
            {
              if (LIKELY(discountInfo->bookingCode().empty()))
              {
                if (!bkgCodes.empty()) // CAT25 Resulting Fare Prime RBD exists
                {
                  // New code below for CAT25 Mandates
                  if (primeSector)
                  {
                    if ((*iterTvl) ==
                        pTS) // test Prime RBD for a primary or secondary sector (primeSector=blank)
                    {
                      primeDone = true;
                      validatePrimeRBDRec1AndUpdateStatus(
                          *airSeg, bkgCodes, cosVec, paxTfare, segStat, dispRec1);
                      continue;
                    }
                    else // secondary sector
                    {
                      if (!secondaryDone)
                      {
                        if (!fbrPTFBaseFare->isSpecifiedFare()) // FBR Specified
                        {
                          if (validateConvention2(
                                  paxTfare,
                                  smfFbrWithPrimeSector ? paxTfare : *(fbrPTFBaseFare->baseFare()),
                                  airSeg,
                                  isRuleZero,
                                  true) == RET_NO_MATCH)
                          {
                            secondaryDone = true;
                            continue;
                          }
                        }
                        secondaryDone = true;
                      }
                    }
                  }
                }
                else // CAT 25 Prime RBD is empty
                  return validReturnType = RET_CONTINUE; // continue with the BASE FARE
              }
              else
              {
                bkgCodes.clear();
                bkgCodes.push_back(discountInfo->bookingCode());
              }
            }
          } // CAT19-22 fare
          else // resulting fare is No CAT19-22 fare
          {
            if (!bkgCodes.empty())
            {
              // New code below for CAT25 Mandates
              if (primeSector)
              {
                if ((*iterTvl) ==
                    pTS) // test Prime RBD for a primary or secondary sector (primeSector=blank)
                {
                  primeDone = true;
                  validatePrimeRBDRec1AndUpdateStatus(
                      *airSeg, bkgCodes, cosVec, paxTfare, segStat, dispRec1);
                  continue;
                }
                else // secondary sector
                {
                  if (!secondaryDone)
                  {
                    if (!fbrPTFBaseFare->isSpecifiedFare()) // FBR Specified
                    {
                      validateConvention2(paxTfare,
                                          smfFbrWithPrimeSector ? paxTfare
                                                                : *(fbrPTFBaseFare->baseFare()),
                                          airSeg,
                                          isRuleZero,
                                          true);
                      secondaryDone = true;
                      continue;
                    }
                    secondaryDone = true;
                  }
                }
              }
            }
            else // CAT 25 Prime RBD is empty
              return validReturnType = RET_CONTINUE; // continue with the BASE FARE
          }
        }

        //----------------------------------------------------------------------------
        validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat, dispRec1);
        continue;
      } // If carriers are the same
      else
      {
        if (bkgCodeItem) // BookingCodeTableItemNumber exists (logic)
        {
          // New code below for CAT25 Mandates
          if ((*iterTvl) ==
              pTS) // test Prime RBD for a primary or secondary sector (primeSector=blank)
          {
            validatePrimeRBDRec1AndUpdateStatus(
                *airSeg, bkgCodes, cosVec, paxTfare, segStat, dispRec1);
            continue;
          }
          // Record 6 Convention 1 validation for the Travel Segment
          validateRecord6Conv1(
              paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, fbrPTFBaseFare);
        }
        else // no Cat25 Resulting Fare T999 and carriers do NOT match
        {
          printDiag411Message(9); //"SKIP PRIME RBD VALIDATION - CARRIERS DO NOT MATCH"
          printDiag411BlankLine();
          if (fbrPTFBaseFare->isSpecifiedFare()) // FBR Specified
          {
            // Record 6 Convention 1 validation for the Travel Segment
            validateRecord6Conv1(
                paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, fbrPTFBaseFare);
          }
          validReturnType = RET_CONTINUE;
        }
      }
    } // If Segment status is NoMatch or Not Processed
    else
    {
      printDiag411SegmentStatus(segStat); // BOOKING CODE VALIDATION STATUS -
      // PASS/FAIL/MIXED/NOMATCH
      printDiag411BlankLine();
    }

  } // For loop Travel Segment

  if (primeDone && secondaryDone)
  {
    StatusReturnType checkRetType = NO_MATCH_NOT_PROCESSED; // assume it ready for processing
    bool ret1 = true;
    bool ret2 = true;
    if (_newPaxTypeFare)
    {
      checkRetType = CheckBkgCodeSegmentStatus(*_newPaxTypeFare); // get segment status
      if (checkRetType != PASS && checkRetType != FAIL && checkRetType != MIXED)
      {
        ret1 = primeRBDRec3CAT25Mandates(*_newPaxTypeFare);
      }
    }
    checkRetType = CheckBkgCodeSegmentStatus(paxTfare); // get segment status
    if (checkRetType != PASS && checkRetType != FAIL && checkRetType != MIXED)
    {
      ret2 = primeRBDRec3CAT25Mandates(paxTfare);
    }
    if (ret1 && ret2)
      validReturnType = RET_PASS;
  }
  if (!fbrPTFBaseFare->isSpecifiedFare() && (validReturnType == RET_CONTINUE)) // FBR calculated
    return validReturnType;

  validReturnType = setPaxTypeFareBkgCodeStatus(paxTfare);

  return validReturnType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FBR RBD:
// Prime RBD applies to Cat25 Owning/Governing cxr secondary sector(s) with NOMATCH status
// R6/C1 T999 applies to the Cat25 Non-Governing crs secondary sectors
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::primeRBDRec3CAT25Mandates(PaxTypeFare& paxTfare)
{
  // FareByRule PaxTypeFare
  const FBRPaxTypeFareRuleData* fbrPTFBaseFare = paxTfare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
  const CategoryRuleInfo* fbrRuleInfo = fbrPTFBaseFare->categoryRuleInfo(); // Retrieve Rec 2 FBR

  paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B

  // Need to go thru all travel sectors and apply RBD for the sectors
  // with NoMatch and NoYetProcess statuses

  // Get statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();
  uint16_t iCOS = 0;
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();

  if (sizeCOS != _mkt->travelSeg().size()) // prevent segmentation violation
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::baseFarePrimeRBDRec1():size problem");
    return false;
  }
  const std::vector<ClassOfService*>* cosVec = nullptr;

  for (; (iterTvl != iterTvlEnd) && (iCOS < sizeCOS); iterTvl++, iCOS++)
  {
    cosVec = _mkt->classOfServiceVec()[iCOS];

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);

    if (airSeg == nullptr)
      continue;

    cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);
    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      printDiag411TravelSegInfo(*iterTvl);

      if (!fbrPTFBaseFare->isSpecifiedFare() && (airSeg->carrier() == fbrRuleInfo->carrierCode()))
      {
        validatePrimeRBDRec1AndUpdateStatus(*airSeg, bkgCodes, cosVec, paxTfare, segStat);
        continue;
      }
      else
      {
        // Record 6 Convention 1 validation for the Travel Segment
        validateRecord6Conv1(paxTfare, airSeg, cosVec, segStat, bkgCodes, *iterTvl, fbrPTFBaseFare);
      }
    } // If Segment status is NoMatch or Not Processed
  } // For loop Travel Segment
  StatusReturnType checkRetType = NO_MATCH_NOT_PROCESSED; // assume it ready for processing

  checkRetType = CheckBkgCodeSegmentStatus(paxTfare);

  if (checkRetType != PASS)
    return false;

  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Get Prime RBD from the Pax Type Fare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
FareBookingCodeValidator::getPrimeR1RBDs(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes)
{
  paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
  std::vector<const FareClassAppSegInfo*>* fcasVec = nullptr;
  fcasVec = paxTfare.fcasOfOverFlownBkgCds();
  if (UNLIKELY(fcasVec))
  {
    std::vector<const FareClassAppSegInfo*>::iterator cIt = fcasVec->begin();
    std::vector<const FareClassAppSegInfo*>::iterator cIte = fcasVec->end();
    for (; cIt != cIte; cIt++)
    {
      for (int i = 0; i < 8; ++i)
      {
        if (!(*cIt)->_bookingCode[i].empty())
        {
          bkgCodes.push_back((*cIt)->_bookingCode[i]);
        }
        else
          break;
      }
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Get to know to what cortege the PaxTypeFare belongs
// If its INF cortege then check for PaxType and number of seats for actual PaxType.
// If actual PaxType not infant or number of seats not = 0, then
// create a new PaxTypeFare and add it to the INF cortege.
//
// This logic will not work for PU scope !!!!
//

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
FareBookingCodeValidator::isItInfantFare(PaxTypeFare& paxTypeFare, TravelSeg* seg)
{
  // Orginize the BIG loop for all PaxTypeFareCorteges on the Fare Market
  PaxTypeBucketVecI paxTypeCortegeVecIter;
  PaxTypeBucketVecI paxTypeCortegeVecIterEnd;

  PaxTypeBucketVec& paxTypeCortegeVec = _mkt->paxTypeCortege();

  paxTypeCortegeVecIter = _mkt->paxTypeCortege().begin();

  paxTypeCortegeVecIterEnd = _mkt->paxTypeCortege().end();

  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];
    PaxTypeFarePtrVec& paxTypeFareVec = paxTypeCortege.paxTypeFare();

    PaxTypeFarePtrVecI iterB = paxTypeFareVec.begin();
    PaxTypeFarePtrVecI iterE = paxTypeFareVec.end();

    const PaxType* paxType = paxTypeCortege.requestedPaxType();

    const PaxTypeInfo& pInfo = paxType->paxTypeInfo();
    bool infant = PaxTypeUtil::isInfant(pInfo.paxType(), paxTypeFare.vendor());

    if (!infant)
      continue;
    // Orginize the loop thru reqPaxType Cortege PaxTypeFare on the Fare Market

    for (; iterB != iterE; iterB++)
    {
      PaxTypeFare* paxTFare = *iterB;

      if (paxTFare == &paxTypeFare)
      {
        bool seats = true;
        if ((paxType->paxTypeInfo().numberSeatsReq() * paxType->number()) == 0)
          seats = false;
        if (infant && !seats)
        {
          if (_fu)
            return true;

          const PaxTypeInfo*& pInfon = paxTFare->actualPaxType()->paxTypeInfo();
          bool infantn = PaxTypeUtil::isInfant(pInfon->paxType(), paxTFare->vendor());
          if ((infantn && pInfon->numberSeatsReq() != 0) || !infantn)
          {
            if (!_newPaxTypeFare)
            {
              _newPaxTypeFare = paxTFare->clone(_trx.dataHandle(), false);

              paxTypeCortege.paxTypeFare().erase(iterB);
              paxTypeCortege.paxTypeFare().push_back(_newPaxTypeFare);
            }
            // populate new paxtypefare
            TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin();

            TravelSegPtrVecI iterTvlEnd = _mkt->travelSeg().end();

            for (uint16_t iCOS = 0; iterTvl != iterTvlEnd; iterTvl++, iCOS++)
            {
              if (seg == *iterTvl)
              {
                PaxTypeFare::SegmentStatus& segStatn =
                    (_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                        ? _newPaxTypeFare->segmentStatus()[iCOS]
                        : _newPaxTypeFare->segmentStatusRule2()[iCOS];

                segStatn._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
                turnOffNoMatchNotYetProcessedStatus(segStatn);
              }
            }
          }
          return true;
        }
      }
    }
  }
  return false;
}

bool
FareBookingCodeValidator::setIndforWPBkgStatus(PaxTypeFare& paxTfare)
{
  if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N))
    return false;

  const CabinType& fareCabin = paxTfare.cabin();

  bool ind = false;

  int reBook = 0;
  int commonFail = 0;
  int lowerCabin = 0;
  int higherCabin = 0;
  int sameCabin = 0;

  BookingCode bkgCode;

  using SegmentStatusVecI = std::vector<PaxTypeFare::SegmentStatus>::iterator;

  uint16_t tvlSegSize = _mkt->travelSeg().size();
  uint16_t ptfSegSize = 0;
  if (_fu)
    ptfSegSize = _fu->segmentStatus().size();
  else
    ptfSegSize = paxTfare.segmentStatus().size();
  uint16_t sizeCOS = _mkt->classOfServiceVec().size();
  if (UNLIKELY(tvlSegSize != sizeCOS || // prevent segmentation violation
               ptfSegSize != sizeCOS ||
               tvlSegSize != ptfSegSize))
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::setFinalPTFBkgStatus():size problem");
    return false;
  }

  TravelSegPtrVecIC iterTvl = _mkt->travelSeg().begin();
  TravelSegPtrVecIC iterTvlE = _mkt->travelSeg().end();
  SegmentStatusVecI iterSegStat = paxTfare.segmentStatus().begin();
  SegmentStatusVecI iterSegStatEnd = paxTfare.segmentStatus().end();
  if (_fu)
  {
    iterSegStat = _fu->segmentStatus().begin();
    iterSegStatEnd = _fu->segmentStatus().end();
  }

  // ClassOfService vector of ClassOfServices vectors for each TravelSeg in the FareMarket
  uint16_t iCOS = 0;

  const std::vector<ClassOfService*>* cosVec = nullptr;
  const std::vector<ClassOfService*>* cosVecFlow = nullptr;

  for (; (iterTvl != iterTvlE) && (iterSegStat != iterSegStatEnd) && (iCOS < sizeCOS);
       iterTvl++, iterSegStat++, iCOS++)
  {
    cosVec = getCosFromFlowCarrierJourneySegment(*iterTvl);

    if (LIKELY(cosVec == nullptr))
    {
      cosVec = _mkt->classOfServiceVec()[iCOS];
      const AirSeg* airSeg1 = dynamic_cast<const AirSeg*>(*iterTvl);
      if (airSeg1 == nullptr)
      {
        tvlSegSize--;
        continue;
      }
      if (UNLIKELY(_statusType == STATUS_RULE2))
      {
        if ((*iterTvl)->carrierPref() != nullptr)
        {
          if ((*iterTvl)->carrierPref()->availabilityApplyrul2st() == NO &&
              !airSeg1->localJourneyCarrier() &&
              !airSeg1->flowJourneyCarrier()) // db availabilityIgrul2st
          {
            // if Rule 2 then use the Local availability and not the thru
            cosVec = &(airSeg1->classOfService());
          }
        }
      }
      else
      {
        if (LIKELY(_statusType == STATUS_RULE1))
        {
          // check for Flow Journey Carrier
          if (flowJourneyCarrier(*iterTvl) && !(paxTfare.fareMarket()->flowMarket()))
          {
            // if we are not processing the flow market than
            // get the availability from the flow market
            cosVecFlow = flowMarketAvail(*iterTvl, *iterSegStat, iCOS);
            if (cosVecFlow != nullptr)
              cosVec = cosVecFlow;
          }

          else if (UNLIKELY(localJourneyCarrier(*iterTvl)))
          {
            // use the Local availability and not the thru
            cosVec = &(airSeg1->classOfService());
          }
        }

        else if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
        {
          if (localJourneyCarrier(*iterTvl))
          {
            cosVecFlow = flowMarketAvail(*iterTvl, *iterSegStat, iCOS);
            if (cosVecFlow != nullptr)
              cosVec = cosVecFlow;
          }
        }
      }
    }

    bkgCode = (**iterTvl).getBookingCode();

    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;
    if (UNLIKELY(segStat._bkgCodeSegStatus.isNull()))
    {
      continue;
    }
    if (UNLIKELY(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_SURFACE)))
    {
      tvlSegSize--;
    }
    else
    {
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
      {
        ++commonFail;

        if (UNLIKELY(_trx.getRequest()->isLowFareRequested() && // WPNC entry
                     segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)))
          ++reBook;

        // ClassOfService vector for the particular Travel Segment
        COSInnerPtrVecIC classOfS = cosVec->begin();

        for (; classOfS != cosVec->end(); classOfS++)
        {
          if (bkgCode.compare((*classOfS)->bookingCode()) == 0)
          {
            if (fareCabin == (*classOfS)->cabin())
            {
              ++sameCabin;
            }
            else if (fareCabin < (*classOfS)->cabin())
            {
              ++lowerCabin;
            }
            else if (fareCabin > (*classOfS)->cabin())
            {
              ++higherCabin;
            }
          } // if found Booking Code in the Class Of Service
        } // loop for Class of service objects for the particular Travel Segment
      } // if FAIL segment status is Found
    } // if SURFACE segment status
  } // loop for all Travel Segment
  if (_trx.getRequest()->isLowFareRequested()) // WPNC entry
  {
    if ((commonFail == tvlSegSize && reBook != 0) || (commonFail != tvlSegSize))
      return true;
    return false;
  }
  if (paxTfare.isNormal()) // Normal fare processing
  {
    if (commonFail == tvlSegSize || commonFail == 0)
      ind = false;
    else if (!_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA) &&
             !paxTfare.isFareByRule())
    {
      ind = true;
    }
  }
  else // Special fare processing
  {
    if (!_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA) &&
        //               (commonFail == higherCabin) )
        (commonFail != 0 && lowerCabin == 0))
      ind = true;
  }
  return ind;
}

void
FareBookingCodeValidator::setFinalBkgStatus(PaxTypeFare& paxTfare)
{
  PaxTypeFare::BookingCodeStatus& ptfStat =
      (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();

  // if command pricing e.g WPQY26 then PASS the fare
  if (UNLIKELY(paxTfare.validForCmdPricing(false)))
  {
    if (!_fu)
      paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
    ptfStat.set(PaxTypeFare::BKS_PASS);
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);
  }
  else if ((paxTfare.isNegotiated() &&
            !_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) ||
           ptfStat.isSet(PaxTypeFare::BKS_FAIL_TAG_N))
  {
    ptfStat.set(PaxTypeFare::BKS_FAIL);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);
  }
  else
  {
    ptfStat.set(PaxTypeFare::BKS_MIXED);

    if (UNLIKELY(_trx.getOptions() && _trx.getOptions()->isRtw()))
    {
      ptfStat.set(PaxTypeFare::BKS_FAIL);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
    }
    else
    {
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
    }
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set up the booking Code status in the PaxTypeFare
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FareBookingCodeValidator::ValidateReturnType
FareBookingCodeValidator::setPTFBkgStatus(PaxTypeFare& paxTfare, StatusReturnType checkRetType)
{
  ValidateReturnType validReturnType = RET_PASS; // defult setup

  PaxTypeFare::BookingCodeStatus& ptfStat =
      (_fu) ? _fu->bookingCodeStatus() : paxTfare.bookingCodeStatus();
  ptfStat.set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);

  switch (checkRetType)
  {
  case PASS:
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_FLOW_AVAIL);
    }
    else if (LIKELY(_statusType != STATUS_RULE2))
    {
      ptfStat.set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    }
    ptfStat.set(PaxTypeFare::BKS_PASS);
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    validReturnType = RET_PASS;
    break;

  case FAIL:
    // if command pricing e.g WPQY26 then PASS the fare
    if (UNLIKELY(paxTfare.validForCmdPricing(false)))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS);
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);

      printDiag411Message(41); // COMMAND PRICING RBD STATUS - PASS
      validReturnType = RET_PASS;

      break;
    }
    if ((_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR) &&
        (ptfStat.isSet(PaxTypeFare::BKS_PASS) || ptfStat.isSet(PaxTypeFare::BKS_MIXED)))
    {
      break;
    }

    ptfStat.set(PaxTypeFare::BKS_FAIL);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    validReturnType = RET_FAIL;
    break;

  case MIXED:
    validReturnType = RET_MIXED;
    // if command pricing e.g WPQY26 then PASS the fare
    if (UNLIKELY(paxTfare.validForCmdPricing(false)))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS);
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(43); // COMMAND PRICING RBD STATUS - PASS
      validReturnType = RET_PASS;
      break;
    }
    if (paxTfare.isNegotiated() && !_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
    {
      ptfStat.set(PaxTypeFare::BKS_FAIL);
      ptfStat.set(PaxTypeFare::BKS_PASS, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      validReturnType = RET_FAIL;
      break;
    }
    if (_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
    {
      ptfStat.set(PaxTypeFare::BKS_MIXED_FLOW_AVAIL);
      if (ptfStat.isSet(PaxTypeFare::BKS_FAIL))
      {
        ptfStat.set(PaxTypeFare::BKS_PASS);
        ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      }
      break;
    }
    ptfStat.set(PaxTypeFare::BKS_MIXED);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    ptfStat.set(PaxTypeFare::BKS_FAIL, false);
    break;

  case NO_MATCH_NOT_PROCESSED:
    // if command pricing e.g WPQY26 then PASS the fare
    if (UNLIKELY(paxTfare.validForCmdPricing(false)))
    {
      if (!_fu)
        paxTfare.cpFailedStatus().set(PaxTypeFare::PTFF_RBD);
      ptfStat.set(PaxTypeFare::BKS_PASS);
      ptfStat.set(PaxTypeFare::BKS_FAIL, false);
      ptfStat.set(PaxTypeFare::BKS_MIXED, false);
      printDiag411Message(44); // COMMAND PRICING RBD STATUS - PASS
      validReturnType = RET_PASS;
      break;
    }
    if (UNLIKELY(_statusType == STATUS_FLOW_FOR_LOCAL_JRNY_CXR &&
                 (ptfStat.isSet(PaxTypeFare::BKS_PASS) || ptfStat.isSet(PaxTypeFare::BKS_MIXED))))
    {
      break;
    }
    ptfStat.set(PaxTypeFare::BKS_FAIL);
    ptfStat.set(PaxTypeFare::BKS_MIXED, false);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
    validReturnType = RET_FAIL;
    break;

  default:
    break;
  } // switch

  if (UNLIKELY(_trx.getOptions() && _trx.getOptions()->isRtw() &&
               ptfStat.isSet(PaxTypeFare::BKS_MIXED)))
  {
    ptfStat.set(PaxTypeFare::BKS_FAIL);
    ptfStat.set(PaxTypeFare::BKS_PASS, false);
  }

  if (UNLIKELY(_newPaxTypeFare && (_newPaxTypeFare == &paxTfare)))
    printDiag411Message(66); // "FOR INFANT PASSENGER TYPE"

  printDiag411BkgCodeStatus(paxTfare);

  if (UNLIKELY(_newPaxTypeFare && (_newPaxTypeFare == &paxTfare)))
    printDiag411BlankLine();

  if ((PricingTrx::IS_TRX == _trx.getTrxType()) &&
      (checkRetType == NONE_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED))
  {
    const CabinType& fareCabin = paxTfare.cabin();
    if (fareCabin.isPremiumEconomyClass() || fareCabin.isEconomyClass())
    {
      ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(&_trx);
      std::vector<ShoppingTrx::Leg>& legs = shoppingTrx->legs();
      ShoppingTrx::Leg currentLeg = legs[(paxTfare.fareMarket())->legIndex()];
      CabinType cab;
      cab.setPremiumEconomyClass();
      if (currentLeg.preferredCabinClass() < cab)
      {
        validReturnType = RET_FAIL;
      }
    }
  }

  return validReturnType;
}

//--------------------------------------------------------------------------------
bool
FareBookingCodeValidator::checkBookedClassAvail(const AirSeg& airSeg)
{
  // validate WPNC
  if (_trx.getRequest()->isLowFareRequested() && !_rexLocalMarketWPvalidation &&
      (_trx.getRequest()->upSellEntry() || PricingTrx::IS_TRX == _trx.getTrxType() ||
       PricingTrx::FF_TRX == _trx.getTrxType() ||
       (_trx.getTrxType() == PricingTrx::MIP_TRX && _trx.billing()->actionCode() != "WPNI.C" &&
        _trx.billing()->actionCode() != "WFR.C") ||
       airSeg.realResStatus() == QF_RES_STATUS))
    return true;

  return false;
}

//--------------------------------------------------------------------------------
bool
FareBookingCodeValidator::processFare(const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.fare()->isIndustry())
    return true;
  if (paxTypeFare.fare()->isPublished())
    return true;
  if (paxTypeFare.fare()->isConstructed())
    return true;
  if (paxTypeFare.isDiscounted())
    return true;
  if (LIKELY(paxTypeFare.isFareByRule()))
    return true;
  return false;
}

//--------------------------------------------------------------------------------
void
FareBookingCodeValidator::setCat31RbdInfo()
{
  _cat31Validator = _mkt->getCat31RbdValidator();
  if (!_cat31Validator)
  {
    _cat31Validator =
        &_trx.dataHandle().safe_create<Cat31FareBookingCodeValidator>(_trx, *_mkt, *_itin);
    _mkt->setCat31RbdValidator(_cat31Validator);
  }
}

//--------------------------------------------------------------------------------
void
FareBookingCodeValidator::setBkgStatus(PaxTypeFare& ptf,
                                       Cat31FareBookingCodeValidator::Result resultCat31) const
{
  PaxTypeFare::BookingCodeStatus& bookingCodeStatus =
      _fu ? _fu->bookingCodeStatus() : ptf.bookingCodeStatus();

  if (resultCat31 == Cat31FareBookingCodeValidator::FAILED)
  {
    bookingCodeStatus.set(PaxTypeFare::BKS_FAIL, true);
  }
  else if (resultCat31 == Cat31FareBookingCodeValidator::PASSED)
  {
    bookingCodeStatus.set(PaxTypeFare::BKS_PASS, true);
  }
  else if (resultCat31 == Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2)
  {
    bookingCodeStatus.set(PaxTypeFare::BKS_NEED_FARE_PATH, true);
  }

  std::vector<PaxTypeFare::SegmentStatus>& segmentStatuses =
      _fu ? _fu->segmentStatus()
          : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR) ? ptf.segmentStatus()
                                                             : ptf.segmentStatusRule2());

  for (PaxTypeFare::SegmentStatus& segmentStatus : segmentStatuses)
  {
    if (resultCat31 == Cat31FareBookingCodeValidator::FAILED)
    {
      segmentStatus._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
    }
    else if (resultCat31 == Cat31FareBookingCodeValidator::PASSED)
    {
      segmentStatus._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
    }
  }
}

//--------------------------------------------------------------------------------
void
FareBookingCodeValidator::initFuSegStatus()
{
  if (_fu == nullptr)
    return;
  if (_fu->segmentStatus().size() == _mkt->travelSeg().size())
    return;

  _fu->segmentStatus().reserve(_fu->segmentStatus().size() + _mkt->travelSeg().size());
  PaxTypeFare::SegmentStatus segStat;
  for (const auto& ts : _mkt->travelSeg())
  {
    segStat._bkgCodeSegStatus.setNull();

    if (ts->isAir())
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
    else
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
    _fu->segmentStatus().push_back(segStat);
  }
}

const BookingCodeExceptionSequenceList&
FareBookingCodeValidator::getBookingCodeExceptionSequence(const VendorCode& vendor,
                                                          const CarrierCode& carrier,
                                                          const TariffNumber& ruleTariff,
                                                          const RuleNumber& rule,
                                                          Indicator conventionNo,
                                                          bool& isRuleZero) const
{
  const DateTime& travelDate = _trx.adjustedTravelDate(_itin->travelSeg().front()->departureDT());

  return _trx.dataHandle().getBookingCodeExceptionSequence(vendor,
                                                           carrier,
                                                           ruleTariff,
                                                           rule,
                                                           conventionNo,
                                                           travelDate,
                                                           isRuleZero,
                                                           !_useBKGExceptionIndex);
}

void
FareBookingCodeValidator::setIgnoreCat31KeepFareFail(
    PaxTypeFare::BookingCodeStatus& bookingCodeStatus, PaxTypeFare& paxTypeFare)
{
  if (bookingCodeStatus.isSet(PaxTypeFare::BKS_FAIL))
  {
    if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
                 paxTypeFare.retrievalInfo() != nullptr && paxTypeFare.retrievalInfo()->keep()))
    {
      bookingCodeStatus.set(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE, true);
    }
  }
}

void
FareBookingCodeValidator::validatePrimeRBDRec1AndUpdateStatus(
    AirSeg& travelSeg,
    std::vector<BookingCode>& bkgCodes,
    const std::vector<ClassOfService*>* cosVec,
    PaxTypeFare& paxTfare,
    PaxTypeFare::SegmentStatus& segStat,
    bool dispRec1)
{
  if (validateSectorPrimeRBDRec1(travelSeg, bkgCodes, cosVec, paxTfare, segStat, dispRec1))
  {
    setPassBkgCodeSegmentStatus(segStat, 1);
  }
  else
  {
    setFailPrimeRBDInternationalStatus(segStat, 2); // "PRIME RBD VALIDATION STATUS - FAIL"
  }
}

bool
FareBookingCodeValidator::prevalidateRecord1BPrimeBkgCode(PaxTypeFare& paxTfare)
{
  size_t iCOS = 0;
  const size_t sizeCOS = _mkt->classOfServiceVec().size();
  std::vector<BookingCode> bkgCodes;
  bool result = true;

  paxTfare.getPrimeBookingCode(bkgCodes);

  if (sizeCOS != _mkt->travelSeg().size())
  {
    LOG4CXX_DEBUG(logger, "FareBookingCodeValidator::validateRBDInternational():size problem");
    return false;
  }

  for (TravelSegPtrVecI iterTvl = _mkt->travelSeg().begin(), iterTvlEnd = _mkt->travelSeg().end();
       iterTvl != iterTvlEnd && iCOS < sizeCOS;
       iterTvl++, iCOS++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    if (airSeg == nullptr)
      continue;

    PaxTypeFare::SegmentStatus& segStat =
        (_fu) ? _fu->segmentStatus()[iCOS] : ((_statusType != STATUS_FLOW_FOR_LOCAL_JRNY_CXR)
                                                  ? paxTfare.segmentStatus()[iCOS]
                                                  : paxTfare.segmentStatusRule2()[iCOS]);
    const std::vector<ClassOfService*>* cosVec = getAvailability(*iterTvl, segStat, paxTfare, iCOS);

    if (cosVec == nullptr)
      cosVec = _mkt->classOfServiceVec()[iCOS];

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
    {
      if (airSeg->carrier() == paxTfare.carrier())
      {
        if (validateSectorPrimeRBDRec1(*airSeg, bkgCodes, cosVec, paxTfare, segStat))
        {
          setPassBkgCodeSegmentStatus(segStat, 1);
        }
        else
        {
          result = false;
          break;
        }
      }
    }
  }

  result = result && (CheckBkgCodeSegmentStatus(paxTfare) == PASS);

  return result;
}

RepricingTrx*
FareBookingCodeValidator::getRepricingTrx(PricingTrx& trx,
                                          std::vector<TravelSeg*>& tvlSeg,
                                          FMDirection fmDirectionOverride)
{
  RepricingTrx* rpTrx = nullptr;

  try
  {
    rpTrx = TrxUtil::reprice(trx, tvlSeg, fmDirectionOverride, true);
  }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger,
                  "FareBookingCodeValidator: Exception during repricing with " << ex.code() << " - "
                                                                               << ex.message());
    return nullptr;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "FareBookingCodeValidator: Unknown exception during repricing");
    return nullptr;
  }

  return rpTrx;
}
bool
FareBookingCodeValidator::validateRBDRec1PriceByCabin(AirSeg& travelSeg,
                                                      std::vector<BookingCode>& bkgCodes,
                                                      PaxTypeFare& paxTfare,
                                                      PaxTypeFare::SegmentStatus& segStat)
{
  std::vector<ClassOfService*>* rbdRec1CabinVec = nullptr;
  DataHandle& dataHandle = _trx.dataHandle();
  dataHandle.get(rbdRec1CabinVec);
  getRBDRec1Cabin(travelSeg, bkgCodes, rbdRec1CabinVec);

  if (!rbdRec1CabinVec->empty())
  {
    for (auto cosv : *rbdRec1CabinVec)
    {
      ClassOfService& cos = *cosv;
      if (travelSeg.bookedCabin() != cos.cabin())
      {
        continue;
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
        segStat._bkgCodeReBook = cos.bookingCode();
        segStat._reBookCabin = cos.cabin();
        printDiag411Message(35); // APPLICABLE COS
        printDiag411RebookBkg(segStat._bkgCodeReBook);
        return true;
      }
    }
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
    printDiag411Message(68); // CABIN NOT MATCH
    return false;
  }
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
  segStat._bkgCodeReBook = bkgCodes[0];
  segStat._reBookCabin = paxTfare.cabin();
  printDiag411Message(35); // APPLICABLE COS
  printDiag411RebookBkg(segStat._bkgCodeReBook);
  return true;
}

void
FareBookingCodeValidator::getRBDRec1Cabin(AirSeg& travelSeg,
                                          std::vector<BookingCode>& bkgCodes,
                                          std::vector<ClassOfService*>* rbdRec1CabinVec)
{
  if (!fallback::fallbackPriceByCabinActivation(&_trx) &&
      !_trx.getRequest()->isjumpUpCabinAllowed())
  {
    RBDByCabinUtil rbdCabin(_trx, RBD_VAL);
    rbdCabin.getCabinsByRbd(travelSeg, bkgCodes, rbdRec1CabinVec);
  }
  return;
}

void
FareBookingCodeValidator::setFinalStatusForPriceByCabin(PaxTypeFare& paxTfare)
{
  int index = 0;
  for (auto tvlSeg : paxTfare.fareMarket()->travelSeg())
  {
    if (tvlSeg->isAir() && tvlSeg->rbdReplaced())
    {
      if (paxTfare.segmentStatus()[index]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      {
        if (paxTfare.segmentStatus()[index]._bkgCodeReBook.empty())
        {
          paxTfare.segmentStatus()[index]._bkgCodeReBook = tvlSeg->getBookingCode();
          paxTfare.segmentStatus()[index]._reBookCabin = tvlSeg->bookedCabin();
          paxTfare.segmentStatus()[index]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
        }
        else if (paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS_FLOW_AVAIL) ||
                 paxTfare.bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
        {
          if (paxTfare.segmentStatusRule2()[index]._bkgCodeReBook.empty())
          {
            paxTfare.segmentStatusRule2()[index]._bkgCodeReBook = tvlSeg->getBookingCode();
            paxTfare.segmentStatusRule2()[index]._reBookCabin = tvlSeg->bookedCabin();
            paxTfare.segmentStatusRule2()[index]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
          }
        }
      }
      else
      {
        paxTfare.segmentStatus()[index]._bkgCodeReBook = tvlSeg->getBookingCode();
        paxTfare.segmentStatus()[index]._reBookCabin = tvlSeg->bookedCabin();
        paxTfare.segmentStatus()[index]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);

        if (paxTfare.segmentStatusRule2()[index]._bkgCodeReBook.empty())
        {
          paxTfare.segmentStatusRule2()[index]._bkgCodeReBook = tvlSeg->getBookingCode();
          paxTfare.segmentStatusRule2()[index]._reBookCabin = tvlSeg->bookedCabin();
          paxTfare.segmentStatusRule2()[index]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
        }
      }
    }
    index++;
  }
}

void
FareBookingCodeValidator::setJumpedDownCabinAllowedStatus(PaxTypeFare& paxTfare)
{
  bool jumpedDownAllowed = false;
  for (TravelSeg* ts: paxTfare.fareMarket()->travelSeg())
  {
    std::vector<ClassOfService*>::const_iterator cosItr = ts->classOfService().begin();
    CabinType legPreferredCabin = _trx.legPreferredCabinClass()[ts->legId()];
    bool preferredCabinOffered = false;
    bool validSegment = false;
    bool isJumpedDownCabinOffered = false;

    while ((cosItr != ts->classOfService().end()) && (!preferredCabinOffered))
    {
      ClassOfService* cos = *cosItr;
      if (cos->cabin() == legPreferredCabin)
      {
        preferredCabinOffered = true;
        validSegment = true;
      }
      else if (!isJumpedDownCabinOffered)
      {
        CabinType currCabin = legPreferredCabin;
        CabinType nextJumpedDownCabin = legPreferredCabin;
        CabinType travelSegCabin = cos->cabin();

        while ((!isJumpedDownCabinOffered) && (!nextJumpedDownCabin.isInvalidClass()))
        {
          nextJumpedDownCabin = CabinType::addOneLevelToCabinType(currCabin);

          if (travelSegCabin == nextJumpedDownCabin)
          {
            isJumpedDownCabinOffered = true;
            validSegment = true;
          }
          currCabin = nextJumpedDownCabin;
        }
      }
      ++cosItr;
    }
    if(validSegment)
    {
      if (preferredCabinOffered)
        jumpedDownAllowed |=false;
      else if (isJumpedDownCabinOffered)
        jumpedDownAllowed |= true;
    }
  }
  paxTfare.jumpedDownCabinAllowed() = jumpedDownAllowed;
}

} // namespace tse
