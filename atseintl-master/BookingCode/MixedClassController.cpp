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
#include "BookingCode/MixedClassController.h"

#include "BookingCode/DifferentialValidator.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"
#include "Util/BranchPrediction.h"

#include <iostream>
#include <vector>

// all typdefs should be moved to corresponding headers

namespace tse
{

static Logger
logger("atseintl.BookingCode.MixedClassController");

bool
MixedClassController::validate(FarePath& fp, // Needed to retrieve the requested PaxType
                               PricingUnit& pu)
{
  LOG4CXX_DEBUG(logger, "Entered MixedClassController::validate(FarePath, PricingUnit)");

  return validate(fp, pu, *fp.itin());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  The Mixed Class validation process invokes by this method from FVO.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
MixedClassController::validate(FareMarket& mkt, Itin& itin)
{
  PaxTypeBucketVecI paxTypeCortegeVecIter;
  PaxTypeBucketVecI paxTypeCortegeVecIterEnd;

  _farePath = nullptr;
  _fareUsage = nullptr;
  _pricingUnit = nullptr;

  if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
  {
    return; // mixed class processing for the International only
  }

  _mkt = &mkt;
  _itin = &itin;

  FareBookingCodeValidator fbcv(_mccTrx, mkt, _itin); // create an object
  _fbcv = &fbcv;

  // Orginize the BIG loop for all PaxTypeFareCorteges on the Fare Market

  PaxTypeBucketVec& paxTypeCortegeVec = mkt.paxTypeCortege();

  paxTypeCortegeVecIter = mkt.paxTypeCortege().begin();
  paxTypeCortegeVecIterEnd = mkt.paxTypeCortege().end();

  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];
    PaxTypeFarePtrVec paxTypeFareVec = paxTypeCortege.paxTypeFare();

    _paxType = paxTypeCortege.requestedPaxType(); // need to pass to diff & he process

    unsigned int paxVecSize = paxTypeFareVec.size();

    // Orginize the loop thru reqPaxType Cortege PaxTypeFare on the Fare Market

    for (unsigned int j = 0; j < paxVecSize; j++)
    {
      PaxTypeFare* paxTFare = paxTypeFareVec[j];

      _paxTfare = paxTFare;

      if (paxTFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS) ||
          paxTFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        continue;
      }
      if(serviceDetermination(_mkt, _paxTfare) == HARD_FAIL)
      {
        paxTFare->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, true);
      }
      if (paxTFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED) &&
          paxTFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
      {
        continue;
      }
      processSingleEntity();
    }
  }
}

bool
MixedClassController::validate(FarePath& fp, // Needed to retrieve the requested PaxType
                               PricingUnit& pu,
                               Itin& itin)
{
  LOG4CXX_DEBUG(logger, "Entered MixedClassController::validate(FarePath, PricingUnit, Itin)");
  _farePath = &fp;
  _itin = &itin;
  _pricingUnit = &pu;
  createDiag();

  FareMarket* fmU = pu.fareUsage().front()->paxTypeFare()->fareMarket();
  FareBookingCodeValidator fbcv(_mccTrx, *fmU, _itin); // create an FBCV object
  _fbcv = &fbcv;

  _paxType = fp.paxType();

  for (auto fu : pu.fareUsage())
  {
    _failMixed = FAIL_NONE;

    PaxTypeFare* ptf = fu->paxTypeFare();

    if (UNLIKELY(_mccTrx.getRequest()->originBasedRTPricing() && ptf->isDummyFare()))
      continue;

    _fareUsage = fu;
    _paxTfare = ptf;
    _mkt = ptf->fareMarket();

    diagFm();
    if (_mccTrx.getTrxType() != PricingTrx::MIP_TRX || !_mkt->getMergedAvailability())
    {
      ProcessMixReturnType returnType = shouldProcessMixClass();
      if (returnType == FAIL_FARE_PATH)
        return false;
      if (returnType == CHECK_NEXT_FU)
        continue;
    }
    // FareUsage has MIXED status
    if (!processSingleEntity())
    {
      diag(PRIME_RBD_PASS2);
      continue;
    }
    else
    {
      if (fu->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        // Fail Fare Usage
        diagFuFail(*fu);
        if (validatingCat31KeepFare())
        {
          _fareUsage->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE, true);
          _farePath->bookingCodeFailButSoftPassForKeepFare() = true;
          diag(CAT31_KEEP_IGNORE_FAIL);
          continue;
        }
        LOG4CXX_DEBUG(logger, "Leaving MixedClassController::validate(): failure");
        endDiag();
        return false;
      }
      // PASS Fare Usage
      fuPass();
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving MixedClassController::validate(): success");
  diag(MIX_PASS);
  endDiag();
  return true;
}

//--------------------------------------------------------------------------------------
MixedClassController::ProcessMixReturnType
MixedClassController::shouldProcessMixClass()
{
  if (UNLIKELY(!allValidCabins()))
  {
    diag(INVALID_CABIN);
    LOG4CXX_DEBUG(logger, "MixedClassController::shouldProcessMixClass(): invalid cabins");
    endDiag();
    return FAIL_FARE_PATH;
  }

  if (allSegsPassBookingCode())
  {
    diag(PRIME_RBD_PASS);
    return CHECK_NEXT_FU;
  }

  if (validatingCat31KeepFare())
    return shouldProcessMixClassCat31();

  if (_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
  {
    diag(DOMESTIC_FM);
    return CHECK_NEXT_FU;
  }

  // FAILED prime booking code or in previous Mix class validation
  if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
  {
    diag(PRIME_RBD_FAIL);
    LOG4CXX_DEBUG(logger, "MixedClassController::shouldProcessMixClass(): fare already failed");
    endDiag();
    return FAIL_FARE_PATH;
  }

  if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
  {
    diag(PRIME_RBD_PASS);
    return CHECK_NEXT_FU;
  }

  if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
  {
    if (_fbcv->validate(*_fareUsage) && _fareUsage->isBookingCodePass())
    {
      diag(PRIME_RBD_PASS2);
      return CHECK_NEXT_FU;
    }
    diag(PRIME_RBD_FAIL1);
    LOG4CXX_DEBUG(logger, "MixedClassController::shouldProcessMixClass(): fare failed");
    endDiag();
    return FAIL_FARE_PATH;
  }
  return PROCESS_MIX_CLASS;
}

//--------------------------------------------------------------------------------------
MixedClassController::ProcessMixReturnType
MixedClassController::shouldProcessMixClassCat31() const
{
  if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
  {
    diag(CAT31_PRIME_RBD_PU);
    _fbcv->validate(*_fareUsage);
  }
  else
  {
    diag(CAT31_PRIME_RBD_FC);
  }

  if (_mkt->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
  {
    if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
    {
      _fareUsage->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE, true);
      _farePath->bookingCodeFailButSoftPassForKeepFare() = true;
      diag(CAT31_KEEP_IGNORE_FAIL);
    }
    else
    {
      diag(PRIME_RBD_PASS2);
    }
    return CHECK_NEXT_FU;
  }

  if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
  {
    diag(PRIME_RBD_PASS);
    return CHECK_NEXT_FU;
  }

  if (_fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
  {
    diag(PRIME_RBD_FAIL);
    if (serviceDetermination() != DIFFERENTIAL)
    {
      _fareUsage->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE, true);
      _farePath->bookingCodeFailButSoftPassForKeepFare() = true;
      diag(CAT31_KEEP_IGNORE_FAIL);
      return CHECK_NEXT_FU;
    }
  }
  return PROCESS_MIX_CLASS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// serviceDetermination - analize the RBD validation status of the booking codes in the travel
// segments.
// Compare cabin  of fail booking code vs. fare cabin. Count all different conditions.
// Return the condition corresponding to the service will be called.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MixedClassController::FailedBkgCodeEnum
MixedClassController::serviceDetermination(void) const
{
  FailedBkgCodeEnum returnType = ERROR;
  int commonFail = 0;
  int lowerCabin = 0;
  int higherCabin = 0;
  int sameCabin = 0;

  uint16_t tsSize = _mkt->travelSeg().size();
  uint16_t cosSize = _mkt->classOfServiceVec().size();
  uint16_t segStSize = _paxTfare->segmentStatus().size();

  // ERROR - Business logic error
  if (UNLIKELY(tsSize != cosSize || cosSize != segStSize || segStSize != tsSize))
  {
    LOG4CXX_ERROR(logger,
                  "Wrong size, mkt seg:" << tsSize << " cos:" << cosSize << " ss:" << segStSize);
    LOG4CXX_ERROR(logger, "Leaving MixedClassController::serviceDetermination()");
    return returnType;
  }
  bool wpnc = _mccTrx.getRequest()->isLowFareRequested();
  bool availFound = false;
  BookingCode bkgCode;
  const CabinType& fareCabin = _paxTfare->cabin();
  CabinType segmentCabin = _paxTfare->cabin();
  TravelSegPtrVecIC iterTvl = _mkt->travelSeg().begin();
  SegmentStatusVecCI iterSegStat = _paxTfare->segmentStatus().begin();

  // ClassOfService vector of ClassOfServices vectors for each TravelSeg in the FareMarket
  COSPtrVecIC iterCOS = _mkt->classOfServiceVec().begin();

  for (; iterTvl != _mkt->travelSeg().end(); iterTvl++, iterSegStat++, iterCOS++)
  {
    availFound = false; // no availability found
    bkgCode = (**iterTvl).getBookingCode(); // BC in the TravelSeg
    segmentCabin = (**iterTvl).bookedCabin(); // cabin for the BC in travelSeg
    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;

    if (wpnc && !segStat._bkgCodeReBook.empty() &&
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      bkgCode = segStat._bkgCodeReBook;
    }

    if (!(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_SURFACE)))
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
        // ClassOfService vector for the particular Travel Segment
        COSInnerPtrVecIC classOfS = (*iterCOS)->begin();

        for (; classOfS != (*iterCOS)->end(); classOfS++)
        {
          ClassOfService& cOs = **classOfS;

          //@TODO need to change the retrieving the only first char in the BookingCode
          // Because it is incorrect

          if (cOs.bookingCode().find(bkgCode[0], 0) == 0)
          {
            availFound = true; // availability found
            if (fareCabin == cOs.cabin())
            {
              ++sameCabin;
            }
            else if (fareCabin < cOs.cabin())
            {
              ++lowerCabin;
            }
            else if (fareCabin > cOs.cabin())
            {
              ++higherCabin;
              collectHighCabin(cOs.cabin());
            }
          } // if found Booking Code in the Class Of Service
        } // loop for Class of service objects for the particular Travel Segment
        if (!availFound)
        {
          if (fareCabin == segmentCabin)
          {
            ++sameCabin;
          }
          else if (fareCabin < segmentCabin)
          {
            ++lowerCabin;
          }
          else if (fareCabin > segmentCabin)
          {
            ++higherCabin;
            collectHighCabin(segmentCabin);
          }
        }
      } // if FAIL segment status is Found
    } // if SURFACE segment status
  } // loop for all Travel Segment

  if (_paxTfare->isNormal()) // Normal fare processing
  {
    if (commonFail == higherCabin)
    {
      returnType = DIFFERENTIAL;
    }
    else if (commonFail == lowerCabin)
    {
    }
    else if (commonFail != 0 && commonFail == sameCabin)
    {
      if (LIKELY(wpnc))
        returnType = DIFFERENTIAL;
      else
        returnType = HARD_FAIL;
    }
    else if ((sameCabin > 0) && (higherCabin > 0) && (lowerCabin == 0))
    {
      returnType = DIFFERENTIAL;
    }
  }
  else // Special fare processing
      if (commonFail != 0 && lowerCabin == 0) // RB Driver v3.4 with the word SAME added in 1.2.2
  {
    returnType = HARD_FAIL;
  }
  else
  {
    returnType = ERROR;
  }
  return returnType;
}

bool
MixedClassController::processSingleEntity()
{
  PaxTypeFare::BookingCodeStatus& bks = bookingCodeStatus();

  if (!_paxTfare->isValidNoBookingCode())
    return false;

  if (LIKELY(!validatingCat31KeepFare()))
  {
    if (bks.isSet(PaxTypeFare::BKS_PASS) || bks.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) ||
        bks.isSet(PaxTypeFare::BKS_FAIL))
      return false;

    if (!(bks.isSet(PaxTypeFare::BKS_MIXED)))
      return false;
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // This is the major loop for all PaxTypeFares in the FareMarket
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (UNLIKELY(!allValidCabins()))
  {
    bks.set(PaxTypeFare::BKS_FAIL);
    return false;
  }

  bool rv = false;
  if (_paxTfare->isNormal()) // Does Fare Normal ?
  {
    FailedBkgCodeEnum goWhere = serviceDetermination(); // WP && WPNC

    switch (goWhere)
    {
    case DIFFERENTIAL:
    {
      if (_mkt->travelSeg().size() == 1) // Only one Travel Segment on the FareMarket
      {
        bks.set(PaxTypeFare::BKS_FAIL);
        bks.set(PaxTypeFare::BKS_PASS, false);
        bks.set(PaxTypeFare::BKS_MIXED, false);
        _failMixed = FAIL_DIFF;
        break;
      }

      if (_mccTrx.getOptions()->isRtw() ||
          _paxTfare->isFareByRule())
      {
        bks.set(PaxTypeFare::BKS_FAIL);
        bks.set(PaxTypeFare::BKS_MIXED, false);
        _failMixed = FAIL_DIFF;
        break;
      }

      // Failed RBD validated for Cat 23 check.
      // Analyze failed RBD cabin to determine should it go directly to Differential
      // or, Carrier Preference table should be called to check permission for
      // differential calculation between Premium Business and Business, or Premium
      // Economy and Economy

      if (_fareUsage && !validForDiffCalculation())
        break;

      // Diffrerentials should be done in "Pricing" only
      if (_fareUsage != nullptr)
      {
        DifferentialValidator diffV;
        rv = diffV.validate(
            _mccTrx, *_farePath, *_pricingUnit, *_fareUsage, *_itin, *_paxType, *_fbcv);
        if (rv)
        {
          bks.set(PaxTypeFare::BKS_PASS);
        }
        else
        {
          bks.set(PaxTypeFare::BKS_FAIL);
          _failMixed = FAIL_DIFF;
        }
        bks.set(PaxTypeFare::BKS_MIXED, false);
      }
    }
    break;

    default:
      bks.set(PaxTypeFare::BKS_PASS, false);
      bks.set(PaxTypeFare::BKS_MIXED, false);
      bks.set(PaxTypeFare::BKS_FAIL);
    }
  }
  else // Fare is Special
  {
    bks.set(PaxTypeFare::BKS_FAIL);
    bks.set(PaxTypeFare::BKS_PASS, false);
    bks.set(PaxTypeFare::BKS_MIXED, false);
  }
  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//

PaxTypeFare::BookingCodeStatus&
MixedClassController::bookingCodeStatus() const
{
  return (!_fareUsage) ? _paxTfare->bookingCodeStatus() : _fareUsage->bookingCodeStatus();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Validate all segment's statuses in the FareMarket and set up the status for the
// PaxTypeFare itself
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
MixedClassController::diagFuFail(FareUsage& fu) const
{
  if (_diag == nullptr)
    return;

  DiagCollector& diag1 = *_diag;
  if (_failMixed == FAIL_DIFF)
  {
    if (fu.differSeqNumber() == 23)
      diag1 << "    - CAT23 - DIFF NOT PERMITTED \n";
    else if (fu.differSeqNumber() == 4)
      diag1 << "    - PREM BUSINESS AND BUSINESS - DIFF NOT PERMITTED \n";
    else if (fu.differSeqNumber() == 7)
      diag1 << "    - PREM ECONOMY AND ECONOMY - DIFF NOT PERMITTED \n";
    else
      diag1 << "    - FAIL DIFFERENTIAL \n";
  }
  else
    diag1 << "    - FAIL - SPEC FARE MUST BE IN HIGH CABIN IN ALL SEGM \n";
}

//-------------------------------------------------------------------
void
MixedClassController::fuPass()
{
  if (_fareUsage->differentialAmt())
  {
    _farePath->plusUpFlag() = true;
    _farePath->plusUpAmount() += _fareUsage->differentialAmt();
    _farePath->increaseTotalNUCAmount(_fareUsage->differentialAmt());
  }

  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag1 = *_diag;

  diag1 << "    - PASS    ";
  if (_fareUsage->differentialAmt())
    diag1 << "DIFF AMT - " << _fareUsage->differentialAmt() << "   ";

  diag1 << " \n";
}

//-------------------------------------------------------------------
bool
MixedClassController::allValidCabins() const
{
  if (UNLIKELY(_mkt == nullptr))
    return true;

  return std::all_of(_mkt->travelSeg().cbegin(),
                     _mkt->travelSeg().cend(),
                     [](const auto* tvlSeg)
                     {
    const auto* airSeg = tvlSeg->toAirSeg();
    return (
        LIKELY(!airSeg || airSeg->segmentType() == Open || airSeg->bookedCabin().isValidCabin()));
  });
}

//-------------------------------------------------------------------
void
MixedClassController::createDiag()
{
  DiagnosticTypes diagType = _mccTrx.diagnostic().diagnosticType();
  if (LIKELY(!(_mccTrx.diagnostic().isActive()) || diagType != Diagnostic420))
  {
    _diag = nullptr;
    return;
  }

  bool diagNeeded = false;
  bool slashParam = false;

  DiagParamMapVecI beginD = _mccTrx.diagnostic().diagParamMap().begin();
  DiagParamMapVecI endD = _mccTrx.diagnostic().diagParamMap().end();
  size_t len = 0;

  for (const auto* fareUsage : _pricingUnit->fareUsage())
  {
    beginD = _mccTrx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
    std::string specifiedFM("");
    std::string compFM("");
    // procees /FMDFWLON
    if (beginD != endD)
    {
      slashParam = true;
      specifiedFM = (*beginD).second;
      if (!(specifiedFM.empty()))
      {
        compFM = fareUsage->paxTypeFare()->fareMarket()->boardMultiCity() +
                 fareUsage->paxTypeFare()->fareMarket()->offMultiCity();
        if (compFM == specifiedFM)
        {
          diagNeeded = true;
        }
        else
        {
          diagNeeded = false;
          continue;
        }
      }
    }

    // process /FCY26
    beginD = _mccTrx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
    if (beginD != endD)
    {
      slashParam = true;
      len = ((*beginD).second).size();
      if (len != 0)
      {
        if (((*beginD).second).substr(0, len) == fareUsage->paxTypeFare()->fareClass())
          diagNeeded = true;
        else
          diagNeeded = false;
      }
    }

    // process /FBY26
    beginD = _mccTrx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);
    if (beginD != endD)
    {
      slashParam = true;
      len = ((*beginD).second).size();
      if (len != 0)
      {
        if (((*beginD).second).substr(0, len) ==
            fareUsage->paxTypeFare()->createFareBasis(_mccTrx, false))
          diagNeeded = true;
        else
          diagNeeded = false;
      }
    }
    if (!slashParam || diagNeeded)
      break;
  }

  if (slashParam)
  {
    if (!diagNeeded)
    {
      _diag = nullptr;
      return;
    }
  }
  _diag = DCFactory::instance()->create(_mccTrx);
  _diag->enable(Diagnostic420);
  DiagCollector& diag1 = *_diag;
  diag1 << " \n";
  diag1 << "******** PRICING UNIT MIXED CLASS VALIDATION DIAG 420 ******** \n";
  FarePath& fp = *(_farePath);
  diag1 << "FARE PATH " << fp << " \n";
  diag1 << "VALIDATING FARE USAGE : \n";
}

//-------------------------------------------------------------------
void
MixedClassController::diagFm() const
{
  if (LIKELY(_diag == nullptr))
    return;

  DiagCollector& diag1 = *_diag;

  diag1 << "  " << std::setw(3) << _mkt->boardMultiCity() << "-" << std::setw(2)
        << _mkt->governingCarrier() << "-" << std::setw(3) << _mkt->offMultiCity() << "  :  "
        << _paxTfare->fareClass();

  if (validatingCat31KeepFare())
  {
    if (_paxTfare->isNormal())
      diag1 << "  NL ";
    else
      diag1 << "  SP ";
    diag1 << "CAB:" << _paxTfare->cabin();
    diag1 << " KEEP FARE ";
  }

  diag1 << " \n";
}

//-------------------------------------------------------------------
void
MixedClassController::diag(MccDiagMsgs diagMsg) const
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag1 = *_diag;

  switch (diagMsg)
  {
  case INVALID_CABIN:
    diag1 << "    - FAIL UNKNOWN CABINS PRESENT IN PNR \n";
    break;
  case DOMESTIC_FM:
    diag1 << "    - DOMESTIC - VALIDATION SKIPPED \n";
    break;
  case PRIME_RBD_FAIL:
    diag1 << "    - ALREADY FAILED BY PRIME RBD OR PREVIOUS MIXED CLASS \n";
    break;
  case PRIME_RBD_PASS:
    diag1 << "    - ALREADY PASSED BY PRIME RBD VALIDATION  \n";
    break;
  case PRIME_RBD_PASS2:
    diag1 << "    - PASSED   - PRIMARY RBD \n";
    break;
  case MIX_PASS:
    diag1 << "\n -- ALL FARES IN PRICING UNIT - PASS MIXED CLASS VALIDATION -- \n \n";
    break;
  case PRIME_RBD_FAIL1:
    diag1 << "    - FAILED BY PRIME RBD VALIDATION  \n";
    break;

  case CAT31_KEEP_IGNORE_FAIL:
    diag1 << "    - CAT31 KEEP FARE IGNORE FAIL \n";
    break;

  case CAT31_KEEP_PROCESSED_EARLIER:
    diag1 << "    - CAT31 KEEP FARE PROCESSED EARLIER \n";
    break;

  case CAT31_PRIME_RBD_PU:
    diag1 << "    - CAT31 PRIME RBD VALIDATED IN PU LEVEL \n";
    break;

  case CAT31_PRIME_RBD_FC:
    diag1 << "    - CAT31 PRIME RBD VALIDATED IN FC LEVEL \n";
    break;

  default:
    diag1 << "     - NO MESSAGE \n";
    break;
  }
}

//-------------------------------------------------------------------
void
MixedClassController::endDiag()
{
  if (LIKELY(_diag == nullptr))
    return;
  *(_diag) << "************************************************************** \n";
  _diag->flushMsg();
  _diag->disable(Diagnostic420);
  _diag = nullptr;
}

//-------------------------------------------------------------------
bool
MixedClassController::validatingCat31KeepFare() const
{
  return (_mccTrx.excTrxType() == PricingTrx::AR_EXC_TRX && _fareUsage != nullptr &&
          _fareUsage->isKeepFare());
}

//-------------------------------------------------------------------
bool
MixedClassController::allSegsPassBookingCode() const
{
  if (_fareUsage == nullptr || _fareUsage->segmentStatus().empty())
    return false;

  return std::all_of(_fareUsage->segmentStatus().cbegin(),
                     _fareUsage->segmentStatus().cend(),
                     [](const auto& segStat)
                     { return segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS); });
}

//-------------------------------------------------------------------
void
MixedClassController::collectHighCabin(CabinType cabin) const
{
  // this method is for Premium Cabin logic only
  if (cabin.isPremiumBusinessClass()) // cabin 4
    _isHighCabin4 = true;
  else if (cabin.isPremiumEconomyClass())
    _isHighCabin7 = true; // cabin 7
}

//-------------------------------------------------------------------
bool
MixedClassController::validForDiffCalculation()
{
  PaxTypeFare::BookingCodeStatus& bks = bookingCodeStatus();

  DifferentialValidator diffV;
  diffV.diffTrx() = &_mccTrx;
  //  Cat 23 check
  if (!diffV.validateMiscFareTag(*_paxTfare, true))
  {
    _fareUsage->differSeqNumber() = 23;
  }
  // Analyze failed RBD cabin to determine should it go directly to Differential
  // or, Carrier Preference table should be called to check permission for
  // differential calculation between Premium Business and Business, or Premium
  // Economy and Economy
  else if ((_paxTfare->cabin().isBusinessClass() && _isHighCabin4) ||
           (_paxTfare->cabin().isEconomyClass() && _isHighCabin7))
  {
    // Read Carrier Preference table to check Diff permission indicators
    // Check Carrier Preference table to see if surface is allowed
    // at fare break point
    const CarrierPreference* cxrPref = getCarrierPreference();
    if (cxrPref)
    {
      if (_paxTfare->cabin().isBusinessClass() && _isHighCabin4 &&
          cxrPref->applyPremBusCabinDiffCalc() != 'Y')
      {
        _fareUsage->differSeqNumber() = 4;
      }
      else if (_paxTfare->cabin().isEconomyClass() && _isHighCabin7 &&
               cxrPref->applyPremEconCabinDiffCalc() != 'Y')
      {
        _fareUsage->differSeqNumber() = 7;
      }
    }
  }
  if (_fareUsage->differSeqNumber())
  {
    bks.set(PaxTypeFare::BKS_FAIL);
    bks.set(PaxTypeFare::BKS_MIXED, false);
    _failMixed = FAIL_DIFF;
    return false;
  }
  return true;
}

const CarrierPreference*
MixedClassController::getCarrierPreference()
{
  return _mccTrx.dataHandle().getCarrierPreference(_mkt->governingCarrier(),
                                                   _mccTrx.ticketingDate());
}

MixedClassController::FailedBkgCodeEnum
MixedClassController::serviceDetermination(FareMarket* fm, PaxTypeFare* ptf)
{
  _mkt = fm;
  _paxTfare = ptf;
  return serviceDetermination();
}

} // namespace tse
