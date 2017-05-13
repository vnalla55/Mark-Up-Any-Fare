//-----------------------------------------------------------------------------
//
//  File:     BookingCodeExceptionValidator.cpp
//            **This is NOT a C++ Exception **
//
//  Author :  Kul Shekhar
//            Linda Dillahunty
//
//  Copyright Sabre 2004
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "BookingCode/BookingCodeExceptionValidator.h"

#include "BookingCode/BCETuning.h"
#include "BookingCode/BookingCodeExceptionIndex.h"
#include "Common/AltPricingUtil.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/RtwUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag405Collector.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"
#include "Common/BookingCodeUtil.h"

#include <boost/bind.hpp>

#include <string>
#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackPriceByCabinActivation)
FALLBACK_DECL(fallbackAAExcludedBookingCode);

// Static initializers of constants
const int16_t BookingCodeExceptionValidator::BCE_FLT_NOMATCH = -1;

const CarrierCode BookingCodeExceptionValidator::BCE_DOLLARDOLLARCARRIER = "$$";
const CarrierCode BookingCodeExceptionValidator::BCE_XDOLLARCARRIER = "X$";
const CarrierCode BookingCodeExceptionValidator::BCE_ANYCARRIER = "";

const EquipmentType BookingCodeExceptionValidator::BCE_EQUIPBLANK = "";

const TSICode BookingCodeExceptionValidator::BCE_NO_TSI = 0;
const TSICode BookingCodeExceptionValidator::BCE_POS_TSI33 = 33;

const char* BookingCodeExceptionValidator::errorCodes[] = {
  "NO ERR",          "***FARE LOC NULL**", "***FLT LOC NULL**",
  "**FAIL DATE**\n", " **FAIL DOW**\n",    " **FAIL TIME**\n"
};

static Logger
logger("atseintl.BookingCode.BookingCodeExceptionValidator");

bool
BookingCodeExceptionValidator::validate(PricingTrx& trx,
                                        const BookingCodeExceptionSequenceList& bceSequenceList,
                                        PaxTypeFare& paxTypeFare,
                                        AirSeg* pAirSegConv1,
                                        BCETuning& bceTuning,
                                        FareUsage* fu,
                                        bool r1T999,
                                        bool cat25PrimeSector)
{
  LOG4CXX_DEBUG(logger, "Entered BookingCodeExceptionValidator::validate()");
  // if pAirSegConv1 == NULL then processing Record 1 or Record 6 Convention 2
  // else processing Reord 6 Convention 1
  _cat25PrimeSector = cat25PrimeSector;
  _rec1T999Cat25R3 = r1T999;
  _fu = fu;
  _bceTuning = &bceTuning;
  if (_bceTuning->_sequencesProcessed.size() != bceSequenceList.size())
    _stopTuning = true;
  createDiag(trx, paxTypeFare);
  setFirstLastAirSeg(paxTypeFare);
  const BookingCodeExceptionSequenceListCI sequenceIter = bceSequenceList.begin();
  const BookingCodeExceptionSequenceListCI sequenceIterEnd = bceSequenceList.end();

  if (UNLIKELY(sequenceIter == sequenceIterEnd))
  {
    LOG4CXX_DEBUG(logger, "Leaving BookingCodeExceptionValidator::validate(): failure");
    return false; // There are no sequences.
  }
  diagHeader(trx, pAirSegConv1, paxTypeFare);

  // ------------------------ TABLE TYPE (Byte 9)-------------------------------
  // Determine if table type applies to constructed or specified fares. Note
  // that this flag applies to all sequences for a given item number.

  switch ((*sequenceIter)->tableType())
  {
  case BCE_FARE_CONSTRUCTED:
    if (!paxTypeFare.isConstructed())
    {
      LOG4CXX_DEBUG(logger, "Leaving BookingCodeExceptionValidator::validate(): failure");
      return false;
    }
    break;

  case BCE_FARE_SPECIFIED: // not costructed = specified
    if (paxTypeFare.isConstructed())
    {
      LOG4CXX_DEBUG(logger, "Leaving BookingCodeExceptionValidator::validate(): failure");
      return false;
    }
    break;

  default:
    break;
  }

  // initialize _fltResultVector - it stores each flights
  // matching sequence number and segment number
  const uint32_t airSegSize = paxTypeFare.fareMarket()->travelSeg().size();
  resetFltResult(BCE_ALL_SEQUENCES, airSegSize);

  resetFirstMatchingSeqs(paxTypeFare);

  _statusType = STATUS_RULE1;
  validateSequence(trx, bceSequenceList, paxTypeFare, pAirSegConv1);
  diagResults(trx, paxTypeFare, pAirSegConv1);
  validateAsBooked(trx, bceSequenceList, paxTypeFare, pAirSegConv1);

  const bool rule2 = tryRule2(trx, paxTypeFare);
  // If its a WPNC entry then see if we should try Rule 2
  if (UNLIKELY(rule2))
  {
    resetFltResult(BCE_ALL_SEQUENCES, airSegSize);
    _statusType = STATUS_RULE2;
    std::vector<int32_t> saveFirstMatchingSeqs = _firstMatchingSeqs;
    validateSequence(trx, bceSequenceList, paxTypeFare, pAirSegConv1);
    diagResults(trx, paxTypeFare, pAirSegConv1);
    _firstMatchingSeqs = saveFirstMatchingSeqs;
    validateAsBooked(trx, bceSequenceList, paxTypeFare, pAirSegConv1);
  }

  // If Journey activated then lets try the Carriers with LOCAL
  // Journey active again to know their booking code status against
  // flow availability
  if (tryLocalWithFlowAvail(trx, paxTypeFare))
  {
    resetFltResult(BCE_ALL_SEQUENCES, airSegSize);
    _statusType = STATUS_JORNY;
    std::vector<int32_t> saveFirstMatchingSeqs = _firstMatchingSeqs;
    validateSequence(trx, bceSequenceList, paxTypeFare, pAirSegConv1);
    diagResults(trx, paxTypeFare, pAirSegConv1);
    _firstMatchingSeqs = saveFirstMatchingSeqs;
    validateAsBooked(trx, bceSequenceList, paxTypeFare, pAirSegConv1);
  }
  endDiag();
  LOG4CXX_DEBUG(logger, "Leaving BookingCodeExceptionValidator::validate(): success");
  return true;
  // we always pass because the individual flights are marked as to pass or fail
  // and the caller to this function is expected to check the status of those
  // individual flights
} // end validate

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::validateSequence(
    PricingTrx& trx,
    const BookingCodeExceptionSequenceList& bceSequenceList,
    PaxTypeFare& paxTypeFare,
    const AirSeg* pAirSeg)
{
  // if pAirSeg == NULL then processing Record 1 or Record 6 Convention 2
  // else processing Reord 6 Convention 1

  if (pAirSeg != nullptr &&
      !paxTypeFare.validForCmdPricing(false)) // Don't change 1st char for CommandPricing
  {
    std::string fareBasis = paxTypeFare.createFareBasis(trx, false);
    std::string fareClass = paxTypeFare.fareClass().c_str();
    uint16_t len = (fareBasis.size() < fareClass.size()) ? fareBasis.size() : fareClass.size();

    if ((len > 1) && (fareBasis.at(1) == '/'))
      len = 1;

    const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(paxTypeFare.fare());
    if ((paxTypeFare.fareMarket()->governingCarrier() == pAirSeg->carrier()) && indFare != nullptr &&
        indFare->changeFareClass() && paxTypeFare.getChangeFareBasisBkgCode().empty() &&
        !_bkgYYupdate && (len > 1))
    {
      analyzeSequencies(trx, bceSequenceList, paxTypeFare, pAirSeg);
    }
  }

  if (LIKELY(_useBKGExceptionIndex))
    validateSequenceList(trx, bceSequenceList, paxTypeFare, pAirSeg);
  else
    legacyValidateSequenceList(trx, bceSequenceList, paxTypeFare, pAirSeg);

  doDiag(bceSequenceList.size(), _iSequence);
  return;
}

//----------------------------------------------------------------------------

std::set<CarrierCode>
BookingCodeExceptionValidator::collectCarriersForChainsLookup(PaxTypeFare& paxTypeFare, bool chart2)
{
  std::set<CarrierCode> result;
  if (chart2)
  {
    Fare& fare = *(paxTypeFare.fare());
    if (fare.isIndustry())
      result.insert(INDUSTRY_CARRIER);
    result.insert(fare.carrier());
  }
  else
  {
    for (const TravelSeg* ts : paxTypeFare.fareMarket()->travelSeg())
    {
      if (ts->isAir())
      {
        const AirSeg* airSeg = static_cast<const AirSeg*>(ts);
        result.insert(airSeg->carrier());
      }
    }
  }

  result.insert(DOLLAR_CARRIER);

  return result;
}

//----------------------------------------------------------------------------
int
BookingCodeExceptionValidator::validateSequenceList(
    PricingTrx& trx,
    const BookingCodeExceptionSequenceList& bceSequenceList,
    PaxTypeFare& paxTypeFare,
    const AirSeg* pAirSeg)
{
  BookingCodeExceptionIndex::ConstPtr index;
  BookingCodeExceptionChains::Ptr chains;

  index =
      bceSequenceList.getIndex(boost::bind(&BookingCodeExceptionIndex::create, _1, pAirSeg != nullptr));

  diagIndex(trx, *index, paxTypeFare.fcaFareType(), paxTypeFare.fareClass());

  chains = index->lookupChains(collectCarriersForChainsLookup(paxTypeFare, pAirSeg != nullptr),
                               paxTypeFare.fcaFareType(),
                               paxTypeFare.fareClass());

  bool st = _stopTuning;
  _stopTuning = true;

  _iSequence = 0;
  _iSegment = 0;

  const BookingCodeExceptionSequence* bceSequence = nullptr;
  DateTime ticketDate = trx.dataHandle().ticketDate();
  if (UNLIKELY(ticketDate.isEmptyDate()))
    ticketDate = DateTime::localTime();

  initRtwPreferredCabin(trx, paxTypeFare);

  while ((bceSequence = chains->nextSequence()))
  {
    if (LIKELY(ticketDate <= bceSequence->expireDate()))
    {
      doDiag(*bceSequence);

      if (validateSingleSequence(trx, *bceSequence, paxTypeFare, pAirSeg))
      {
        doDiag();
        break;
      }
    }

    _iSequence++;
  }

  _stopTuning = st;

  return _iSequence;
}

//----------------------------------------------------------------------------
int
BookingCodeExceptionValidator::legacyValidateSequenceList(
    PricingTrx& trx,
    const BookingCodeExceptionSequenceList& bceSequenceList,
    PaxTypeFare& paxTypeFare,
    const AirSeg* pAirSeg)
{
  _iSequence = 0;
  _iSegment = 0;

  initRtwPreferredCabin(trx, paxTypeFare);

  uint16_t seqListSize = bceSequenceList.size();
  BookingCodeExceptionSequenceListCI sequenceIter = bceSequenceList.begin();
  const BookingCodeExceptionSequenceListCI sequenceIterEnd = bceSequenceList.end();
  for (; sequenceIter != sequenceIterEnd; sequenceIter++, _iSequence++)
  {
    const BookingCodeExceptionSequence& bceSequence = (**sequenceIter);
    doDiag(bceSequence);

    if (!_stopTuning)
    {
      if (_iSequence >= seqListSize)
      {
        _stopTuning = true;
      }
      else
      {
        BCESequenceProcessed& tuneSeq = _bceTuning->_sequencesProcessed[_iSequence];
        if (tuneSeq._seqProcessedOnce)
        {
          if (tuneSeq._noMatchSequence)
          {
            otherDiag(TUNING_SEQ_FAIL);
            continue;
          }
        }
        else
        {
          tuneSeq._sequenceNumber = bceSequence.seqNo();
          tuneSeq._noMatchSequence = false;
          tuneSeq._seqProcessedOnce = true;
          tuneSeq._fareProcessedSeq = paxTypeFare.fareClass();
        }
      }
    }

    if (validateSingleSequence(trx, bceSequence, paxTypeFare, pAirSeg))
    {
      doDiag();
      break;
    }

  } // end of sequenceIter iterator loop

  return _iSequence; // return number of sequences processed
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateSingleSequence(
    PricingTrx& trx,
    const BookingCodeExceptionSequence& bceSequence,
    PaxTypeFare& paxTypeFare,
    const AirSeg* pAirSeg)
{
  // --------------- CONSTRUCT/SPECIFIED TAG (Byte 14)----------------------
  switch (bceSequence.constructSpecified())
  {
  case BCE_FARE_CONSTRUCTED:
    if (!paxTypeFare.isConstructed())
    {
      doDiag(bceSequence, paxTypeFare, SEQTYPE_CONSTR_FARE);
      return false; // go to next sequence
    }
    break; // process this sequence

  case BCE_FARE_SPECIFIED:
    if (paxTypeFare.isConstructed())
    {
      doDiag(bceSequence, paxTypeFare, SEQTYPE_SPEC_FARE);
      return false; // go to next sequence, notConstructed = specified
    }
    break; // process this sequence

  default: // process this sequence
    break;
  }

  if (LIKELY(bceSequence.segCnt() != 0))
  {
    RtwPreferredCabinContext rtwPrefCabinCxt;
    prepareSequenceValidationRtw(rtwPrefCabinCxt, paxTypeFare);

    if (pAirSeg == nullptr)
    {
      if (allFltsDone(-1))
        return true;

      // validate sequence for both RBDs if the Restriction tag 'B' and 'D'
      if (!validateSequenceForBothRBDs(bceSequence))
        doDiag(bceSequence, paxTypeFare, SEQ_SEG_DUAL_RBD);
      else
      // validating table 999 from Record 1 / Record 6 Convention 2
         validateSegment(trx, bceSequence, paxTypeFare, -1, nullptr, nullptr);
    }
    else
    {
      // validating table 999 from Record 6 Convention 1
      const TravelSegVectorCI trvlIterBeg = paxTypeFare.fareMarket()->travelSeg().begin();
      const TravelSegVectorCI trvlIterEnd = paxTypeFare.fareMarket()->travelSeg().end();
      TravelSegVectorCI trvlIter = trvlIterBeg;
      int16_t iFlt = 0;
      for (iFlt = 0; trvlIter != trvlIterEnd; trvlIter++, iFlt++)
      {
        if ((*trvlIter) == pAirSeg)
          break;
      }

      if (allFltsDone(iFlt))
        return true;

      validateSegment(trx, bceSequence, paxTypeFare, iFlt, nullptr, nullptr);
    } // end if-else if(pAirSeg == 0)

    checkSequenceValidationRtw(rtwPrefCabinCxt, bceSequence, paxTypeFare);
  } // end if((*sequenceIter)->segCnt() != 0)

  return false;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateSequenceForBothRBDs(
    const BookingCodeExceptionSequence& bceSequence)
{
  return std::none_of(bceSequence.segmentVector().cbegin(),
                      bceSequence.segmentVector().cend(),
                      [](const auto* bceSegment)
                      {
    return (bceSegment->restrictionTag() == BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE ||
            bceSegment->restrictionTag() == BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE) &&
           (bceSegment->bookingCode1().empty() || bceSegment->bookingCode2().empty());
  });
  }

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::validateSegment(PricingTrx& trx,
                                               const BookingCodeExceptionSequence& bceSequence,
                                               PaxTypeFare& paxTypeFare,
                                               int16_t iFlt,
                                               FarePath* pfarePath,
                                               PricingUnit* pPU)
{
  bool bIfSegmentPassed = false;
  bool bSegmentPassed = false;
  bool isMultiSegmentSeq = (bceSequence.segmentVector().size() > 1);
  bool isIfTagAndFirstSegment = false;
  bool isIfTag =
      (bceSequence.ifTag() == BCE_IF_FARECOMPONENT || bceSequence.ifTag() == BCE_IF_ANY_TVLSEG);
  uint16_t fltIndex = 0;
  uint16_t segmentIndex = 0;
  BCEReturnTypes returnVal = BCE_PASS;

  const Fare& fare = *(paxTypeFare.fare());

  const TravelSegVectorCI trvlIterBeg = paxTypeFare.fareMarket()->travelSeg().begin();
  const TravelSegVectorCI trvlIterEnd = paxTypeFare.fareMarket()->travelSeg().end();
  TravelSegVectorCI trvlIter = trvlIterBeg;

  BookingCodeExceptionSegmentVectorCI segmentIter = bceSequence.segmentVector().begin();
  const BookingCodeExceptionSegmentVectorCI segmentIterEnd = bceSequence.segmentVector().end();

  const uint16_t segSize =
      bceSequence.segmentVector().size() > 0 ? bceSequence.segmentVector().size() : 1;
  const uint16_t fltSize = paxTypeFare.fareMarket()->travelSeg().size() > 0
                               ? paxTypeFare.fareMarket()->travelSeg().size()
                               : 1;
  int16_t segMatch[segSize];
  int16_t fltMatch[fltSize];
  PaxTypeFare::SegmentStatusVec prevSegStatusVec;
  for (int16_t i = 0; i < segSize; ++i)
    segMatch[i] = BCE_FLT_NOMATCH;

  for (int16_t i = 0; i < fltSize; ++i)
    fltMatch[i] = BCE_FLT_NOMATCH;

  // Preserve PaxTypeFare::SegmentStatusVec results to reset in case
  // we skip the sequence
  saveSegStatusResults(paxTypeFare, prevSegStatusVec);

  for (; segmentIter != segmentIterEnd; ++segmentIter, ++segmentIndex)
  {
    isIfTagAndFirstSegment = (segmentIndex == 0 && isIfTag);
    if (isIfTagAndFirstSegment)
      segMatch[segmentIndex] = BCE_SEG_IFTAG;

    bSegmentPassed = false;
    if (bceSequence.ifTag() == BCE_CHAR_BLANK)
    {
      bIfSegmentPassed = true;
    }
    else // If Tag is 1 or 2 -- so did the if segment pass ?
    {
      if (UNLIKELY(segmentIndex > 0 && !bIfSegmentPassed))
      { // The IFTAG segment did not pass, fail this sequence.
        doDiag(bceSequence, **segmentIter, 0, nullptr, IFTAG_FAIL);
        resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
        return; // next sequence
      }
    }
    const BookingCodeExceptionSegment& bceSegment = (**segmentIter);
    fltIndex = 0;
    trvlIter = trvlIterBeg;

    for (int16_t i = 0; i < fltSize; ++i)
      if (_fltResultVector[i].second != BCE_FLT_NOMATCH && fltMatch[i] != BCE_SEC_ARNK)
        fltMatch[i] = _fltResultVector[i].second;
    for (; trvlIter != trvlIterEnd; trvlIter++, fltIndex++)
    {
      //  if the segment is a open segment -- it will be an air seg with items empty
      //  so deal with it in each validate function.

      // if we came here from validatePU function then we need to validate only one travelSeg,
      // point correctly to that travelSeg before proceeding further
      if (iFlt != -1)
      {
        // make sure that even while processing the Record 6 Convention 1
        // We go thru all the flights if the IFTAG == 1 or == 2 and we are
        // processing the first BCE segment
        if (bceSequence.ifTag() == BCE_CHAR_BLANK || segmentIndex > 0)
        {
          if (fltIndex != iFlt)
            continue;
        }
      }
      const AirSeg* pAirSeg(nullptr);
      if ((*trvlIter)->isAir())
      {
        pAirSeg = static_cast<const AirSeg*>(*trvlIter);
      }
      else
      {
        fltMatch[fltIndex] = BCE_SEC_ARNK;
        continue;
      }
      if (_cat25PrimeSector)
      {
        if (paxTypeFare.fareMarket()->primarySector() == *trvlIter)
        {
          doDiag(bceSequence, bceSegment, fltIndex, pAirSeg, SKIP_FLT_CAT25);
          continue;
        }
      }

      if (_statusType != STATUS_RULE1)
      {
        // go to next sequence if there was no match found while doing Rule 1
        if (bceSequence.ifTag() == BCE_CHAR_BLANK || segmentIndex > 0)
        {
          if (_firstMatchingSeqs[fltIndex] == BCE_FLT_NOMATCH)
          {
            doDiag(bceSequence, bceSegment, fltIndex, pAirSeg, NO_SEQ_MATCHED_IN_RULE1);
            continue;
          }
          // if its not the sequence matched in Rule 1 then skip to next sequence
          if (_firstMatchingSeqs[fltIndex] != 0 &&
              (uint32_t)_firstMatchingSeqs[fltIndex] != bceSequence.seqNo())
          {
            doDiag(bceSequence, bceSegment, fltIndex, pAirSeg, NOT_FIRST_MATCHING_SEQ);
            continue;
          }
          // else put 0 in _firstMatchingSeqs[fltIndex] to indicate that we can
          // start processing the sequences for Rule from this sequence on
          _firstMatchingSeqs[fltIndex] = 0;
        }
      }
      // skip if this flight has already PASSED
      if (_fltResultVector[fltIndex].first != BCE_FLT_NOMATCH &&
          _fltResultVector[fltIndex].second != BCE_FLT_NOMATCH)
      {
        doDiag(bceSequence, bceSegment, fltIndex, pAirSeg, SKIP_FLT);
        continue; // we have already matched this flight
      }

      const AirSeg& airSeg = *pAirSeg;
      // ------------------------ PRIME (Byte 8) -------------------------------
      if (UNLIKELY(bceSequence.primeInd() == BCE_PRIME && airSeg.carrier() == fare.carrier()))
      {
        doDiag(bceSequence, bceSegment, fare, pAirSeg, PRIME_FAIL);
        continue; //   to next flight
      }

      if (UNLIKELY(!_stopTuning))
      {
        setISegment(segmentIndex, pAirSeg);
        BCESegmentProcessed& tuningSegm =
            _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
        if (tuningSegm._segmentProcessedOnce)
        {
          if (tuningSegm._noMatchSegment)
          {
            doDiag(bceSequence, bceSegment, fare, pAirSeg, TUNING_SEGM_FAIL);
            continue;
          }
        }
        else
        {
          tuningSegm._segmentProcessedOnce = true;
          tuningSegm._fareProcessedSegment = paxTypeFare.fareClass();
          tuningSegm._ifTag = bceSequence.ifTag();
          // if(segmentIndex == 0 && bceSequence.ifTag() == BCE_IF_FARECOMPONENT)
        }
      }
      // The returnVal can be BCE_PASS, BCE_NEXT_FLT, BCE_NEXT_SEQUENCE
      // In most functions only BCE_PASS and BCE_NEXT_FLT are returned.
      returnVal = validateCarrier(bceSequence, bceSegment, fare, airSeg, iFlt, fltIndex);
      if (UNLIKELY(returnVal == BCE_NEXT_SEQUENCE))
      {
        resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
        return; // next sequence
      }
      if (returnVal == BCE_NEXT_FLT)
        continue;

      if (!_useBKGExceptionIndex || !bceSequence.isSkipMatching())
      {
        returnVal = validatePrimarySecondary(
            bceSequence, bceSegment, fare, paxTypeFare.fareMarket()->primarySector(), *trvlIter);

        if (UNLIKELY(returnVal == BCE_NEXT_SEQUENCE))
        {
          resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
          return; // next sequence
        }

        if (UNLIKELY(returnVal == BCE_FROMTO_PRI)) // need to see if this is next to primary
        {
          const TravelSeg* primarySector = paxTypeFare.fareMarket()->primarySector();

          if (((trvlIter != trvlIterBeg) && (*(trvlIter - 1) == primarySector)) ||
              ((trvlIter != trvlIterEnd) && (*(trvlIter + 1) == primarySector)))
          {
            returnVal = BCE_PASS;
          }
          else
          {
            otherDiag(SEG_FAIL_BCE_FROMTO_PRI);
            returnVal = BCE_NEXT_FLT;
          }
        }

        if (returnVal == BCE_NEXT_FLT)
          continue;

        returnVal = validateFlights(bceSequence, bceSegment, airSeg);

        if (returnVal == BCE_NEXT_FLT)
        {
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::FLIGHT;
            }
          }
          continue;
        }

        returnVal = validateEquipment(bceSequence, bceSegment, airSeg);

        if (returnVal == BCE_NEXT_FLT)
        {
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::EQUIPMENT;
            }
          }
          continue;
        }

        returnVal = validatePortionOfTravel(bceSequence, bceSegment, fare, airSeg);

        if (returnVal == BCE_NEXT_FLT)
          continue;

        returnVal =
            validateTSI(bceSequence, bceSegment, trx, paxTypeFare, *trvlIter, pfarePath, pPU);

        if (returnVal == BCE_NEXT_FLT)
        {
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::TSI;
            }
          }
          continue;
        }

        returnVal = validateLocation(trx, bceSequence, bceSegment, paxTypeFare, airSeg);

        if (returnVal == BCE_NEXT_SEQUENCE)
        {
          resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
          return; // next sequence
        }
        if (returnVal == BCE_NEXT_FLT)
        {
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::LOCATION;
            }
          }
          continue;
        }

        returnVal = validatePointOfSale(bceSequence, bceSegment, trx, airSeg);

        if (returnVal == BCE_NEXT_FLT)
        {
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::POINT_OF_SALE;
            }
          }
          continue;
        }

        returnVal = validateSoldTag(bceSequence, bceSegment, trx);

        if (UNLIKELY(returnVal == BCE_NEXT_FLT))
        {
          if (!_stopTuning)
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::SOLD_TAG;
            }
          }
          continue;
        }

        returnVal = validateDateTimeDOW(bceSequence, bceSegment, trx, paxTypeFare, airSeg);

        if (returnVal == BCE_NEXT_SEQUENCE)
        {
          resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            _bceTuning->_sequencesProcessed[_iSequence]._noMatchSequence = true;
            _bceTuning->_sequencesProcessed[_iSequence]._fareProcessedSeq = paxTypeFare.fareClass();
            tuningSegm._noMatchSegment = true;
            tuningSegm._reasonNoMatch = BCESegmentProcessed::DATE_TIME_DOW;
          }
          return; // next sequence
        }
        if (returnVal == BCE_NEXT_FLT)
        {
          if (UNLIKELY(!_stopTuning))
          {
            BCESegmentProcessed& tuningSegm =
                _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
            if (tuningSegm._ifTag != BCE_IF_FARECOMPONENT)
            {
              tuningSegm._noMatchSegment = true;
              tuningSegm._reasonNoMatch = BCESegmentProcessed::DATE_TIME_DOW;
            }
          }
          continue;
        }
      }

      if (!_useBKGExceptionIndex || !bceSequence.isSkipFareClassOrTypeMatching())
      {
        returnVal = validateFareclassType(bceSequence, bceSegment, paxTypeFare, airSeg);

        if (returnVal == BCE_NEXT_SEQUENCE)
        {
          resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
          return; // next sequence
        }
        if (UNLIKELY(returnVal == BCE_NEXT_FLT))
          continue;
      }

      // If we got this far then the segment match passed.
      bSegmentPassed = true;

      // if the  segment is zero, we passed the IF TAG
      if (segmentIndex == 0)
      {
        bIfSegmentPassed = true;
        if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT || bceSequence.ifTag() == BCE_IF_ANY_TVLSEG)
        {
          doDiag(bceSequence, bceSegment, 0, pAirSeg, IFTAG_PASS);
        }
      }

      if (bceSequence.ifTag() == BCE_CHAR_BLANK || segmentIndex > 0)
      {
        // if here that means it is an AND segment and can have RBD
        // data applied. All segments other than zeroth
        // (when IF TAG ==1 or 2) are always AND and can have RBD
        // applied. Store the this passing sequence and segment
        // number for this flight
        // -- it may get reset in applyBookingCode().
        _fltResultVector[fltIndex].first = bceSequence.seqNo();
        _fltResultVector[fltIndex].second = bceSegment.segNo();
        doDiag(bceSequence, bceSegment, 0, pAirSeg, SEGMENT_PASS);

        // T999 validation
        applyBookingCodeForSingleFlight(trx, bceSequence, paxTypeFare, *trvlIter, fltIndex, iFlt);

        updateRtwPreferredCabin(bceSegment, paxTypeFare, fltIndex);

        PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);

        if (segStatusVec[fltIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH) &&
            !segStatusVec[fltIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_AVAILABILITY))
        {
          // neither match-pass nor match-fail, go to next segment or sequence
          _fltResultVector[fltIndex].first = BCE_FLT_NOMATCH;
          _fltResultVector[fltIndex].second = BCE_FLT_NOMATCH;

          if (LIKELY(fltMatch[fltIndex] == BCE_FLT_NOMATCH))
            fltMatch[fltIndex] = BCE_FLT_SKIPPED;

          if (segMatch[segmentIndex] == BCE_FLT_NOMATCH)
            segMatch[segmentIndex] = BCE_FLT_SKIPPED;

          doDiag(bceSequence, bceSegment, 0, pAirSeg, SEGMENT_SKIP);
        }
        else
        {
          fltMatch[fltIndex] = bceSegment.segNo();
          segMatch[segmentIndex] = fltIndex;

          // save the matching sequence for Rule2/ Local journey with flow avail
          if (_statusType == STATUS_RULE1 && _firstMatchingSeqs[fltIndex] == BCE_FLT_NOMATCH)
            _firstMatchingSeqs[fltIndex] = bceSequence.seqNo();
        }
      }
    } // end of loop _trvlSegIter

    bool isAllFltsMatchedFirstSeg = false;
    bool isAllFltsSkippedFirstSeg = false;
    bool isSegSkipped = false;
    if (isMultiSegmentSeq && !isIfTagAndFirstSegment)
    {
      if (segmentIndex == 0)
      {
        isAllFltsSkippedFirstSeg = isAllFlightsSkipped(fltMatch, fltSize);
        if (!isAllFltsSkippedFirstSeg)
          isAllFltsMatchedFirstSeg = isAllFlightsMatched(fltMatch, fltSize);
      }
      else
      {
        isSegSkipped = (segMatch[segmentIndex] == BCE_FLT_SKIPPED ||
                        segMatch[segmentIndex] == BCE_FLT_NOMATCH);
      }
    }

    if (bSegmentPassed == false || isAllFltsSkippedFirstSeg || isAllFltsMatchedFirstSeg ||
        isSegSkipped)
    {
      // We have looped thru all of the flights for this segment
      // and none passed. Therefore, this sequence does not match,
      // reset any thing saved in the fltResultVec for this
      // sequence and go on to the next.

      if (isAllFltsMatchedFirstSeg || isSegSkipped)
        restoreSegStatusResults(paxTypeFare, prevSegStatusVec);
      resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
      return; // next sequence
    }
  } // end of loop segmentIter

  if (isMultiSegmentSeq && !isIfTagAndFirstSegment && isSegmentNoMatched(segMatch, segSize))
  {
    resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
    restoreSegStatusResults(paxTypeFare, prevSegStatusVec);
    return;
  }
} // end validateSegment

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateCarrier(const BookingCodeExceptionSequence& bceSequence,
                                               const BookingCodeExceptionSegment& bceSegment,
                                               const Fare& fare,
                                               const AirSeg& airSeg,
                                               const int16_t iFlt,
                                               const uint16_t fltIndex)
{
  if (UNLIKELY(bceSegment.viaCarrier() == BCE_DOLLARDOLLARCARRIER ||
      bceSegment.viaCarrier() == BCE_ANYCARRIER))
  {
    return BCE_PASS;
  }

  if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1)
  {
    if (bceSegment.viaCarrier() == BCE_XDOLLARCARRIER)
    {
      if (fare.isIndustry())
        return BCE_PASS;

      if (LIKELY(iFlt != -1))
      {
        // only when processing record 6 conv 1 we should do this check
        // If X$ found in the IFTAG segment when processing record 6 convention 1
        // then if the carrier of the flight that we are processing for rec 6 conv 1
        // does not match with 'owning carrier' of fare then we PASS IFTAG
        if (iFlt != fltIndex)
        {
          doDiag(bceSequence, bceSegment, fare, &airSeg, CXR_FAIL_NXTFLT);
          return BCE_NEXT_FLT;
        }
        else
        {
          if (LIKELY(airSeg.carrier() != fare.carrier()))
            return BCE_PASS;
        }
      }
    }
    if (bceSegment.viaCarrier() == INDUSTRY_CARRIER)
    {
      if (fare.isIndustry())
        return BCE_PASS;
    }
    if (LIKELY(bceSegment.viaCarrier() == fare.carrier()))
    {
      return BCE_PASS;
    }
    doDiag(bceSequence, bceSegment, fare, &airSeg, CXR_FAIL_NXTSEQ);
    return BCE_NEXT_SEQUENCE;
  }
  else // if(bceSequence._ifTag == BCE_MATCH_ANY_TVLSEG || blank)
  {
    if (bceSegment.viaCarrier() == BCE_XDOLLARCARRIER)
    {
      if (fare.isIndustry() || (airSeg.carrier() != fare.carrier()))
        return BCE_PASS;
    }
    if (bceSegment.viaCarrier() == airSeg.carrier())
    {
      return BCE_PASS;
    }
    doDiag(bceSequence, bceSegment, fare, &airSeg, CXR_FAIL_NXTFLT);
    return BCE_NEXT_FLT;
  }
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validatePrimarySecondary(
    const BookingCodeExceptionSequence& bceSequence,
    const BookingCodeExceptionSegment& bceSegment,
    const Fare& fare,
    const TravelSeg* primarySector,
    const TravelSeg* travelSeg)
{
  if (bceSegment.primarySecondary() == BCE_CHAR_BLANK)
  {
    return BCE_PASS;
  }
  if (UNLIKELY((bceSegment.primarySecondary() == BCE_SEG_PRIMARY ||
       bceSegment.primarySecondary() == BCE_SEG_SECONDARY ||
       bceSegment.primarySecondary() == BCE_SEG_FROMTO_PRIMARY) &&
      (fare.isDomestic() || fare.isTransborder() || fare.isForeignDomestic())))
  {
    doDiag(bceSequence, bceSegment, 0, nullptr, PRIMRY_SECNDRY_NXTSEQ);
    return BCE_NEXT_SEQUENCE;
  }

  if (bceSegment.primarySecondary() == BCE_SEG_PRIMARY)
  {
    if (travelSeg == primarySector)
    {
      return BCE_PASS;
    }
  }
  else if (UNLIKELY(bceSegment.primarySecondary() == BCE_SEG_FROMTO_PRIMARY))
  {
    // This travel seg need to be Secondary next to Primary
    if (travelSeg == primarySector)
    {
      return BCE_NEXT_FLT;
    }
    else
    {
      return BCE_FROMTO_PRI; // See if this is next to primary after return
    }
  }
  else // must be Secondary because we already checked for blank
  {
    if (travelSeg != primarySector)
    {
      return BCE_PASS;
    }
  }
  doDiag(bceSequence, bceSegment, 0, nullptr, PRIMRY_SECNDRY_NXTFLT);
  return BCE_NEXT_FLT;
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateFlights(const BookingCodeExceptionSequence& bceSequence,
                                               const BookingCodeExceptionSegment& bceSegment,
                                               const AirSeg& airSeg)
{
  if (bceSegment.flight1() <= 0)
  {
    return BCE_PASS;
  }
  switch (bceSegment.fltRangeAppl())
  {
  case BCE_FLTINDIVIDUAL:
  {
    if (airSeg.flightNumber() != bceSegment.flight1())
    {
      if (bceSegment.flight2() == 0 || airSeg.flightNumber() != bceSegment.flight2())
      {
        doDiag(bceSequence, bceSegment, 0, &airSeg, FLT_INDIVIDUAL_FAIL);
        return BCE_NEXT_FLT;
      }
    }
    return BCE_PASS;
  }
  case BCE_FLTRANGE:
  {
    if (airSeg.flightNumber() < bceSegment.flight1() ||
        airSeg.flightNumber() > bceSegment.flight2())
    {
      doDiag(bceSequence, bceSegment, 0, &airSeg, FLT_RANGE_FAIL);
      return BCE_NEXT_FLT;
    }
    return BCE_PASS;
  }

  default:
  {
    return BCE_PASS; // Flight Range is not valid or blank.
  }
  } // end of switch
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateEquipment(const BookingCodeExceptionSequence& bceSequence,
                                                 const BookingCodeExceptionSegment& bceSegment,
                                                 const AirSeg& airSeg)
{
  if (bceSegment.equipType() == BCE_EQUIPBLANK || bceSegment.equipType().empty())
  {
    return BCE_PASS;
  }

  if (bceSegment.equipType() == airSeg.equipmentType())
  {
    return BCE_PASS;
  }
  doDiag(bceSequence, bceSegment, 0, &airSeg, EQUIPMENT_FAIL);
  return BCE_NEXT_FLT;
}

// TODO  -----
// Remember in each of these functions to handle the case where the
// pFlight->xxxxxx is blank or zero or empty -- as it might be on
// an open sector.  Most of the time -- it looks like it will naturally get
// taken care of because it won't match the bceSegment data -- but be aware.
//
//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validatePortionOfTravel(
    const BookingCodeExceptionSequence& bceSequence,
    const BookingCodeExceptionSegment& bceSegment,
    const Fare& fare,
    const AirSeg& airSeg)
{
  const std::string tvlPortion = bceSegment.tvlPortion();
  const IATAAreaCode& origArea = airSeg.origin()->area();
  const IATAAreaCode& destArea = airSeg.destination()->area();

  switch (tvlPortion[0])
  {
  case 'A': // if tvlPortion = "AT"
  {
    // pass if one end point of the sector is in area 1 and other point of the
    // sector is in  area 2/3 AND global direction of fare is Atlantic ocean

    if (fare.globalDirection() == GlobalDirection::AT && origArea != destArea &&
        (origArea == IATA_AREA1 || destArea == IATA_AREA1))
    {
      if (destArea == IATA_AREA2 || destArea == IATA_AREA3 || origArea == IATA_AREA2 ||
          origArea == IATA_AREA3)
      {
        return BCE_PASS;
      }
    }
    break;
  }

  case 'C':
  {
    if (tvlPortion[1] == 'A') // if tvlPortion == "CA"
    {
      // Canadian Domestic
      if (airSeg.origin()->nation() == CANADA && airSeg.destination()->nation() == CANADA)
      {
        return BCE_PASS;
      }
    }
    else
    {
      if (tvlPortion[1] == 'O') // if tvlPortion = "CO"
      {
        // Controlling Portion
        if ((origArea != destArea) || (airSeg.origin()->nation() != airSeg.destination()->nation()))
        {
          return BCE_PASS;
        }
      }
    }
    break;
  }

  case 'D': // if tvlPortion = "DO"
  {
    // Domestic
    if (LocUtil::isDomestic(*(airSeg.origin()), *(airSeg.destination())) ||
        LocUtil::isForeignDomestic(*(airSeg.origin()), *(airSeg.destination())))
    {
      return BCE_PASS;
    }
    break;
  }

  case 'E': // if tvlPortion = "EH"
  {
    // eastern hemisphere
    // Both end points of the flight are in Area 02 or One end point is in area 02
    // and other in Area 03 then pass

    if ((origArea == IATA_AREA2 && destArea == IATA_AREA2) ||
        (origArea == IATA_AREA2 && destArea == IATA_AREA3) ||
        (destArea == IATA_AREA2 && origArea == IATA_AREA3))
    {
      return BCE_PASS;
    }
    break;
  }

  case 'F':
  {
    if (tvlPortion[1] == 'D') // if tvlPortion = "FD"
    {
      // Domestic except US/CA
      if (LocUtil::isForeignDomestic(*(airSeg.origin()), *(airSeg.destination())) &&
          !LocUtil::isDomestic(*(airSeg.origin()), *(airSeg.destination())))
      {
        return BCE_PASS;
      }
    }
    else
    {
      if (tvlPortion[1] == 'E') // if tvlPortion = "FE"
      {
        // Fare East
        // Pass if all points are in area 3
        if (origArea == IATA_AREA3 && destArea == IATA_AREA3)
        {
          return BCE_PASS;
        }
      }
    }
    break;
  }

  case 'P': // if tvlPortion = "PA"
  {
    // pass if one end point of the sector is in area 1 and other point of the
    // sector is in  area 2/3 AND global direction of fare refers to Pacific ocean

    if (fare.globalDirection() == GlobalDirection::PA && origArea != destArea &&
        (origArea == IATA_AREA1 || destArea == IATA_AREA1))
    {
      if (destArea == IATA_AREA2 || destArea == IATA_AREA3 || origArea == IATA_AREA2 ||
          origArea == IATA_AREA3)
      {
        return BCE_PASS;
      }
    }
    break;
  }

  case 'T': // if tvlPortion = "TB"
  {
    if (tvlPortion[1] == 'B') // if tvlPortion = "TB" Transborder US/CA
    {
      if (LocUtil::isTransBorder(*(airSeg.origin()), *(airSeg.destination())))
        return BCE_PASS;
    }

    if (tvlPortion[1] == 'M') // if tvlPortion = "TM" Transborder US/Mexico
    {
      if (LocUtil::isUS(*(airSeg.origin())) && LocUtil::isMexico(*(airSeg.destination())))
        return BCE_PASS;

      if (LocUtil::isMexico(*(airSeg.origin())) && LocUtil::isUS(*(airSeg.destination())))
        return BCE_PASS;
    }
    break;
  }
  case 'U': // if tvlPortion = "US"
  {
    // US Domestic
    if (airSeg.origin()->nation() == UNITED_STATES &&
        airSeg.destination()->nation() == UNITED_STATES)
    {
      return BCE_PASS;
    }
    break;
  }
  case 'W': // if tvlPortion = "WH"
  {
    // Western Hemisphere
    if (origArea == IATA_AREA1 && destArea == IATA_AREA1)
    {
      return BCE_PASS;
    }
    break;
  }

  default:
  {
    return BCE_PASS;
  }
  } // end switch
  doDiag(bceSequence, bceSegment, 0, &airSeg, PORTION_OF_TVL_FAIL);
  return BCE_NEXT_FLT;
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateTSI(const BookingCodeExceptionSequence& bceSequence,
                                           const BookingCodeExceptionSegment& bceSegment,
                                           PricingTrx& trx,
                                           PaxTypeFare& paxTypeFare,
                                           const TravelSeg* travelSeg,
                                           const FarePath* pfarePath,
                                           const PricingUnit* pPU)
{
  const TSICode bceTSI = bceSegment.tsi();
  RuleUtil::TravelSegWrapperVector applTvlSegs;
  bool tsiResult = false;
  const FareMarket* fareMarket = paxTypeFare.fareMarket();

  LocKey locKey1;
  LocKey locKey2;

  locKey1.locType() = LOCTYPE_NONE;
  locKey2.locType() = LOCTYPE_NONE;

  if (bceTSI == BCE_NO_TSI)
  {
    return BCE_PASS;
  }

  tsiResult = RuleUtil::scopeTSIGeo(bceTSI,
                                    locKey1,
                                    locKey2,
                                    RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                    false,
                                    false,
                                    false,
                                    trx,
                                    pfarePath,
                                    nullptr,
                                    pPU,
                                    fareMarket,
                                    trx.getRequest()->ticketingDT(),
                                    applTvlSegs,
                                    Diagnostic405);

  if (LIKELY(tsiResult))
  {
    RuleUtil::TravelSegWrapperVectorCI applTvlSegsIt = applTvlSegs.begin();
    const RuleUtil::TravelSegWrapperVectorCI applTvlSegsEnd = applTvlSegs.end();

    for (; applTvlSegsIt != applTvlSegsEnd; ++applTvlSegsIt)
    {
      if ((*applTvlSegsIt)->travelSeg() == travelSeg)
        break;
    }

    if (applTvlSegsIt == applTvlSegsEnd)
    {
      doDiag(bceSequence, bceSegment, 0, nullptr, TSI_FC_FAIL);
      return BCE_NEXT_FLT;
    }
  }
  else
  {
    doDiag(bceSequence, bceSegment, 0, nullptr, TSI_FC_BAD2);
    return BCE_NEXT_FLT;
  }

  return BCE_PASS;
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateLocation(PricingTrx& trx,
                                                const BookingCodeExceptionSequence& bceSequence,
                                                const BookingCodeExceptionSegment& bceSegment,
                                                const PaxTypeFare& paxTypeFare,
                                                const AirSeg& airSeg)
{
  if (bceSegment.loc1().empty() && bceSegment.loc2().empty())
  {
    return BCE_PASS;
  }

  const Fare& fare = *(paxTypeFare.fare());

  const Loc* fareOrigLoc = paxTypeFare.fareMarket()->origin();
  const Loc* fareDestLoc = paxTypeFare.fareMarket()->destination();

  ErrorCode eCode = NO_ERR;

  if (UNLIKELY(fareOrigLoc == nullptr || fareDestLoc == nullptr))
  {
    eCode = FARE_LOC_NULL;
  }

  if (UNLIKELY(airSeg.origin() == nullptr || airSeg.destination() == nullptr))
  {
    eCode = FLT_LOC_NULL;
  }

  if (UNLIKELY(eCode != NO_ERR))
  {
    if (UNLIKELY(diag405()))
    {
      *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
               << " SEGNUM: " << bceSegment.segNo() << "    " << errorCodes[eCode] << "\n"
               << "   FARE ORIG DEST: " << fare.origin() << fare.destination()
               << " BCELOC1LOC2: " << bceSegment.loc1() << bceSegment.loc2()
               << "\n   IFTAG: " << bceSequence.ifTag() << bceSegment.directionInd()
               << " NEXT SEQUENCE \n";
    }
    return BCE_NEXT_SEQUENCE;
  }

  bool failSequence = false;
  bool failFlight = false;
  switch (bceSegment.directionInd())
  {
  case '1': // Travel from Loc1 to Loc2
  {
    if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1) // if IFTag == 1
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*fareOrigLoc,
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*fareDestLoc,
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }
    }
    else // if IFTag == blank or 2
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*(airSeg.origin()),
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failFlight = true;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*(airSeg.destination()),
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failFlight = true;
        break;
      }
    }
    break;
  }

  case '2': // Travel from Loc2 to Loc1
  {
    if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1) // if IFTag == 1
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*fareDestLoc,
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*fareOrigLoc,
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }
    }
    else // if IFTag == blank or 2
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*(airSeg.destination()),
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failFlight = true;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*(airSeg.origin()),
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failFlight = true;
        break;
      }
    }
    break;
  }

  case '3': // Fares originating Loc1
  {
    if (paxTypeFare.directionality() == TO)
    {
      // if the fare is INBOUND then LOC1 should be checked against the
      // destination because destination is actually the origination point
      // in this case. This is valid only for directionality values 3 and 4
      // and not for values 1 and 2
      fareDestLoc = paxTypeFare.fareMarket()->origin();
      fareOrigLoc = paxTypeFare.fareMarket()->destination();
    }
    if (LIKELY(bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1)) // if IFTag == 1
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*fareOrigLoc,
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*fareDestLoc,
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }
    }
    else // When directional indicator == 3 the IFTag must not be blank or 2
    {
      failFlight = true;
      break;
    }
    break;
  }
  case '4': // Fares originating Loc2
  {
    if (paxTypeFare.directionality() == TO)
    {
      // if the fare is INBOUND then LOC1 should be checked against the
      // destination because destination is actually the origination point
      // in this case. This is valid only for directionality values 3 and 4
      // and not for values 1 and 2
      fareDestLoc = paxTypeFare.fareMarket()->origin();
      fareOrigLoc = paxTypeFare.fareMarket()->destination();
    }
    if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1) // if IFTag == 1
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*fareDestLoc,
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*fareOrigLoc,
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        failSequence = true;
        break;
      }
    }
    else // When directional indicator == 3 the IFTag must not blank or 2
    {
      failFlight = true;
      break;
    }
    break;
  }
  case ' ': // Travel/fare between Loc 1 and Loc2
  {
    bool origInLoc1 = false;
    bool origInLoc2 = false;
    bool destInLoc1 = false;
    bool destInLoc2 = false;
    if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1) // if IFTag == 1
    {
      if (bceSegment.loc1() == bceSegment.loc2() &&
          bceSegment.loc1Type() == bceSegment.loc2Type() && !bceSegment.loc1().empty())
      {
        // WITHIN loc1
        if (!(LocUtil::isInLoc(*fareOrigLoc,
                               bceSegment.loc1Type(),
                               bceSegment.loc1(),
                               ATPCO_VENDOR_CODE,
                               RESERVED,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT()) &&
              LocUtil::isInLoc(*fareDestLoc,
                               bceSegment.loc1Type(),
                               bceSegment.loc1(),
                               ATPCO_VENDOR_CODE,
                               RESERVED,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT())))
        {
          failSequence = true;
          break;
        }
      }
      else
      {
        // BETWEEN loc1/loc2
        if (bceSegment.loc1().empty() || LocUtil::isInLoc(*fareOrigLoc,
                                                          bceSegment.loc1Type(),
                                                          bceSegment.loc1(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          origInLoc1 = true;
        }

        if (bceSegment.loc2().empty() || LocUtil::isInLoc(*fareDestLoc,
                                                          bceSegment.loc2Type(),
                                                          bceSegment.loc2(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          destInLoc2 = true;
        }

        if (bceSegment.loc2().empty() || LocUtil::isInLoc(*fareOrigLoc,
                                                          bceSegment.loc2Type(),
                                                          bceSegment.loc2(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          origInLoc2 = true;
        }

        if (bceSegment.loc1().empty() || LocUtil::isInLoc(*fareDestLoc,
                                                          bceSegment.loc1Type(),
                                                          bceSegment.loc1(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          destInLoc1 = true;
        }

        if (!((origInLoc1 && destInLoc2) || (origInLoc2 && destInLoc1)))
        {
          failSequence = true;
          break;
        }
      }
    }
    else // if IFTag == blank or 2
    {
      if (bceSegment.loc1() == bceSegment.loc2() &&
          bceSegment.loc1Type() == bceSegment.loc2Type() && !bceSegment.loc1().empty())
      {
        // WITHIN loc1
        if (!(LocUtil::isInLoc(*(airSeg.origin()),
                               bceSegment.loc1Type(),
                               bceSegment.loc1(),
                               ATPCO_VENDOR_CODE,
                               RESERVED,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT()) &&
              LocUtil::isInLoc(*(airSeg.destination()),
                               bceSegment.loc1Type(),
                               bceSegment.loc1(),
                               ATPCO_VENDOR_CODE,
                               RESERVED,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT())))
        {
          failFlight = true;
          break;
        }
      }
      else
      {
        // BETWEEN loc1/loc2
        if (bceSegment.loc1().empty() || LocUtil::isInLoc(*(airSeg.origin()),
                                                          bceSegment.loc1Type(),
                                                          bceSegment.loc1(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          origInLoc1 = true;
        }

        if (bceSegment.loc2().empty() || LocUtil::isInLoc(*(airSeg.destination()),
                                                          bceSegment.loc2Type(),
                                                          bceSegment.loc2(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          destInLoc2 = true;
        }

        if (bceSegment.loc2().empty() || LocUtil::isInLoc(*(airSeg.origin()),
                                                          bceSegment.loc2Type(),
                                                          bceSegment.loc2(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          origInLoc2 = true;
        }

        if (bceSegment.loc1().empty() || LocUtil::isInLoc(*(airSeg.destination()),
                                                          bceSegment.loc1Type(),
                                                          bceSegment.loc1(),
                                                          ATPCO_VENDOR_CODE,
                                                          RESERVED,
                                                          LocUtil::OTHER,
                                                          GeoTravelType::International,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
        {
          destInLoc1 = true;
        }

        if (!((origInLoc1 && destInLoc2) || (origInLoc2 && destInLoc1)))
        {
          failFlight = true;
          break;
        }
      }
    }
    break;
  }
  default: // if direction indicaor is anyrhing else then 1,2,3,4 or blank
  {
    failSequence = true;
    break;
  }
  } // end SWITCH
  if (failSequence)
  {
    doDiag(bceSequence, bceSegment, fare, nullptr, LOC_FAIL_NXTSEQ);
    return BCE_NEXT_SEQUENCE;
  }

  if (failFlight)
  {
    doDiag(bceSequence, bceSegment, 0, &airSeg, LOC_FAIL_NXTFLT);
    return BCE_NEXT_FLT;
  }

  return BCE_PASS;
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validatePointOfSale(const BookingCodeExceptionSequence& bceSequence,
                                                   const BookingCodeExceptionSegment& bceSegment,
                                                   PricingTrx& trx,
                                                   const AirSeg& airSeg)
{
  if (bceSegment.posLoc().empty() || bceSegment.posLocType() == '\0')
  {
    return BCE_PASS;
  }
  else
  {
    const bool agentInPOSLocation =
        LocUtil::isInLoc(*(trx.getRequest()->ticketingAgent()->agentLocation()),
                         bceSegment.posLocType(),
                         bceSegment.posLoc(),
                         ATPCO_VENDOR_CODE,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT());

    if (UNLIKELY(bceSegment.posTsi() == BCE_POS_TSI33)) // agent must not be in POS Location
    {
      if (!agentInPOSLocation)
      {
        return BCE_PASS;
      }
    }
    else // agent must be in POS Location
    {
      if (agentInPOSLocation)
      {
        return BCE_PASS;
      }
    }
  }
  doDiag(bceSequence, bceSegment, trx, POINT_OF_SALE_FAIL);
  return BCE_NEXT_FLT;
}

BCEReturnTypes
BookingCodeExceptionValidator::validateSoldTag(const BookingCodeExceptionSequence& bceSequence,
                                               const BookingCodeExceptionSegment& bceSegment,
                                               PricingTrx& trx)
{
  switch (bceSegment.soldInOutInd())
  {
  case '1': // SITI
    if (_itin.intlSalesIndicator() == Itin::SITI)
    {
      return BCE_PASS;
    }
    break;

  case '2': // SOTO
    if (_itin.intlSalesIndicator() == Itin::SOTO)
    {
      return BCE_PASS;
    }
    break;

  case '3': // SITO
    if (_itin.intlSalesIndicator() == Itin::SITO)
    {
      return BCE_PASS;
    }
    break;

  case '4': // SOTI
    if (_itin.intlSalesIndicator() == Itin::SOTI)
    {
      return BCE_PASS;
    }
    break;

  default: // BLANK OR NULL CHAR
    return BCE_PASS;
  }

  doDiag(bceSequence, bceSegment, trx, SOLD_TAG_FAIL);
  return BCE_NEXT_FLT;
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateDateTimeDOW(const BookingCodeExceptionSequence& bceSequence,
                                                   const BookingCodeExceptionSegment& bceSegment,
                                                   PricingTrx& trx,
                                                   const PaxTypeFare& paxTypeFare,
                                                   const AirSeg& airSeg)
{
  bool arrivalTSI = false;
  bool checkOrig = true;
  bool checkDest = false;

  const uint16_t bceStartYear = bceSegment.tvlEffYear();
  const uint16_t bceStartMonth = bceSegment.tvlEffMonth();
  const uint16_t bceStartDay = bceSegment.tvlEffDay();

  const uint16_t bceStopYear = bceSegment.tvlDiscYear();
  const uint16_t bceStopMonth = bceSegment.tvlDiscMonth();
  const uint16_t bceStopDay = bceSegment.tvlDiscDay();

  const uint16_t bceStartTime = bceSegment.tvlStartTime();
  const uint16_t bceStopTime = bceSegment.tvlEndTime();

  DateTime dateUsed = airSeg.departureDT(); // initialize

  int fltDOW = 0;
  ErrorCode eCode;

  const TSICode tsi = bceSegment.tsi();

  if (UNLIKELY(tsi != 0 && RuleUtil::getTSIOrigDestCheck(tsi, trx, checkOrig, checkDest) && !checkOrig))
  {
    arrivalTSI = true;
    dateUsed = airSeg.arrivalDT();
  }

  uint16_t fltTime = dateUsed.totalMinutes();

  if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1) // if IFTag == 1
  {
    bool failSequence = false;
    if (UNLIKELY(arrivalTSI))
    {
      if (_lastAirsegInMkt != nullptr)
      {
        dateUsed = _lastAirsegInMkt->arrivalDT();
      }
    }
    else
    {
      if (LIKELY(_firstAirsegInMkt != nullptr))
      {
        dateUsed = _firstAirsegInMkt->departureDT();
      }
    }

    // first validate the start date
    if (!(validateStartDate(dateUsed, bceStartYear, bceStartMonth, bceStartDay)))
    {
      failSequence = true;
      eCode = FAIL_DATE;
    }

    // Now validate the stop date
    if (!failSequence)
    {
      if (!(validateStopDate(dateUsed, bceStopYear, bceStopMonth, bceStopDay)))
      {
        failSequence = true;
        eCode = FAIL_DATE;
      }
    }
    // validate D O W
    if (!failSequence)
    {
      fltDOW = dateUsed.dayOfWeek();
      if (fltDOW == 0)
        fltDOW = 7; // in DateTime 0...6, in database 1...7
      if (UNLIKELY(!bceSegment.daysOfWeek().empty() &&
          bceSegment.daysOfWeek().find_first_of(static_cast<char>(fltDOW + '0')) ==
              std::string::npos)) // lint !e530
      {
        failSequence = true;
        eCode = FAIL_DOW;
      }
    }

    // validate start stop Time
    if (!failSequence)
    {
      fltTime = dateUsed.totalMinutes();
      if (UNLIKELY((bceStartTime != 0 && bceStartTime != 65535 && fltTime < bceStartTime) ||
          (bceStopTime != 0 && bceStopTime != 65535 && fltTime > bceStopTime)))
      {
        failSequence = true;
        eCode = FAIL_TIME;
      }
    }
    if (failSequence)
    {
      if (UNLIKELY(diag405() && !_shortDiag))
      {
        *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
                 << " SEGNUM: " << bceSegment.segNo() << errorCodes[eCode]
                 << "   BCE TVLEFFDATE: " << bceStartMonth << "/" << bceStartDay << "/"
                 << bceStartYear << "  BCE TVLDISCDATE: " << bceStopMonth << "/" << bceStopDay
                 << "/" << bceStopYear << " \n"
                 << "   BCE STARTTIME: " << bceStartTime << "  BCE STOPTIME: " << bceStopTime
                 << "  FLT TIME: " << fltTime << " \n"
                 << "   BCE DOW: " << bceSegment.daysOfWeek() << "  FLT DOW: " << fltDOW
                 << "  TSI: " << bceSegment.tsi();
        if (arrivalTSI)
          *(_diag) << " - ARRIVAL TSI";
        *(_diag) << "\n   NEXT SEQUENCE \n";
      }
      return BCE_NEXT_SEQUENCE;
    }
  }
  else // IFtag blank or 2
  {
    bool failFlight = false;

    // first validate the start date
    if (!(validateStartDate(dateUsed, bceStartYear, bceStartMonth, bceStartDay)))
    {
      failFlight = true;
      eCode = FAIL_DATE;
    }

    // Now validate the stop date
    if (!failFlight)
    {
      if (!(validateStopDate(dateUsed, bceStopYear, bceStopMonth, bceStopDay)))
      {
        failFlight = true;
        eCode = FAIL_DATE;
      }
    }
    // validate D O W
    if (!failFlight)
    {
      fltDOW = dateUsed.dayOfWeek();
      if (fltDOW == 0)
        fltDOW = 7; // in DateTime 0...6, in database 1...7
      if (UNLIKELY(!bceSegment.daysOfWeek().empty() &&
          bceSegment.daysOfWeek().find_first_of(static_cast<char>(fltDOW + '0')) ==
              std::string::npos))
      {
        failFlight = true;
        eCode = FAIL_DOW;
      }
    }

    // validate start stop Time
    if (!failFlight)
    {
      fltTime = dateUsed.totalMinutes();
      if (UNLIKELY((bceStartTime != 0 && bceStartTime != 65535 && fltTime < bceStartTime) ||
          (bceStopTime != 0 && bceStopTime != 65535 && fltTime > bceStopTime)))
      {
        failFlight = true;
        eCode = FAIL_TIME;
      }
    }
    if (failFlight)
    {
      if (UNLIKELY(diag405() && !_shortDiag))
      {
        *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
                 << " SEGNUM: " << bceSegment.segNo() << errorCodes[eCode]
                 << "   BCE TVLEFFDATE: " << bceStartMonth << "/" << bceStartDay << "/"
                 << bceStartYear << "  BCE TVLDISCDATE: " << bceStopMonth << "/" << bceStopDay
                 << "/" << bceStopYear << " \n"
                 << "   BCE STARTTIME: " << bceStartTime << "  BCE STOPTIME: " << bceStopTime
                 << "  FLT TIME: " << fltTime << " \n"
                 << "   BCE DOW: " << bceSegment.daysOfWeek() << "  FLT DOW: " << fltDOW
                 << "  TSI: " << bceSegment.tsi();
        if (arrivalTSI)
          *(_diag) << " - ARRIVAL TSI";
        *(_diag) << "\n   NEXT FLT \n";
      }
      return BCE_NEXT_FLT;
    }
  }
  return BCE_PASS;
}

//----------------------------------------------------------------------------
BCEReturnTypes
BookingCodeExceptionValidator::validateFareclassType(
    const BookingCodeExceptionSequence& bceSequence,
    const BookingCodeExceptionSegment& bceSegment,
    const PaxTypeFare& paxTypeFare,
    const AirSeg& airSeg)
{
  const FareClassCode& bceFareClass = bceSegment.fareclass();
  const FareClassCode& paxFareClass = paxTypeFare.fareClass();
  if (bceFareClass.empty())
    return BCE_PASS;

  switch (bceSegment.fareclassType())
  {
  case 'T':
  {
    if (RuleUtil::matchFareType(bceFareClass.c_str(), paxTypeFare.fcaFareType()))
      return BCE_PASS;
    break;
  } // end case 'T'

  case 'F':
  {
    if (bceFareClass == paxFareClass)
      return BCE_PASS;
    break;
  } // end case 'F'

  case 'M':
  {
    if (RuleUtil::matchFareClass(bceFareClass.c_str(), paxFareClass.c_str()))
      return BCE_PASS;
    break;
  } // end case 'M'

  case 'A':
  {
    if (bceFareClass[0] == paxFareClass[0])
      return BCE_PASS;
    break;
  } // end case 'A'

  default:
    break;
  }
  if (UNLIKELY(diag405() && !_shortDiag))
  {
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL FARE CLASS**\n"
             << "   FARE CLASS CODE: " << paxTypeFare.fareClass()
             << "    FARE TYPE: " << paxTypeFare.fcaFareType()
             << "\n   BCEFARECLASSTYPE: " << bceSegment.fareclassType()
             << "    BCEFARECLASS: " << bceSegment.fareclass() << " NEXT SEQUENCE \n";
  }
  return BCE_NEXT_SEQUENCE;
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::resetFltResult(const uint32_t seqNo, const uint32_t airSegSize)
{
  if (_fltResultVector.size() != airSegSize)
    _fltResultVector.resize(airSegSize);

  for (uint16_t i = 0; i < airSegSize; i++)
  {
    if (_fltResultVector[i].first == (int32_t)seqNo || seqNo == BCE_ALL_SEQUENCES)
    {
      _fltResultVector[i].first = BCE_FLT_NOMATCH;
      _fltResultVector[i].second = BCE_FLT_NOMATCH;
    }
  }
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::processRestrictionTag(PricingTrx& trx,
                                                     const uint32_t& itemNo,
                                                     const BookingCodeExceptionSegment& bceSegment,
                                                     const TravelSeg* tvlSeg,
                                                     PaxTypeFare::SegmentStatusVec& segStatVec,
                                                     uint16_t airIndex,
                                                     PaxTypeFare& paxTypeFare,
                                                     int16_t iFlt)
{
  if (trx.getRequest()->isLowFareNoAvailability()) // WPNCS entry
  {
    validateWPNCS(trx, itemNo, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, iFlt);
    return;
  }

  switch (bceSegment.restrictionTag())
  {
  case BCE_PERMITTED:
  {
    permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_REQUIRED:
  {
    requiredTagR(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_PERMITTED_IF_PRIME_NOT_OFFER:
  {
       // skip this for inifini. APO-43304
       if (AltPricingUtil::ignoreAvail(trx) &&
           !trx.getRequest()->ticketingAgent()->infiniUser())
       {
          permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
          return;
       }

    permittedIfPrimeNotOfferTagO(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_PERMITTED_IF_PRIME_NOT_AVAIL:
  {
       if (AltPricingUtil::ignoreAvail(trx) &&
          !trx.getRequest()->ticketingAgent()->infiniUser())
       {
          permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
          return;
       }


    permittedIfPrimeNotAvailTagA(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_REQUIRED_IF_PRIME_NOT_OFFER:
  {
       if (AltPricingUtil::ignoreAvail(trx) &&
          !trx.getRequest()->ticketingAgent()->infiniUser())
       {
          permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
          return;
       }


    requiredIfPrimeNotOfferTagG(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_REQUIRED_IF_PRIME_NOT_AVAIL:
  {
       if (AltPricingUtil::ignoreAvail(trx) &&
          !trx.getRequest()->ticketingAgent()->infiniUser())
       {
          permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
          return;
       }


    requiredIfPrimeNotAvailTagH(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_REQUIRED_WHEN_OFFERED:
  {
      if (AltPricingUtil::ignoreAvail(trx) &&
          trx.getRequest()->ticketingAgent()->axessUser())
      {
         permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
         return;
      }


    requiredWhenOfferTagW(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_REQUIRED_WHEN_AVAIL:
  {
      if (AltPricingUtil::ignoreAvail(trx) &&
          trx.getRequest()->ticketingAgent()->axessUser())
      {
         permittedTagP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
         return;
      }


    requiredWhenAvailTagV(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

// BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE
  case BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE:
  {
    if(iFlt != -1) // it's not R6/C1 and not for R1T999 Cat25 R3
    {
      otherDiag(SKIP_SEQ_REST_B_D_R6C1);
      return;
    }
    if(_rec1T999Cat25R3) // it's not R6/C1 and not for R1T999 Cat25 R3
    {
      otherDiag(SKIP_SEQ_REST_B_D_CAT25);
      return;
    }


    rbd2PermittedWhenRbd1AvailTagB(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }
// BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE

  case BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE:
  {
    if(iFlt != -1) // it's not R6/C1 and not for R1T999 Cat25 R3
    {
      otherDiag(SKIP_SEQ_REST_B_D_R6C1);
      return;
    }
    if(_rec1T999Cat25R3) // it's not R6/C1 and not for R1T999 Cat25 R3
    {
      otherDiag(SKIP_SEQ_REST_B_D_CAT25);
      return;
    }


    rbd2RequiredWhenRbd1AvailTagD(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_ADDITIONAL_DATA_APPLIES:
  {
    additionalDataApplyTagU(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_STANDBY:
  {
    standbyTagS(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_NOT_PERMITTED:
  {
    notPermittedTagX(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }

  case BCE_DOES_NOT_EXIST:
  {
    notExistTagN(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
    return;
  }
  } // end of switch
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::permittedTagP(PricingTrx& trx,
                                             const BookingCodeExceptionSegment& bceSegment,
                                             const TravelSeg* tvlSeg,
                                             PaxTypeFare::SegmentStatusVec& segStatVec,
                                             uint16_t airIndex,
                                             PaxTypeFare& paxTypeFare,
                                             bool rbd2Only)
{
  _restTagProcessed = BCE_PERMITTED;

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex,
                  paxTypeFare, FLT_NOMATCH_BCE, rbd2Only);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::requiredTagR(PricingTrx& trx,
                                            const BookingCodeExceptionSegment& bceSegment,
                                            const TravelSeg* tvlSeg,
                                            PaxTypeFare::SegmentStatusVec& segStatVec,
                                            uint16_t airIndex,
                                            PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_REQUIRED;

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::permittedIfPrimeNotOfferTagO(
    PricingTrx& trx,
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    uint16_t airIndex,
    PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_PERMITTED_IF_PRIME_NOT_OFFER;
  if (isPrimeOffered(trx, tvlSeg, segStatVec, airIndex, paxTypeFare))
  {
    tagFltNOMATCH(segStatVec, airIndex);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return; // continue to the next flight.
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, FLT_NOMATCH_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::permittedIfPrimeNotAvailTagA(
    PricingTrx& trx,
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    uint16_t airIndex,
    PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_PERMITTED_IF_PRIME_NOT_AVAIL;
  if (isPrimeAvailable(trx, tvlSeg, segStatVec, airIndex, paxTypeFare))
  {
    tagFltNOMATCH(segStatVec, airIndex);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return; // continue to the next flight.
  }

  BcValidation(trx,
                  bceSegment,
                  tvlSeg,
                  segStatVec,
                  airIndex,
                  paxTypeFare,
                  FLT_NOMATCH_BCE,
                  false,
                  BOOKING_CODE_NOT_AVAILABLE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::requiredIfPrimeNotOfferTagG(
    PricingTrx& trx,
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    uint16_t airIndex,
    PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_REQUIRED_IF_PRIME_NOT_OFFER;
  if (isPrimeOffered(trx, tvlSeg, segStatVec, airIndex, paxTypeFare))
  {
    tagFltNOMATCH(segStatVec, airIndex);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return; // continue to the next flight.
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::requiredIfPrimeNotAvailTagH(
    PricingTrx& trx,
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    uint16_t airIndex,
    PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_REQUIRED_IF_PRIME_NOT_AVAIL;
  if (isPrimeAvailable(trx, tvlSeg, segStatVec, airIndex, paxTypeFare))
  {
    tagFltNOMATCH(segStatVec, airIndex);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return; // continue to the next flight.
  }

  BcValidation(trx,
                  bceSegment,
                  tvlSeg,
                  segStatVec,
                  airIndex,
                  paxTypeFare,
                  FLT_FAIL_BCE,
                  false,
                  BOOKING_CODE_NOT_AVAILABLE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::requiredWhenOfferTagW(PricingTrx& trx,
                                                     const BookingCodeExceptionSegment& bceSegment,
                                                     const TravelSeg* tvlSeg,
                                                     PaxTypeFare::SegmentStatusVec& segStatVec,
                                                     uint16_t airIndex,
                                                     PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_REQUIRED_WHEN_OFFERED;
  if (!isBceOffered(bceSegment, trx, tvlSeg, segStatVec, airIndex, paxTypeFare))
  {
    tagFltNOMATCH_NEW(segStatVec, airIndex, BOOKING_CODE_NOT_OFFERED);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return;
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::requiredWhenAvailTagV(PricingTrx& trx,
                                                     const BookingCodeExceptionSegment& bceSegment,
                                                     const TravelSeg* tvlSeg,
                                                     PaxTypeFare::SegmentStatusVec& segStatVec,
                                                     uint16_t airIndex,
                                                     PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_REQUIRED_WHEN_AVAIL;
  if (!isBceAvailable(bceSegment, trx, tvlSeg, segStatVec, airIndex, paxTypeFare))
  {
    tagFltNOMATCH_NEW(segStatVec, airIndex, BOOKING_CODE_NOT_AVAILABLE);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return;
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::rbd2PermittedWhenRbd1AvailTagB(PricingTrx& trx,
                                                     const BookingCodeExceptionSegment& bceSegment,
                                                     const TravelSeg* tvlSeg,
                                                     PaxTypeFare::SegmentStatusVec& segStatVec,
                                                     uint16_t airIndex,
                                                     PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  if (!isBceAvailable(bceSegment, trx, tvlSeg, segStatVec, airIndex, paxTypeFare, true))
  {
    tagFltNOMATCH_NEW(segStatVec, airIndex, BOOKING_CODE_NOT_AVAILABLE);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return;
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec,
                  airIndex, paxTypeFare, FLT_NOMATCH_BCE, true);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::rbd2RequiredWhenRbd1AvailTagD(PricingTrx& trx,
                                                     const BookingCodeExceptionSegment& bceSegment,
                                                     const TravelSeg* tvlSeg,
                                                     PaxTypeFare::SegmentStatusVec& segStatVec,
                                                     uint16_t airIndex,
                                                     PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;
  if (!isBceAvailable(bceSegment, trx, tvlSeg, segStatVec, airIndex, paxTypeFare, true))
  {
    tagFltNOMATCH_NEW(segStatVec, airIndex, BOOKING_CODE_NOT_AVAILABLE);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return;
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec,
                  airIndex, paxTypeFare, FLT_FAIL_BCE, true);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::additionalDataApplyTagU(
    PricingTrx& trx,
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    uint16_t airIndex,
    PaxTypeFare& paxTypeFare)
{
  // TODO  Talk to Jim Firth -- what do we do with 'U'  do we not program for it ??
  _restTagProcessed = BCE_ADDITIONAL_DATA_APPLIES;
  tagFltNOMATCH(segStatVec, airIndex);
  diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::standbyTagS(PricingTrx& trx,
                                           const BookingCodeExceptionSegment& bceSegment,
                                           const TravelSeg* tvlSeg,
                                           PaxTypeFare::SegmentStatusVec& segStatVec,
                                           uint16_t airIndex,
                                           PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_STANDBY;
  if (tvlSeg->resStatus() == CONFIRM_RES_STATUS)
  {
    tagFltNOMATCH(segStatVec, airIndex);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return; // continue to the next flight
  }

  BcValidation(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::notPermittedTagX(PricingTrx& trx,
                                                const BookingCodeExceptionSegment& bceSegment,
                                                const TravelSeg* tvlSeg,
                                                PaxTypeFare::SegmentStatusVec& segStatVec,
                                                uint16_t airIndex,
                                                PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_NOT_PERMITTED;

  const BookingCodeValidationStatus vStat =
      validateBC_NEW(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare);
  if (vStat != BOOKING_CODE_PASSED)
  {
    tagFltNOMATCH_NEW(segStatVec, airIndex, BOOKING_CODE_PASSED);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return;
  }

  tagFltFAIL(segStatVec, airIndex, vStat);

  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, false);
  if (trx.getRequest()->isLowFareRequested())
  {
    // WPNC entry
    segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
    segStatVec[airIndex]._bkgCodeReBook.clear();
    segStatVec[airIndex]._reBookCabin.setUndefinedClass();
  }
  diagBc(tvlSeg, segStatVec[airIndex], FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::notExistTagN(PricingTrx& trx,
                                            const BookingCodeExceptionSegment& bceSegment,
                                            const TravelSeg* tvlSeg,
                                            PaxTypeFare::SegmentStatusVec& segStatVec,
                                            uint16_t airIndex,
                                            PaxTypeFare& paxTypeFare)
{
  _restTagProcessed = BCE_DOES_NOT_EXIST;
  tagFltFAILTagN(paxTypeFare, segStatVec, airIndex, trx);
  diagBc(tvlSeg, segStatVec[airIndex], FLT_FAIL_BCE);
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateBC(PricingTrx& trx,
                                          const BookingCodeExceptionSegment& bceSegment,
                                          const TravelSeg* tvlSeg,
                                          PaxTypeFare::SegmentStatusVec& segStatVec,
                                          uint16_t airIndex,
                                          PaxTypeFare& paxTypeFare,
                                          bool rbd2Only)
{
  if (validateWP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, rbd2Only))
    return true;

  if (isAsBookedStatus())
    return false;

  // WPNC entry
  if (trx.getRequest()->isLowFareRequested())
  {
    if (validateWPNC(bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, trx, rbd2Only))
      return true;
  }
  return false;
}

BookingCodeValidationStatus
BookingCodeExceptionValidator::validateBC_NEW(PricingTrx& trx,
                                              const BookingCodeExceptionSegment& bceSegment,
                                              const TravelSeg* tvlSeg,
                                              PaxTypeFare::SegmentStatusVec& segStatVec,
                                              uint16_t airIndex,
                                              PaxTypeFare& paxTypeFare,
                                              bool rbd2Only)
{
  if (validateWP(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, rbd2Only))
    return BOOKING_CODE_PASSED;

  if (isAsBookedStatus())
    return BOOKING_CODE_NOT_OFFERED;

  // WPNC entry
  if (trx.getRequest()->isLowFareRequested())
  {
    return validateWPNC_NEW(bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, trx, rbd2Only);
  }
  return BOOKING_CODE_NOT_OFFERED;
}

//----------------------------------------------------------------------------

bool
BookingCodeExceptionValidator::validateWP(PricingTrx& trx,
                                          const BookingCodeExceptionSegment& bceSegment,
                                          const TravelSeg* tvlSeg,
                                          PaxTypeFare::SegmentStatusVec& segStatVec,
                                          const uint16_t airIndex,
                                          PaxTypeFare& paxTypeFare,
                                          bool rbd2Only)
{
  if (UNLIKELY(airIndex > segStatVec.size()))
    return false;

  if (checkBookedClassAvail(trx, tvlSeg))
    return false;

  const BookingCode& bceBkgCode1 = bceSegment.bookingCode1();
  const BookingCode& bceBkgCode2 = bceSegment.bookingCode2();

  std::vector<BookingCode> tempBkCodes;
  std::vector<BookingCode> tsBkCodes{tvlSeg->getBookingCode()};
  bool isDiff = BookingCodeUtil::validateExcludedBookingCodes(trx,
                                                              tsBkCodes,
                                                              segStatVec[airIndex],
                                                              tempBkCodes,
                                                              _diag);

  bool checkBookingCode2 = (tvlSeg->getBookingCode() == bceBkgCode2);
  if (!fallback::fallbackAAExcludedBookingCode(&trx))
  {
    checkBookingCode2 = (checkBookingCode2 && isDiff);
  }

  if(rbd2Only)
  {
    if (checkBookingCode2 || skipCat31Flown(trx, tvlSeg))
    {
      segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS);
      segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, false);
      segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);
      paxTypeFare.s8BFBookingCode() = bceBkgCode2;
      return true;
    }
    return false;
  }
  else
  if ((isDiff && tvlSeg->getBookingCode() == bceBkgCode1) || checkBookingCode2 ||
      skipCat31Flown(trx, tvlSeg))
  {
    segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS);
    segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, false);
    segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);
    if (tvlSeg->getBookingCode() == bceBkgCode1)
      paxTypeFare.s8BFBookingCode() = bceBkgCode1;
    else if (tvlSeg->getBookingCode() == bceBkgCode2)
      paxTypeFare.s8BFBookingCode() = bceBkgCode2;
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateWPNC(const BookingCodeExceptionSegment& bceSegment,
                                            const TravelSeg* tvlSeg,
                                            PaxTypeFare::SegmentStatusVec& segStatVec,
                                            const uint16_t airIndex,
                                            PaxTypeFare& paxTypeFare,
                                            PricingTrx& trx,
                                            bool rbd2Only)
{
  if (airIndex > segStatVec.size())
    return false;

  validateCabinForDifferential(trx, tvlSeg, paxTypeFare);

  const std::vector<ClassOfService*>* cosVec =
      getAvailability(trx, tvlSeg, segStatVec[airIndex], paxTypeFare, airIndex);
  if (cosVec == nullptr)
  {
    if (airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size())
      return false;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }

  if (cosVec == nullptr)
    return false;

  bool wpaNoMatchProcess = false;
  bool cabinCheckOK = false;

  if ((trx.altTrxType() == PricingTrx::WPA || trx.altTrxType() == PricingTrx::WP_NOMATCH ||
       trx.altTrxType() == PricingTrx::WPA_NOMATCH) &&
      !trx.noPNRPricing())
  {
    // WPA nomatch path or WP nomatch path or WPA'XM
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);
    if (fcConfig != nullptr)
    {
      if (fcConfig->wpaNoMatchHigherCabinFare() == '1')
        wpaNoMatchProcess = true;
    }
  }

  uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(trx, paxTypeFare);

  const BookingCode& bceBkgCode1 = bceSegment.bookingCode1();
  const BookingCode& bceBkgCode2 = bceSegment.bookingCode2();
  COSInnerPtrVecIC classOfS = cosVec->begin();
  const COSInnerPtrVecIC classOfSEnd = cosVec->end();
  for (; classOfS != classOfSEnd; classOfS++)
  {
    ClassOfService& cs = *(*classOfS);

    if (wpaNoMatchProcess)
    {
      if (cs.cabin() == tvlSeg->bookedCabin())
        cabinCheckOK = true;
      else
        cabinCheckOK = false;
    }
    else
    {
      if (cs.cabin() <= tvlSeg->bookedCabin())
        cabinCheckOK = true;
      else
        cabinCheckOK = false;
    }
    if(rbd2Only)
    {
      if (cs.bookingCode() == bceBkgCode2 &&
          cs.numSeats() >= numSeatsRequired && cabinCheckOK)
      {
        segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_DUAL_RBD_PASS, true);
        segStatVec[airIndex]._dualRbd1 = bceBkgCode1;
        setRebookRBD(trx, paxTypeFare, tvlSeg, segStatVec, airIndex, cs);
        return true;
      }
    }
    else
    if ((cs.bookingCode() == bceBkgCode1 || cs.bookingCode() == bceBkgCode2) &&
        cs.numSeats() >= numSeatsRequired && cabinCheckOK)
    {
      setRebookRBD(trx, paxTypeFare, tvlSeg, segStatVec, airIndex, cs);
      return true;
    }
  }
  return false;
}

void
BookingCodeExceptionValidator::setRebookRBD(PricingTrx& trx,
                                            PaxTypeFare& paxTypeFare,
                                            const TravelSeg* tvlSeg,
                                            PaxTypeFare::SegmentStatusVec& segStatVec,
                                            uint16_t airIndex,
                                            const ClassOfService& cs)
{
   segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
   segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);
   segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, false);
   if (LIKELY(tvlSeg->getBookingCode() != cs.bookingCode() ||
       tvlSeg->realResStatus() == QF_RES_STATUS || (PricingTrx::IS_TRX == trx.getTrxType()) ||
       (PricingTrx::FF_TRX == trx.getTrxType()) ||
       (PricingTrx::MIP_TRX == trx.getTrxType() && trx.billing()->actionCode() != "WPNI.C" &&
       trx.billing()->actionCode() != "WFR.C")))
   {
     rebookSegmentWPNC(trx, paxTypeFare, airIndex, cs, segStatVec);
   }

   paxTypeFare.s8BFBookingCode() = cs.bookingCode();
}

void
BookingCodeExceptionValidator::rebookSegmentWPNC(PricingTrx& trx,
                                                 PaxTypeFare& paxTypeFare,
                                                 uint16_t airIndex,
                                                 const ClassOfService& cs,
                                                 PaxTypeFare::SegmentStatusVec& segStatVec)
{
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
  segStatVec[airIndex]._bkgCodeReBook = cs.bookingCode();
  segStatVec[airIndex]._reBookCabin = cs.cabin();
}

BookingCodeValidationStatus
BookingCodeExceptionValidator::validateWPNC_NEW(const BookingCodeExceptionSegment& bceSegment,
                                                const TravelSeg* tvlSeg,
                                                PaxTypeFare::SegmentStatusVec& segStatVec,
                                                const uint16_t airIndex,
                                                PaxTypeFare& paxTypeFare,
                                                PricingTrx& trx,
                                                bool rbd2Only)
{
  if (UNLIKELY(airIndex > segStatVec.size()))
    return BOOKING_CODE_NOT_OFFERED;

  validateCabinForDifferential(trx, tvlSeg, paxTypeFare);

  const std::vector<ClassOfService*>* cosVec =
      getAvailability(trx, tvlSeg, segStatVec[airIndex], paxTypeFare, airIndex);
  if (cosVec == nullptr)
  {
    if (UNLIKELY(airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size()))
      return BOOKING_CODE_NOT_OFFERED;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }

  if (UNLIKELY(cosVec == nullptr))
  {
    return BOOKING_CODE_NOT_OFFERED;
  }

  bool wpaNoMatchProcess = false;
  bool cabinCheckOK = false;

  if (UNLIKELY((trx.altTrxType() == PricingTrx::WPA || trx.altTrxType() == PricingTrx::WP_NOMATCH ||
       trx.altTrxType() == PricingTrx::WPA_NOMATCH) &&
      !trx.noPNRPricing()))
  {
    // WPA nomatch path or WP nomatch path or WPA'XM
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);
    if (fcConfig != nullptr)
    {
      if (fcConfig->wpaNoMatchHigherCabinFare() == '1')
        wpaNoMatchProcess = true;
    }
  }

  uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(trx, paxTypeFare);
  const AirSeg& pAirSeg = dynamic_cast<const AirSeg&>(*tvlSeg);
  const BookingCode& bceBkgCode1 = bceSegment.bookingCode1();
  const BookingCode& bceBkgCode2 = bceSegment.bookingCode2();
  CabinType bceCabin1 = cabinWpncs(trx, pAirSeg,
                                   bceSegment.bookingCode1(),
                                   paxTypeFare,
                                   0,
                                   bceSegment);
  CabinType bceCabin2 = cabinWpncs(trx, pAirSeg,
                                   bceSegment.bookingCode2(),
                                   paxTypeFare,
                                   0,
                                   bceSegment);

  COSInnerPtrVecIC classOfS = cosVec->begin();
  const COSInnerPtrVecIC classOfSEnd = cosVec->end();

  BcvsNotAvailableWins prioritizer;
  for (; classOfS != classOfSEnd; classOfS++)
  {
    ClassOfService& cs = *(*classOfS);

    if (UNLIKELY(!_rtwPreferredCabin.empty() && !_rtwPreferredCabin[airIndex].isUndefinedClass()))
      cabinCheckOK = cs.cabin() == _rtwPreferredCabin[airIndex];

    else if (UNLIKELY(wpaNoMatchProcess ||
             (!fallback::fallbackPriceByCabinActivation(&trx) &&
              !trx.getRequest()->isjumpUpCabinAllowed())))
      cabinCheckOK = (cs.cabin() == tvlSeg->bookedCabin());
    else
      cabinCheckOK = (cs.cabin() <= tvlSeg->bookedCabin());

    bool noBookingCode = true;

    if (!fallback::fallbackPriceByCabinActivation(&trx) &&
        !trx.getRequest()->isjumpUpCabinAllowed())
    {
      if(rbd2Only)
      {
        if (cs.cabin() != bceCabin2)
        {
          prioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED); // BOOKING_CODE_CABIN_NOT_MATCH
          cabinCheckOK = false;
        }
      }
      else
      {
        if (cs.cabin() != bceCabin1 && cs.cabin() != bceCabin2 )
        {
          prioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED); // BOOKING_CODE_CABIN_NOT_MATCH
          cabinCheckOK = false;
        }
      }
    }

    if(rbd2Only)
      noBookingCode = (cs.bookingCode() != bceBkgCode2);
    else
      noBookingCode = ((cs.bookingCode() != bceBkgCode1) && (cs.bookingCode() != bceBkgCode2));

    if (noBookingCode || !cabinCheckOK)
    {
      prioritizer.updateStatus(BOOKING_CODE_NOT_OFFERED);
      continue;
    }
    std::vector<BookingCode> tempBkVec;
    std::vector<BookingCode> csVec;
    csVec.push_back(cs.bookingCode());

    if (!BookingCodeUtil::validateExcludedBookingCodes(trx, 
                        csVec, segStatVec[airIndex], tempBkVec, _diag))
    {
      prioritizer.updateStatus(BOOKING_CODE_IS_EXCLUDED);
      continue;
    }

    // We want to report lack of availability for
    // this class of service only if there are booking codes.
    if (cs.numSeats() < numSeatsRequired)
    {
      prioritizer.updateStatus(BOOKING_CODE_NOT_AVAILABLE);
      continue;
    }

    setRebookRBD(trx, paxTypeFare, tvlSeg, segStatVec, airIndex, cs);
    return BOOKING_CODE_PASSED;
  }
  return prioritizer.getStatus();
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::tagFltNOMATCH(PaxTypeFare::SegmentStatusVec& segStatVec,
                                             const uint16_t airIndex)
{
  if (airIndex >= segStatVec.size())
    return;

  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);
  _fltResultVector[airIndex].first = BCE_FLT_NOMATCH;
  _fltResultVector[airIndex].second = BCE_FLT_NOMATCH;
}

void
BookingCodeExceptionValidator::tagFltNOMATCH_NEW(PaxTypeFare::SegmentStatusVec& segStatVec,
                                                 const uint16_t airIndex,
                                                 BookingCodeValidationStatus validationStatus)
{
  if (UNLIKELY(airIndex >= segStatVec.size()))
    return;

  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);
  _fltResultVector[airIndex].first = BCE_FLT_NOMATCH;
  _fltResultVector[airIndex].second = BCE_FLT_NOMATCH;

  setFlagForBcvStatus(segStatVec[airIndex]._bkgCodeSegStatus, validationStatus);
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::tagFltFAIL(PaxTypeFare::SegmentStatusVec& segStatVec,
                                              const uint16_t airIndex,
                                              BookingCodeValidationStatus validationStatus)
{
  if (UNLIKELY(airIndex >= segStatVec.size()))
    return;

  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, false);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);

  setFlagForBcvStatus(segStatVec[airIndex]._bkgCodeSegStatus, validationStatus);

  _fltResultVector[airIndex].second = BCE_FLT_FAILED;
}

//----------------------------------------------------------------------------
void
BookingCodeExceptionValidator::tagFltFAILTagN(PaxTypeFare& paxTypeFare,
                                              PaxTypeFare::SegmentStatusVec& segStatVec,
                                              const uint16_t airIndex,
                                              PricingTrx& trx)
{
  paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_TAG_N);

  if (UNLIKELY(airIndex >= segStatVec.size()))
    return;

  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_TAG_N);

  // If BCE data does not exist, we assume that there are no
  // booking codes.
  tagFltFAIL(segStatVec, airIndex, BOOKING_CODE_NOT_OFFERED);
}

//----------------------------------------------------------------------------
BookingCodeExceptionSegment*
BookingCodeExceptionValidator::getSegment(const BookingCodeExceptionSequence& bceSequence,
                                          const int16_t segmentNo) const
{
  auto it = std::find_if(bceSequence.segmentVector().cbegin(),
                         bceSequence.segmentVector().cend(),
                         [segmentNo](const auto* segment)
                         { return segment->segNo() == segmentNo; });
  return it != bceSequence.segmentVector().cend() ? *it : nullptr;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::isPrimeOffered(PricingTrx& trx,
                                              const TravelSeg* tvlSeg,
                                              PaxTypeFare::SegmentStatusVec& segStatVec,
                                              uint16_t airIndex,
                                              PaxTypeFare& paxTypeFare)
{
  if (airIndex > segStatVec.size())
    return false;

  const std::vector<ClassOfService*>* cosVec =
      getAvailability(trx, tvlSeg, segStatVec[airIndex], paxTypeFare, airIndex);
  if (cosVec == nullptr)
  {
    if (airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size())
      return false;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }

  if (cosVec == nullptr)
    return false;

  std::vector<BookingCode> primeBookingCodeVec;
  paxTypeFare.getPrimeBookingCode(primeBookingCodeVec);
  const uint16_t primeBCSize = primeBookingCodeVec.size();

  COSInnerPtrVecIC classOfS = cosVec->begin();
  const COSInnerPtrVecIC classOfSEnd = cosVec->end();

  for (uint16_t primeIndex = 0; primeIndex < primeBCSize; primeIndex++)
  {
    // consider the booked class of service as always offered
    if (!checkBookedClassOffer(trx, tvlSeg))
    {
      if (tvlSeg->getBookingCode() == primeBookingCodeVec[primeIndex])
        return true;
    }
    classOfS = cosVec->begin();
    for (; classOfS != classOfSEnd; classOfS++)
    {
      ClassOfService& cs = *(*classOfS);
      if (primeBookingCodeVec[primeIndex] == cs.bookingCode())
      {
        return true;
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::isPrimeAvailable(PricingTrx& trx,
                                                const TravelSeg* tvlSeg,
                                                PaxTypeFare::SegmentStatusVec& segStatVec,
                                                uint16_t airIndex,
                                                PaxTypeFare& paxTypeFare)
{
  if (airIndex > segStatVec.size())
    return false;

  const std::vector<ClassOfService*>* cosVec =
      getAvailability(trx, tvlSeg, segStatVec[airIndex], paxTypeFare, airIndex);
  if (cosVec == nullptr)
  {
    if (airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size())
      return false;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }

  if (cosVec == nullptr)
    return false;

  std::vector<BookingCode> primeBookingCodeVec;
  paxTypeFare.getPrimeBookingCode(primeBookingCodeVec);
  const uint16_t primeBCSize = primeBookingCodeVec.size();

  COSInnerPtrVecIC classOfS = cosVec->begin();
  const COSInnerPtrVecIC classOfSEnd = cosVec->end();

  uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(trx, paxTypeFare);

  for (uint16_t primeIndex = 0; primeIndex < primeBCSize; primeIndex++)
  {
    // consider the booked class of service as always available
    if (!checkBookedClassAvail(trx, tvlSeg))
    {
      if (tvlSeg->getBookingCode() == primeBookingCodeVec[primeIndex])
        return true;
    }
    classOfS = cosVec->begin();
    for (; classOfS != classOfSEnd; classOfS++)
    {
      ClassOfService& cs = *(*classOfS);
      if (primeBookingCodeVec[primeIndex] == cs.bookingCode() && cs.numSeats() >= numSeatsRequired)
      {
        return true;
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::isBceOffered(const BookingCodeExceptionSegment& bceSegment,
                                            PricingTrx& trx,
                                            const TravelSeg* tvlSeg,
                                            PaxTypeFare::SegmentStatusVec& segStatVec,
                                            uint16_t airIndex,
                                            PaxTypeFare& paxTypeFare)
{
  if (UNLIKELY(airIndex > segStatVec.size()))
    return false;

  const std::vector<ClassOfService*>* cosVec =
      getAvailability(trx, tvlSeg, segStatVec[airIndex], paxTypeFare, airIndex);
  if (cosVec == nullptr)
  {
    if (UNLIKELY(airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size()))
      return false;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }

  if (UNLIKELY(cosVec == nullptr))
    return false;

  const BookingCode& bceBkgCode1 = bceSegment.bookingCode1();
  const BookingCode& bceBkgCode2 = bceSegment.bookingCode2();
  COSInnerPtrVecIC classOfS = cosVec->begin();
  const COSInnerPtrVecIC classOfSEnd = cosVec->end();

  // consider the booked class of service as always offered
  if (!checkBookedClassOffer(trx, tvlSeg))
  {
    if (tvlSeg->getBookingCode() == bceBkgCode1 || tvlSeg->getBookingCode() == bceBkgCode2)
    {
      return true;
    }
  }

  for (; classOfS != classOfSEnd; classOfS++)
  {
    ClassOfService& cs = *(*classOfS);
    if (cs.bookingCode() == bceBkgCode1 || cs.bookingCode() == bceBkgCode2)
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::isBceAvailable(const BookingCodeExceptionSegment& bceSegment,
                                              PricingTrx& trx,
                                              const TravelSeg* tvlSeg,
                                              PaxTypeFare::SegmentStatusVec& segStatVec,
                                              uint16_t airIndex,
                                              PaxTypeFare& paxTypeFare,
                                              bool checkRbd1Only)
{
  if (UNLIKELY(airIndex > segStatVec.size()))
    return false;

  const std::vector<ClassOfService*>* cosVec = nullptr;

  if(checkRbd1Only && localJourneyCarrier(trx, tvlSeg))
  {
    if (UNLIKELY(airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size()))
      return false;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }
  else
    cosVec = getAvailability(trx, tvlSeg, segStatVec[airIndex], paxTypeFare, airIndex);

  if (cosVec == nullptr)
  {
    if (UNLIKELY(airIndex > paxTypeFare.fareMarket()->classOfServiceVec().size()))
      return false;
    cosVec = paxTypeFare.fareMarket()->classOfServiceVec()[airIndex];
  }

  if (UNLIKELY(cosVec == nullptr))
    return false;

  const BookingCode& bceBkgCode1 = bceSegment.bookingCode1();
  const BookingCode& bceBkgCode2 = bceSegment.bookingCode2();
  COSInnerPtrVecIC classOfS = cosVec->begin();
  const COSInnerPtrVecIC classOfSEnd = cosVec->end();

  uint16_t numSeatsRequired = 0;
  if ( trx.getRequest()->ticketingAgent()->infiniUser() )
      numSeatsRequired =  PaxTypeUtil::numSeatsForFareWithoutIgnoreAvail( trx, paxTypeFare);
  else
     numSeatsRequired = PaxTypeUtil::numSeatsForFare(trx, paxTypeFare);

  // consider the booked class of service as always available
  if (!checkBookedClassAvail(trx, tvlSeg, checkRbd1Only))
  {
    if(checkRbd1Only)
    {
      if (tvlSeg->getBookingCode() == bceBkgCode1)
      {
        return true;
      }
    }
    else
    if (tvlSeg->getBookingCode() == bceBkgCode1 || tvlSeg->getBookingCode() == bceBkgCode2)
    {
      return true;
    }
  }

  for (; classOfS != classOfSEnd; classOfS++)
  {
    ClassOfService& cs = *(*classOfS);
    if(checkRbd1Only)
    {
       if ((cs.bookingCode() == bceBkgCode1) && cs.numSeats() >= numSeatsRequired)
       {
         return true;
       }
    }
    else
    if ((cs.bookingCode() == bceBkgCode1 || cs.bookingCode() == bceBkgCode2) &&
        cs.numSeats() >= numSeatsRequired)
    {
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::allFltsDone(const int16_t iFlt)
{
  if (iFlt > -1 && iFlt < static_cast<int>(_fltResultVector.size())) // just check one flight
  {
    if (_fltResultVector[iFlt].first == BCE_FLT_NOMATCH)
    {
      return false;
    }
    return true;
  }
  const uint16_t airSegSize = _fltResultVector.size();
  uint16_t i = 0;
  bool result = true;
  for (i = 0; i < airSegSize; i++)
  {
    if (_fltResultVector[i].first == BCE_FLT_NOMATCH)
    {
      result = false;
    }
  }
  return result;
}

//---------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::createDiag(PricingTrx& trx, const PaxTypeFare& paxTfare)
{
  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (UNLIKELY(trx.diagnostic().isActive() &&
               (diagType == Diagnostic405 || diagType == Diagnostic404)))
  {
    DiagParamMapVecI endI = trx.diagnostic().diagParamMap().end();
    DiagParamMapVecI beginI = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
    size_t len = 0;
    std::string specifiedFM("");
    std::string compFM("");
    // procees /FMDFWLON
    if (beginI != endI)
    {
      specifiedFM = (*beginI).second;
      if (!(specifiedFM.empty()))
      {
        compFM = paxTfare.fareMarket()->boardMultiCity() + paxTfare.fareMarket()->offMultiCity();
        if (compFM != specifiedFM)
          return;
      }
    }

    // process /FCY26
    beginI = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
    if (beginI != endI)
    {
      len = ((*beginI).second).size();
      if (len != 0)
      {
        if ((*beginI).second != paxTfare.fareClass())
          return;
      }
    }

    // process /FBY26
    beginI = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);
    if (beginI != endI)
    {
      len = ((*beginI).second).size();
      if (len != 0)
      {
        if ((*beginI).second != paxTfare.createFareBasis(trx, false))
          return;
      }
    }

    // process /SOSHORT
    _shortDiag = false;
    beginI = trx.diagnostic().diagParamMap().find(Diagnostic::WPNC_SOLO_TEST);
    if (beginI != endI)
    {
      len = ((*beginI).second).size();
      if (len != 0)
      {
        if ((*beginI).second == "SHORT")
          _shortDiag = true;
      }
    }

    // process /DDNOTUNE
    beginI = trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
    if (beginI != endI)
    {
      len = ((*beginI).second).size();
      if (len != 0)
      {
        if ((*beginI).second == "NOTUNE")
          _stopTuning = true;
      }
    }

    _diag = DCFactory::instance()->create(trx);
    if (_diag != nullptr)
    {
      _diag->enable(Diagnostic404, Diagnostic405);
    }
  }
}
//---------------------------------------------------------------------------------
Diag405Collector*
BookingCodeExceptionValidator::diag405()
{
  if (LIKELY(!_diag))
    return nullptr;
  return dynamic_cast<Diag405Collector*>(_diag);
}

//---------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::endDiag()
{
  if (UNLIKELY(diag405()))
  {
    _diag->flushMsg();
    _diag->disable(Diagnostic404, Diagnostic405);
    _diag = nullptr;
  }
}

//---------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::diagHeader(PricingTrx& trx,
                                          const AirSeg* pAirSegConv1,
                                          PaxTypeFare& paxTypeFare)
{
  if (LIKELY(diag405() == nullptr))
  {
    return;
  }
  *(_diag) << " \n ** START DIAG 405 BOOKINGCODE EXCEPTION TABLE 999 ** " << std::endl;

  if (pAirSegConv1 == nullptr)
  {
    *(_diag) << "   TABLE 999 PROCESSING FROM REC 1 OR REC 6 CONV2 "
             << "\n \n";
  }
  else
  {
    *(_diag) << "   TABLE 999 PROCESSING FROM REC 6 CONV 1 "
             << "\n \n";
  }
  *(_diag) << "   " << paxTypeFare.fareMarket()->boardMultiCity() << "-"
           << paxTypeFare.fareMarket()->offMultiCity() << "  ";
  *(_diag) << "FC: " << paxTypeFare.fareClass()
           << " FB: " << paxTypeFare.createFareBasis(trx, false) << " PRIME BC: ";

  std::vector<BookingCode> primeBookingCodeVec;
  paxTypeFare.getPrimeBookingCode(primeBookingCodeVec);
  const uint16_t primeBCSize = primeBookingCodeVec.size();
  for (uint16_t primeIndex = 0; primeIndex < primeBCSize; primeIndex++)
  {
    *(_diag) << primeBookingCodeVec[primeIndex] << " ";
  }
  *(_diag) << std::endl;
  *(_diag) << "   FLIGHTS COVERED BY THE CURRENT FARE : " << std::endl;
  int16_t fltIndex = 1;

  TravelSegVectorCI trvlIterBeg = paxTypeFare.fareMarket()->travelSeg().begin();
  TravelSegVectorCI trvlIterEnd = paxTypeFare.fareMarket()->travelSeg().end();
  TravelSegVectorCI trvlIter = trvlIterBeg;
  for (; trvlIter != trvlIterEnd; trvlIter++, fltIndex++)
  {
    *(_diag) << "   " << fltIndex << ". ";
    AirSeg* pAirSeg = dynamic_cast<AirSeg*>(*trvlIter);
    if (pAirSeg == nullptr)
    {
      *(_diag) << "ARUNK " << std::endl;
      continue;
    }
    *(_diag) << pAirSeg->carrier() << " " << std::setw(4) << pAirSeg->flightNumber()
             << pAirSeg->getBookingCode() << " " << pAirSeg->origAirport()
             << pAirSeg->destAirport();
    if (trx.getRequest()->isLowFareRequested())
    {
      // if WPNC entry
      if ((*trvlIter)->carrierPref() != nullptr)
      {
        *(_diag) << "  SOLO CXR:";
        if ((*trvlIter)->carrierPref()->availabilityApplyrul2st() == NO) // db availabilityIgrul2st
          *(_diag) << "Y  ";
        else
          *(_diag) << "N  ";

        *(_diag) << "JRNY TYPE:";

        if (pAirSeg->flowJourneyCarrier())
          *(_diag) << "F";
        else if (pAirSeg->localJourneyCarrier())
          *(_diag) << "L";
        else
          *(_diag) << "N";
      }
    }
    *(_diag) << " \n";
  }
  *(_diag) << "\n   FLIGHTS PROCESSED IN THIS TABLE 999 PROCESSING : " << std::endl;
  if (pAirSegConv1 == nullptr)
  {
    *(_diag) << "   ** ALL DISPLAYED ABOVE ** " << std::endl;
  }
  else
  {
    *(_diag) << "   1. " << pAirSegConv1->carrier() << " " << std::setw(4)
             << pAirSegConv1->flightNumber() << pAirSegConv1->getBookingCode() << " "
             << pAirSegConv1->origAirport() << pAirSegConv1->destAirport() << std::endl;
  }
  *(_diag) << " \n";

  if (paxTypeFare.isFareByRule())
  {
    const PaxTypeFareRuleData* paxTypeFareRuleData =
        paxTypeFare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);
    if (paxTypeFareRuleData)
    {
      const FBRPaxTypeFareRuleData* fbrPaxTypeFareRuleData =
          PTFRuleData::toFBRPaxTypeFare(paxTypeFareRuleData);

      if (fbrPaxTypeFareRuleData)
      {
        if (fbrPaxTypeFareRuleData->isBaseFareAvailBkcMatched())
        {
          *(_diag) << "   **BASE FARE PRIME RBD AVAILABILTY REQUIREMENT EXISTS**";
          *(_diag) << " \n";
        }
      }
    }
  }
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::diagResults(PricingTrx& trx,
                                           PaxTypeFare& paxTypeFare,
                                           const AirSeg* pAirSegConv1)
{
  if (LIKELY(diag405() == nullptr))
    return;

  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);

  switch (_statusType)
  {
  case STATUS_RULE1:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS RULE 1       " << std::endl;
    break;
  }
  case STATUS_RULE1_AS_BOOKED:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS RULE 1 - AS BOOKED " << std::endl;
    break;
  }
  case STATUS_RULE2:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS RULE 2       " << std::endl;
    break;
  }
  case STATUS_RULE2_AS_BOOKED:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS RULE 2 - AS BOOKED " << std::endl;
    break;
  }
  case STATUS_JORNY:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS JOURNEY      " << std::endl;
    break;
  }
  case STATUS_JORNY_AS_BOOKED:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS JOURNEY - AS BOOKED " << std::endl;
    break;
  }
  default:
  {
    *(_diag) << "\n \n \n          PAXTYPEFARE:SEGSTATUS --UNKNOWN-- " << std::endl;
  }
  }

  std::string strStatus = "NO MATCH";

  AirSeg* airSeg = nullptr;
  for (uint16_t i = 0; i < segStatusVec.size(); i++)
  {
    strStatus = "NO MATCH";
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      strStatus = "PASS    ";
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
      strStatus = "FAIL    ";
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
      strStatus = "NOMATCH ";

    if (i >= paxTypeFare.fareMarket()->travelSeg().size())
    {
      *(_diag) << "FLT INDX: " << i;
    }
    else
    {
      airSeg = dynamic_cast<AirSeg*>(paxTypeFare.fareMarket()->travelSeg()[i]);
      if (airSeg == nullptr)
        *(_diag) << "ARUNK    ";
      else
        *(_diag) << airSeg->carrier() << " " << std::setw(4) << airSeg->flightNumber() << "  ";
    }
    *(_diag) << " STATUS: " << strStatus;
    *(_diag) << " BKGCODE: ";
    if (segStatusVec[i]._bkgCodeReBook.empty())
      *(_diag) << " ";
    else
      *(_diag) << segStatusVec[i]._bkgCodeReBook;

    *(_diag) << " SEQ/SEG: " << _fltResultVector[i].first << "/" << _fltResultVector[i].second
             << std::endl;
  }

  *(_diag) << "************************************************** " << std::endl;
  *(_diag) << " \n";
  std::string s1;
  if (trx.getRequest()->isLowFareRequested())
  {
    // if WPNC entry
    if (_statusType == STATUS_RULE1)
    {
      if (trx.getRequest()->ticketingAgent() == nullptr ||
          trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
      {
        *(_diag) << " CUSTOMER/AGENT TJR POINTER: NULL " << std::endl;
      }
      else
      {
        if (trx.getRequest()->ticketingAgent()->agentTJR()->availabilityIgRul2St() == YES)
          s1 = "NO ";
        else
          s1 = "YES";

        *(_diag) << " SOLO CUSTOMER: " << s1;
      }

      if (trx.getOptions()->soloActiveForPricing())
        s1 = "YES";
      else
        s1 = "NO ";

      *(_diag) << " SOLO ACTIVE: " << s1 << " \n";

      if (trx.getOptions()->journeyActivatedForPricing())
        s1 = "YES";
      else
        s1 = "NO ";

      *(_diag) << " JOURNEY ACTIVE: " << s1;

      if (trx.getOptions()->applyJourneyLogic())
        s1 = "YES";
      else
        s1 = "NO";
      *(_diag) << "  APPLY JOURNEY: " << s1;
      *(_diag) << " \n";

      if (paxTypeFare.fareMarket()->flowMarket())
        s1 = "YES";
      else
        s1 = "NO";

      *(_diag) << " FLOW FARE MARKET: " << s1 << std::endl;

      if (doAsBooked(trx, paxTypeFare, pAirSegConv1))
        *(_diag) << "**** WILL TRY AS BOOKED PROCESS **** " << std::endl;

      if (tryRule2(trx, paxTypeFare))
        *(_diag) << "**** WILL TRY RULE 2 PROCESS **** " << std::endl;

      if (tryLocalWithFlowAvail(trx, paxTypeFare))
        *(_diag) << "**** WILL TRY LOCAL JOURNEY CARRIERS PROCESS  **** " << std::endl;

      *(_diag) << "************************************************** " << std::endl;
    }
    else if (_statusType == STATUS_RULE2 || _statusType == STATUS_JORNY)
    {
      if (doAsBooked(trx, paxTypeFare, pAirSegConv1))
        *(_diag) << "**** WILL TRY AS BOOKED PROCESS **** " << std::endl;

      *(_diag) << "************************************************** " << std::endl;
    }
    else
    {
      *(_diag) << "************************************************** " << std::endl;
    }
    *(_diag) << " \n";
  }
}

//-----------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::diagIndex(PricingTrx& trx,
                                         const BookingCodeExceptionIndex& index,
                                         const FareType& ft,
                                         const FareClassCode& fcc)
{
  Diag405Collector* diag = diag405();
  if (UNLIKELY(diag && trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INDEX"))
  {
    *diag << "FareType: " << ft << " FareClassCode: " << fcc << std::endl;
    *diag << index;
  }
}

//-----------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(const BookingCodeExceptionSequence& bceSequence,
                                      const PaxTypeFare& fare,
                                      BCEErroMsgs errMsg)
{
  if (diag405() == nullptr)
    return;
  if (_shortDiag)
    return;

  switch (errMsg)
  {
  case SEQTYPE_CONSTR_FARE:
  {
    *(_diag) << " SEQUENCE IS NOT PROCESSED - FOR CONSTRUCTED FARES ONLY\n";
    break;
  }
  case SEQTYPE_SPEC_FARE:
  {
   *(_diag) << " SEQUENCE IS NOT PROCESSED - FOR SPECIFIED FARES ONLY\n";
    break;
  }
  case SEQ_SEG_DUAL_RBD:
  {
    *(_diag) << " SEQUENCE IS NOT PROCESSED - SEGMENT MUST HAVE BOTH RBD\n";
    break;
  }
  default:
  {
    *(_diag) << "UNKNOWN DIAG MSG\n";
    break;
  }
  }

  const AirSeg* airSeg = nullptr;

  for (const auto* bceSegment : bceSequence.segmentVector())
  {
    switch (errMsg)
    {
    case SEQTYPE_CONSTR_FARE:
    case SEQTYPE_SPEC_FARE:
      doDiag(bceSequence, *bceSegment, *(fare.fare()), airSeg, SEQTYPE_CONSTRSPEC_FARE);
    break;
    case SEQ_SEG_DUAL_RBD:
      doDiag(bceSequence, *bceSegment, *(fare.fare()), airSeg, SEQ_MISS_DUAL_RBD);
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(const BookingCodeExceptionSequence& bceSequence,
                                      const BookingCodeExceptionSegment& bceSegment,
                                      const Fare& fare,
                                      const AirSeg* airSeg,
                                      BCEDiagMsgs diagMsg)
{
  if (LIKELY(diag405() == nullptr))
    return;
  if (_shortDiag)
    return;

  *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
           << " SEGNUM: " << bceSegment.segNo();
  switch (diagMsg)
  {
  case PRIME_FAIL:
  {
    *(_diag) << " **FAILED PRIME**\n"
             << "   FLIGHT: " << airSeg->carrier() << airSeg->flightNumber()
             << " PRIME IS X CARRIER MATCHES "
             << "\n   FLIGHTCARRIER: " << airSeg->carrier() << " FARECARRIER: " << fare.carrier()
             << " NEXT FLT " << std::endl;
    break;
  }
  case XDOLLAR_CXR_FAIL:
  {
    *(_diag) << " **FAIL CARRIER X$**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << " FARE CXR: " << fare.carrier() << " NEXT FLT " << std::endl;
    break;
  }
  case CXR_FAIL_NXTSEQ:
  {
    *(_diag) << " **FAIL CARRIER**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << " BCESEGCXR: " << bceSegment.viaCarrier() << " FARECXR: " << fare.carrier()
             << " NEXT SEQUENCE " << std::endl;
    break;
  }
  case CXR_FAIL_NXTFLT:
  {
    *(_diag) << " **FAIL CARRIER**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << " BCESEGCXR: " << bceSegment.viaCarrier() << " NEXT FLT " << std::endl;
    break;
  }
  case LOC_FAIL_NXTSEQ:
  {
    *(_diag) << " **FAIL LOCATION**\n"
             << "   FARE ORIG DEST: " << fare.origin() << fare.destination()
             << "      BCELOC1/LOC2: " << bceSegment.loc1Type() << "-" << bceSegment.loc1() << "/"
             << bceSegment.loc2Type() << "-" << bceSegment.loc2()
             << "\n   IFTAG: " << bceSequence.ifTag()
             << "     DIRECTIONIND: " << bceSegment.directionInd() << " NEXT SEQUENCE "
             << std::endl;
    break;
  }
  case SEQTYPE_CONSTRSPEC_FARE:
  {
    *(_diag) << "\n   BCESEGCXR: " << bceSegment.viaCarrier()
             << "               FARECXR: " << fare.carrier()
             << "\n   FARE ORIG DEST: " << fare.origin() << fare.destination()
             << "      BCELOC1LOC2: " << bceSegment.loc1() << bceSegment.loc2()
             << "\n   IFTAG: " << bceSequence.ifTag()
             << "  DIRECTIONIND: " << bceSegment.directionInd()
             << "   BCEFARECLASS: " << bceSegment.fareclass()
             << "\n   RESTRICTIONIND: " << bceSegment.restrictionTag()
             << "   BCEBKG1: " << bceSegment.bookingCode1()
             << "   BCEBKG2: " << bceSegment.bookingCode2() << std::endl;
    break;
  }
  case SEQ_MISS_DUAL_RBD:
  {
    *(_diag) << "\n   BCESEGCXR: " << bceSegment.viaCarrier()
             << "               FARECXR: " << fare.carrier()
             << "\n   FARE ORIG DEST: " << fare.origin() << fare.destination()
             << "      BCELOC1LOC2: " << bceSegment.loc1() << bceSegment.loc2()
             << "\n   IFTAG: " << bceSequence.ifTag()
             << "  DIRECTIONIND: " << bceSegment.directionInd()
             << "   BCEFARECLASS: " << bceSegment.fareclass()
             << "\n   RESTRICTIONIND: " << bceSegment.restrictionTag()
             << "   BCEBKG1: " << bceSegment.bookingCode1()
             << "   BCEBKG2: " << bceSegment.bookingCode2() << std::endl;
    break;
  }
  case TUNING_SEGM_FAIL:
  {
    *(_diag) << " **FAIL TUNING ** \n";
    BCESegmentProcessed& tuningSegm =
        _bceTuning->_sequencesProcessed[_iSequence]._segmentsProcessed[_iSegment];
    *(_diag) << "   FARE: " << tuningSegm._fareProcessedSegment << "      REASON: ";
    switch (tuningSegm._reasonNoMatch)
    {
    case BCESegmentProcessed::CARRIER:
    {
      *(_diag) << "CARRIER \n";
      break;
    }
    case BCESegmentProcessed::PRIMARY_SECONDARY:
    {
      *(_diag) << "PRIMARY SECONDARY \n";
      break;
    }
    case BCESegmentProcessed::FLIGHT:
    {
      *(_diag) << "FLIGHTS \n";
      break;
    }
    case BCESegmentProcessed::EQUIPMENT:
    {
      *(_diag) << "EQUIPMENT \n";
      break;
    }
    case BCESegmentProcessed::PORTION_OF_TRAVEL:
    {
      *(_diag) << "PORTION OF TRAVEL \n";
      break;
    }
    case BCESegmentProcessed::TSI:
    {
      *(_diag) << "TSI \n";
      break;
    }
    case BCESegmentProcessed::LOCATION:
    {
      *(_diag) << "LOCATION \n";
      break;
    }
    case BCESegmentProcessed::POINT_OF_SALE:
    {
      *(_diag) << "POINT OF SALE \n";
      break;
    }
    case BCESegmentProcessed::SOLD_TAG:
    {
      *(_diag) << "SOLD TAG \n";
      break;
    }
    case BCESegmentProcessed::DATE_TIME_DOW:
    {
      *(_diag) << "DATE TIME DOW \n";
      break;
    }
    default:
    {
      *(_diag) << "UNKNOWN \n";
      break;
    }
    }
    break;
  }
  default:
  {
    *(_diag) << "UNKNOWN DIAG MSG\n";
    break;
  }
  }
}

//------------------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(const BookingCodeExceptionSequence& bceSequence,
                                      const BookingCodeExceptionSegment& bceSegment,
                                      PricingTrx& trx,
                                      BCEDiagMsgs diagMsg)
{
  if (LIKELY(diag405() == nullptr))
    return;
  if (_shortDiag)
    return;

  *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
           << " SEGNUM: " << bceSegment.segNo();

  switch (diagMsg)
  {
  case POINT_OF_SALE_FAIL:
  {
    *(_diag) << " **FAIL POINT OF SALE**\n"
             << "   AGENT LOCATION: " << trx.getRequest()->ticketingAgent()->agentCity()
             << " BCEPOS LOC: " << bceSegment.posLoc() << " POS TSI:" << bceSegment.posTsi()
             << " NEXT FLT " << std::endl;
    break;
  }
  case SOLD_TAG_FAIL:
  {
    *(_diag) << " **FAIL SOLD TAG**\n"
             << "   BCE SOLD IND: " << bceSegment.soldInOutInd()
             << " SALELOC : " << TrxUtil::saleLoc(trx)->nation()
             << " TICKETINGLOC : " << TrxUtil::ticketingLoc(trx)->nation() << " NEXT FLT "
             << std::endl;
    break;
  }
  default:
  {
    *(_diag) << "UNKNOWN DIAG MSG\n";
    break;
  }
  }
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(const BookingCodeExceptionSequence& bceSequence,
                                      const BookingCodeExceptionSegment& bceSegment,
                                      uint16_t fltIndex,
                                      const AirSeg* airSeg,
                                      BCEDiagMsgs diagMsg)
{
  if (LIKELY(diag405() == nullptr))
    return;

  switch (diagMsg)
  {
  case IFTAG_FAIL:
  {
    if (_shortDiag)
      return;

    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << "\n    **IF TAG FAILED**   NEXT SEQUENCE " << std::endl;
    break;
  }
  case SKIP_FLT:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << "\n   FLIGHT: " << airSeg->carrier() << " "
             << airSeg->flightNumber()
             << " SKIPPING ALREADY MATCHED IT WITH: " << _fltResultVector[fltIndex].first << " "
             << _fltResultVector[fltIndex].second << std::endl;
    break;
  }
  case SKIP_FLT_CAT25:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << "\n   FLIGHT: " << airSeg->carrier() << " "
             << airSeg->flightNumber() << " SKIPPING CAT25 PRIME SECTOR " << std::endl;
    break;
  }
  case SEGMENT_PASS:
  {
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **MATCH SEGMENT**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber() << std::endl;
    break;
  }
  case SEGMENT_SKIP:
  {
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **SKIP**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber() << std::endl;
    break;
  }
  case PRIMRY_SECNDRY_NXTSEQ:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL PRIMARY SECONDARY**\n"
             << "   BCEPRIMARYSECONDARY: " << bceSegment.primarySecondary()
             << "\n   FARE IS DOMESTIC OR TRANSBORDER OR FOREIGNDOMESTIC FAILED  NEXT SEQUENCE "
             << std::endl;
    break;
  }
  case PRIMRY_SECNDRY_NXTFLT:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL PRIMARY SECONDARY**\n"
             << "   PRIMARYSECONDARY: " << bceSegment.primarySecondary() << " NEXT FLT "
             << std::endl;
    break;
  }
  case FLT_INDIVIDUAL_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL FLT INDIVIDUAL**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << " BCEFLT1: " << bceSegment.flight1() << " BCEFLT2: " << bceSegment.flight2()
             << " NEXT FLT " << std::endl;
    break;
  }
  case FLT_RANGE_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL FLT RANGE**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << " BCEFLT1: " << bceSegment.flight1() << " BCEFLT2: " << bceSegment.flight2()
             << " NEXT FLT " << std::endl;
    break;
  }
  case EQUIPMENT_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL EQUIPMENT**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << " FLT EQUIP: " << airSeg->equipmentType() << " BCEEQUIP: " << bceSegment.equipType()
             << " NEXT FLT " << std::endl;
    break;
  }
  case PORTION_OF_TVL_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL PORTION TVL**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber() << " "
             << airSeg->origAirport() << "-" << airSeg->destAirport()
             << " ORIG/DEST AREA: " << airSeg->origin()->area() << "/"
             << airSeg->destination()->area() << " \n"
             << "   BCEPORTION OF TRAVEL: " << bceSegment.tvlPortion() << " NEXT FLT " << std::endl;
    break;
  }
  case TSI_PU_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL TSI-PU**\n"
             << "   NEXT FLT " << std::endl;
    break;
  }
  case TSI_PU_BAD:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **BAD TSI-PU**\n"
             << "   NEXT FLT " << std::endl;
    break;
  }
  case TSI_FC_REVAL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **REVAL TSI-FC**\n"
             << "   NEXT FLT " << std::endl;
    break;
  }
  case TSI_FC_BAD:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **BAD TSI1-FC**\n"
             << "   NEXT FLT " << std::endl;
    break;
  }
  case TSI_FC_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL TSI-FC**\n"
             << "   NEXT FLT " << std::endl;
    break;
  }
  case TSI_FC_BAD2:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **BAD TSI2-FC**\n"
             << "   NEXT FLT " << std::endl;
    break;
  }
  case LOC_FAIL_NXTFLT:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **FAIL LOCATION**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber() << " "
             << airSeg->origAirport() << airSeg->destAirport()
             << "      BCELOC1/LOC2: " << bceSegment.loc1Type() << "-" << bceSegment.loc1() << "/"
             << bceSegment.loc2Type() << "-" << bceSegment.loc2()
             << "\n   IFTAG: " << bceSequence.ifTag()
             << " DIRECTIONIND: " << bceSegment.directionInd() << " NEXT FLT " << std::endl;
    break;
  }
  case IFTAG_PASS:
  {
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << " **IFTAG SEG MATCH**\n"
             << "   FLIGHT: " << airSeg->carrier() << " " << airSeg->flightNumber()
             << "   IFTAG : " << bceSequence.ifTag() << std::endl;
    break;
  }
  case NOT_FIRST_MATCHING_SEQ:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << "\n   FLIGHT: " << airSeg->carrier() << " "
             << airSeg->flightNumber() << " SKIP FLT-SEQ DID NOT MATCH WHILE RULE1 " << std::endl;
    break;
  }
  case NO_SEQ_MATCHED_IN_RULE1:
  {
    if (_shortDiag)
      return;
    *(_diag) << " ITEM: " << bceSequence.itemNo() << " SEQNUM: " << bceSequence.seqNo()
             << " SEGNUM: " << bceSegment.segNo() << "\n   FLIGHT: " << airSeg->carrier() << " "
             << airSeg->flightNumber() << " SKIP FLT-NO SEQ MATCHED WHILE RULE1 " << std::endl;
    break;
  }
  default:
  {
    break;
  }
  }
}

//--------------------------------------------------------------
void
BookingCodeExceptionValidator::diagApplyBc(PricingTrx& trx,
                                           const BookingCodeExceptionSequence& bceSequence,
                                           const BookingCodeExceptionSegment& bceSegment)
{
  if (LIKELY(diag405() == nullptr))
    return;
  *(_diag) << " \n";
  *(_diag) << "   APPLY BOOKINGCODE FROM SEQ/SEG: " << bceSequence.seqNo() << "/"
           << bceSegment.segNo() << " REST TAG: " << bceSegment.restrictionTag() << std::endl;
  *(_diag) << "   BCE BKGCODES: " << bceSegment.bookingCode1();

  if (bceSegment.bookingCode2().empty())
    *(_diag) << "  ";
  else
    *(_diag) << " " << bceSegment.bookingCode2();

  *(_diag) << "   ";

  bool ignoreAvail = AltPricingUtil::ignoreAvail(trx);
  bool wpas = trx.getRequest()->isWpas();

  switch (bceSegment.restrictionTag())
  {
  case BCE_PERMITTED:
  {
    *(_diag) << "PERMITTED";
    break;
  }
  case BCE_REQUIRED:
  {
    *(_diag) << "REQUIRED";
    break;
  }
  case BCE_PERMITTED_IF_PRIME_NOT_OFFER:
  {
    *(_diag) << "BCE PERMITTED WHEN PRIME NOT OFFERED";
    if (ignoreAvail)
      *(_diag) << " / IGNORE AVL";
    break;
  }
  case BCE_PERMITTED_IF_PRIME_NOT_AVAIL:
  {
    *(_diag) << "BCE PERMITTED WHEN PRIME NOT AVAIL";
    if (ignoreAvail)
      *(_diag) << " / IGNORE AVL";
    if (wpas)
      *(_diag) << " / SKIP AVL";
    break;
  }
  case BCE_REQUIRED_IF_PRIME_NOT_OFFER:
  {
    *(_diag) << "BCE REQUIRED WHEN PRIME NOT OFFERED";
    if (ignoreAvail)
      *(_diag) << " / IGNORE AVL";
    break;
  }
  case BCE_REQUIRED_IF_PRIME_NOT_AVAIL:
  {
    *(_diag) << "BCE REQUIRED WHEN PRIME NOT AVAIL";
    if (ignoreAvail)
      *(_diag) << " / IGNORE AVL";
    if (wpas)
      *(_diag) << " / SKIP AVL";
    break;
  }
  case BCE_REQUIRED_WHEN_OFFERED:
  {
    *(_diag) << "BCE REQUIRED WHEN OFFERED";
    break;
  }
  case BCE_REQUIRED_WHEN_AVAIL:
  {
    *(_diag) << "BCE REQUIRED WHEN AVAIL";
    if (wpas)
      *(_diag) << " / SKIP AVL";
    break;
  }
  case BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE:
  {
    *(_diag) << "RBD " << bceSegment.bookingCode2() << " PERMITTED IF " << bceSegment.bookingCode1()
             << " AVAIL";
    if (ignoreAvail)
      *(_diag) << " / IGNORE AVL";
    if (wpas)
      *(_diag) << " / SKIP AVL FOR " << bceSegment.bookingCode1();
    break;
  }
  case BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE:
  {
    *(_diag) << "RBD " << bceSegment.bookingCode2() << " REQUIRED IF " << bceSegment.bookingCode1()
             << " AVAIL";
    if (ignoreAvail)
      *(_diag) << " / IGNORE AVL";
    if (wpas)
      *(_diag) << " / SKIP AVL FOR " << bceSegment.bookingCode1();
    break;
  }
  case BCE_ADDITIONAL_DATA_APPLIES:
  {
    *(_diag) << "BCE ADDITIONAL DATA APPLY";
    break;
  }
  case BCE_STANDBY:
  {
    *(_diag) << "BCE FOR UNCONFIRMED FLIGHTS";
    break;
  }
  case BCE_NOT_PERMITTED:
  {
    *(_diag) << "BCE NOT PERMITTED";
    break;
  }
  case BCE_DOES_NOT_EXIST:
  {
    *(_diag) << "BCE DOES NOT EXIST";
    break;
  }
  default:
  {
    *(_diag) << "UNKNOWN RESTR TAG";
    break;
  }
  }
  *(_diag) << std::endl;
}

//--------------------------------------------------------------
void
BookingCodeExceptionValidator::diagBc(const TravelSeg* tvlSeg,
                                      const PaxTypeFare::SegmentStatus& segStat,
                                      BCEDiagMsgs diagMsg)
{
  if (LIKELY(diag405() == nullptr))
    return;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
  *(_diag) << "   FLIGHT: " << airSeg->carrier() << " " << std::setw(4) << airSeg->flightNumber()
           << airSeg->getBookingCode();

  if (airSeg->unflown())
    *(_diag) << "      ";
  else
    *(_diag) << " FLOWN";

  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
      !segStat._bkgCodeReBook.empty())
  {
    *(_diag) << " REBOOK: " << segStat._bkgCodeReBook << "     ";
  }
  else
  {
    *(_diag) << "               ";
  }

  switch (diagMsg)
  {
  case FLT_PASS_BCE:
  {
    *(_diag) << "**** PASS ****" << std::endl;
    break;
  }
  case FLT_FAIL_BCE:
  {
    *(_diag) << "**** FAIL ****" << std::endl;
    break;
  }
  case FLT_NOMATCH_BCE:
  {
    *(_diag) << "**** NOMATCH ****" << std::endl;
    break;
  }
  case FLT_FAIL_CABIN:
  {
    *(_diag) << "*** FAIL CABIN ***" << std::endl;
    break;
  }
  default:
  {
    *(_diag) << "** INVALID STATUS **" << std::endl;
    break;
  }
  }
}
//--------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(const AirSeg* airSeg, uint16_t fltIndex)
{
  if (diag405() == nullptr)
  {
    return;
  }
  *(_diag) << " \n   SKIP APPLY BOOKINGCODE FOR FLIGHT: " << airSeg->carrier() << " "
           << airSeg->flightNumber()
           << "\n   ALREADY PROCESSED USING SEQNUM/SEGNUM : " << _fltResultVector[fltIndex].first
           << "/" << _fltResultVector[fltIndex].second << std::endl;
}

//-------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(const BookingCodeExceptionSequence& bceSequence)
{
  if (LIKELY(diag405() == nullptr))
    return;
  if (_shortDiag)
    return;
  *(_diag) << "\n \n";

  *(_diag) << _iSequence + 1 << "."
           << " SEQUENCE NUMBER: " << bceSequence.seqNo() << "  IF TAG: " << bceSequence.ifTag()
           << "  SEGMENT COUNT: " << bceSequence.segCnt() << " \n";
}

//-------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag(uint16_t bceSeqSize, uint16_t seqProcessed)
{
  if (UNLIKELY(diag405() != nullptr))
  {
    *(_diag) << "\n \n TOTAL NUMBER OF SEQUENCES PROCESSED/PRESENT: " << seqProcessed << "/"
             << bceSeqSize << std::endl;
  }
}

//-------------------------------------------------------------------
void
BookingCodeExceptionValidator::doDiag()
{
  if (UNLIKELY(diag405() != nullptr && !_shortDiag))
  {
    *(_diag) << " SKIPPING THIS AND ALL OTHER SEQUENCES/RESULT ALREADY FOUND \n";
  }
}

//-------------------------------------------------------------------
void
BookingCodeExceptionValidator::otherDiag(BCEDiagMsgs diagMsg)
{
  if (diag405() == nullptr)
    return;

  switch (diagMsg)
  {
  case TUNING_SEQ_FAIL:
  {
    if (_shortDiag)
      return;
    *(_diag) << "TUNING - SEQ FAILED BY FARE: "
             << _bceTuning->_sequencesProcessed[_iSequence]._fareProcessedSeq << " \n";
    break;
  }
  case SEQ_FAIL_ANALYZE_FM_LOC:
  {
    if (_shortDiag)
      return;
    *(_diag) << "**FAIL SEQUENCE: BY FUNCTION ANALYZE-FM-LOC** \n";
    break;
  }
  case SEG_FAIL_BCE_FROMTO_PRI:
  {
    if (_shortDiag)
      return;
    *(_diag) << "FLT FAIL BCE-FROMTO-PRI CHECK AFTER VALIDATEPRIMARYSECONDARY FUNC \n";
    break;
  }
  case SKIP_SEQ_REST_B_D_R6C1:
  {
    *(_diag) << " *** SKIP SEQUENCE: REST TAG B/D NOT APPLICABLE IN R6/C1\n";
    break;
  }
  case SKIP_SEQ_REST_B_D_CAT25:
  {
    *(_diag) << " *** SKIP SEQUENCE: REST TAG B/D NOT APPLICABLE IN CAT25 R3\n";
    break;
  }
  default:
    break;
  }
}

//-------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::setFirstLastAirSeg(PaxTypeFare& paxTypeFare)
{
  // set the first valid air Segment for this fare market
  TravelSegVectorCI tvlIT = paxTypeFare.fareMarket()->travelSeg().begin();
  _firstAirsegInMkt = dynamic_cast<AirSeg*>(*tvlIT);
  while (_firstAirsegInMkt == nullptr && tvlIT != paxTypeFare.fareMarket()->travelSeg().end())
  {
    ++tvlIT;
    _firstAirsegInMkt = dynamic_cast<AirSeg*>(*tvlIT);
  }

  if (UNLIKELY(tvlIT == paxTypeFare.fareMarket()->travelSeg().end()))
    _firstAirsegInMkt = nullptr;

  // set the last valid air Segment for this fare market
  tvlIT = paxTypeFare.fareMarket()->travelSeg().end();
  --tvlIT; // point to the last travelSeg
  _lastAirsegInMkt = dynamic_cast<AirSeg*>(*tvlIT);

  while (_lastAirsegInMkt == nullptr && tvlIT != paxTypeFare.fareMarket()->travelSeg().begin())
  {
    --tvlIT;
    _lastAirsegInMkt = dynamic_cast<AirSeg*>(*tvlIT);
  }
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::tryRule2(PricingTrx& trx, PaxTypeFare& paxTypeFare) const
{
  // if its not a WPNC entry - do not try Rule2
  if (!(trx.getRequest()->isLowFareRequested()))
    return false;

  // do not try rule 2 if it is shopping path 1 (itin selector)
  if (trx.isShopping())
    return false;
  if (LIKELY(trx.getTrxType() != PricingTrx::PRICING_TRX))
    return false;

  if (!(trx.getOptions()->soloActiveForPricing()))
    return false;

  // if only one flight in this fare component then no need to try Rule 2
  const uint16_t airSegSize = paxTypeFare.fareMarket()->travelSeg().size();
  if (airSegSize < 2)
    return false;

  // Only try Rule 2 if :
  // 1. All the flights of the carriers for which Rule2 is activated and
  //    Journey not activated did not PASS Table 999 using Rule 1
  // 2. Atleast one sequence was matched while trying Rule 1
  // 3. All NON Rule2 carrier flights passed

  TravelSegVectorCI tvlI = paxTypeFare.fareMarket()->travelSeg().begin();
  const TravelSegVectorCI tvlE = paxTypeFare.fareMarket()->travelSeg().end();
  PaxTypeFare::SegmentStatusVec& segStatusVec = paxTypeFare.segmentStatus();
  bool allFlightsPassed = true;
  bool atleastOneMatchFound = false;
  bool nonRule2CarrierFailed = false;
  bool atleastOneSoloCxrPresent = false;
  uint16_t i = 0;
  const AirSeg* airSeg = nullptr;
  for (; i < airSegSize && tvlI != tvlE; tvlI++, i++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;
    if ((*tvlI)->carrierPref() == nullptr)
      return false;

    if ((*tvlI)->carrierPref()->availabilityApplyrul2st() == NO && !airSeg->flowJourneyCarrier() &&
        !airSeg->localJourneyCarrier()) // db availabilityIgrul2st
    {
      atleastOneSoloCxrPresent = true;
      if (!(segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS)))
        allFlightsPassed = false;

      if (_firstMatchingSeqs[i] != BCE_FLT_NOMATCH)
        atleastOneMatchFound = true;
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
  if (allFlightsPassed || nonRule2CarrierFailed || !atleastOneMatchFound)
    return false;

  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Analyze sequences and find the 1st one that will be used to change the 1st
// char of the YY Fare Basis Code
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
BookingCodeExceptionValidator::analyzeSequencies(
    PricingTrx& trx,
    const BookingCodeExceptionSequenceList& bceSequenceList,
    PaxTypeFare& paxTypeFare,
    const AirSeg* pAirSeg)
{
  LOG4CXX_DEBUG(logger, "Entered BookingCodeExceptionValidator::analyzeSequencies()");

  if (UNLIKELY(bceSequenceList.empty()))
  {
    LOG4CXX_DEBUG(logger, "Leaving BookingCodeExceptionValidator::analyzeSequencies(): failure");
    return; // There are no sequences.
  }

  bool bceBkgUpdated = false;
  for (const auto* sequence : bceSequenceList)
  {
    if (LIKELY(sequence->segCnt() != 0))
    {
      // validating table 999 from Record 6 Convention 1
      for (const auto* segment : sequence->segmentVector())
      {
        const BookingCodeExceptionSegment& bceSegment = *segment;
        if (validateCarrier(*sequence, bceSegment, paxTypeFare, *pAirSeg) &&
            validateFCType(bceSegment, paxTypeFare) &&
            analyzeFMLoc(trx, paxTypeFare, *sequence, bceSegment))
        {
          if (analyzeSeg(paxTypeFare, bceSegment))
          {
            bceBkgUpdated = true;
            break;
          }
        }
        else
          break;
      }
      if (bceBkgUpdated)
        break;
    } // end if((*sequenceIter)->segCnt() != 0)
  } // end of sequenceIter iterator loop
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Analyze sequences and find the 1st one that will be used to change the 1st
// char of the YY Fare Basis Code
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
BookingCodeExceptionValidator::analyzeSeg(PaxTypeFare& paxTypeFare,
                                          const BookingCodeExceptionSegment& bceSegment)
{
  LOG4CXX_DEBUG(logger, "Entered BookingCodeExceptionValidator::analyzeSeg()");

  BookingCode bkg;

  bool conditionalData = false;

  conditionalData =
      (bceSegment.fltRangeAppl() != BCE_CHAR_BLANK || bceSegment.flight1() > 0 ||
       (!bceSegment.equipType().empty() && bceSegment.equipType() != BCE_EQUIPBLANK) ||
       !bceSegment.tvlPortion().empty() || bceSegment.posTsi() > 0 ||
       !bceSegment.posLoc().empty() ||
       (bceSegment.posLocType() != BCE_CHAR_BLANK && bceSegment.posLocType() != '\0') ||
       (bceSegment.soldInOutInd() != BCE_CHAR_BLANK && bceSegment.soldInOutInd() != '\0') ||
       bceSegment.tvlEffYear() > 0 || bceSegment.tvlEffMonth() > 0 || bceSegment.tvlEffDay() > 0 ||
       bceSegment.tvlDiscYear() > 0 || bceSegment.tvlDiscMonth() > 0 ||
       bceSegment.tvlDiscDay() > 0 || !bceSegment.daysOfWeek().empty() ||
       (bceSegment.tvlStartTime() != 0 && bceSegment.tvlStartTime() != 65535) ||
       (bceSegment.tvlEndTime() != 0 && bceSegment.tvlEndTime() != 65535) ||
       !bceSegment.arbZoneNo().empty());

  if (conditionalData || bceSegment.restrictionTag() == BCE_STANDBY || //  S
      bceSegment.restrictionTag() == BCE_NOT_PERMITTED) //  X
  {
    return false;
  }

  if (!conditionalData && (bceSegment.restrictionTag() == BCE_PERMITTED_IF_PRIME_NOT_OFFER || // O
                           bceSegment.restrictionTag() == BCE_PERMITTED_IF_PRIME_NOT_AVAIL || // A
                           bceSegment.restrictionTag() == BCE_REQUIRED_IF_PRIME_NOT_OFFER || // G
                           bceSegment.restrictionTag() == BCE_REQUIRED_IF_PRIME_NOT_AVAIL || // H
                           bceSegment.restrictionTag() == BCE_ADDITIONAL_DATA_APPLIES || // U
                           bceSegment.restrictionTag() == BCE_DOES_NOT_EXIST)) // N
  {
    bkg.clear(); // do not change 1st char
    _bkgYYupdate = true;
    paxTypeFare.setChangeFareBasisBkgCode(bkg);
    return _bkgYYupdate;
  }

  if (!conditionalData && (bceSegment.restrictionTag() == BCE_PERMITTED || // P
                           bceSegment.restrictionTag() == BCE_REQUIRED || // R
                           bceSegment.restrictionTag() == BCE_REQUIRED_WHEN_OFFERED || // W
                           bceSegment.restrictionTag() == BCE_REQUIRED_WHEN_AVAIL)) // V
  {
    bkg = bceSegment.bookingCode1();
    if (paxTypeFare.getAllowedChangeFareBasisBkgCode() != ' ' &&
        bkg.find(paxTypeFare.getAllowedChangeFareBasisBkgCode()) == std::string::npos)
      paxTypeFare.setRequestedFareBasisInvalid();

    _bkgYYupdate = true;
    paxTypeFare.setChangeFareBasisBkgCode(bkg);
    return _bkgYYupdate;
  }

  return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Analyze geographic locations of the fare market against locations in the bceSegment,
// if they are present
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
BookingCodeExceptionValidator::analyzeFMLoc(PricingTrx& trx,
                                            const PaxTypeFare& paxTypeFare,
                                            const BookingCodeExceptionSequence& bceSequence,
                                            const BookingCodeExceptionSegment& bceSegment) const
{
  LOG4CXX_DEBUG(logger, "Entered BookingCodeExceptionValidator::analyzeFMLoc()");

  if (bceSegment.loc1().empty() && bceSegment.loc2().empty())
    return true; // BCE_PASS;

  bool indLoc = true;
  const Loc* fareOrigLoc = paxTypeFare.fareMarket()->origin();
  const Loc* fareDestLoc = paxTypeFare.fareMarket()->destination();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // code is done according to FRD A_F_Industry Fares_V3.5
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++

  bool origInLoc1 = false;
  bool origInLoc2 = false;
  bool destInLoc1 = false;
  bool destInLoc2 = false;

  if (bceSegment.loc1().empty() || LocUtil::isInLoc(*fareOrigLoc,
                                                    bceSegment.loc1Type(),
                                                    bceSegment.loc1(),
                                                    ATPCO_VENDOR_CODE,
                                                    RESERVED,
                                                    LocUtil::OTHER,
                                                    GeoTravelType::International,
                                                    EMPTY_STRING(),
                                                    trx.getRequest()->ticketingDT()))
  {
    origInLoc1 = true;
  }

  if (bceSegment.loc2().empty() || LocUtil::isInLoc(*fareDestLoc,
                                                    bceSegment.loc2Type(),
                                                    bceSegment.loc2(),
                                                    ATPCO_VENDOR_CODE,
                                                    RESERVED,
                                                    LocUtil::OTHER,
                                                    GeoTravelType::International,
                                                    EMPTY_STRING(),
                                                    trx.getRequest()->ticketingDT()))
  {
    destInLoc2 = true;
  }

  if (bceSegment.loc2().empty() || LocUtil::isInLoc(*fareOrigLoc,
                                                    bceSegment.loc2Type(),
                                                    bceSegment.loc2(),
                                                    ATPCO_VENDOR_CODE,
                                                    RESERVED,
                                                    LocUtil::OTHER,
                                                    GeoTravelType::International,
                                                    EMPTY_STRING(),
                                                    trx.getRequest()->ticketingDT()))
  {
    origInLoc2 = true;
  }

  if (bceSegment.loc1().empty() || LocUtil::isInLoc(*fareDestLoc,
                                                    bceSegment.loc1Type(),
                                                    bceSegment.loc1(),
                                                    ATPCO_VENDOR_CODE,
                                                    RESERVED,
                                                    LocUtil::OTHER,
                                                    GeoTravelType::International,
                                                    EMPTY_STRING(),
                                                    trx.getRequest()->ticketingDT()))
  {
    destInLoc1 = true;
  }

  switch (bceSegment.directionInd())
  {
  case '1': // Travel from Loc1 to Loc2
  case '2': // Travel from Loc2 to Loc1
  case ' ': // Travel/fare between Loc 1 and Loc2
  {
    if (!((origInLoc1 && destInLoc2) || (origInLoc2 && destInLoc1)))
      indLoc = false;
    break;
  }

  case '3': // Fares originating Loc1
  {
    if (paxTypeFare.directionality() == TO)
    {
      // if the fare is INBOUND then LOC1 should be checked against the
      // destination because destination is actually the origination point
      // in this case. This is valid only for directionality values 3 and 4
      // and not for values 1 and 2
      fareDestLoc = paxTypeFare.fareMarket()->origin();
      fareOrigLoc = paxTypeFare.fareMarket()->destination();
    }
    if (LIKELY(bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1)) // if IFTag == 1
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*fareOrigLoc,
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        indLoc = false;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*fareDestLoc,
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        indLoc = false;
        break;
      }
      break;
    }
    else
    {
      indLoc = false;
      break;
    }
  }
  case '4': // Fares originating Loc2
  {
    if (paxTypeFare.directionality() == TO)
    {
      // if the fare is INBOUND then LOC1 should be checked against the
      // destination because destination is actually the origination point
      // in this case. This is valid only for directionality values 3 and 4
      // and not for values 1 and 2
      fareDestLoc = paxTypeFare.fareMarket()->origin();
      fareOrigLoc = paxTypeFare.fareMarket()->destination();
    }
    if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1) // if IFTag == 1
    {
      if (!bceSegment.loc1().empty() && // validate loc 1
          !LocUtil::isInLoc(*fareDestLoc,
                            bceSegment.loc1Type(),
                            bceSegment.loc1(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        indLoc = false;
        break;
      }

      if (!bceSegment.loc2().empty() && // validate loc 2
          !LocUtil::isInLoc(*fareOrigLoc,
                            bceSegment.loc2Type(),
                            bceSegment.loc2(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()))
      {
        indLoc = false;
        break;
      }
      break;
    }
    else
    {
      indLoc = false;
      break;
    }
  }
  default: // if direction indicaor is anything else then 1,2,3,4 or blank
  {
    indLoc = false;
    break;
  }
  }
  return indLoc;
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateCarrier(const BookingCodeExceptionSequence& bceSequence,
                                               const BookingCodeExceptionSegment& bceSegment,
                                               const PaxTypeFare& fare,
                                               const AirSeg& airSeg)
{
  if (UNLIKELY(bceSegment.viaCarrier() == BCE_DOLLARDOLLARCARRIER ||
      bceSegment.viaCarrier() == BCE_ANYCARRIER))
    return true;

  if (bceSequence.ifTag() == BCE_IF_FARECOMPONENT && bceSegment.segNo() == 1)
  {
    if (bceSegment.viaCarrier() == BCE_XDOLLARCARRIER)
    {
      if (LIKELY(fare.fare()->isIndustry()))
        return true;

      if (airSeg.carrier() != fare.carrier())
        return true;
    }
    if (bceSegment.viaCarrier() == INDUSTRY_CARRIER)
    {
      if (LIKELY(fare.fare()->isIndustry()))
        return true;
    }
    return (UNLIKELY(bceSegment.viaCarrier() == fare.carrier()));
  }
  else // if(bceSequence._ifTag == BCE_MATCH_ANY_TVLSEG || blank)
  {
    if (UNLIKELY(bceSegment.viaCarrier() == BCE_XDOLLARCARRIER))
    {
      if (fare.fare()->isIndustry() || (airSeg.carrier() != fare.carrier()))
        return true;
    }
    return (bceSegment.viaCarrier() == airSeg.carrier());
  }
}

//----------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateFCType(const BookingCodeExceptionSegment& bceSegment,
                                              const PaxTypeFare& paxTypeFare)
{
  const FareClassCode bceFareClass = bceSegment.fareclass();
  if (bceFareClass.empty())
    return true;

  const FareClassCode paxFareClass = paxTypeFare.fareClass();
  switch (bceSegment.fareclassType())
  {
  case 'T':
    return (RuleUtil::matchFareType(bceFareClass.c_str(), paxTypeFare.fcaFareType()));
  case 'F':
    return (bceFareClass == paxFareClass);
  case 'M':
    return (RuleUtil::matchFareClass(bceFareClass.c_str(), paxFareClass.c_str()));
  case 'A':
    return (bceFareClass[0] == paxFareClass[0]);
  default:
    return false;
  }
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::flowJourneyCarrier(PricingTrx& trx, const TravelSeg* tvlSeg) const
{
  if (UNLIKELY(trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(trx.getOptions()->journeyActivatedForPricing()))
      return false;
  }
  else if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(trx.getOptions()->journeyActivatedForShopping()))
      return false;
  }

  if (!(trx.getOptions()->applyJourneyLogic()))
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

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::localJourneyCarrier(PricingTrx& trx, const TravelSeg* tvlSeg) const
{
  if (UNLIKELY(trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(trx.getOptions()->journeyActivatedForPricing()))
      return false;
  }
  else if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(trx.getOptions()->journeyActivatedForShopping()))
      return false;
  }

  if (!(trx.getOptions()->applyJourneyLogic()))
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

//-------------------------------------------------------------------------------
std::vector<ClassOfService*>*
BookingCodeExceptionValidator::flowMarketAvail(PricingTrx& trx,
                                               const TravelSeg* tvlSeg,
                                               FareMarket* fm,
                                               uint16_t airIndex)
{
  std::vector<ClassOfService*>* cosVec = nullptr;
  if (UNLIKELY(trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    cosVec = pricingAvail(trx, tvlSeg);
  }
  else if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    cosVec = shoppingAvail(trx, tvlSeg, fm, airIndex);
  }
  return cosVec;
}

//-------------------------------------------------------------------------------
std::vector<ClassOfService*>*
BookingCodeExceptionValidator::pricingAvail(PricingTrx& trx, const TravelSeg* tvlSeg)
{
  if (trx.itin().size() > 1)
    return nullptr;

  Itin& itin = *trx.itin().front();
    return JourneyUtil::availability(const_cast<TravelSeg*>(tvlSeg), &itin);
}

//-------------------------------------------------------------------------------
std::vector<ClassOfService*>*
BookingCodeExceptionValidator::shoppingAvail(PricingTrx& trx,
                                             const TravelSeg* tvlSeg,
                                             FareMarket* fm,
                                             uint16_t airIndex)
{
  bool useFmAvail = airIndex < fm->classOfServiceVec().size();
  std::vector<ClassOfService*>* cosVec = useFmAvail ? fm->classOfServiceVec()[airIndex] : nullptr;

  if (flowJourneyCarrier(trx, tvlSeg))
  {
    if (partOfJourney(trx, tvlSeg))
    {
      cosVec = &ShoppingUtil::getMaxThruClassOfServiceForSeg(trx, const_cast<TravelSeg*>(tvlSeg));
      if (LIKELY(cosVec != nullptr))
      {
        if (cosVec->empty())
          cosVec = nullptr;
      }
    }
  }
  else if (LIKELY(localJourneyCarrier(trx, tvlSeg)))
  {
    if (partOfJourney(trx, tvlSeg))
      cosVec = &ShoppingUtil::getMaxThruClassOfServiceForSeg(trx, const_cast<TravelSeg*>(tvlSeg));
    if (LIKELY(cosVec != nullptr))
    {
      if (cosVec->empty())
        cosVec = nullptr;
    }
  }

  return cosVec;
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::tryLocalWithFlowAvail(PricingTrx& trx, PaxTypeFare& paxTypeFare)
    const
{
  if (UNLIKELY(trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    if (!(trx.getOptions()->journeyActivatedForPricing()))
      return false;

    if (!(trx.getOptions()->applyJourneyLogic())) // do not move outside if-else : tuning reason
      return false;

    if (!_partOfLocalJny)
      return false;
  }
  else if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!(trx.getOptions()->journeyActivatedForShopping()))
      return false;

    if (!(trx.getOptions()->applyJourneyLogic())) // do not move outside if-else : tuning reason
      return false;

    if (!_partOfLocalJny)
      return false;

    if (UNLIKELY(!journeyExistInItin(trx)))
      return false;
  }
  else
  {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateStartDate(const DateTime& fltDate,
                                                 const uint16_t bceYear,
                                                 const uint16_t bceMonth,
                                                 const uint16_t bceDay) const
{
  if (bceYear != 0)
  {
    if (fltDate.year() < bceYear)
      return false;
    if (fltDate.year() > bceYear)
      return true;
  }

  if (bceMonth != 0)
  {
    if (fltDate.month() < bceMonth)
      return false;
    if (fltDate.month() > bceMonth)
      return true;
  }

  if (bceDay != 0)
  {
    if (fltDate.day() < bceDay)
      return false;
    if (fltDate.day() > bceDay)
      return true;
  }
  return true;
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateStopDate(const DateTime& fltDate,
                                                const uint16_t bceYear,
                                                const uint16_t bceMonth,
                                                const uint16_t bceDay) const
{
  if (bceYear != 0)
  {
    if (fltDate.year() > bceYear)
      return false;
    if (fltDate.year() < bceYear)
      return true;
  }

  if (bceMonth != 0)
  {
    if (fltDate.month() > bceMonth)
      return false;
    if (fltDate.month() < bceMonth)
      return true;
  }

  if (bceDay != 0)
  {
    if (fltDate.day() > bceDay)
      return false;
    if (fltDate.day() < bceDay)
      return true;
  }
  return true;
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::setISegment(uint16_t segmentIndex, const AirSeg* airSeg)
{
  if (_stopTuning)
    return false;
  BCESequenceProcessed& tuningSeq = _bceTuning->_sequencesProcessed[_iSequence];
  uint16_t tuningSegsSize = tuningSeq._segmentsProcessed.size();
  uint16_t i = 0;
  bool tuningSegmFound = false;
  for (i = 0; i < tuningSegsSize; i++)
  {
    BCESegmentProcessed& tuningSegm = tuningSeq._segmentsProcessed[i];
    if (tuningSegm._segmentNumber != segmentIndex + 1)
      continue;
    if (tuningSegm._airSeg != airSeg)
      continue;
    tuningSegmFound = true;
    break;
  }

  if (tuningSegmFound)
  {
    _iSegment = i;
  }
  else
  {
    BCESegmentProcessed tuningSegm;
    tuningSeq._segmentsProcessed.push_back(tuningSegm);
    tuningSeq._segmentsProcessed[tuningSegsSize]._airSeg = airSeg;
    tuningSeq._segmentsProcessed[tuningSegsSize]._segmentNumber = segmentIndex + 1;
    tuningSeq._segmentsProcessed[tuningSegsSize]._noMatchSegment = false;
    tuningSeq._segmentsProcessed[tuningSegsSize]._segmentProcessedOnce = false;
    tuningSeq._segmentsProcessed[tuningSegsSize]._reasonNoMatch =
        BCESegmentProcessed::DEFAULT_VALUE;
    _iSegment = tuningSegsSize;
  }
  return true;
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::journeyExistInItin(PricingTrx& trx) const
{
  return !_itin.oAndDMarkets().empty();
}

//-------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::partOfJourney(PricingTrx& trx, const TravelSeg* tvlSeg) const
{
  return JourneyUtil::checkIfSegInFlowOd(tvlSeg, _itin.segmentOAndDMarket());
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::validateWPNCS(PricingTrx& trx,
                                             const uint32_t& itemNo,
                                             const BookingCodeExceptionSegment& bceSegment,
                                             const TravelSeg* tvlSeg,
                                             PaxTypeFare::SegmentStatusVec& segStatVec,
                                             uint16_t airIndex,
                                             PaxTypeFare& paxTypeFare,
                                             int16_t iFlt)
{
  if (bceSegment.restrictionTag() == BCE_DOES_NOT_EXIST)
  {
    tagFltFAILTagN(paxTypeFare, segStatVec, airIndex, trx);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_FAIL_BCE);
    return;
  }

  if (bceSegment.restrictionTag() == BCE_STANDBY ||
      bceSegment.restrictionTag() == BCE_NOT_PERMITTED ||
      bceSegment.restrictionTag() == BCE_ADDITIONAL_DATA_APPLIES)
  {
    tagFltNOMATCH(segStatVec, airIndex);
    diagBc(tvlSeg, segStatVec[airIndex], FLT_NOMATCH_BCE);
    return;
  }

  const AirSeg& airSeg = dynamic_cast<const AirSeg&>(*tvlSeg);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, false);
  segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, false);

  bool primeRBD = validateWPNCSFromPrimeRBD(bceSegment);

  // validate from prime RBD
  if (primeRBD)
  {
    bool bookedInPrime = false;
    CabinType cabin;
    std::vector<BookingCode> primeBookingCodeVec;
    std::vector<BookingCode> TempPrimeBookingCodeVec;
    paxTypeFare.getPrimeBookingCode(primeBookingCodeVec);
    uint16_t primeBCSize;
    if (!BookingCodeUtil::validateExcludedBookingCodes(trx, 
                                      primeBookingCodeVec, 
                                      segStatVec[airIndex], 
                                      TempPrimeBookingCodeVec, 
                                      _diag))
    {
      segStatVec[airIndex]._bkgCodeSegStatus = PaxTypeFare::BKSS_NOMATCH;
      return;
    }
    if (TempPrimeBookingCodeVec.size() > 0)
    {
      primeBookingCodeVec.clear();
      primeBookingCodeVec = TempPrimeBookingCodeVec;
    }
    primeBCSize = primeBookingCodeVec.size();
    for (uint16_t primeIndex = 0; primeIndex < primeBCSize; primeIndex++)
    {
      if (airSeg.getBookingCode() == primeBookingCodeVec[primeIndex])
      {
        if (!fallback::fallbackPriceByCabinActivation(&trx) &&
            !trx.getRequest()->isjumpUpCabinAllowed())
        {
          cabin = cabinWpncs(trx, airSeg,
                             primeBookingCodeVec[primeIndex],
                             paxTypeFare, itemNo, bceSegment);
          if (cabin == airSeg.bookedCabin())
          {
            bookedInPrime = true;
            break;
          }
        }
        else
        {
          bookedInPrime = true;
          break;
        }
      }
    }
    if (!bookedInPrime)
    {
      if (primeBookingCodeVec.size() > 0)
      {
        if (!fallback::fallbackPriceByCabinActivation(&trx) &&
            !trx.getRequest()->isjumpUpCabinAllowed())
        {
          validateRBDRec1PriceByCabin(trx, tvlSeg, airIndex, primeBookingCodeVec, paxTypeFare, segStatVec);
          return;
        }
        else
        {
          segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
          segStatVec[airIndex]._bkgCodeReBook = primeBookingCodeVec[0];
          segStatVec[airIndex]._reBookCabin = cabinWpncs(trx, airSeg,
                                                         primeBookingCodeVec[0],
                                                         paxTypeFare,
                                                         itemNo,
                                                         bceSegment);
          paxTypeFare.s8BFBookingCode() = primeBookingCodeVec[0];
        }
      }
      else
      {
        tagFltFAIL(segStatVec, airIndex, BOOKING_CODE_NOT_OFFERED);
        diagBc(tvlSeg, segStatVec[airIndex], FLT_FAIL_BCE);
        return;
      }

    }
    diagBc(tvlSeg, segStatVec[airIndex], FLT_PASS_BCE);
    return;
  }
  // check if airSeg.getBookingCode not in EBC
  bool bAnyValidBookingCodeAvailable = false;
  std::vector<BookingCode> BookingCodes;
  std::vector<BookingCode> DummyVec;
  BookingCodes.push_back(airSeg.getBookingCode());
  bAnyValidBookingCodeAvailable =  !(BookingCodeUtil::validateExcludedBookingCodes(trx, 
                                      BookingCodes, 
                                      segStatVec[airIndex], 
                                      DummyVec, 
                                      _diag));
  // validate from booking code in the BCE sequence
  if (!(airSeg.getBookingCode() == bceSegment.bookingCode1() ||
        airSeg.getBookingCode() == bceSegment.bookingCode2()) || bAnyValidBookingCodeAvailable)
  {
    segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
    if( iFlt == -1 &&
       (bceSegment.restrictionTag() == BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE ||
        bceSegment.restrictionTag() == BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE)   )
    {
      std::vector<BookingCode> BookingCodeVec;
      BookingCodeVec.push_back(bceSegment.bookingCode2());
      std::vector<BookingCode> TempPrimeBookingCodeVec;
      if (!BookingCodeUtil::validateExcludedBookingCodes(trx, 
                                      BookingCodeVec, 
                                      segStatVec[airIndex], 
                                      TempPrimeBookingCodeVec, 
                                      _diag))
      {
        segStatVec[airIndex]._bkgCodeSegStatus = PaxTypeFare::BKSS_NOMATCH;
        segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
        return;
      }

      BookingCode bceSegmentBkingCode2;
      if (TempPrimeBookingCodeVec.size() > 0)
      {
        segStatVec[airIndex]._bkgCodeReBook = TempPrimeBookingCodeVec[0];
        bceSegmentBkingCode2 = TempPrimeBookingCodeVec[0];
      }
      else
      {
        segStatVec[airIndex]._bkgCodeReBook = bceSegment.bookingCode2();
        bceSegmentBkingCode2 = bceSegment.bookingCode2();
      }
      segStatVec[airIndex]._reBookCabin = cabinWpncs(trx, airSeg,
                                                     bceSegmentBkingCode2,// bceSegment.bookingCode2(),
                                                     paxTypeFare,
                                                     itemNo,
                                                     bceSegment);
      paxTypeFare.s8BFBookingCode() = bceSegmentBkingCode2;
    }
    else
    {
      // If bceSegment.bookingCode1() not match EBC -> bkCode = bceSegment.bookingCode1() 
      // else if bceSegment.bookingCode2() not match EBC -> bkCode = bceSegment.bookingCode2() 
      std::vector<BookingCode> BookingCodeVec;
      BookingCodeVec.push_back(bceSegment.bookingCode1());
      std::vector<BookingCode> TempPrimeBookingCodeVec;
      BookingCode bceSegmentBkingCode;
      if (!BookingCodeUtil::validateExcludedBookingCodes(trx, 
                                      BookingCodeVec, 
                                      segStatVec[airIndex], 
                                      TempPrimeBookingCodeVec, 
                                      _diag))
      {
        // cleanup any residue
        BookingCodeVec.clear();
        TempPrimeBookingCodeVec.clear();
        BookingCodeVec.push_back(bceSegment.bookingCode2());
        if ((bceSegment.bookingCode2() == "") || (!BookingCodeUtil::validateExcludedBookingCodes(trx, 
                                      BookingCodeVec, 
                                      segStatVec[airIndex], 
                                      TempPrimeBookingCodeVec, 
                                      _diag)))
        {
          segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
          segStatVec[airIndex]._bkgCodeSegStatus = PaxTypeFare::BKSS_NOMATCH;
          return;
        }
        else
        {
          if (TempPrimeBookingCodeVec.size() > 0)
          {
            segStatVec[airIndex]._bkgCodeReBook = TempPrimeBookingCodeVec[0];
            bceSegmentBkingCode = TempPrimeBookingCodeVec[0];
          }
          else
          {
            segStatVec[airIndex]._bkgCodeReBook = bceSegment.bookingCode2();
            bceSegmentBkingCode = bceSegment.bookingCode2();
          }
        }
      }
      else
      {
        if (TempPrimeBookingCodeVec.size() > 0)
        {
          segStatVec[airIndex]._bkgCodeReBook = TempPrimeBookingCodeVec[0];
          bceSegmentBkingCode = TempPrimeBookingCodeVec[0];
        }
        else
        {
          segStatVec[airIndex]._bkgCodeReBook = bceSegment.bookingCode1();
          bceSegmentBkingCode = bceSegment.bookingCode1();
        }
      }
      segStatVec[airIndex]._bkgCodeReBook = bceSegmentBkingCode; 
      segStatVec[airIndex]._reBookCabin = cabinWpncs(trx, airSeg,
                                                     bceSegmentBkingCode,
                                                     paxTypeFare,
                                                     itemNo,
                                                     bceSegment);
      paxTypeFare.s8BFBookingCode() = bceSegmentBkingCode;
    }
  }
  if (!fallback::fallbackPriceByCabinActivation(&trx))
  {
    checkPriceByCabin(trx, segStatVec, airIndex, tvlSeg, paxTypeFare);
  }
  else
    diagBc(tvlSeg, segStatVec[airIndex], FLT_PASS_BCE);
}

//--------------------------------------------------------------------------------
CabinType
BookingCodeExceptionValidator::cabinWpncs(PricingTrx& trx,
                                          const AirSeg& airSeg,
                                          const BookingCode& bc,
                                          const PaxTypeFare& paxTypeFare,
                                          const uint32_t& itemNo,
                                          const BookingCodeExceptionSegment& bceSegment) const
{
  if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    RBDByCabinUtil rbdCabin(trx, T999_VAL);
    return rbdCabin.getCabinByRbdByType(airSeg , bc, paxTypeFare, itemNo, bceSegment);
  }
  else
  {
    CabinType cb;
    cb.setInvalidClass();
    const DateTime& travelDate = trx.adjustedTravelDate(airSeg.departureDT());

    const Cabin* cabin = trx.dataHandle().getCabin(airSeg.carrier(), bc, travelDate);
    if (cabin)
    {
      cb = cabin->cabin();
    }
    else
    {
      if (airSeg.segmentType() == Open)
      {
        // for OPEN segments try to get the cabin value using todays date
        cabin = trx.dataHandle().getCabin(airSeg.carrier(), bc, trx.transactionStartTime());
        if (cabin)
          cb = cabin->cabin();
      }
    }
    return cb;
  }
}

const std::vector<ClassOfService*>*
BookingCodeExceptionValidator::getCosFromFlowCarrierJourneySegment(const TravelSeg* tvlSeg)
{
  const OAndDMarket* od = JourneyUtil::getOAndDMarketFromSegment(tvlSeg, &_itin);
  if (od && (od->isFlowCarrierJourney() || od->isJourneyByMarriage()))
    return od->getCosVector(tvlSeg);

  return nullptr;
}

//--------------------------------------------------------------------------------
const std::vector<ClassOfService*>*
BookingCodeExceptionValidator::getAvailability(PricingTrx& trx,
                                               const TravelSeg* travelSeg,
                                               PaxTypeFare::SegmentStatus& segStat,
                                               PaxTypeFare& paxTypeFare,
                                               uint16_t airIndex)
{
  const std::vector<ClassOfService*>* cosVec = getCosFromFlowCarrierJourneySegment(travelSeg);
  if (UNLIKELY(cosVec))
    return cosVec;

  if (UNLIKELY(_statusType == STATUS_RULE2 || _statusType == STATUS_RULE2_AS_BOOKED))
  {
    if (!travelSeg->carrierPref() || travelSeg->carrierPref()->availabilityApplyrul2st() == YES)
      return nullptr;

    if (const AirSeg* airSeg = travelSeg->toAirSeg())
    {
      if (airSeg->flowJourneyCarrier() || airSeg->localJourneyCarrier())
        return nullptr;
    }
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
    return (&(travelSeg->classOfService()));
  }

  if (_statusType == STATUS_RULE1 || _statusType == STATUS_RULE1_AS_BOOKED)
  {
    if (flowJourneyCarrier(trx, travelSeg) && !(paxTypeFare.fareMarket()->flowMarket()))
    {
      return (flowMarketAvail(trx, travelSeg, paxTypeFare.fareMarket(), airIndex));
    }
    else if (localJourneyCarrier(trx, travelSeg))
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
      return (&(travelSeg->classOfService()));
    }
  }
  else if (LIKELY(_statusType == STATUS_JORNY || _statusType == STATUS_JORNY_AS_BOOKED))
  {
    if (localJourneyCarrier(trx, travelSeg))
    {
      const std::vector<ClassOfService*>* cosVecFlow =
          flowMarketAvail(trx, travelSeg, paxTypeFare.fareMarket(), airIndex);
      if (cosVecFlow != nullptr)
        return cosVecFlow;
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
      return (&(travelSeg->classOfService()));
    }
  }
  return nullptr;
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::resetFirstMatchingSeqs(const PaxTypeFare& paxTypeFare)
{
  const auto airSegSize = paxTypeFare.fareMarket()->travelSeg().size();
  _firstMatchingSeqs.assign(airSegSize, BCE_FLT_NOMATCH);
}

//--------------------------------------------------------------------------------
PaxTypeFare::SegmentStatusVec&
BookingCodeExceptionValidator::getSegStatusVec(PaxTypeFare& paxTypeFare)
{
  if (_fu != nullptr)
    return (_fu->segmentStatus());

  switch (_statusType)
  {
  case STATUS_RULE1:
    return paxTypeFare.segmentStatus();
  case STATUS_RULE1_AS_BOOKED:
    return _asBookedStatus;
  case STATUS_RULE2:
    return paxTypeFare.segmentStatusRule2();
  case STATUS_RULE2_AS_BOOKED:
    return _asBookedStatus;
  case STATUS_JORNY:
    return paxTypeFare.segmentStatusRule2();
  case STATUS_JORNY_AS_BOOKED:
    return _asBookedStatus;
  default:
    return _asBookedStatus;
  }

  return _asBookedStatus;
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::validateAsBooked(
    PricingTrx& trx,
    const BookingCodeExceptionSequenceList& bceSequenceList,
    PaxTypeFare& paxTypeFare,
    const AirSeg* pAirSegConv1)
{
  if (!doAsBooked(trx, paxTypeFare, pAirSegConv1))
    return;

  if (!statusToAsBooked())
    return;

  std::vector<int32_t> saveFirstMatchingSeqs = _firstMatchingSeqs;
  const uint32_t airSegSize = paxTypeFare.fareMarket()->travelSeg().size();
  resetFltResult(BCE_ALL_SEQUENCES, airSegSize);
  resetAsBooked(paxTypeFare);
  validateSequence(trx, bceSequenceList, paxTypeFare, pAirSegConv1);
  diagResults(trx, paxTypeFare, pAirSegConv1);
  statusFromAsBooked();
  adjustStatus(trx, paxTypeFare, pAirSegConv1);

  if(paxTypeFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N))
  {
    PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);

    TravelSegVectorCI tvlI = paxTypeFare.fareMarket()->travelSeg().begin();
    const TravelSegVectorCI tvlE = paxTypeFare.fareMarket()->travelSeg().end();
    for(uint16_t i = 0; i < airSegSize && tvlI != tvlE; ++tvlI, ++i)
    {
      if(segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_DUAL_RBD_PASS))
      {
        paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_TAG_N, false);
        break;
      }
    }

    if(!paxTypeFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N))
    {
      tvlI = paxTypeFare.fareMarket()->travelSeg().begin();
      for(uint16_t i = 0; i < airSegSize && tvlI != tvlE; ++tvlI, ++i)
      {
        if(segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_TAG_N))
          segStatusVec[i]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_TAG_N, false);
      }
    }
  }
  _firstMatchingSeqs = saveFirstMatchingSeqs;
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::doAsBooked(PricingTrx& trx,
                                          PaxTypeFare& paxTypeFare,
                                          const AirSeg* pAirSegConv1)
{
  if (!trx.getRequest()->isLowFareRequested())
    return false;

  if (checkBookedClassAvail(trx, nullptr))
    return false;

  if (allSegsStatusQF(trx, paxTypeFare, pAirSegConv1))
    return false;

  const uint16_t airSegSize = paxTypeFare.fareMarket()->travelSeg().size();

  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);

  if (airSegSize != segStatusVec.size())
    return false;

  TravelSegVectorCI tvlI = paxTypeFare.fareMarket()->travelSeg().begin();
  const TravelSegVectorCI tvlE = paxTypeFare.fareMarket()->travelSeg().end();
  bool atleastOneRebookRequired = false;
  uint16_t i = 0;
  const AirSeg* airSeg = nullptr;
  for (; i < airSegSize && tvlI != tvlE; tvlI++, i++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;

    if (pAirSegConv1 != nullptr) // for R6/C1 only do 1 flight
    {
      if (pAirSegConv1 != airSeg)
        continue;
    }

    if (_cat25PrimeSector)
    {
      if (paxTypeFare.fareMarket()->primarySector() == *tvlI)
        continue;
    }

    if (!segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      continue;
    if (!segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      continue;
    if (segStatusVec[i]._bkgCodeReBook.empty())
      continue;

    if (segStatusVec[i]._reBookCabin < airSeg->bookedCabin())
      return false;

    atleastOneRebookRequired = true;
    break;
  }

  if (!atleastOneRebookRequired)
    return false;

  return true;
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::statusToAsBooked()
{
  switch (_statusType)
  {
  case STATUS_RULE1:
  {
    _statusType = STATUS_RULE1_AS_BOOKED;
    break;
  }
  case STATUS_RULE2:
  {
    _statusType = STATUS_RULE2_AS_BOOKED;
    break;
  }
  case STATUS_JORNY:
  {
    _statusType = STATUS_JORNY_AS_BOOKED;
    break;
  }
  default:
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::statusFromAsBooked()
{
  switch (_statusType)
  {
  case STATUS_RULE1_AS_BOOKED:
  {
    _statusType = STATUS_RULE1;
    break;
  }
  case STATUS_RULE2_AS_BOOKED:
  {
    _statusType = STATUS_RULE2;
    break;
  }
  case STATUS_JORNY_AS_BOOKED:
  {
    _statusType = STATUS_JORNY;
    break;
  }
  default:
    _statusType = STATUS_RULE1;
  }

  return true;
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::resetAsBooked(const PaxTypeFare& paxTypeFare)
{
  const uint16_t airSegSize = paxTypeFare.fareMarket()->travelSeg().size();
  if (_asBookedStatus.size() != airSegSize)
    _asBookedStatus.resize(airSegSize);

  for (auto& segStat : _asBookedStatus)
  {
    segStat._bkgCodeSegStatus.setNull();
    segStat._bkgCodeReBook.clear();
  }
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::adjustStatus(PricingTrx& trx,
                                            PaxTypeFare& paxTypeFare,
                                            const AirSeg* pAirSegConv1)
{
  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);
  const uint16_t airSegSize = paxTypeFare.fareMarket()->travelSeg().size();

  if (airSegSize != segStatusVec.size() || airSegSize != _asBookedStatus.size())
    return;

  TravelSegVectorCI tvlI = paxTypeFare.fareMarket()->travelSeg().begin();
  const TravelSegVectorCI tvlE = paxTypeFare.fareMarket()->travelSeg().end();
  uint16_t i = 0;
  for (; i < airSegSize && tvlI != tvlE; ++tvlI, ++i)
  {
    const AirSeg* airSeg(nullptr);
    if ((*tvlI)->isAir())
    {
      airSeg = static_cast<const AirSeg*>(*tvlI);
    }
    else
    {
      continue;
    }
    if (pAirSegConv1 != nullptr) // for R6/C1 only do 1 flight
    {
      if (pAirSegConv1 != airSeg)
        continue;
    }

    if (_cat25PrimeSector)
    {
      if (paxTypeFare.fareMarket()->primarySector() == *tvlI)
        continue;
    }

    if (!segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      continue;
    if (!segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      continue;
    if (segStatusVec[i]._bkgCodeReBook.empty())
      continue;
    if (!_asBookedStatus[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      continue;
    if (_asBookedStatus[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      continue;
    if (!_asBookedStatus[i]._bkgCodeReBook.empty())
      continue;
    if (segStatusVec[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
      _asBookedStatus[i]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK);
    segStatusVec[i] = _asBookedStatus[i];
  }
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::checkBookedClassAvail(PricingTrx& trx,
                                                     const TravelSeg* tvlSeg,
                                                     bool checkRbd1Only) const
{
  if (!trx.getRequest()->isLowFareRequested())
    return false;

  if (UNLIKELY(trx.getRequest()->upSellEntry()))
    return true;

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.billing()->actionCode() != "WPNI.C" &&
      trx.billing()->actionCode() != "WFR.C")
    return true;

  if (PricingTrx::IS_TRX == trx.getTrxType() || PricingTrx::FF_TRX == trx.getTrxType()) // siriwan
  {
    return true;
  }

  if (tvlSeg != nullptr)
  {
    if (tvlSeg->realResStatus() == QF_RES_STATUS)
      return true;
  }

  return (checkRbd1Only && isWaitlist(*tvlSeg));
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::checkBookedClassOffer(PricingTrx& trx, const TravelSeg* tvlSeg) const
{
  if (!trx.getRequest()->isLowFareRequested())
    return false;

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.billing()->actionCode() != "WPNI.C" &&
      trx.billing()->actionCode() != "WFR.C")
    return true;

  if (LIKELY(tvlSeg != nullptr))
  {
    if (UNLIKELY(tvlSeg->realResStatus() == QF_RES_STATUS))
      return true;
  }

  return false;
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::validateWPNCSFromPrimeRBD(
    const BookingCodeExceptionSegment& bceSegment) const
{
  return (bceSegment.restrictionTag() == BCE_PERMITTED_IF_PRIME_NOT_OFFER ||
          bceSegment.restrictionTag() == BCE_PERMITTED_IF_PRIME_NOT_AVAIL ||
          bceSegment.restrictionTag() == BCE_REQUIRED_IF_PRIME_NOT_OFFER ||
          bceSegment.restrictionTag() == BCE_REQUIRED_IF_PRIME_NOT_AVAIL);
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::allSegsStatusQF(PricingTrx& trx,
                                               PaxTypeFare& paxTypeFare,
                                               const AirSeg* pAirSegConv1) const
{
  if (!trx.getRequest()->isLowFareRequested())
    return false;

  if (trx.getTrxType() != PricingTrx::PRICING_TRX)
    return false;

  if (pAirSegConv1 != nullptr)
  {
    if (pAirSegConv1->realResStatus() == QF_RES_STATUS)
      return true;
  }

  auto& travelSegments = paxTypeFare.fareMarket()->travelSeg();
  return std::all_of(travelSegments.cbegin(),
                     travelSegments.cend(),
                     [](const auto* tvlSeg)
                     { return !tvlSeg->isAir() || tvlSeg->realResStatus() == QF_RES_STATUS; });
}

//--------------------------------------------------------------------------------
bool
BookingCodeExceptionValidator::skipCat31Flown(PricingTrx& trx, const TravelSeg* tvlSeg) const
{
  if (!RexBaseTrx::isRexTrxAndNewItin(trx))
    return false;

  if (!_skipFlownSegCat31)
    return false;

  const AirSeg* airSeg = tvlSeg->toAirSeg();
  return airSeg && !airSeg->unflown();
}

//--------------------------------------------------------------------------------
void
BookingCodeExceptionValidator::validateCabinForDifferential(PricingTrx& trx,
                                                            const TravelSeg* tvlSeg,
                                                            PaxTypeFare& paxTypeFare) const
{
  if (tvlSeg->bookedCabin() > paxTypeFare.cabin())
  {
    paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_REQ_LOWER_CABIN);
  }
}

void
BookingCodeExceptionValidator::applyBookingCodeForSingleFlight(
    PricingTrx& trx,
    const BookingCodeExceptionSequence& bceSequence,
    PaxTypeFare& paxTypeFare,
    TravelSeg* tvlSeg,
    uint16_t airIndex,
    int16_t iFlt)
{
  if (UNLIKELY(iFlt != -1 && airIndex != iFlt))
    return;

  if (UNLIKELY(tvlSeg == nullptr))
    return;

  const AirSeg* pAirSeg = tvlSeg->toAirSeg();
  if (UNLIKELY(pAirSeg == nullptr))
    return;

  const uint32_t airSize = paxTypeFare.fareMarket()->travelSeg().size();
  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);
  if (UNLIKELY(segStatusVec.size() != airSize ||
      paxTypeFare.fareMarket()->classOfServiceVec().size() != airSize))
  {
    LOG4CXX_DEBUG(logger, "ERROR IN APPLYBOOKINGCODE FUNCTION-TRAVELSEG VECTOR SIZE INCORRECT");
    return;
  }

  if (UNLIKELY(airIndex >= segStatusVec.size() || airIndex >= _fltResultVector.size()))
    return;

  if (_fltResultVector[airIndex].first == BCE_FLT_NOMATCH ||
      segStatusVec[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS) ||
      segStatusVec[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
    return;

  const BookingCodeExceptionSegment* pSegment =
      getSegment(bceSequence, _fltResultVector[airIndex].second);

  if (UNLIKELY(pSegment == nullptr))
  {
    resetFltResult(bceSequence.seqNo(), paxTypeFare.fareMarket()->travelSeg().size());
    return;
  }

  diagApplyBc(trx, bceSequence, *pSegment);
  processRestrictionTag(trx, bceSequence.seqNo(),
                        *pSegment, tvlSeg, segStatusVec, airIndex, paxTypeFare, iFlt);
}

bool
BookingCodeExceptionValidator::isSegmentNoMatched(int16_t segMatch[], uint16_t segSize) const
{
  for (uint16_t i = 0; i < segSize; ++i)
    if (segMatch[i] == BCE_FLT_NOMATCH || segMatch[i] == BCE_FLT_SKIPPED)
      return true;
  return false;
}

bool
BookingCodeExceptionValidator::isAllFlightsMatched(int16_t fltMatch[], uint16_t fltSize) const
{
  bool allFltMatched = false;
  for (uint16_t i = 0; i < fltSize; ++i)
  {
    if (fltMatch[i] == BCE_SEC_ARNK)
      continue;

    if (fltMatch[i] >= 0)
      allFltMatched = true;
    else
    {
      allFltMatched = false;
      break;
    }
  }
  return allFltMatched;
}

bool
BookingCodeExceptionValidator::isAllFlightsSkipped(int16_t fltMatch[], uint16_t fltSize) const
{
  bool allFltSkipped = false;
  for (uint16_t i = 0; i < fltSize; ++i)
  {
    if (fltMatch[i] == BCE_SEC_ARNK)
      continue;

    if (fltMatch[i] == BCE_FLT_SKIPPED)
      allFltSkipped = true;
    else
    {
      allFltSkipped = false;
      break;
    }
  }
  return allFltSkipped;
}

void
BookingCodeExceptionValidator::saveSegStatusResults(PaxTypeFare& paxTypeFare,
                                                    PaxTypeFare::SegmentStatusVec& prevSegStatusVec)
{
  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);
  for (const auto& segStatus : segStatusVec)
    prevSegStatusVec.push_back(segStatus);
}

void
BookingCodeExceptionValidator::restoreSegStatusResults(
    PaxTypeFare& paxTypeFare, PaxTypeFare::SegmentStatusVec& prevSegStatusVec)
{
  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(paxTypeFare);
  for (size_t i = 0; i < prevSegStatusVec.size() && i < segStatusVec.size(); ++i)
    segStatusVec[i] = prevSegStatusVec[i];
}

void
BookingCodeExceptionValidator::setFlagForBcvStatus(PaxTypeFare::BkgCodeSegStatus& segStatus,
                                                   BookingCodeValidationStatus validationStatus)
{
  if (validationStatus == BOOKING_CODE_NOT_OFFERED)
  {
    segStatus.set(PaxTypeFare::BKSS_FAIL_OFFER);
  }
  else if (validationStatus == BOOKING_CODE_NOT_AVAILABLE)
  {
    segStatus.set(PaxTypeFare::BKSS_FAIL_AVAILABILITY);
  }
  else if (validationStatus ==BOOKING_CODE_CABIN_NOT_MATCH)
  {
    segStatus.set(PaxTypeFare::BKSS_FAIL_CABIN);
  }
}

void
BookingCodeExceptionValidator::BcValidation(PricingTrx& trx,
                                               const BookingCodeExceptionSegment& bceSegment,
                                               const TravelSeg* tvlSeg,
                                               PaxTypeFare::SegmentStatusVec& segStatVec,
                                               uint16_t airIndex,
                                               PaxTypeFare& paxTypeFare,
                                               BCEDiagMsgs diagMsg,
                                               bool rbd2Only,
                                               BookingCodeValidationStatus forceBcvs)
{
  const BookingCodeValidationStatus vStat =
      validateBC_NEW(trx, bceSegment, tvlSeg, segStatVec, airIndex, paxTypeFare, rbd2Only);
  if (vStat == BOOKING_CODE_PASSED)
  {
    if(rbd2Only)
    {
       segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_DUAL_RBD_PASS, true);
       segStatVec[airIndex]._dualRbd1 = bceSegment.bookingCode1();
    }
    diagBc(tvlSeg, segStatVec[airIndex], FLT_PASS_BCE);
    return;
  }
  BookingCodeValidationStatus toPass = vStat;
  if (forceBcvs != BOOKING_CODE_STATUS_NOT_SET)
  {
    toPass = forceBcvs;
  }

  if (diagMsg == FLT_NOMATCH_BCE)
  {
    tagFltNOMATCH_NEW(segStatVec, airIndex, toPass);
  }
  else if (LIKELY(diagMsg == FLT_FAIL_BCE))
  {
    tagFltFAIL(segStatVec, airIndex, toPass);
  }

  diagBc(tvlSeg, segStatVec[airIndex], diagMsg);
}

bool
BookingCodeExceptionValidator::isRtwPreferredCabinApplicable(PricingTrx& trx,
                                                             const PaxTypeFare& ptf) const
{
  if (isAsBookedStatus())
    return false;

  if (LIKELY(!RtwUtil::isRtw(trx)))
    return false;

  // Apply fix to WPNC only
  if (trx.altTrxType() != PricingTrx::WP || !trx.getRequest()->isLowFareRequested())
    return false;

  return ptf.cabin().isEconomyClass() || ptf.cabin().isBusinessClass() ||
         ptf.cabin().isFirstClass();
}

void
BookingCodeExceptionValidator::initRtwPreferredCabin(PricingTrx& trx, const PaxTypeFare& ptf)
{
  if (UNLIKELY(isRtwPreferredCabinApplicable(trx, ptf)))
    _rtwPreferredCabin.assign(ptf.fareMarket()->travelSeg().size(), CabinType());
  else
    _rtwPreferredCabin.clear();
}

void
BookingCodeExceptionValidator::prepareSequenceValidationRtw(RtwPreferredCabinContext& context,
                                                            PaxTypeFare& ptf)
{
  if (LIKELY(_rtwPreferredCabin.empty()))
    return;

  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(ptf);
  context.prevFltResults = _fltResultVector;
  context.prevSegStats = segStatusVec;
  context.prevRtwPC = _rtwPreferredCabin;

  for (size_t i = 0; i < _rtwPreferredCabin.size(); ++i)
  {
    if (_rtwPreferredCabin[i].isUndefinedClass())
      continue;
    _fltResultVector[i] = TravelSegResult(BCE_FLT_NOMATCH, BCE_FLT_NOMATCH);
    PaxTypeFare::SegmentStatus& segStat = segStatusVec[i];
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, false);
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, false);
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
    segStat._bkgCodeReBook.clear();
    segStat._reBookCabin.setUndefinedClass();
  }
}

void
BookingCodeExceptionValidator::checkSequenceValidationRtw(
    const RtwPreferredCabinContext& context,
    const BookingCodeExceptionSequence& bceSeq,
    PaxTypeFare& ptf)
{
  if (LIKELY(_rtwPreferredCabin.empty()))
    return;

  PaxTypeFare::SegmentStatusVec& segStatusVec = getSegStatusVec(ptf);

  for (size_t i = 0; i < context.prevRtwPC.size(); ++i)
  {
    const CabinType& prefCabin = context.prevRtwPC[i];
    if (prefCabin.isUndefinedClass())
      continue;

    const TravelSeg& ts = *ptf.fareMarket()->travelSeg()[i];
    PaxTypeFare::SegmentStatus& segStat = segStatusVec[i];
    PaxTypeFare::BkgCodeSegStatus& bkcStat = segStat._bkgCodeSegStatus;

    const bool isPreferredFound = getPassedCabin(segStat, ts) == prefCabin;
    const bool isNoMatch = !bkcStat.isSet(PaxTypeFare::BKSS_PASS) &&
                           !bkcStat.isSet(PaxTypeFare::BKSS_FAIL);
    const bool isContinue =
        isNoMatch ||
        shouldEnterRtwPrefCabin(getSegment(bceSeq, _fltResultVector[i].second), segStat, prefCabin);
    const bool isFail = !isPreferredFound && !isContinue;

    if (isFail || isPreferredFound)
      _rtwPreferredCabin[i].setUndefinedClass();

    if (!isPreferredFound)
    {
      _fltResultVector[i] = context.prevFltResults[i];
      segStat = context.prevSegStats[i];
    }
  }
}

void
BookingCodeExceptionValidator::updateRtwPreferredCabin(
    const BookingCodeExceptionSegment& matchedBceSegment, PaxTypeFare& ptf, uint32_t fltIndex)
{
  if (LIKELY(_rtwPreferredCabin.empty() || !_rtwPreferredCabin[fltIndex].isUndefinedClass()))
    return;

  const PaxTypeFare::SegmentStatusVec& stats = getSegStatusVec(ptf);
  const TravelSeg& ts = *ptf.fareMarket()->travelSeg()[fltIndex];
  const PaxTypeFare::SegmentStatus& segStat = stats[fltIndex];

  if (ts.bookedCabin() != ptf.cabin())
    return;

  if (shouldEnterRtwPrefCabin(&matchedBceSegment, segStat, ts.bookedCabin()))
    _rtwPreferredCabin[fltIndex] = ts.bookedCabin();
}

bool
BookingCodeExceptionValidator::shouldEnterRtwPrefCabin(
    const BookingCodeExceptionSegment* matchedBceSegment,
    const PaxTypeFare::SegmentStatus& segStat,
    CabinType preferredCabin) const
{
  return matchedBceSegment && matchedBceSegment->restrictionTag() == BCE_PERMITTED &&
         segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS) &&
         segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
         segStat._reBookCabin < preferredCabin;
}

CabinType
BookingCodeExceptionValidator::getPassedCabin(const PaxTypeFare::SegmentStatus& segStat,
                                              const TravelSeg& ts) const
{
  if (!segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
    return CabinType();

  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    return segStat._reBookCabin;

  return ts.bookedCabin();
}

void
BookingCodeExceptionValidator::checkPriceByCabin(PricingTrx& trx,
                                                 PaxTypeFare::SegmentStatusVec& segStatVec,
                                                 uint16_t airIndex,
                                                 const TravelSeg* tvlSeg,
                                                 PaxTypeFare& paxTypeFare)
{
  if (!trx.getRequest()->isjumpUpCabinAllowed() &&
      !segStatVec[airIndex]._reBookCabin.isUndefinedClass())
  {
    if (segStatVec[airIndex]._reBookCabin != tvlSeg->bookedCabin())
    {
      segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
      segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, false);
      tagFltFAIL(segStatVec, airIndex, BOOKING_CODE_NOT_OFFERED);
      segStatVec[airIndex]._bkgCodeReBook.clear();
      segStatVec[airIndex]._reBookCabin.setUndefinedClass();
      paxTypeFare.s8BFBookingCode().clear();
      diagBc(tvlSeg, segStatVec[airIndex], FLT_FAIL_CABIN);
    }
    else
      diagBc(tvlSeg, segStatVec[airIndex], FLT_PASS_BCE);
  }
}
void
BookingCodeExceptionValidator::validateRBDRec1PriceByCabin(PricingTrx& trx,
                                                           const TravelSeg* tvlSeg,
                                                           uint16_t airIndex,
                                                           std::vector<BookingCode>& primeBookingCodeVec,
                                                           PaxTypeFare& paxTypeFare,
                                                           PaxTypeFare::SegmentStatusVec& segStatVec)
{
  if (primeBookingCodeVec.size() > 1)
  {
    AirSeg& airSeg = dynamic_cast<AirSeg&>(const_cast<TravelSeg&>(*tvlSeg));
    std::vector<ClassOfService*>* cosTempVec = nullptr;
    DataHandle& dataHandle = trx.dataHandle();
    dataHandle.get(cosTempVec);
    RBDByCabinUtil rbdCabin(trx, RBD_VAL);
    rbdCabin.getCabinsByRbd(airSeg, primeBookingCodeVec, cosTempVec);
    for (auto cosv : *cosTempVec)
    {
      ClassOfService& cos = *cosv;
      if (tvlSeg->bookedCabin() != cos.cabin())
      {
        continue;
      }
      else
      {
        segStatVec[airIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
        segStatVec[airIndex]._bkgCodeReBook = cos.bookingCode();
        segStatVec[airIndex]._reBookCabin = tvlSeg->bookedCabin();
        paxTypeFare.s8BFBookingCode() = cos.bookingCode();
        diagBc(tvlSeg, segStatVec[airIndex], FLT_PASS_BCE);
        return;
      }
    }
  }
  tagFltFAIL(segStatVec, airIndex, BOOKING_CODE_NOT_OFFERED);
  segStatVec[airIndex]._bkgCodeReBook.clear();
  segStatVec[airIndex]._reBookCabin.setUndefinedClass();
  paxTypeFare.s8BFBookingCode().clear();
  diagBc(tvlSeg, segStatVec[airIndex], FLT_FAIL_CABIN); // CABIN NOT MATCH
  return ;
}

}
