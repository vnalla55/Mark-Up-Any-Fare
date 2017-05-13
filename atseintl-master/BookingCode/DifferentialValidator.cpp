//-------------------------------------------------------------------
//
//  File:        DifferentilaValidator.cpp
//  Created:     May 27, 2004
//  Authors:     Alexander Zagrebin, Sergey Romanchenko
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

#include "BookingCode/DifferentialValidator.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigMan.h"
#include "Common/FareMarketUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Differentials.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MiscFareTag.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/HIPMinimumFare.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <string>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.BookingCode.DifferentialValidator");

void
DifferentialValidator::initialize(DiagCollector* diag,
                                  PricingTrx* trx,
                                  FareMarket* mkt,
                                  PaxTypeFare* paxTfare,
                                  FarePath* fareP,
                                  PricingUnit* pricingU,
                                  FareUsage* fareU,
                                  const Itin* itin,
                                  const PaxType* reqPaxType,
                                  FareBookingCodeValidator* fbcv)
{
  _diag = diag;
  _diffTrx = trx;
  _mkt = mkt;
  _paxTfare = paxTfare;
  _farePath = fareP;
  _pricingUnit = pricingU;
  _fareUsage = fareU;
  _itin = itin;
  _reqPaxType = reqPaxType;
  _fBCValidator = fbcv;
}

//***************************************************************************
//    Guide through Differential validation process and show diagnostic
//***************************************************************************
bool
DifferentialValidator::validate(PricingTrx& trx,
                                FarePath& fareP,
                                PricingUnit& pricingU,
                                FareUsage& fareU,
                                const Itin& itin,
                                const PaxType& reqPaxType,
                                FareBookingCodeValidator& fbcv)
{
  LOG4CXX_DEBUG(logger,
                "Entered DifferentialValidator::validate(PricingTrx, FarePath, "
                "PricingUnit, FareUsage, Itin, PaxType)");
  initialize(nullptr,
             &trx,
             fareU.paxTypeFare()->fareMarket(),
             fareU.paxTypeFare(),
             &fareP,
             &pricingU,
             &fareU,
             &itin,
             &reqPaxType,
             &fbcv);
  fareU.mixClassStatus() = PaxTypeFare::MX_DIFF;
  fareU.differentialPlusUp().clear();
  fareU.differentialAmt() = 0;
  bool rv = false;

  try
  {
    rv = validateDiff();
    if (rv)
      copyDifferentialData();
    std::vector<DifferentialData*>& differV = differential();
    if (differV.empty())
    {
      LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validate(): failure");
      return false;
    }
    bool passed = true;

    rv = rv && passed;
    if (passed)
    {
      processDiag();
    }

    LowerSegmentNumber numberSort(_itin);
    std::stable_sort(differV.begin(), differV.end(), numberSort);
  }
  catch (std::string& s)
  {
    LOG4CXX_ERROR(logger, s);
    LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validate(): failure");
    return false;
  }
  LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validate(): " << (rv ? "true" : "false"));
  return rv;
}

//***************************************************************************
//             Guide through Differential validation process
//***************************************************************************
bool
DifferentialValidator::validateDiff(void)
{
  LOG4CXX_DEBUG(logger, "Entered DifferentialValidator::validateDiff()");

  const Itin& itin = *itinerary();
  Indicator diffNumber = NUMBER; // start number of differential Tag
  uint16_t thruNumber = 1; // start thru number for individual Diff Sector
  Indicator diffLetter = START_LETTER; // start letter of differential Tag
  Indicator prevdiffLetter = BLANK; // store prev letter of differential Tag
  DifferentialData::FareTypeDesignators previousCabin(
      DifferentialData::BLANK_FTD); // default cabin, before process start
  bool firstCabinCompare = true; // check if first cabin comparsion
  TravelSegPtrVecI itTvl = _mkt->travelSeg().begin();
  int travelSegNumber = 0;
  DifferentialData* diffItem = nullptr;

  // Main loop for the all Travel Segment in the Fare Component
  if (_mkt->travelSeg().size() != _fareUsage->segmentStatus().size())
  {
    LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
    return false;
  }

  // adjusting dates for reissue transactions

  if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
    adjustRexPricingDates();

  SegmentStatusVecI iterSegStat = _fareUsage->segmentStatus().begin();

  for (; itTvl != _mkt->travelSeg().end(); itTvl++, iterSegStat++, travelSegNumber++)
  {
    AirSeg* airSegPtr = dynamic_cast<AirSeg*>(*itTvl);
    if (!airSegPtr) // do not do analyze for the non-air segment
    {
      continue;
    }

    // Put travel segment to vector for the getting the FareMarket

    std::vector<TravelSeg*> segOrderVec;

    segOrderVec.push_back(*itTvl);

    AirSeg& airSeg = *airSegPtr;

    // Differential sector determination validation
    DifferentialData::FareTypeDesignators cabin(DifferentialData::BLANK_FTD);
    DifferFailBkgCodeEnum diffCandid = analyzeFailBkgCode(travelSegNumber, cabin);

    if (diffCandid == CABIN_LOWER)
    {
      diffNumber = NUMBER; // next differ sector will start from the beginning
      if (prevdiffLetter != BLANK)
        diffLetter = prevdiffLetter + 1; // next differ item will have the next letter
      continue;
    }
    if (!diffItem)
    {
      diffTrx()->dataHandle().get(diffItem);
    }
    if (diffCandid == CABIN_EQUAL)
    {
      // Cabin of Failed Booking Code is Same as the cabin of Through Fare
      switch (cabin)
      {
      case DifferentialData::PREMIUM_FIRST_FTD: // First premium fare cabin (highest if active)
      case DifferentialData::FIRST_FTD: // First class fare cabin
      case DifferentialData::PREMIUM_BUSINESS_FTD: // Business Premium class fare cabin
      case DifferentialData::PREMIUM_BUSINESS_FTD_NEW: // New Business Premium class fare cabin
      case DifferentialData::BUSINESS_FTD: // Business class fare cabin
      case DifferentialData::PREMIUM_ECONOMY_FTD: // Premium economy
        //+++++++++++++++++++++++++++++++++++++++++
        // Business class fare cabin
        //+++++++++++++++++++++++++++++++++++++++++
        // Check to see if this sector fails in RB validation of all local fares that
        // are applicable to the Business fare cabin booked? 1.2.

        if (!currentSectorLocalFareRBDValidation(travelSegNumber))
        {
          diffNumber = NUMBER; // next differ sector tag will start from the beginning

          if (prevdiffLetter != BLANK)
            diffLetter = prevdiffLetter + 1;
          if (!adjacentSectorDetermination(
                  *diffItem, travelSegNumber, diffNumber, diffLetter, thruNumber))
          {
            return false;
          }
        }
        else // some of the local fares PASS RBD validation for that sector
        {
          //*****************************************************************************
          // Check to see if there is an adjacent failed RB sector that is in the higher
          // fare cabin than the through fare component being validated
          // in the previous & next travel sector    1.2.1
          //*****************************************************************************
          // Check to see if there is an adjacent failed RB sector that
          // is in the higher fare cabin in the previous travel sector

          if (!adjacentSectorDetermination(
                  *diffItem, travelSegNumber, diffNumber, diffLetter, thruNumber))
          {
            if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
              restorePricingDates();

            return false;
          }
        } // else
        break;

      default: //  ECONOMY_FTD  -- ECONOMY   (1.3)
        //++++++++++++++++++++++++++++++++++++++
        // ECONOMY class fare cabin
        //++++++++++++++++++++++++++++++++++++++
        // Check to see if this sector fails in RB validation of all local fares that
        // are applicable to the Economy fare cabin booked? 1.3.

        if (!currentSectorLocalFareRBDValidation(travelSegNumber))
        {
          diffNumber = NUMBER; // next differ sector tag will start from the beginning

          if (prevdiffLetter != BLANK)

            diffLetter = prevdiffLetter + 1;
          if (!adjacentSectorDetermination(
                  *diffItem, travelSegNumber, diffNumber, diffLetter, thruNumber))
          {
            return false;
          }
        }
        else
        {
          //*1.3.1.**********************************************************************
          // Check to see if there is an adjacent failed RB sector that is in the higher
          // fare cabin than the through fare component being validated
          // in the previous & next travel sector    1.3.1
          //*****************************************************************************
          // Check to see if there is an adjacent failed RB sector that
          // is in the higher fare cabin in the previous travel sector
          if (!adjacentSectorDetermination(
                  *diffItem, travelSegNumber, diffNumber, diffLetter, thruNumber))
          {
            if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
              restorePricingDates();

            return false;
          } // if, adjacentSectorDetermination
        } // else, some fare passed the RBD validation for Economy fare
      } // switch
    }
    else if (diffCandid == CABIN_HIGHER)
    {
      // Cabin of Failed Booking Code is Higher than cabin of Through Fare

      moveCarrierAndMarket(diffItem, airSeg, segOrderVec);

      //@TODO moved down      prevdiffLetter =  diffLetter;      // store letter for next iteration
      if (firstCabinCompare)
      {
        previousCabin = cabin; //  assign first processed cabin
        firstCabinCompare = false;
      }
      else if (cabin == previousCabin)
      {
        if (prevdiffLetter == diffLetter) // add on 06/29
          diffNumber++; // increase differential number
      }
      else
      {
        diffNumber++;
        previousCabin = cabin;
      }
      prevdiffLetter = diffLetter; // store letter for next iteration

      // populate differential item by a few data

      diffItem->cabin() = cabin;
      diffItem->tag() += (diffNumber);
      diffItem->tag() += (diffLetter);
      diffItem->tag() += (cabin);
      diffItem->thruNumber() = thruNumber;
      thruNumber++;

      diffItem->setSameCarrier() = true;
      diffItem->stops() = (airSeg.hiddenStops().empty()) ? false : true;
      diffItem->travelSeg().push_back(*(itTvl));
      diffItem->tripType() =
          (_pricingUnit->puType() == PricingUnit::Type::ONEWAY) ? ONE_WAY_TRIP : ROUND_TRIP;
      diffItem->bookingCode() = airSeg.getBookingCode();

      wpncFindFareForDiffSector(*diffItem);

      std::vector<DifferentialData*>& differV = differential();
      differV.push_back(diffItem); // move diff item to the vector in Fare
    }
    else
    {
      continue;
    }
    diffItem = nullptr; // Prepare for the new differential item
  } // For Travel Seg Loop
  {
    std::vector<DifferentialData*>& differV = differential();
    if (differV.empty()) // Have we got any differential items?
    {
      StatusReturnType status = CheckBkgCodeSegmentStatus();
      switch (status)
      {
      case PASS:
        if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
          restorePricingDates();

        LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): success");
        return true;
      case FAIL:
        if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
          restorePricingDates();

        LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
        return false;
      default:
        break;
      }
    } // if (differV().empty ())
    else
    {
      // Continue this process, when the OFF point of the fare component is reached determine,
      // are all DIFF sectors consecutive? 3.1  If yes, are the BOARD point of the first DIFF sector
      // and the OFF point of the last DIFF sector same as the BOARD, OFF point of the through fare
      // component being validated?
      // 3.1.1 If yes, fail the through fare component.  Exit DIFF.
      std::vector<DifferentialData*>::iterator id = differV.begin();
      std::vector<DifferentialData*>::iterator idF = differV.begin();
      DifferentialData* idL = differV.back();

      (*id)->setDiffFMFalse() = false;
      uint16_t diffTlvNumber = 0;

      if ((*id)->travelSeg().empty() || idL->travelSeg().empty() || _mkt->travelSeg().empty())
      {
        LOG4CXX_ERROR(logger,
                      "DifferentialValidator::validateDiff ( void ) - Empty travel segment vector");
      }
      else
      {
        LocCode loc1 = (*id)->travelSeg().front()->boardMultiCity();
        LocCode loc2 = idL->travelSeg().back()->offMultiCity();
        LocCode mktLoc1 = _mkt->travelSeg().front()->boardMultiCity();
        LocCode mktLoc2 = _mkt->travelSeg().back()->offMultiCity();

        if ((loc1 == mktLoc1) && (loc2 == mktLoc2))
        {
          for (; id != differV.end(); id++)
          {
            diffTlvNumber += (*id)->travelSeg().size();
          }
          if (diffTlvNumber == _mkt->travelSeg().size())
          {
            (*idF)->setDiffFMFalse() = true;
          }
          if ((*idF)->isDiffFMFalse())
          {
            if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
              restorePricingDates();

            LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
            return false;
          }
        }
      }
    } // differV() Not empty()
  }

  // Retrieve Differential table  from DB or Cache

  const DateTime& travelDate = _diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX)
                                   ? _travelDate
                                   : (**(itin.travelSeg().begin())).departureDT();

  const std::vector<Differentials*>& differList =
      diffTrx()->dataHandle().getDifferentials(_mkt->governingCarrier(), travelDate);

  if (differList.empty())
  {
    if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
      restorePricingDates();

    throw std::invalid_argument("DIFFERENTIAL TABLE IS EMPTY");
  }

  //***************************************************************************
  //  Differential sectors validation process vs. Differential Table
  //***************************************************************************

  if (!diffSectorValidation(diffItem, differList))
  {
    if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
      restorePricingDates();

    LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
    return false;
  }
  {
    std::vector<DifferentialData*>& differV = differential();
    std::vector<DifferentialData*>::iterator id = differV.begin();
    for (; id != differV.end(); id++)
    {
      (*id)->throughFare() = throughFare();
      if ((*id)->amount() < 0.0)
      {
        (*id)->amount() = 0.0;
      }
    }
  }

  if (!checkDifferentialVector())
  {
    if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
      restorePricingDates();

    LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
    return false;
  }
  validateConsecutiveSectors();

  //***************************************************************************
  // Consolidate Differential sectors
  //***************************************************************************
  if (!consolidate(differList))
  {
    if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
      restorePricingDates();

    LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
    return false;
  }

  {
    if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
      restorePricingDates();

    std::vector<DifferentialData*>& differV = differential();
    if (!differV.empty()) // Have we got any differential items?
    {
      bool halFound;

      halFound = highAndLowFound();

      LOG4CXX_DEBUG(logger,
                    "Leaving DifferentialValidator::validateDiff(): highAndLowFound() - "
                        << (halFound ? "true" : "false"));
      return halFound;
    } // if (!differV().empty ())
    else
    {
      LOG4CXX_DEBUG(logger, "Leaving DifferentialValidator::validateDiff(): failure");
      return false;
    }
  }
}

void
DifferentialValidator::adjustRexPricingDates()
{
  _travelDate = _itin->travelDate();

  _diffTrx->dataHandle().setTicketDate(_paxTfare->retrievalDate());

  if (_travelDate < _paxTfare->retrievalDate())
    _travelDate = _paxTfare->retrievalDate();
}

void
DifferentialValidator::restorePricingDates()
{
  _diffTrx->dataHandle().setTicketDate(_diffTrx->ticketingDate());
}

//**********************************************************************************
// Determine Fail RBD Booking Code in the FareMarket and analyze its cabin
// compare to through fare booking code
//**********************************************************************************
DifferentialValidator::DifferFailBkgCodeEnum
DifferentialValidator::analyzeFailBkgCode(const int tSN,
                                          DifferentialData::FareTypeDesignators& fareTypeDes) const
{
  const PaxTypeFare& paxTfare = *throughFare();
  const FareUsage* fu = fareUsage();
  DifferFailBkgCodeEnum segCabinStatus = CABIN_LOWER;
  fareTypeDes = DifferentialData::BLANK_FTD;
  CabinType segCabin;
  const CabinType& fareCabin = paxTfare.cabin(); // Fares cabin

  uint16_t tsSize = _mkt->travelSeg().size();
  uint16_t cosSize = _mkt->classOfServiceVec().size();
  uint16_t segStSize = fu->segmentStatus().size();
  // ERROR - Business logic error
  if ((tSN >= tsSize) || tsSize != cosSize || cosSize != segStSize || segStSize != tsSize)
  {
    LOG4CXX_ERROR(logger, "Leaving DifferentialValidator::analyzeFailBkgCode(): wrong size");
    return segCabinStatus;
  }

  //  Empty Vector of booking codes
  std::vector<BookingCode> bkgCodes;

  TravelSeg* tvl = _mkt->travelSeg()[tSN];
  ClassOfServicePV vCOS = _mkt->classOfServiceVec()[tSN];
  const PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tSN];
  AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);
  if (airSeg != nullptr)
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
      bkgCodes.clear();
      paxTfare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
      segCabin = airSeg->bookedCabin(); // AirSeg cabin

      if (bkgCodes.empty()) // No book. codes in Rec 1
      {
        return segCabinStatus;
      }

      // Find the Booking code for the travel segment and through fare and
      // their associated cabin number from the Class Of Service vector

      if (diffTrx()->getRequest()->isLowFareRequested() && // Is it a WPNC entry?
          (fareCabin.isEconomyClass()) &&
          (segCabin.isEconomyClass()))
      {
        // need to determine available inventory for the Premium Economy, (Premium)Business &
        // (Premium)First cabin;
        // if any found, then continue with segCabin = PREMIUM_ECONOMY_CABIN, if no available
        // inventory found, then continue with segCabin == ECONOMY_CABIN
        CarrierCode cxr = _mkt->governingCarrier();
        std::vector<ClassOfService*>* vCosForBF = nullptr;
        vCosForBF = getAvailability(tSN);
        if (vCosForBF)
        {
          COSInnerPtrVecIC cosIter = vCosForBF->begin();
          uint16_t numSeats = PaxTypeUtil::numSeatsForFare(*diffTrx(), paxTfare);

          for (; cosIter != vCosForBF->end(); cosIter++)
          {
            if ((*cosIter)->cabin() < segCabin && (*cosIter)->numSeats() >= numSeats)
            {
              if (applyPremEconCabinDiffCalc(cxr))
                segCabin.setPremiumEconomyClass();
              else
                segCabin.setBusinessClass();
              break;
            }
          }
        }
      }

      if (segCabin.isUndefinedClass())
      {
        // ClassOfService vector of vectors
        COSInnerPtrVecIC classOfS = vCOS->begin();

        for (; classOfS != vCOS->end(); classOfS++)
        {
          BookingCode bk;

          SegmentStatusVec ssv = fu->segmentStatus();
          if (diffTrx()->getRequest()->isLowFareRequested() && // Is it a WPNC entry?
              ssv[tSN]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)) // Was it rebooked?
            bk = ssv[tSN]._bkgCodeReBook;
          else
            bk = airSeg->getBookingCode();

          if (bk.compare((*classOfS)->bookingCode()) == 0)
          {
            segCabin = (*classOfS)->cabin();
            break;
          }
        } // for COS
      }
      if (segCabin.isUndefinedClass() || fareCabin.isUndefinedClass())
      {
        return segCabinStatus;
      }
      if (segCabin < fareCabin) // lower number, means higher cabin
      {
        segCabinStatus = CABIN_HIGHER;
      }
      else if (segCabin == fareCabin)
        segCabinStatus = CABIN_EQUAL;
    } // if FAIL status
  } // if AirSeg
  fareTypeDes = cabinTypeToFTD(segCabin);
  return segCabinStatus;
}

//*1.2 & 1.3.****************************************************************************
// Check to see if this sector fails in RB validation of all local fares that
// are applicable to the Business or Economy fare cabin booked?
//*****************************************************************************
bool
DifferentialValidator::currentSectorLocalFareRBDValidation(int tSN)
{
  const PaxTypeFare& paxTfare = *throughFare();

  //  Now it's time to go thru all PaxTypeFares try to find any fare
  //  where the RBD was passed.

  uint16_t tsSize = _mkt->travelSeg().size();

  // ERROR - Business logic error
  if (tSN >= tsSize)
  {
    LOG4CXX_ERROR(
        logger, "Leaving DifferentialValidator::currentSectorLocalFareRBDValidation(): wrong size");
    return false;
  }

  TravelSeg* tvl = _mkt->travelSeg()[tSN];
  const AirSeg* psTravelSeg = dynamic_cast<const AirSeg*>(tvl);

  if (!psTravelSeg)
    return false;

  const FareMarket* localFM = nullptr;
  std::vector<TravelSeg*> segOrderVec;
  segOrderVec.clear();
  segOrderVec.push_back(tvl);

  // Need to get carrier for the sector it could be different then defult carrier
  // in case of the GoverningCarrierOverrige entry

  CarrierCode carrier = getCarrierCode(*psTravelSeg, &segOrderVec);

  localFM = getFareMarket(segOrderVec, &carrier, &psTravelSeg->carrier());

  if (!localFM)
  {
    LOG4CXX_ERROR(logger,
                  "7) FARE MARKET: " << segOrderVec.front()->boardMultiCity() << "-" << carrier
                                     << "-" << segOrderVec.back()->offMultiCity()
                                     << " NOT FOUND IN \"CURRENT SECTOR LOCAL RBD PROCESS\"");
    return false;
  }

  PaxTypeFarePtrVec paxTypeFareVec;

  getPaxTypeFareVec(paxTypeFareVec, *localFM);

  PaxTypeFare* ptf = nullptr;

  for (PaxTypeFarePtrVecIC itPTFare = paxTypeFareVec.begin(); itPTFare != paxTypeFareVec.end();
       itPTFare++) // Big LOOP for all PaxTypeFare vectors
  {
    ptf = *itPTFare;
    //  Initialize Iterator for the major loop through all Fares for the ONE PaxType

    if (paxTfare.isNormal() && // Does Fare Normal ?
        (paxTfare.fareTypeDesignator() == ptf->fareTypeDesignator())) // same as through fare
    {
      if (ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL)) // Does Fare already FAIL ?
      {
        continue;
      }
      if (ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED)) // Not processed
      {
        if (!isBkgCodeStatusPass(*ptf))
        {
          continue;
        }
      }
      else
      {
        return true; // RBD has Pass status
      }
    }
  } // for PaxType Fare Loop

  return false; // No fares have RBD Pass status
}

//**********************************************************************************
// Determination of the adjacent sectors on the Fare Market,
// Include the First, Business and Economy logics to proceed
//*1.2.1.or 1.3.1.(1.2.2 or 1.3.2.)+++++++++++++++++++++++++++++++++++++++++++++++++
// Check to see if there is an adjacent failed RB sector that is in the higher
// fare cabin than the through fare component being validated
// in the previous & next travel sector
//**********************************************************************************
bool
DifferentialValidator::adjacentSectorDetermination(DifferentialData& diffData,
                                                   int tSN,
                                                   Indicator& diffNumber,
                                                   const Indicator& diffLetter,
                                                   uint16_t& thruNumber)
{
  DifferentialData* diffItemNew = nullptr;
  bool previous = false; // adjacent differential sector not created or fail
  bool next = false; // adjacent differential sector not created or fail

  uint16_t tsSize = _mkt->travelSeg().size();

  // ERROR - Business logic error
  if (tSN >= tsSize)
  {
    LOG4CXX_ERROR(logger,
                  "Leaving DifferentialValidator::adjacentSectorDetermination(): wrong size");
    return false;
  }
  // Check to see if there is an adjacent failed RB sector that
  // is in the higher fare cabin in the previous travel sector

  if (tSN)
  {
    previous =
        adjacentSectorDiff(diffData, tSN, PREVIOUS, diffNumber, diffLetter, thruNumber);

    // Get object for the NEXT adjacent validation
    if (!previous)
    {
      diffTrx()->dataHandle().get(diffItemNew);
      diffNumber++;
    }
    else
      return previous;
  }

  // Check to see if there is an adjacent failed RB sector that
  // is in the higher fare cabin in the next travel sector
  if (tSN + 1 < tsSize)
  {
    if (diffItemNew)
      next = adjacentSectorDiff(
          *diffItemNew, tSN, NEXT, diffNumber, diffLetter, thruNumber);
    else
      next = adjacentSectorDiff(diffData, tSN, NEXT, diffNumber, diffLetter, thruNumber);
  }
  return previous || next;
}

//**********************************************************************************
// Validate all segment's statuses in the FareMarket and set up the status for the
// FareUsage itself
//**********************************************************************************
DifferentialValidator::StatusReturnType
DifferentialValidator::CheckBkgCodeSegmentStatus() const // get segment status
{
  const FareUsage* fu = fareUsage();
  bool segFail = false;
  bool segPass = false;

  uint16_t tsSize = _mkt->travelSeg().size();
  uint16_t segStSize = fu->segmentStatus().size();

  // ERROR - Business logic error
  if (tsSize != segStSize)
  {
    LOG4CXX_ERROR(logger, "Leaving DifferentialValidator::CheckBkgCodeSegmentStatus(): wrong size");
    return FAIL;
  }

  //  Gets the booking Code statuses for each segment in the Fare Market

  SegmentStatusVecCI iterSegStat = fu->segmentStatus().begin();
  TravelSegPtrVecIC iterTvl = _mkt->travelSeg().begin();

  for (; iterTvl != _mkt->travelSeg().end(); iterTvl++, iterSegStat++)
  {
    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;
    AirSeg* airSeg = (dynamic_cast<AirSeg*>(*iterTvl));

    if (airSeg != nullptr)
    {
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION))
      {
        segPass = true;
      }
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC) ||
               segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
      {
        segFail = true;
      }
    }
  }

  // Now it's time to set up the condition of the PaxTypeFare for the future validation or Stop

  if (segPass && !segFail)
    return PASS;

  return FAIL;
}

//**********************************************************************************
// Differential sector validation
//**********************************************************************************
bool
DifferentialValidator::diffSectorValidation(DifferentialData* diffData,
                                            const std::vector<Differentials*>& differList)
{
  _fareUsage->hipExemptInd() = 'N'; // initialize an indicator

  DifferentialsPtrListIC diffIter = differList.begin();

  for (; diffIter != differList.end(); diffIter++)
  {
    Differentials& di = **diffIter;
    if (!validateThruFare(di))
    {
      continue;
    }

    // At this point all Match for the Through Fare is done
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Continue do match for the Intermediate Points
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    bool noMatch = true; // default
    bool match1A = false; // match Loc1A/2A
    bool match1B = false; // match Loc1B/2B

    if (diffData == nullptr) // primary differential sector validation
    {
      std::vector<DifferentialData*>& differV = differential();
      DifferentialDataPtrVecI itDiffItem = differV.begin();

      for (; itDiffItem != differV.end(); itDiffItem++)
      {
        DifferentialData& iDiffItem = **itDiffItem;
        if (iDiffItem.status() == DifferentialData::SC_ADJACENT_FAIL)
          continue;
        if (iDiffItem.calculationIndicator() != BLANK) // sector already processed, skip
          continue;
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // Try to match the Intermediate Locations if they are present
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // Match are depends on directionality of the THROUGH Fare
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        if (!intermediateParamMatch(di, iDiffItem))
        {
          if (iDiffItem.status() == DifferentialData::SC_FAILED)
            return false;
          else
            continue;
        }
        // Check to see how many differential items passes validation
        if (di.intermedLoc1a().locType() != BLANK)
        {
          if (iDiffItem.status() == DifferentialData::SC_MATCH_1A)
            match1A = true;
          else if (iDiffItem.status() == DifferentialData::SC_MATCH_1B)
            match1B = true;
          else
            noMatch = true;
        }
        continue; // have to go through for each Diff sector in vector
      } // for loop Differential Items
    }
    else // consolidate differential sector validation
    {
      if (diffData->status() == DifferentialData::SC_ADJACENT_FAIL)
        continue;
      if (!intermediateParamMatch(di, *diffData))
      {
        if (diffData->status() == DifferentialData::SC_FAILED)
          return false;
        else
          continue;
      }
      // Check to see how many differential items passes validation
      if (di.intermedLoc1a().locType() != BLANK)
      {
        if (diffData->status() == DifferentialData::SC_MATCH_1A)
          match1A = true;
        else if (diffData->status() == DifferentialData::SC_MATCH_1B)
          match1B = true;
        else
          noMatch = true;
      }
    }

    // Analyze applicability of Differential Table item
    if ((di.intermedLoc1a().locType() != BLANK) && (di.intermedLoc1b().locType() != BLANK))
    {
      if (!(noMatch && match1A && match1B))
        continue;
    }
    else if ((di.intermedLoc1a().locType() != BLANK) && (di.intermedLoc1b().locType() == BLANK))
    {
      if (!(noMatch && match1A))
        continue;
    }

    if (diffData == nullptr) // primary differential sector validation, all sector have to go thru
    {
      std::vector<DifferentialData*>& differV = differential();
      DifferentialDataPtrVecI itDiffItem = differV.begin();
      bool notpop = false;
      for (; itDiffItem != differV.end(); itDiffItem++)
      {
        DifferentialData& iDiffItem = **itDiffItem;
        if (iDiffItem.status() == DifferentialData::SC_ADJACENT_FAIL)
          continue;

        if (iDiffItem.calculationIndicator() == BLANK) // sector not processed, continue
          notpop = true;
      }
      if (notpop)
        continue; // continue look thru Differential Table
    }

    if (_fareUsage->calculationInd() != BLANK)
      break; // out from Differential Table
  } // for loop Differential Table

  if (diffData && (diffData->status() != DifferentialData::SC_ADJACENT_FAIL))
    fareSelection(*diffData);
  else
    fareSelection();

  return true;
} // lint !e550 - removes lint warning that rv is not accessed

//**********************************************************************************
// Validate Through fare directionality vs. Differential table Item
//**********************************************************************************
bool
DifferentialValidator::thruFareDirectionality(const Differentials& diffTable) const
{
  const PaxTypeFare& paxTfare = *throughFare();

  Indicator direction = diffTable.directionality(); // validate Loc1 & Loc2;

  if (LIKELY(direction == DF_BETWEEN)) // Between
  {
    return matchLocation(diffTable.loc1(), // LocKey
                         diffTable.loc1().loc(), // LocCode
                         diffTable.loc2(),
                         diffTable.loc2().loc(),
                         *_mkt,
                         direction);
  }
  else if (direction == DF_FROM) // Diff Table required From
  {
    if ((paxTfare.directionality() == FROM)) // Through fare Outbound  "F"
    {
      return matchLocation(diffTable.loc1(), // LocKey
                           diffTable.loc1().loc(), // LocCode
                           diffTable.loc2(),
                           diffTable.loc2().loc(),
                           *_mkt,
                           direction);
    }
    else
    { // Through fare is Inbound
      return matchLocation(diffTable.loc2(), // LocKey
                           diffTable.loc2().loc(), // LocCode
                           diffTable.loc1(),
                           diffTable.loc1().loc(),
                           *_mkt,
                           direction);
    }
  }
  else if (direction == DF_WITHIN) // Within - origin & destin must match loc1 only
  {
    return matchLocation(diffTable.loc1(), // LocKey
                         diffTable.loc1().loc(), // LocCode
                         diffTable.loc2(),
                         diffTable.loc2().loc(),
                         *_mkt,
                         direction);
  }
  else if (direction == DF_ORIGIN) // Origination
  {
    return matchLocation(diffTable.loc1(), // LocKey
                         diffTable.loc1().loc(), // LocCode
                         diffTable.loc2(),
                         diffTable.loc2().loc(),
                         direction);
  }
  return true;
}

//**********************************************************************************
// Match/Nomatch the geographic board point of the itinenary  to the
// Loc1Type & Loc1
//**********************************************************************************
bool
DifferentialValidator::matchLocation(const LocKey& loc1k,
                                     const LocCode& loc1c,
                                     const LocKey& loc2k,
                                     const LocCode& loc2c,
                                     const Indicator& direction) const
{
  // match the origin of journey to Loc1

  TravelSegPtrVecIC itTvl = _itin->travelSeg().begin();

  for (; itTvl != _itin->travelSeg().end(); itTvl++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*itTvl);
    if (airSeg)
      break;
  }
  // reference to the last elem in vector
  std::vector<TravelSeg*>::const_reference tvl = _itin->travelSeg().back();
  return (matchGeo(
      loc1k, loc1c, loc2k, loc2c, (*(*itTvl)->origin()), (*(tvl->destination())), direction));
}

//**********************************************************************************
// Match/Nomatch the geographic via location in the fare component to the
// viaLocType & viaLoc
//**********************************************************************************
bool
DifferentialValidator::matchLocation(const LocKey& loc1k, const LocCode& loc1c) const
{
  bool segStat = false; //   default is fail

  // match the via location in the fare component

  TravelSegPtrVecIC itTvl = _mkt->travelSeg().begin();

  for (++itTvl; itTvl != _mkt->travelSeg().end(); itTvl++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*itTvl);

    if (!airSeg)
      continue;
    if (matchGeo(loc1k, loc1c, (*(*itTvl)->origin())))
    {
      segStat = true;
      break;
    }
  }
  return segStat;
}

bool
DifferentialValidator::matchGeo(const LocKey& loc1k, const LocCode& loc1c, const Loc& origin) const
{
  return (LocUtil::isInLoc(origin,
                           loc1k.locType(),
                           loc1c,
                           SABRE_USER,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           _diffTrx->getRequest()->ticketingDT()));
}

//**********************************************************************************
// Match/Nomatch the geographic board or off point of the through fare component
// Loc1Type & Loc1; Loc2Type & Loc2; ViaLoc Type & ViaLoc;
//**********************************************************************************
bool
DifferentialValidator::matchLocation(const LocKey& loc1k,
                                     const LocCode& loc1c,
                                     const LocKey& loc2k,
                                     const LocCode& loc2c,
                                     const FareMarket& mkt,
                                     const Indicator& direction) const
{

  // match the original direction

  bool rv =
      matchGeo(loc1k, loc1c, loc2k, loc2c, (*(mkt.origin())), (*(mkt.destination())), direction);

  if (!rv && direction != DF_FROM && direction != DF_WITHIN) // FROM or WITHIN
  {
    rv = matchGeo(loc1k, loc1c, loc2k, loc2c, (*(mkt.destination())), (*(mkt.origin())), direction);
  }
  return rv;
}

//----------------------------------------------------------------------------
// matchGeo()         Through Fare
//----------------------------------------------------------------------------
bool
DifferentialValidator::matchGeo(const LocKey& loc1k,
                                const LocCode& loc1c,
                                const LocKey& loc2k,
                                const LocCode& loc2c,
                                const Loc& origin,
                                const Loc& destination,
                                const Indicator& direction) const
{
  VendorCode vendorCode = SABRE_USER;
  bool ret = false;

  if (direction == DF_FROM) // FROM
  {
    return ((LocUtil::isInLoc(origin,
                              loc1k.locType(),
                              loc1c,
                              vendorCode,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _diffTrx->getRequest()->ticketingDT())) &&
            (LocUtil::isInLoc(destination,
                              loc2k.locType(),
                              loc2c,
                              vendorCode,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _diffTrx->getRequest()->ticketingDT())));
  }
  else if (UNLIKELY(direction == DF_WITHIN)) // WITHIN
  {
    return (((LocUtil::isInLoc(origin,
                               loc1k.locType(),
                               loc1c,
                               vendorCode,
                               MANUAL,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               _diffTrx->getRequest()->ticketingDT())) &&
             (LocUtil::isInLoc(destination,
                               loc1k.locType(),
                               loc1c,
                               vendorCode,
                               MANUAL,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               _diffTrx->getRequest()->ticketingDT()))));
  }
  else if (LIKELY(direction == DF_BETWEEN)) // BETWEEN

  {
    return ((LocUtil::isInLoc(origin,
                              loc1k.locType(),
                              loc1c,
                              vendorCode,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _diffTrx->getRequest()->ticketingDT())) &&
            (LocUtil::isInLoc(destination,
                              loc2k.locType(),
                              loc2c,
                              vendorCode,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _diffTrx->getRequest()->ticketingDT())));
  }
  else if (direction == DF_ORIGIN) // ORIGINATION
  {
    if (LocUtil::isInLoc(origin,
                         loc1k.locType(),
                         loc1c,
                         vendorCode,
                         MANUAL,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         _diffTrx->getRequest()->ticketingDT()))
    {
      if (loc2k.locType() != BLANK)
      {
        if (LocUtil::isInLoc(destination,
                             loc2k.locType(),
                             loc2c,
                             vendorCode,
                             MANUAL,
                             LocUtil::OTHER,
                             GeoTravelType::International,
                             EMPTY_STRING(),
                             _diffTrx->getRequest()->ticketingDT()))
        {
          ret = true; // both location match
        }
        else
          ret = false; // destination does not match
      }
      else
        ret = true; // origin match, destination does not need to match
    }
    else
      ret = false; // origin does not match
  }
  return ret;
}

//**********************************************************************************
// Match/Nomatch the geographic board or off point of the Differential Sectors
// Loc1AType & Loc1A; Loc2AType & Loc2A; AND Loc1BType & Loc1B; Loc2BType & Loc2B;
//**********************************************************************************
bool
DifferentialValidator::matchLocation(const LocKey& loc1k,
                                     const LocCode& loc1c,
                                     const LocKey& loc2k,
                                     const LocCode& loc2c,
                                     const DifferentialData& diffSeg,
                                     const Indicator& direction) const
{
  const PaxTypeFare& paxTfare = *throughFare();
  // match the original direction

  if (direction == DF_FROM) // FROM
  {
    if (paxTfare.directionality() == FROM)
    {
      return (matchGeo(loc1k,
                       loc1c,
                       loc2k,
                       loc2c,
                       (*(diffSeg.origin())),
                       (*(diffSeg.destination())),
                       direction));
    }
    else // fare inbound
    {
      return (matchGeo(loc1k,
                       loc1c,
                       loc2k,
                       loc2c,
                       (*(diffSeg.destination())),
                       (*(diffSeg.origin())),
                       direction));
    }
  }
  else if (direction == DF_BETWEEN) // direction 'B' - BETWEEN
  {
    if (matchGeo(loc1k,
                 loc1c,
                 loc2k,
                 loc2c,
                 (*(diffSeg.origin())),
                 (*(diffSeg.destination())),
                 direction))
    {
      return true;
    }
    else
      return (matchGeo(loc1k,
                       loc1c,
                       loc2k,
                       loc2c,
                       (*(diffSeg.destination())),
                       (*(diffSeg.origin())),
                       direction));
  }
  else // direction 'W' - WITHIN
  {
    return (matchGeo(
        loc1k, loc1c, loc2k, loc2c, (*(diffSeg.origin())), (*(diffSeg.destination())), direction));
  }
}

//**********************************************************************************
// Match/Nomatch validation for the Intermediate sector
//**********************************************************************************
bool
DifferentialValidator::intermediateParamMatch(const Differentials& diffTable,
                                              DifferentialData& diffSector) const
{
  if (!matchIntermTypeCxrBkg(diffSector,
                             diffSector.carrier(),
                             diffSector.bookingCode(),
                             diffSector.cabin(),
                             diffTable))
  {
    return false; // continue;
  }

  if (!validateIntermediates(diffTable, diffSector))
  {
    return false; // continue
  }
  return true;
}

// ***********************************************************************************
// Match Carriers and Intermediate Loc1A & Loc2A, Try to Match an intermediate Carrier,
// Validate the Differential Calculation field & populate the FareUsage
// ***********************************************************************************
bool
DifferentialValidator::validateIntermediates(const Differentials& diffTable,
                                             DifferentialData& diffSector) const
{
  if (diffTable.intermedLoc1a().locType() != BLANK)
  {
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Match Carriers and Intermediate Loc1A & Loc2A
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Try to Match an intermediate Carrier
    // ++++++++++++++++++++++++++++++++++++++++++++

    countStopsCarriers(diffSector, diffTable);
    Indicator fAppl = diffTable.flightAppl();

    if (fAppl != NONSTOP && fAppl != SAMECARRIER && fAppl != LAST_FIRST && fAppl != BLANK)
    {
      LOG4CXX_ERROR(logger, "RULDIF table has wrong FLIGHTAPPL data");
      return false;
    }

    if (((fAppl == NONSTOP) && diffSector.stops()) || // Non stop/ direct flight
        ((fAppl == SAMECARRIER) && !diffSector.isSameCarrier())) // Same carrier
    {
      return false; //      continue;
    }
    if (diffTable.directionality() == BLANK)
    {
      diffSector.status() = DifferentialData::SC_FAILED;
      return false;
    }

    // Flight Application is 'L' or BLANK

    if (!validateFlightAppL(diffSector, diffTable))
    {
      return false; // continue;
    }
  } // No Restrictions for the Intermediate points for this fare

  // ++++++++++++++++++++++++++++++++++++++++++++
  // Validate the Differential Calculation field
  // ++++++++++++++++++++++++++++++++++++++++++++
  if (!finalyzeDiffSectorValidation(diffSector, diffTable))
  {
    return false; // continue;
  }
  return true;
}

//**********************************************************************************
// Match/Nomatch validation for the Intermediate sector
//**********************************************************************************
bool
DifferentialValidator::validateFlightAppL(DifferentialData& diffSeg, const Differentials& diffTable)
    const
{
  PaxTypeFare& paxTfare = *throughFare();
  bool match = false;

  Indicator direction = diffTable.directionality();

  if (diffTable.flightAppl() == LAST_FIRST) // Flight Application is 'Last/First'
  {
    if (direction == DF_BETWEEN) // Between
    {
      if (matchLoc1A2A1B2B(diffTable.intermedLoc1a(), // LocKey
                           diffTable.intermedLoc1a().loc(), // LocCode
                           diffTable.intermedLoc2a(),
                           diffTable.intermedLoc2a().loc(),
                           diffSeg,
                           diffTable))
      {
        diffSeg.status() = DifferentialData::SC_MATCH_1A;
        match = true;
      }
      else if (diffTable.intermedLoc1b().locType() != BLANK)
      {
        if (matchLoc1A2A1B2B(diffTable.intermedLoc1b(), // LocKey
                             diffTable.intermedLoc1b().loc(), // LocCode
                             diffTable.intermedLoc2b(),
                             diffTable.intermedLoc2b().loc(),
                             diffSeg,
                             diffTable))
        {
          diffSeg.status() = DifferentialData::SC_MATCH_1B;
          match = true;
        }
      }
    } // 'B'
    else if (direction == DF_FROM) // Diff Table required From
    {
      if ((paxTfare.directionality() == FROM)) // Through fare Outbound  "O"
      {
        if (matchLoc1A2A1B2B(diffTable.intermedLoc1a(), // LocKey
                             diffTable.intermedLoc1a().loc(), // LocCode
                             diffTable.intermedLoc2a(),
                             diffTable.intermedLoc2a().loc(),
                             diffSeg,
                             diffTable))
        {
          diffSeg.status() = DifferentialData::SC_MATCH_1A;
          match = true;
        }
        else if (diffTable.intermedLoc1b().locType() != BLANK)
        {
          if (matchLoc1A2A1B2B(diffTable.intermedLoc1b(), // LocKey
                               diffTable.intermedLoc1b().loc(), // LocCode
                               diffTable.intermedLoc2b(),
                               diffTable.intermedLoc2b().loc(),
                               diffSeg,
                               diffTable))
          {
            diffSeg.status() = DifferentialData::SC_MATCH_1B;
            match = true;
          }
        }
      }
      else // Through fare is Inbound
      {
        if (matchLoc1A2A1B2B(diffTable.intermedLoc2a(), // LocKey
                             diffTable.intermedLoc2a().loc(), // LocCode
                             diffTable.intermedLoc1a(),
                             diffTable.intermedLoc1a().loc(),
                             diffSeg,
                             diffTable))
        {
          diffSeg.status() = DifferentialData::SC_MATCH_1A;
          match = true;
        }
        else if (diffTable.intermedLoc1b().locType() != BLANK)
        {
          if (matchLoc1A2A1B2B(diffTable.intermedLoc2b(), // LocKey
                               diffTable.intermedLoc2b().loc(), // LocCode
                               diffTable.intermedLoc1b(),
                               diffTable.intermedLoc1b().loc(),
                               diffSeg,
                               diffTable))
          {
            diffSeg.status() = DifferentialData::SC_MATCH_1B;
            match = true;
          }
        }
      }
    } // From is done
    else if (direction == DF_WITHIN) // Within - origin & destin must match loc1A/2a or loc1B/2B
    {
      if (matchLoc1A2A1B2B(diffTable.intermedLoc1a(), // LocKey
                           diffTable.intermedLoc1a().loc(), // LocCode
                           diffTable.intermedLoc2a(),
                           diffTable.intermedLoc2a().loc(),
                           diffSeg,
                           diffTable))
      {
        diffSeg.status() = DifferentialData::SC_MATCH_1A;
        match = true;
      }
      else if (diffTable.intermedLoc1b().locType() != BLANK)
      {
        if (matchLoc1A2A1B2B(diffTable.intermedLoc1b(), // LocKey
                             diffTable.intermedLoc1b().loc(), // LocCode
                             diffTable.intermedLoc2b(),
                             diffTable.intermedLoc2b().loc(),
                             diffSeg,
                             diffTable))
        {
          diffSeg.status() = DifferentialData::SC_MATCH_1B;
          match = true;
        }
      }
    }
  }

  else // Flight Application is BLANK
  {
    if (direction == DF_BETWEEN) // Between
    {
      if (matchLocation(diffTable.intermedLoc1a(), // LocKey
                        diffTable.intermedLoc1a().loc(), // LocCode
                        diffTable.intermedLoc2a(),
                        diffTable.intermedLoc2a().loc(),
                        diffSeg,
                        direction))
      {
        diffSeg.status() = DifferentialData::SC_MATCH_1A;
        match = true;
      }
    }
    else if (direction == DF_FROM) // Diff Table required From
    {
      if ((paxTfare.directionality() == FROM)) // Through fare Outbound  "O"
      {
        if (matchLocation(diffTable.intermedLoc1a(), // LocKey
                          diffTable.intermedLoc1a().loc(), // LocCode
                          diffTable.intermedLoc2a(),
                          diffTable.intermedLoc2a().loc(),
                          diffSeg,
                          direction))
        {
          diffSeg.status() = DifferentialData::SC_MATCH_1A;
          match = true;
        }
      }
      else
      { // Through fare is Inbound
        if (matchLocation(diffTable.intermedLoc2a(), // LocKey
                          diffTable.intermedLoc2a().loc(), // LocCode
                          diffTable.intermedLoc1a(),
                          diffTable.intermedLoc1a().loc(),
                          diffSeg,
                          direction))
        {
          diffSeg.status() = DifferentialData::SC_MATCH_1A;
          match = true;
        }
      }
    }
    else if (direction == DF_WITHIN) // Within - origin & destin must match loc1 or loc2
    {
      if (matchLocation(diffTable.intermedLoc1a(), // LocKey
                        diffTable.intermedLoc1a().loc(), // LocCode
                        diffTable.intermedLoc2a(),
                        diffTable.intermedLoc2a().loc(),
                        diffSeg,
                        direction))
      {
        diffSeg.status() = DifferentialData::SC_MATCH_1A;
        match = true;
      }
    }
  }
  return match;
}

//**********************************************************************************
// Match/Nomatch the geographic board or off point of the Differential Sectors
// Loc1AType & Loc1A; Loc2AType & Loc2A; AND Loc1BType & Loc1B; Loc2BType & Loc2B;
//**********************************************************************************
bool
DifferentialValidator::matchLoc1A2A1B2B(const LocKey& loc1k,
                                        const LocCode& loc1c,
                                        const LocKey& loc2k,
                                        const LocCode& loc2c,
                                        DifferentialData& diffSeg,
                                        const Differentials& diffTable) const
{
  Indicator direction = diffTable.directionality();

  TravelSegPtrVecIC lastPoint = diffSeg.travelSeg().end();
  TravelSegPtrVecIC firstPoint = diffSeg.travelSeg().end();

  int16_t endNumber = _itin->segmentOrder(diffSeg.travelSeg().back()); // Number of the Last segment

  TravelSegPtrVecIC itTvl = diffSeg.travelSeg().begin();

  if (direction == DF_BETWEEN) // Between
  {
    for (; itTvl != diffSeg.travelSeg().end(); itTvl++) // Loop for all TravelSegment
    {
      if (_itin->segmentOrder(*itTvl) <= endNumber)
      {
        if (matchGeo(loc1k, loc1c, (*(diffSeg.origin()))))
        {
          lastPoint = itTvl;
        }
      }
    } // for loop
    if (lastPoint != diffSeg.travelSeg().end())
    {
      for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
           itTvl++) // Loop for all TravelSegment
      {
        if (_itin->segmentOrder(*itTvl) <= endNumber)
        {
          if (matchGeo(loc2k, loc2c, (*(diffSeg.destination()))))
          {
            firstPoint = itTvl;
            break;
          }
        }
      } // for loop
    } // lastPoint != diffSeg.travelSeg().end()
    else // lastPoint == diffSeg.travelSeg().end()
    {
      // Start matching in reverse direction
      for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
           itTvl++) // Loop for all TravelSegment
      {
        if (_itin->segmentOrder(*itTvl) <= endNumber)
        {
          if (matchGeo(loc1k, loc1c, (*(diffSeg.destination()))))
          {

            lastPoint = itTvl;
          }
        }
      } // for loop
      if (lastPoint == diffSeg.travelSeg().end())
      {
        return false;
      }
      else // lastPoint != diffSeg.travelSeg().end()
      {
        for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
             itTvl++) // Loop for all TravelSegment
        {
          if (_itin->segmentOrder(*itTvl) <= endNumber)
          {

            if (matchGeo(loc2k, loc2c, (*(diffSeg.origin()))))
            {
              firstPoint = itTvl;
              break;
            }
          }
        } // for loop
      } // else, lastPoint != diffSeg.travelSeg().end()
    }
    if (firstPoint == diffSeg.travelSeg().end())

      return false;

    // Match Intermediate parameters in the Loc1A/2A, or Loc1B/2B , if present
    if (distance(firstPoint, lastPoint) <= 0)
    {
      TravelSegPtrVecIC temp = lastPoint;
      lastPoint = firstPoint;
      firstPoint = temp;
    }

    for (; firstPoint != lastPoint; firstPoint++) // Loop for part of  TravelSeg vector
    {
      const AirSeg* airSeg = dynamic_cast<AirSeg*>(*firstPoint);

      if (airSeg)
      {
        BookingCode bc;

        if (!getBookingCode(*firstPoint, *airSeg, bc))
          return false;
        std::vector<CarrierCode> carr;
        CarrierCode carrier = getCarrierCode(*airSeg, nullptr);
        carr.push_back(carrier);
        if (!matchIntermTypeCxrBkg(diffSeg, carr, bc, diffSeg.cabin(), diffTable))
          return false;
      }
    }
  } // 'BETWEEN'  DONE
  else if (direction == DF_FROM) // FROM
  {
    for (; itTvl != diffSeg.travelSeg().end(); itTvl++) // Loop for all TravelSegment
    {
      if (_itin->segmentOrder(*itTvl) <= endNumber)
      {
        if (matchGeo(loc1k, loc1c, (*(diffSeg.origin()))))
        {
          lastPoint = itTvl;
        }
      }
    } // for loop
    if (lastPoint == diffSeg.travelSeg().end())
    {
      return false;
    }
    else
    {
      for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
           itTvl++) // Loop for all TravelSegment
      {
        if (_itin->segmentOrder(*itTvl) <= endNumber)
        {
          if (matchGeo(loc2k, loc2c, (*(diffSeg.destination()))))
          {
            firstPoint = itTvl;
            break;
          }
        }
      } // for loop
      if (firstPoint == diffSeg.travelSeg().end())
      {
        return false;
      }
    } // lastPoint != diffSeg.travelSeg().end()

    // Match Intermediate parameters in the Loc1A/2A, or Loc1B/2B , if present

    if (distance(firstPoint, lastPoint) <= 0)
      return false;

    for (; lastPoint != firstPoint; lastPoint++) // Loop for part of  TravelSeg vector
    {
      const AirSeg* airSeg = dynamic_cast<AirSeg*>(*lastPoint);
      if (airSeg != nullptr)
      {
        BookingCode bc;

        if (!getBookingCode(*lastPoint, *airSeg, bc))
          return false;
        std::vector<CarrierCode> carr;
        CarrierCode carrier = getCarrierCode(*airSeg, nullptr);
        carr.push_back(carrier);
        if (!matchIntermTypeCxrBkg(diffSeg, carr, bc, diffSeg.cabin(), diffTable))
          return false;
      }
    }
  } // 'FROM' DONE
  if (direction == DF_WITHIN) // WITHIN
  {
    for (; itTvl != diffSeg.travelSeg().end(); itTvl++) // Loop for all TravelSegment
    {
      if (_itin->segmentOrder(*itTvl) <= endNumber)
      {
        if (matchGeo(loc1k, loc1c, (*(diffSeg.origin()))))
        {

          lastPoint = itTvl;
        }
      }
    } // for loop
    if (lastPoint == diffSeg.travelSeg().end())
    {
      return false;
    }
    else
    {
      for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
           itTvl++) // Loop for all TravelSegment
      {
        if (_itin->segmentOrder(*itTvl) <= endNumber)
        {
          if (matchGeo(loc1k, loc1c, (*(diffSeg.destination()))))
          {
            firstPoint = itTvl;
            break;
          }
        }
      } // for loop

      if (firstPoint == diffSeg.travelSeg().end())
      {
        return false;
      }
    }
    if (distance(firstPoint, lastPoint) <= 0)
    {
      TravelSegPtrVecIC temp = lastPoint;
      lastPoint = firstPoint;
      firstPoint = temp;
    }

    // Match Intermediate parameters in the Loc1A/2A, or Loc1B/2B , if present

    for (; firstPoint != lastPoint; firstPoint++) // Loop for part of  TravelSeg vector
    {
      const AirSeg* airSeg = dynamic_cast<AirSeg*>(*firstPoint);
      if (airSeg)
      {
        BookingCode bc;

        if (!getBookingCode(*firstPoint, *airSeg, bc))
          return false;
        std::vector<CarrierCode> carr;
        //        carr.push_back (airSeg->carrier());
        CarrierCode carrier = getCarrierCode(*airSeg, nullptr);
        carr.push_back(carrier);
        if (!matchIntermTypeCxrBkg(diffSeg, carr, bc, diffSeg.cabin(), diffTable))
          return false;
      }
    }
  } // 'WITHIN'  DONE
  return true;
}

//**********************************************************************************
// Match Through Booking Code for the Fare
//**********************************************************************************
bool
DifferentialValidator::matchThroughBkgCode(const BookingCode& bkg) const
{
  const FareUsage* fu = fareUsage();
  bool segFail = false;
  bool segPass = false;

  uint16_t tsSize = _mkt->travelSeg().size();
  uint16_t segStSize = fu->segmentStatus().size();

  // ERROR - Business logic error
  if (tsSize != segStSize)
  {
    LOG4CXX_ERROR(logger, "Leaving DifferentialValidator::matchThroughBkgCode(): wrong size");
    return FAIL;
  }

  //  Gets the booking Code statuses for each segment in the Fare Market

  SegmentStatusVecCI iterSegStat = fu->segmentStatus().begin();
  TravelSegPtrVecIC iterTvl = _mkt->travelSeg().begin();

  for (; iterTvl != _mkt->travelSeg().end(); iterTvl++, iterSegStat++)
  {
    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;

    AirSeg* airSeg = (dynamic_cast<AirSeg*>(*iterTvl));

    if (!airSeg)
    {
      continue;
    }
    if (!(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NEED_REVALIDATION)))
    {
      continue;
    }

    BookingCode bc;

    if (!getBookingCode(*iterTvl, *airSeg, bc))
      return false;

    if (bc.compare(bkg) == 0)
      segPass = true;
    else
      segFail = true;
  }

  // Now it's time to set up the condition of the PaxTypeFare for the future validation or Stop

  return (segPass && !segFail);
}

//*************************************************************************************
// Match FareType Designator and Carrier for the Intermediate ( Digfferential sectors)
// For the Flight Appl has 'L' param sets. And, Loc1A/2A and/or Loc1B/2B are set.

//*************************************************************************************
bool
DifferentialValidator::matchIntermTypeCxrBkg(DifferentialData& diffSector,
                                             std::vector<CarrierCode>& carrier,
                                             const BookingCode bkg,
                                             const Indicator cabin,
                                             const Differentials& diffTable,
                                             bool donotErase) const
{

  // ++++++++++++++++++++++++++++++++++++++++++++
  // Try to Match an intermediate Carrier
  // ++++++++++++++++++++++++++++++++++++++++++++

  if (!diffTable.intermedCarrier().empty()) // intermediate carrier presents in the Table
  {
    std::vector<CarrierCode>::iterator cc =
        find(carrier.begin(), carrier.end(), diffTable.intermedCarrier());
    if (cc == carrier.end())
      return false; // go & get next differential sector
    if (!donotErase)
    {
      diffSector.finalizeCarrierAndFareMarketLists(*cc);
    }
    else
    {
      diffSector.cleanUpVectors().push_back(*cc);
    }
  }

  // ++++++++++++++++++++++++++++++++++++++++++++
  // Try to Match the Fare Type Designator
  // ++++++++++++++++++++++++++++++++++++++++++++

  std::ostringstream ss;
  ss << '*' << cabin;
  FareType fareType = ss.str().c_str();

  if (!diffTable.intermedFareType().empty()) // != ' '  )
  {
    if (diffTable.intermedFareType()[0] == cabin)
      return true;


    if (!RuleUtil::matchFareType(diffTable.intermedFareType(), fareType))
    {
      if (cabin == W_TYPE)
      {
        if (diffTable.intermedFareType()[0] != Z_TYPE &&
            (diffTable.intermedFareType()[0] != ALL_TYPE ||
             diffTable.intermedFareType().length() < 2   ||
             diffTable.intermedFareType()[1] != Z_TYPE))
          return false;
      }
      else
        return false;
    }
  }

  // ++++++++++++++++++++++++++++++++++++++++++++
  // Try to Match the Booking Code
  // ++++++++++++++++++++++++++++++++++++++++++++

  if (!diffTable.intermedBookingCode().empty()) // intermediate BkgCode presents in the Table
  {
    if (bkg.compare(diffTable.intermedBookingCode()) != 0)
      return false; // go & get next differential sector
  }
  return true;
}

//*************************************************************************************
// Check for Diff Calculation Indicator. Populate Diff sector & FareUsage, or fail
// Diff sector.
//*************************************************************************************
bool
DifferentialValidator::finalyzeDiffSectorValidation(DifferentialData& diffSector,
                                                    const Differentials& diffTable) const
{
  diffSector.calculationIndicator() = diffTable.calculationInd(); // store calc Indic
  diffSector.differSeqNumber() = diffTable.seqNo(); // store sequence #
  diffSector.intermFareType() = diffTable.intermedFareType(); // for cabin 7(*W,..*Z) check
  diffSector.calcIntermIndicator() = diffTable.calculationInd(); // store original Calc indicator

  if (((diffSector.thruNumber() == 0) && (diffSector.calculationIndicator() != NOT_PERMITTED)) ||
      (diffSector.thruNumber() != 0))
  {
    _fareUsage->differSeqNumber() = diffTable.seqNo(); // store sequence #
    _fareUsage->calculationInd() = diffTable.calculationInd(); // store calc Indic
    _fareUsage->diffCarrier() = diffTable.carrier(); // store carrier
    _fareUsage->hipExemptInd() = diffTable.hipExemptInd(); // store HIP Exempt indicator
  }

  if (diffTable.calculationInd() == NOT_PERMITTED)
  {
    if (!diffSector.intermFareType().empty())
    {
      if (isPremiumEconomyCabin(diffTable.intermedFareType()))
      {
        diffSector.calculationIndicator() = LOW_TYPE;
        // try to apply 'LOW' logic for the different type (W or Z) of Premium Economy cabin
        return true;
      }
      else
      {
        diffSector.status() = DifferentialData::SC_FAILED;
        return false;
      }
    }
    else
    {
      diffSector.status() = DifferentialData::SC_FAILED;
      return false;
    }
  }
  return true;
}

//**********************************************************************************
// Do check Premium Economy cabin (7) when intermedFareType = any W or Z fare types.
//**********************************************************************************
bool
DifferentialValidator::isPremiumEconomyCabin(const FareType& intermFareType) const
{
  if (intermFareType.length() < 2)
    return false;
  if (intermFareType[0] == Z_TYPE || intermFareType[0] == W_TYPE)
    return true;
  if (intermFareType[0] == ALL_TYPE &&
      (intermFareType[1] == W_TYPE || intermFareType[1] == Z_TYPE))
    return true;
  return false;
}

//**********************************************************************************
// Do fare retrieval for the Differential sectors based on the Calculation Indicator
//**********************************************************************************
bool
DifferentialValidator::fareSelection(void)
{
  PaxTypeFare& paxTfare = *throughFare();
  FareUsage* fu = fareUsage();

  std::vector<DifferentialData*>& differV = differential();
  bool rv = true, ret = true;

  DifferentialDataPtrVecI itDiffItem = differV.begin();

  for (; itDiffItem != differV.end(); itDiffItem++)
  {
    DifferentialData& diffItem = **itDiffItem;
    if (diffItem.status() == DifferentialData::SC_ADJACENT_FAIL)
      continue;

    if (diffItem.fareMarket().empty())
      continue;
    if (diffItem.tag()[0] != 'S')
    {
      rv = validateFareTypeDesignators(diffItem, diffItem.fareMarket(), diffItem.carrier());

      // Validate it for the current Carriers, before it was done for the Gov Cxr Override
      if (!rv && !diffItem.fareMarketCurrent().empty())
      {
        rv = validateFareTypeDesignators(
            diffItem, diffItem.fareMarketCurrent(), diffItem.carrierCurrent(), false);
      }
    }
    if (ret)
      ret = rv;

    //-NR- 3.3.4
    // 13.If any DIFF sector that does not have a fare found for the higher class of service as
    // booked,
    //   tagged the sector once the DIFF calculation
    //   for
    //   other sectors in the fare component are completed

    if (!diffItem.fareHigh())
    {
      uint16_t tsSize = paxTfare.fareMarket()->travelSeg().size();
      uint16_t segStSize = fu->segmentStatus().size();

      // ERROR - Business logic error
      if (tsSize != segStSize)
      {
        LOG4CXX_ERROR(logger, "Leaving DifferentialValidator::fareSelection(): wrong size");
        return false;
      }

      TravelSegPtrVecIC iterTvl = paxTfare.fareMarket()->travelSeg().begin();
      TravelSegPtrVecIC iterTvlEnd = paxTfare.fareMarket()->travelSeg().end();
      SegmentStatusVecI iterSegStat = fu->segmentStatus().begin();

      for (; iterTvl != iterTvlEnd; iterTvl++, iterSegStat++)
      {
        const AirSeg* psTravelSeg = dynamic_cast<const AirSeg*>(*iterTvl);

        if (!psTravelSeg)
          continue;

        const FareMarket* psFM = nullptr;

        // Put travel segment to vector for the getting the FareMarket
        std::vector<TravelSeg*> segOrderVec;

        segOrderVec.push_back(*iterTvl);

        const CarrierCode& carrier = getCarrierCode(*psTravelSeg, &segOrderVec);

        psFM = getFareMarket(segOrderVec, &carrier, &psTravelSeg->carrier());

        FareMarket* fm = *(diffItem.fareMarket().begin());

        if (fm == psFM)
        {
          // Reset the value of "ret" to true if the only high fare was not found...
          if (diffItem.fareLow())
            ret = true;
          break;
        }
      }
    }
    if (rv)
    {
      diffItem.status() = DifferentialData::SC_PASSED;
      diffItem.amount() = diffItem.amountFareClassHigh() - diffItem.amountFareClassLow();
    }
    else
      diffItem.status() = DifferentialData::SC_FAILED;
  }
  return ret; // return false only if (no low fare) or/and (no high fare && COS)
}

//**********************************************************************************
// High and low differential fare selection processor
//**********************************************************************************
bool
DifferentialValidator::fareSelection(DifferentialData& diffData)
{
  bool ret = false;

  if (diffData.fareMarket().empty())
    return ret;
  ret = validateFareTypeDesignators(diffData, diffData.fareMarket(), diffData.carrier());
  // Governing Carrier Override
  // The fareMarketCurrent and carrierCurrent vectors could have data
  // Check those two vectors for non-zero if got ret = false, then call it again
  // for non-zero vectors
  if (!ret && !diffData.fareMarketCurrent().empty())
  {
    ret = validateFareTypeDesignators(
        diffData, diffData.fareMarketCurrent(), diffData.carrierCurrent(), false);
  }

  if (ret)
    diffData.amount() = diffData.amountFareClassHigh() - diffData.amountFareClassLow();
  else
    diffData.status() = DifferentialData::SC_FAILED;
  return ret; // return false if no low fare or no high fare found
}

//**********************************************************************************
// Consolidate differential sectors
//**********************************************************************************
// The differentials are stored for both round & one way trips in the fare regardless
// of the the fare itself. This dictates the need in processing both sequences
// separetly.
//**********************************************************************************
bool
DifferentialValidator::consolidate(const std::vector<Differentials*>& differList)
{
  const std::vector<DifferentialData*>& differV = differential();
  bool ret = true;

  std::vector<DifferentialData*> diffLforOneTripType;

  diffLforOneTripType.insert(diffLforOneTripType.end(), differV.begin(), differV.end());
  ret = consolidateDifferentials(diffLforOneTripType, differList);

  return ret;
}

//**********************************************************************************
//           Validate Fare based upon the Micsellaneous Fare Tags
//**********************************************************************************
bool
DifferentialValidator::validateMiscFareTag(const PaxTypeFare& paxTfare, bool throughFare) const
{

  if (paxTfare.isMiscFareTagValid() && paxTfare.miscFareTag() != nullptr)
  {
    const Indicator& diffCalcInd = paxTfare.miscFareTag()->diffcalcInd();
    if (diffCalcInd == CAT23_BYTE10_VALUE_R || (diffCalcInd == NOT_PERMITTED && !throughFare))
      return false;
  }
  return true;
}

//**********************************************************************************
// 3.3.3 Validate consecutive sectors for through fare type designator
//**********************************************************************************
bool
DifferentialValidator::validateFareTypeDesignators(DifferentialData& diffDP,
                                                   std::vector<FareMarket*>& fareMarket,
                                                   std::vector<CarrierCode>& carrier,
                                                   bool gOver) const // Differential data for 1
                                                                     // segment
{
  if (fareMarket.empty() || carrier.empty() || fareMarket.size() != carrier.size())
  {
    LOG4CXX_ERROR(logger, "0) FARE SELECT: \"fareMarket\" OR \"carrier\" CONTAINER IS CORRUPTED");
    return false; // Shouldn't ever happen, but it's better to check
  }

  const PaxTypeFare& through = *throughFare(); // Through fare
  bool rv = false;

  // Differential Calculation would be 0 if no Table Item was found

  const Indicator diffCalculation = _toupper(diffDP.calculationIndicator());

  if (diffCalculation != SAME_TYPE && diffCalculation != LOW_TYPE && diffCalculation != HIGH_TYPE &&
      diffCalculation != NOT_FOUND_IN_TABLE)
    return false;
  CabinType throughFTD = through.cabin();

  if (throughFTD.isPremiumFirstClass())
    return false; // The highest type fare

  CabinType diffFTD;
  size_t posThrough = throughFTD.index();

  if (posThrough == std::string::npos) // lint !e530
    return false; // Shouldn't ever happen, but it's better to check

  uint16_t fmSize = fareMarket.size();
  uint16_t carrSize = carrier.size();

  // ERROR - Business logic error
  if (fmSize > carrSize)
  {
    LOG4CXX_ERROR(logger,
                  "Leaving DifferentialValidator::validateFareTypeDesignators(): wrong size");
    return false;
  }
  DiffDataFMI fmi = fareMarket.begin();
  DiffDataCxrI carr = carrier.begin();
  FareSelectionData fsdGl;

  CabinType failedFareCabin;
  cabinTypeFromFTD(diffDP.cabin(), failedFareCabin);

  if (_diffTrx->getRequest()->isLowFareRequested() &&
      (diffDP.status() == DifferentialData::SC_FAILED))
  {
    return false;
  }

  for (; fmi != fareMarket.end(); fmi++, carr++) // Multiple Governing carriers are allowed now...
  { // This is valid only for consolidation.
    FareMarket* fmiDiff = *fmi;
    if (fmiDiff == nullptr)
      continue;

    FareSelectionDataHelper fsdm(*diffTrx(), diffDP);

    CarrierCode govCXR = *carr;
    if (!getPaxTypeFareVec(fsdm.paxTypeFareVec(),
                           *fmiDiff)) // Retrieve PaxTypeFares for the given PaxType
    {
      return false;
    }
    // The Carrier Application For Industry YY Pricing Table shall be interrogated to determine if

    // the governing carrier of DIFF sector permits using industry fares (YY) in a primary pricing.
    Indicator prPricePr = LOWEST_FARE;

    if (!getIndustryPrimePricingPrecedence(diffDP, govCXR, gOver, fmi, fmiDiff, prPricePr))
      return false;

    const uint8_t MAX_NUM_CARRIERS = (prPricePr == PREFER_CXR_FARE) ? 2 : 1;

    // If we went through this process once and didn't find the LOW fare we should repeat it one
    // more time
    // and try to find the LOW fare among the Economy class fares...
    // only under condition that the through fare is the Business fare

    // fill fare selection data map structure
    fsdm.globalDirection() = fmiDiff->getGlobalDirection();
    fsdm.prPricePr() = prPricePr;
    fsdm.ilow() = false;
    fsdm.slideAllowed() = allowPremiumCabinSlide(*carr);
    fsdm.failedFareCabin() = failedFareCabin;
    fsdm.throughRealFTD() = throughFTD;
    diffDP.setSlideAllow() = fsdm.slideAllowed();

    bool cont = true; // It would be cleaner to use the goto operator instead
    while (cont)
    // for (size_t ilow = 0; cont && ilow < LOW_TIMES; ilow++, diffFTD.setEconomyClass())
    {
      govCXR = *carr; //  05/23/05
      // We'll try two times if the Carrier Application table holds value "C" == PREFER_CXR_FARE,
      // which means that Carrier fares take precedence over Industry fares.

      for (uint8_t iCnt = 0; cont && iCnt < MAX_NUM_CARRIERS; // Max of 2 iterations:
           iCnt++, govCXR = INDUSTRY_CARRIER) // the 1-st is for the real Governing carrier
      // and the 2-nd is for the Industry carrier ... only if needed
      {
        fsdm.govCXR() = govCXR;
        fsdm.throughFTD() = throughFTD;

        rv = processNormalFareSelection(fsdm);
        if (rv)
          cont = false;
      } // for loop ---> ict
      if (!cont)
        break;

      fsdm.ilow() = true; // lok for low fare
      cont = getLowerCabin(throughFTD); // go to lower cabin
      if (!fsdm.matchSlide(fsdm.throughFTD()))
        break;

    } // for loop ---> ilow
    if (fmi == fareMarket.begin())
      fsdm.getValue(fsdGl); // Assign initial value...
    else
    {
      // The logic below was not documented, but implemented based on the communications with
      // business analyst
      if (fsdm.foundLow() && fsdm.foundHigh()) // If both the low & high fares were found
      {
        if (!fsdGl.foundLow() || !fsdGl.foundHigh())
        {
          fsdm.getValue(fsdGl);
          continue;
        }
        fsdm.saveTempFSData(fsdGl);
      }
    }
  } // for loop ---> fmi, carr
  diffDP.fareLow() = fsdGl.fareLow();
  diffDP.amountFareClassLow() = fsdGl.amountFareClassLow();
  diffDP.fareClassLow() = fsdGl.fareClassLow();
  diffDP.fareHigh() = fsdGl.fareHigh();
  diffDP.amountFareClassHigh() = fsdGl.amountFareClassHigh();
  if(diffTrx()->isIataFareSelectionApplicable())
    diffDP.carrierDiff() = fsdGl.diffCxr();

  std::string fcHigh(fsdGl.fareClassHigh().c_str());
  if (fsdGl.fareHigh() && fsdGl.fareHigh()->fare()->isIndustry())
    fsdGl.fareHigh()->insertBookingCode(fcHigh);
  fsdGl.fareClassHigh() = fcHigh;

  diffDP.fareClassHigh() = fsdGl.fareClassHigh();

  if (!diffDP.cleanUpVectors().empty() && diffDP.fareMarket().size() > 1)
  {
    std::vector<CarrierCode>::iterator cc = diffDP.cleanUpVectors().begin();
    for (; cc != diffDP.cleanUpVectors().end(), diffDP.fareMarket().size() > 1; cc++)
      diffDP.finalizeCarrierAndFareMarketLists(*cc);
  }
  // code to return the passed solution when multiple FM are processed
  if(!rv && diffDP.amountFareClassLow() && diffDP.amountFareClassHigh())
    rv = true;

  return rv;
} // lint !e550

//**********************************************************************************
//  Assign money values for high/low Fare components
//**********************************************************************************
void
DifferentialValidator::setDiffLowHigh(FareSelectionData& diffDP, PaxTypeFare& df, FareDest fareDes)
    const

{
  std::string fareBasis = df.createFareBasis(*_diffTrx, false);
  // the length has been changed from 8 to 15 for the PL 16412  - JAL/AXESS
  if (fareBasis.size() > 15)
    fareBasis = fareBasis.substr(0, 14) + "*"; // Cross-of-lorraine?

  if (fareDes == LOW)
  {
    diffDP.fareLow() = &df;
    diffDP.amountFareClassLow() = df.nucFareAmount();
    diffDP.fareClassLow() = fareBasis; // df.fareClass  ();
  }
  else
  {
    diffDP.fareHigh() = &df;
    diffDP.amountFareClassHigh() = df.nucFareAmount();
    diffDP.fareClassHigh() = fareBasis; // df.fareClass  ();
  }
}

//**********************************************************************************
//  Find out if the fare satisfies the requirements imposed on it
//**********************************************************************************
bool
DifferentialValidator::checkDifferentialFare(const PaxTypeFare& df, DifferentialData& diffDP) const

{
  //-NR- The fares in Differential Fare Selection are based on the same Trip Type as the through
  //fare component...

  if (((_pricingUnit->puType() == PricingUnit::Type::ROUNDTRIP ||
        _pricingUnit->puType() == PricingUnit::Type::CIRCLETRIP ||
        _pricingUnit->puType() == PricingUnit::Type::OPENJAW) &&
       df.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) ||
      (_pricingUnit->puType() == PricingUnit::Type::ONEWAY && df.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) ||
      _pricingUnit->puType() == PricingUnit::Type::UNKNOWN)
    return false;

  //-NR- Only the fares of the Normal Fare Type designator are to be used in Differential Fare
  //Selection...

  if (!df.isNormal())
    return false;

  //-NR- Fares Selected for Differential calculation must pass ALL rule validations
  // (stopover, season, day of week, routing, global direction)

  if (!df.isValidNoBookingCode())
    return false;

  // Need to check the passed fare for NP (not processed) or SP (soft-pass) rule status, if so, then
  // send it to PricingRuleController to validate or re-validate the rule(s) for the fare
  // (part of IMAY105 deliverable)

  //-NR- If a Passenger Type is specified in pricing entry, the fares that containing such
  // passenger type or Blank may be selected for Differential calculations; e.g.,

  // Adult passenger type may match fares for Adult or Blank.  In the absence of a matching fare
  // for the specified Passenger Type, fares for ADT or Blank Passenger Type may be selected for
  // that
  // differential market.  If input passenger type is CNN, use fares for ADT or Blank if no CNN fare
  // is
  // published in the differential market...

  if (LIKELY(!requestedPaxType()->paxType().empty()))
  {

    PaxTypeCode rptc = requestedPaxType()->paxType();
    PaxTypeCode aptc = df.actualPaxType()->paxType();
    if ((rptc != aptc && !(rptc == CHILD && aptc == ADULT)) &&
        (rptc != aptc && aptc != ADULT && !aptc.empty()))
      return false;
  }

  //-NR- The directionality of the fares selected for Differentials must match the directionality of
  //the through
  // fare component.

  if ((_paxTfare->directionality() == TO && df.directionality() == FROM) ||
      (_paxTfare->directionality() == FROM && df.directionality() == TO))
    return false;

  if (!validateRules(df))
    return false;

  //-NR- Check that fare passes restrictions imposed by Cat 23...

  if (!validateMiscFareTag(df))
  {
    diffDP.setFailCat23() = true;
    return false;
  }

  return true;
}

bool
DifferentialValidator::validateHighFareForPremiumEconomyCabin(const DifferentialData& diffDP,
                                                              const PaxTypeFare& df) const
{
  if (isPremiumEconomyCabin(diffDP.intermFareType()))
  { // W or Z fareType
    bool intermFareTypeNotPerm = (diffDP.calcIntermIndicator() == NOT_PERMITTED);
    if (intermFareTypeNotPerm)
    {
      if (diffDP.intermFareType() == df.fcaFareType())
        return false;
      if (diffDP.intermFareType()[1] == Z_TYPE &&
          df.fcaFareType()[0] == Z_TYPE)
        return false;
      if (diffDP.intermFareType()[1] == W_TYPE &&
          df.fcaFareType()[0] == W_TYPE)
        return false;
    }
    else
    {  // permitted
      if (diffDP.intermFareType()[0] == ALL_TYPE)  // *generic
      {
        if (diffDP.intermFareType()[1] == Z_TYPE)
        {
          if (df.fcaFareType()[0] != Z_TYPE)
            return false;
        }
        if (diffDP.intermFareType()[1] == W_TYPE)
        {
          if (df.fcaFareType()[0] != W_TYPE)
            return false;
        }
      }
      else
      {
        if (diffDP.intermFareType() != df.fcaFareType())
          return false;
      }
    }
  }
  return true;
}

//**********************************************************************************
// Do validation of the consecutive Area 1 sectors for the given PaxType
//**********************************************************************************
// The differentials are stored for both round & one way trips in the fare regardless
// of the the fare itself. This dictates the need in processing both sequences separetly.
bool
DifferentialValidator::validateConsecutiveSectors(void)
{
  const std::vector<DifferentialData*>& differV = differential();

  bool ret = true;

  ret = validateConsecutiveSectorsForThroughFareType(differV);

  return ret;
}

//**********************************************************************************
// Do validation of the consecutive Area 1 sectors for the specific trip types
// (one way or round)
//**********************************************************************************
bool
DifferentialValidator::validateConsecutiveSectorsForThroughFareType(
    const DifferentialDataPtrVec& diffVforOneTripType) const
{
  PaxTypeFare& thruFare = *throughFare();
  // This process is applicable only to the Area 1, so it can not be multiple governing carriers for
  // it...
  FareMarket* mkt = thruFare.fareMarket();
  CarrierCode govCXR = mkt->governingCarrier();
  const CabinType throughFTD = thruFare.cabin();

  if (diffVforOneTripType.size() <=
      1) // No reason to continue since the max of 1 differential sectors are present
    return false;

  if (!throughFTD.isEconomyClass() && !throughFTD.isPremiumEconomyClass())
    return false; // ... 1 ...

  const AirSeg* psTravelSeg = dynamic_cast<const AirSeg*>(mkt->primarySector());
  if (!psTravelSeg)
  {
    LOG4CXX_ERROR(logger, "1) PRIMARY SECTOR IS NOT AN AIR SECTOR");
    return false;
  }

  const FareMarket* psFM = nullptr;

  // Find FM for the primary (governing) sector...
  std::vector<TravelSeg*> segOrderVec;

  segOrderVec.push_back(mkt->primarySector());

  const CarrierCode& carrier = getCarrierCode(*psTravelSeg, &segOrderVec);

  psFM = getFareMarket(segOrderVec, &carrier, &psTravelSeg->carrier());

  if (!psFM)
    return false;

  if (!compareCabins(*psFM, CABIN_FAMILY_BUSINESS))
    return false; // ... 1.1.1 ...

  // Check that there are more than 1 travel segment...
  if (mkt->travelSeg().size() <= 1)
    return false;

  TravelSegPtrVecIC tsBegin = mkt->travelSeg().begin();
  TravelSegPtrVecIC tsEnd = mkt->travelSeg().end();

  TravelSegPtrVecIC psTS =
      find(tsBegin, tsEnd, psTravelSeg); // Find the primary travel segment in the container

  tsEnd--;
  TravelSegPtrVecIC tsBusFareBegin = psTS;
  TravelSegPtrVecIC tsBusFareEnd = psTS;
  std::deque<const FareMarket*> fmConseq;
  std::deque<bool> govIndic;

  // The idea is to create one loop that will cover all 3 possible cases:
  // 1) when the primary segment is the first in the vector;
  // 2) when the primary segment is the last in the vector;
  // 3) when the primary segment is in the middle of the vector.
  // We also need to look around the primary segment, so if we have a position of primary sector
  // (PS),
  // we'll lookup at the elements that are on the right and on the left side of it moving from
  // closest
  // to furthest. It would be sequence like this:  (n/2) * (-1)^(n%2), where n=1,2,...
  // when "n" is even we at the right side, so we'll push_back element on the deque;
  // when "n" is odd we at the left side, so we'll push_front element on the deque.
  // We also need to check that we are in the boundaries.

  TravelSegPtrVecIC ts = psTS;
  bool leftBound = (ts == tsBegin) ? true : false;
  bool rightBound = (ts == tsEnd) ? true : false;
  bool gOver = true; // validate fareMarket vector in diff sector instead of fareMarketCurrent

  // Loop through travel segments

  for (int iCnt = 1, i = 0, odd = 0; !(leftBound && rightBound); iCnt++,
           odd = iCnt % 2,
           i = (iCnt / 2) * ((odd) ? -1 : 1)) // Add the primary segment to the deque
  {
    if (((odd) && leftBound) || (!(odd) && rightBound))
      continue;
    ts = psTS + i;
    if (odd && ts == tsBegin)
    {
      leftBound = true;
    }
    else if (!odd && ts == tsEnd)
    {
      rightBound = true;
    }
    const AirSeg* travelSeg = dynamic_cast<const AirSeg*>(*ts);
    if (!travelSeg) // Not an Air Segment... so the sequence was interrupted
    {
      if (odd)
        leftBound = true;
      else
        rightBound = true;
      continue;
    }
    if (travelSeg->origin()->area() != IATA_AREA1) // The sector should be within IATA Area 1
      continue;

    segOrderVec.clear();
    segOrderVec.push_back(*ts);

    const FareMarket* fm = nullptr;

    gOver = true;
    const CarrierCode& carrierTs =
        getCarrierCode(*travelSeg, &segOrderVec); // Command Pricing Gov Cxr Over

    fm = getFareMarket(segOrderVec, &carrierTs, &travelSeg->carrier(), &gOver);

    if (!fm)
    {
      LOG4CXX_ERROR(logger,
                    "2) FARE MARKET: " << segOrderVec.front()->boardMultiCity() << "- " << carrierTs
                                       << "-" << segOrderVec.back()->offMultiCity()
                                       << " NOT FOUND IN CONSECUTIVE SECTORS VALIDATION");
      return false;
    }

    if (ts != psTS && // No following checks are necessary for the primary sector
        ((compareFareMarkets(*fm, diffVforOneTripType, gOver) ==
          nullptr) || // Check that FM is in the Differential vector
         //... and
         fm->governingCarrier() != govCXR || // it should be the same carrier ... and
         !compareCabins(*fm, CABIN_FIRST))) // the sector should be booked in First Fare Type
      continue;

    if (!dynamic_cast<const AirSeg*>(*ts))
      continue; // Exclude Surface Segments

    if (odd)
    {
      fmConseq.push_front(fm);
      govIndic.push_front(gOver);
      tsBusFareBegin = ts;
    }
    else
    {
      fmConseq.push_back(fm);
      govIndic.push_back(gOver);

      tsBusFareEnd = ts;
    }
  }

  if (fmConseq.size() <=
      1) // Should be at least 2 elements in container: primary sector and some other sector
    return false;
  {
    const AirSeg* travelSegB = dynamic_cast<const AirSeg*>(*tsBusFareBegin);

    segOrderVec.clear();

    for (; tsBusFareBegin != tsBusFareEnd; tsBusFareBegin++)
    {
      segOrderVec.push_back(*tsBusFareBegin);
    }
    if (tsBusFareBegin == tsBusFareEnd)
      segOrderVec.push_back(*tsBusFareBegin); // put last travel seg in the vector

    const FareMarket* fm = nullptr;
    const CarrierCode& carrierTsB =
        getCarrierCode(*travelSegB, &segOrderVec); // Command Pricing Gov Cxr Over

    fm = getFareMarket(segOrderVec, &carrierTsB, &travelSegB->carrier());

    if (!fm || fm == thruFare.fareMarket())
      return false;

    // Find corresponding Fare of the (Premium) Business Fare Type for a new Fare Market
    bool validated = false;
    PaxTypeFarePtrVec paxTypeFareVec;

    getPaxTypeFareVec(paxTypeFareVec, *fm); // Retrieve PaxTypeFares for the given PaxType
    for (PaxTypeFarePtrVecIC bf = paxTypeFareVec.begin(); bf != paxTypeFareVec.end(); bf++)
    {
      if (!(*bf))
        continue;
      PaxTypeFare& bfr = **bf;
      if (bfr.carrier() != govCXR || // Choose only fares with the Governing Carrier code
          !(bfr.cabin().isBusinessClass() || bfr.cabin().isPremiumBusinessClass()) ||
          !isBkgCodeStatusPass(bfr))
        continue;

      // The First Type Fares for the sectors of a Business through Fare should had passed RBD
      // validation also
      std::deque<const FareMarket*>::const_iterator fmi = fmConseq.begin();
      for (; fmi != fmConseq.end(); fmi++)
      {
        if (!(*fmi) || (*fmi) == psFM) // skip primary sector
          continue;
        PaxTypeFarePtrVec paxTypeFareVec1;

        getPaxTypeFareVec(paxTypeFareVec1, **fmi); // Retrieve PaxTypeFares for the given PaxType

        for (PaxTypeFarePtrVecIC ff = paxTypeFareVec1.begin(); ff != paxTypeFareVec1.end(); ff++)
        {
          if (!(*ff))
            continue;

          PaxTypeFare& ffr = **ff;
          if (ffr.cabin().isFirstClass() && isBkgCodeStatusPass(ffr))
          {
            validated = true;
            break;
          }
        }
        if (!validated)
          return false; // None of the First Type Fares for a market passed RBD validation
      }
    }

    if (!validated)
      return false; // No First Type Fares were found
    // Do a real work RIGHT here...
    {
      // Loop through all consecutive sectors found early:
      std::deque<const FareMarket*>::const_iterator fmi = fmConseq.begin();
      std::deque<bool>::const_iterator gOveri = govIndic.begin();

      DifferentialData* diff = nullptr;
      for (; fmi != fmConseq.end(); fmi++, gOveri++)
      {
        if (!(*fmi) || (*fmi) == psFM)
          continue;
        // NOTE: skip primary sector in this loop
        diff = compareFareMarkets(**fmi, diffVforOneTripType, *gOveri);

        if (diff == nullptr)
          continue;

        if (compareCabins(**fmi, CABIN_BUSINESS))
          diff->tag() += std::string(1, DifferentialData::BUSINESS_FTD);
        else if (compareCabins(**fmi, CABIN_PREMIUM_BUSINESS))
        {
          diff->tag() += std::string(1, DifferentialData::PREMIUM_BUSINESS_FTD_NEW);
        }
      }
    }
  }
  return true;
}

//**********************************************************************************
// Compare fare type designator of the primary sector to the value passed into the
// method through "compCabin" paramater
//**********************************************************************************
bool
DifferentialValidator::compareCabins(const FareMarket& mkt, const TypeOfCabin& compCabin) const
{
  CabinType segCabin;
  const AirSeg* psTravelSeg = dynamic_cast<const AirSeg*>(mkt.primarySector());
  if (!psTravelSeg)
    return false;

  if (mkt.classOfServiceVec().empty())
    return false; // no elements found in container...
  ClassOfServicePrtVecIC iterCOS = mkt.classOfServiceVec().begin();

  if (!*iterCOS)

    return false; // NULL pointer
  COSInnerPtrVecIC classOfS = (*iterCOS)->begin();

  for (; classOfS != (*iterCOS)->end(); classOfS++)
  {
    if (psTravelSeg->getBookingCode() == (*classOfS)->bookingCode()) // Compare 2 strings

    {
      segCabin = (*classOfS)->cabin();
      switch (compCabin)
      {
      case CABIN_PREMIUM_FIRST:
        return segCabin.isPremiumFirstClass();
      case CABIN_FIRST:
        return segCabin.isFirstClass();
      case CABIN_PREMIUM_BUSINESS:
        return segCabin.isPremiumBusinessClass();
      case CABIN_BUSINESS:
        return segCabin.isBusinessClass();
      case CABIN_PREMIUM_ECONOMY:
        return segCabin.isPremiumEconomyClass();
      case CABIN_ECONOMY:
        return segCabin.isEconomyClass();
      case CABIN_FAMILY_FIRST:
        return segCabin.isFirstClass() || segCabin.isPremiumFirstClass();
      case CABIN_FAMILY_BUSINESS:
        return segCabin.isBusinessClass() || segCabin.isPremiumBusinessClass();
      case CABIN_FAMILY_ECONOMY:
        return segCabin.isEconomyClass() || segCabin.isPremiumEconomyClass();
      }
      return false;
    }
  }
  return false;
}

//*******************************************************************************************
// Look through the Differential Vector in the PaxTypeFare to find the  Fare Market pointer.
//********************************************************************************************
DifferentialData*
DifferentialValidator::compareFareMarkets(const FareMarket& mkt,
                                          const DifferentialDataPtrVec& diffVec,
                                          const bool indic) const
{
  DifferentialDataPtrVecIC diffItem = diffVec.begin();
  for (; diffItem != diffVec.end(); diffItem++)
  {
    if (!*diffItem)
      continue;
    DifferentialData& di = **diffItem;
    if (indic)
    {
      std::vector<FareMarket*>::iterator ifm =
          find(di.fareMarket().begin(), di.fareMarket().end(), &mkt);
      if (ifm == (*diffItem)->fareMarket().end())
        continue;
    }
    else
    {
      std::vector<FareMarket*>::iterator ifm =
          find(di.fareMarketCurrent().begin(), di.fareMarketCurrent().end(), &mkt);

      if (ifm == (*diffItem)->fareMarketCurrent().end())
        continue;
    }
    return (*diffItem);
  }
  return nullptr;
}

//*******************************************************************************************
// Return true if both the differential and the through fares are of the same type
// (restricted or unrestricted).
//*******************************************************************************************
bool
DifferentialValidator::checkFareType(const FareType& throughFT,
                                     const PaxTypeFare& diffFare,
                                     bool ignoreFTCheck) const
{
  if (ignoreFTCheck)
    return true;
  bool ret = false;

  if (diffFare.fcaFareType().length() > 1 && throughFT.length() > 1)
  {
    ret = (diffFare.fcaFareType()[1] == throughFT[1]) ? true : false;
  }

  if (!ret)
  {
    ret = RuleUtil::matchFareType(diffFare.fcaFareType(), throughFT);
  }
  return ret;

  //    return  RuleUtil::matchFareType (diffFare.fcaFareType(),throughFT);
}

//*******************************************************************************************
// A virtual method serves as a wrapper around real getIndustryPricingAppl method. Can be
// overriden in CPP UNIT Test.
//*******************************************************************************************
const std::vector<const IndustryPricingAppl*>&
DifferentialValidator::getIndustryPricingAppl(const CarrierCode& carrier,
                                              const GlobalDirection& globalDir,
                                              const DateTime& date) const
{
  return diffTrx()->dataHandle().getIndustryPricingAppl(carrier, globalDir, date);
}

//*******************************************************************************************
//    Consolidate consecutive differential sectors of the same Fare Type Designators.
//*******************************************************************************************
bool
DifferentialValidator::consolidateDifferentials(std::vector<DifferentialData*>& diffLforOneTripType,
                                                const std::vector<Differentials*>& differList)
{
  if (diffLforOneTripType.size() <=
      1) // No reason to continue since the max of 1 differential sectors are present
    return true;
  if (_mkt->travelSeg().size() <= 1) // Check that there are more than 1 travel segment...
    return true;

  bool consecFound = false;
  bool failDiffSec = false;

  unsigned int jmLeftItem = 0;
  unsigned int jmRightItem = 0;

  DifferentialData* diffItem = nullptr;

  DifferentialData* diffItemSaved = nullptr;

  DifferentialValidator::ReturnValues rv = SUCCEEDED;

  // Consecutive sectors should be processed first...

  for (; diffLforOneTripType.size() >= 2 && jmLeftItem < diffLforOneTripType.size(); ++jmLeftItem)
  {
    jmRightItem = jmLeftItem;
    if (!consecutiveSectorsFoundInSequence(diffLforOneTripType, jmLeftItem, consecFound = false))
      return false; // Fatal error

    if (consecFound)
    {
      std::vector<DifferentialData*> diffConsec;

      for (; jmLeftItem >= 0 && jmRightItem < diffLforOneTripType.size(); ++jmRightItem)
      {
        if (diffLforOneTripType[jmLeftItem]->tag()[1] != diffLforOneTripType[jmRightItem]->tag()[1])
          break;
        if (diffLforOneTripType[jmRightItem]->tag().length() == CONSEC_SECTOR)
        {
          diffLforOneTripType[jmRightItem]->tag().erase(
              CONSEC_SECTOR - 1, 1); // Remove the last character in the tag, 2AFB will be 2AF
        }
        diffConsec.push_back(diffLforOneTripType[jmRightItem]);
      }

      unsigned int jmLBItem = jmLeftItem;
      unsigned int jmRBItem = --jmRightItem;

      jmRightItem = jmLeftItem;
      for (++jmRightItem; jmLeftItem >= 0 && diffLforOneTripType.size() >= 2 &&
                              jmRightItem < diffLforOneTripType.size();
           ++jmRightItem)
      {
        rv = consolidateTwoDifferentialSectors(diffLforOneTripType,
                                               differList,
                                               jmLeftItem,
                                               jmRightItem,
                                               diffItem = nullptr,
                                               true,
                                               consecFound);

        if (rv == FATAL_ERROR)
          return false; // Fatal error
        else if (rv == STOP_PROCESSING)
          return true; // No more consolidation allowed
        if (!diffItem)
          continue;

        // Vector of differential sectors creates the Consolidate Differential
        std::vector<DifferentialData*>& diffConsol = diffItem->inConsolDiffData();
        std::vector<DifferentialData*>::const_iterator id = diffConsec.begin();

        for (; id != diffConsec.end(); ++id)
        {
          if (((*id)->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
              ((*id)->status() == DifferentialData::SC_CONSOLIDATED_FAIL))
          {
            diffItem->setConsolidate() = true;
          }
          diffConsol.push_back(*id);
        }

        if (rv == END_OF_SEQUENCE)
        {
          failDiffSec = false;
          MoneyAmount moneySum = 0.f;
          MoneyAmount moneyConsolidate =
              (diffItem->hipAmount() != 0.f) ? diffItem->hipAmount() : diffItem->amount();

          id = diffConsec.begin();
          for (; id != diffConsec.end(); ++id)
          {
            moneySum += (((*id)->hipAmount() != 0.f) ? (*id)->hipAmount() : (*id)->amount());

            if (((*id)->status() != DifferentialData::SC_CONSOLIDATED_PASS) &&
                ((*id)->status() != DifferentialData::SC_PASSED))
              failDiffSec = true;
          }

          if (moneyConsolidate >= moneySum && !failDiffSec)
          {
            if (diffItem)
            {
              std::vector<DifferentialData*>& differV = differential();
              diffItem->status() = DifferentialData::SC_CONSOLIDATED_FAIL;
              differV.push_back(diffItem); // Add a new Fare to a Differential container
              continue;
            }
          }
          for (id = diffConsec.begin(); id != diffConsec.end(); ++id)
          {
            (*id)->status() = DifferentialData::SC_FAILED;
          }

          std::vector<DifferentialData*>& differV = differential();
          if (diffItemSaved)
          {
            diffItemSaved->status() = DifferentialData::SC_CONSOLIDATED_PASS;
            differV.push_back(diffItemSaved); // Add a new Fare to a Differential container
          }
          break;
        }
        else if (rv == FAILED)
        {
          if (diffItem)
          {
            std::vector<DifferentialData*>& differV = differential();
            diffItem->status() = DifferentialData::SC_CONSOLIDATED_FAIL;
            differV.push_back(diffItem); // Add a new Fare to a Differential container
          }
          // Restore diffLforOneTripType LIST to its original state
          std::vector<DifferentialData*>::iterator jmLB = diffLforOneTripType.begin();
          std::vector<DifferentialData*>::iterator jmRB = diffLforOneTripType.begin();

          if (jmLBItem < diffLforOneTripType.size() && jmRBItem < diffLforOneTripType.size() &&
              jmRBItem >= 0)
          {
            jmLB += jmLBItem;
            jmRB += jmRBItem;
          }
          std::vector<DifferentialData*>::iterator jm = diffLforOneTripType.erase(jmLB, jmRB);
          diffLforOneTripType.insert(jm, diffConsec.begin(), diffConsec.end() - 1);

          break;
        }
        else // rv == SUCCEEDED
        {
          diffItemSaved = diffItem;
        }
      }
    }
    else
      break;
  }
  // --NR--
  //  It first tries to consolidate DIFF 1 through 2, if less than 1+2, DIFF 1 through 2 now becomes
  //  DIFF 1 and all
  //  remaining  DIFF sectors are renumbered.  This process is  repeated until  DIFF 1  through 2
  //  is not less than
  //  DIFF 1+2.  Then a consolidation of DIFF 1 through 3, until DIFF 1 through 3 is not less than
  //  DIFF 1+2+3, then
  //  the consolidation of DIFF 1 through 4 is tried.  If less than DIFF 1+2+3+4,  the process is
  //  repeated until it
  //  is no longer less  expensive.
  //  Next attempt is  DIFF 2 through 3,  followed by 2 through 4, 2 through 5, etc.  This process
  //  will be repeated
  //  until all DIFF sectors have been processed.

  // TRACE("");

  unsigned int jmLeftItemKeep = 0;
  jmLeftItem = 0;

  for (; diffLforOneTripType.size() >= 2 && jmLeftItem < diffLforOneTripType.size(); ++jmLeftItem)
  {
    jmLeftItemKeep = jmLeftItem;
    std::vector<DifferentialData*> consCandidates;

    jmRightItem = jmLeftItem;
    for (++jmRightItem; jmRightItem >= 0 && diffLforOneTripType.size() >= 2 &&
                            jmRightItem < diffLforOneTripType.size();
         ++jmRightItem)
    {
      rv = consolidateTwoDifferentialSectors(
          diffLforOneTripType, differList, jmLeftItem, jmRightItem, diffItem = nullptr, true);

      if (rv == FATAL_ERROR)
        return false; // Fatal error
      else if (rv == STOP_PROCESSING)

        return true; // No more consolidation allowed

      if (rv == END_OF_SEQUENCE)
      {
        jmLeftItem = jmLeftItemKeep;
        break;
      }

      if (!diffItem)
        continue;

      if (rv == FAILED)
      {
        if (diffItem)

        {
          std::vector<DifferentialData*>& differV = differential();
          diffItem->status() = DifferentialData::SC_CONSOLIDATED_FAIL;

          differV.push_back(diffItem); // Add a new Fare to a Differential container

          if (consCandidates.empty())
            consCandidates.push_back(diffLforOneTripType[jmLeftItem]);

          consCandidates.push_back(diffLforOneTripType[jmRightItem]);

          // Vector of differential sectors creates the Consolidate Differential
          std::vector<DifferentialData*>& diffConsol = diffItem->inConsolDiffData();
          std::vector<DifferentialData*>::iterator id = consCandidates.begin();
          std::vector<DifferentialData*>::iterator idE = consCandidates.end();
          diffConsol.clear();
          std::vector<DifferentialData*> consTwoCandidates;
          consTwoCandidates.clear();
          if (consCandidates.size() == 0)
          {
            consTwoCandidates.push_back(diffLforOneTripType[jmLeftItem]);
            consTwoCandidates.push_back(diffLforOneTripType[jmRightItem]);
            id = consTwoCandidates.begin();
            idE = consTwoCandidates.end();
          }
          for (; id != idE; ++id)
          {
            if (((*id)->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
                ((*id)->status() == DifferentialData::SC_CONSOLIDATED_FAIL))
            {
              diffItem->setConsolidate() = true;
            }
            diffConsol.push_back(*id);
          }
        }
        continue;
      }
      else // rv == SUCCEEDED
      {
        if (consCandidates.empty())
        {
          consCandidates.push_back(diffLforOneTripType[jmLeftItem]);
        }
        consCandidates.push_back(diffLforOneTripType[jmRightItem]);
        MoneyAmount moneySum = 0.f;
        MoneyAmount moneyConsolidate =
            (diffItem->hipAmount() != 0.f) ? diffItem->hipAmount() : diffItem->amount();

        // if failDiffSec is true then do not do amounts comparision
        failDiffSec = false;

        if (diffItem)
        {
          // Vector of differential sectors creates the Consolidate Differential
          std::vector<DifferentialData*>& diffConsol = diffItem->inConsolDiffData();
          std::vector<DifferentialData*>::iterator id = consCandidates.begin();
          diffConsol.clear();
          for (; id != consCandidates.end(); ++id)
          {
            if (((*id)->status() != DifferentialData::SC_CONSOLIDATED_PASS) &&
                ((*id)->status() != DifferentialData::SC_PASSED))
              failDiffSec = true;

            if (((*id)->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
                ((*id)->status() == DifferentialData::SC_CONSOLIDATED_FAIL))
            {
              diffItem->setConsolidate() = true;
            }
            diffConsol.push_back(*id);
          }
        }

        if (moneySum == 0.f)
        {
          std::vector<DifferentialData*>::iterator id = consCandidates.begin();
          for (; id != consCandidates.end(); ++id)
          {
            DifferentialData& dd = **id;
            moneySum +=
                ((dd.hipAmount() != 0.f) ? dd.hipAmount() : dd.amount());
          }
        }

        if (moneyConsolidate >= moneySum && !failDiffSec)
        {
          if (diffItem)
          {
            std::vector<DifferentialData*>& differV = differential();
            diffItem->status() = DifferentialData::SC_CONSOLIDATED_FAIL;
            differV.push_back(diffItem); // Add a new Fare to a Differential container
            continue;
          }
        }

        std::vector<DifferentialData*>::iterator id = consCandidates.begin();
        for (; id != consCandidates.end(); ++id)
        {
          if (diffItem->status() == DifferentialData::SC_CONSOLIDATED_FAIL) // 06-28-05
            continue; //
          if (diffItem->status() == DifferentialData::SC_CONSOLIDATED_PASS) //
          { //
            (*id)->status() = DifferentialData::SC_CONSOLIDATED_FAIL; //

            continue; //
          } // 06-28-05
          (*id)->status() = DifferentialData::SC_FAILED;
        }

        if (diffLforOneTripType.size() < consCandidates.size())
        {
          LOG4CXX_ERROR(logger,
                        "Leaving DifferentialValidator::consolidateDifferentials(): wrong size");
          return false;
        }

        std::vector<DifferentialData*>::iterator pDf = diffLforOneTripType.begin();
        for (id = consCandidates.begin(), ++id; id != consCandidates.end(); ++id)
        {
          if ((*id)->status() == DifferentialData::SC_CONSOLIDATED_FAIL ||
              (*id)->status() == DifferentialData::SC_CONSOLIDATED_PASS)
            continue;

          for (pDf = diffLforOneTripType.begin(); pDf != diffLforOneTripType.end(); ++pDf)
          {
            if (*id == *pDf)
            {
              diffLforOneTripType.erase(pDf);
              break;
            }
          }
        }

        consCandidates.clear();
        diffLforOneTripType[jmLeftItem] = diffItem;
        consCandidates.push_back(diffItem);
        if (diffItem)
        {
          std::vector<DifferentialData*>& differV = differential();
          diffItem->status() = DifferentialData::SC_CONSOLIDATED_PASS;
          differV.push_back(diffItem); // Add a new Fare to a Differential container
          {
            --jmRightItem;
            continue;
          }
        }
      }
    }
    if (jmRightItem >= diffLforOneTripType.size() && (jmLeftItem + 1) == jmRightItem)
    {
      break;
    }
  }

  // --NR--
  // 10. Once an  attempt has been made to consolidate all  differentials tagged with the
  //    same letter sequence, and additional consolidation attempt will be made starting
  //    with the  board point of the first  sequence of Differential candidates, i.e. 1A
  //    through the off point of the second sequence of Differential candidates, i.e. 3B.
  //    This process will be repeated for sequences, i.e. 1C  through 4D and 1E  through
  //    2F, etc. if they exist.

  jmLeftItem = 0;
  for (; diffLforOneTripType.size() >= 2 && jmLeftItem < diffLforOneTripType.size(); ++jmLeftItem)
  {
    jmRightItem = diffLforOneTripType.size(); // Declare & initialize...
    for (; diffLforOneTripType.size() >= 2;)
    {
      std::vector<DifferentialData*>::iterator jmRight = diffLforOneTripType.begin();

      jmRightItem = findEndElementInNextSequence(jmLeftItem, diffLforOneTripType);
      if (jmRightItem >= diffLforOneTripType.size())
        return true;

      std::vector<std::vector<DifferentialData*>::iterator> consCandidates;
      std::vector<DifferentialData*>::iterator jmLB = diffLforOneTripType.begin();
      std::vector<DifferentialData*>::iterator jmRB = diffLforOneTripType.begin();

      if (jmLeftItem < diffLforOneTripType.size() && jmRightItem < diffLforOneTripType.size())
      {
        jmLB += jmLeftItem;
        jmRB += jmRightItem;
      }

      for (std::vector<DifferentialData*>::iterator id = jmLB;; ++id)
      {
        consCandidates.push_back(id);
        if (id == jmRB)
          break;
      }

      rv = consolidateTwoDifferentialSectors(
          diffLforOneTripType, differList, jmLeftItem, jmRightItem, diffItem = nullptr, false);

      if (rv == FATAL_ERROR)
        return false; // Fatal error
      else if (rv == STOP_PROCESSING)
        return true; // No more consolidation allowed
      if (!diffItem)
        continue;

      if (diffItem)
      {
        // Vector of differential sectors creates the Consolidate Differential
        std::vector<DifferentialData*>& diffConsol = diffItem->inConsolDiffData();
        std::vector<std::vector<DifferentialData*>::iterator>::iterator id = consCandidates.begin();
        diffConsol.clear();

        for (; id != consCandidates.end(); ++id)
        {
          if (((**id)->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
              ((**id)->status() == DifferentialData::SC_CONSOLIDATED_FAIL))
          {
            diffItem->setConsolidate() = true;
          }
          diffConsol.push_back(**id);
        }
      }

      if (rv == FAILED)
      {
        if (diffItem)
        {
          std::vector<DifferentialData*>& differV = differential();
          diffItem->status() = DifferentialData::SC_CONSOLIDATED_FAIL;
          differV.push_back(diffItem); // Add a new Fare to a Differential container
        }
        consCandidates.clear();
        jmLeftItem = jmRightItem;
        break;
      }
      else // rv == SUCCEEDED
      {
        failDiffSec = false;

        MoneyAmount moneySum = 0.f;
        MoneyAmount moneyConsolidate =
            (diffItem->hipAmount() != 0.f) ? diffItem->hipAmount() : diffItem->amount();
        if (moneySum == 0.f)
        {
          std::vector<std::vector<DifferentialData*>::iterator>::iterator id = consCandidates.begin();
          for (; id != consCandidates.end(); ++id)
          {
            DifferentialData& dd = ***id;
            moneySum +=
                ((dd.hipAmount() != 0.f) ? dd.hipAmount() : dd.amount());

            if ((dd.status() != DifferentialData::SC_CONSOLIDATED_PASS) &&
                (dd.status() != DifferentialData::SC_PASSED))
              failDiffSec = true;
          }
        }

        if (moneyConsolidate >= moneySum && !failDiffSec)
        {
          consCandidates.clear();
          jmLeftItem = jmRightItem;

          if (diffItem)
          {
            std::vector<DifferentialData*>& differV = differential();
            diffItem->status() = DifferentialData::SC_CONSOLIDATED_FAIL;
            differV.push_back(diffItem); // Add a new Fare to a Differential container
            continue;
          }
        }

        int consCandidateSize = 0;
        std::vector<std::vector<DifferentialData*>::iterator>::iterator id = consCandidates.begin();
        for (; id != consCandidates.end(); ++id)
        {
          consCandidateSize++;
          (**id)->status() = DifferentialData::SC_FAILED;
        }

        diffLforOneTripType[jmLeftItem] = diffItem;

        // LOOP through the vector backwards because if we do it in the forwards direction the
        // iterators are invalidated by
        //      the deletion of the elements earlier in the other vector.
        // NOTE:  The fact that we have to concern ourselves about this indicates there is a basic
        // problem in this code.
        //        it is too easy to "shoot yourself in the foot (or elsewhere)".  The vector of
        //        iterators should be examined
        //        and replaced with another mechanism (vector of pointers?).  The change below was
        //        made because it is late and
        //        this change is limited in scope to a few lines.

        if (consCandidateSize > 1)
        {
          bool jmRightWasSet = false;
          for (id = consCandidates.end() - 1; consCandidateSize > 1;
               consCandidateSize--) // Don't erase the jmLeft... (++id)
          {
            std::vector<DifferentialData*>::iterator iter = *id;
            --id;
            if (jmRightWasSet)
            {
              diffLforOneTripType.erase(iter);
            }
            else
            {
              jmRight = diffLforOneTripType.erase(iter);
              jmRightWasSet = true;
            }
          }
        }

        consCandidates.clear();
        if (diffItem)
        {
          std::vector<DifferentialData*>& differV = differential();
          diffItem->status() = DifferentialData::SC_CONSOLIDATED_PASS;
          differV.push_back(diffItem); // Add a new Fare to a Differential container
        }
      }
      if (jmRightItem >= diffLforOneTripType.size() || jmRight == diffLforOneTripType.end())
        return true;
    }
  }
  return true;
}

//*******************************************************************************************
//    Return iterator pointed at the last element of the sequence next to the current.
//*******************************************************************************************

unsigned int
DifferentialValidator::findEndElementInNextSequence(
    unsigned int& jmBegin, const std::vector<DifferentialData*>& diffLforOneTripType) const

{
  unsigned int jmLeftItem = jmBegin;
  unsigned int jmRightItem = diffLforOneTripType.size();
  Indicator tagL = Indicator(0);
  Indicator tagR = Indicator(0);

  for (; jmLeftItem < diffLforOneTripType.size() && (tagR - tagL) <= 1; ++jmLeftItem)
  {
    if (diffLforOneTripType[jmLeftItem]->tag().length() != REGULAR_SECTOR)
    {
      LOG4CXX_ERROR(logger, "10) INCORRECT TAG LENGTH IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
      return diffLforOneTripType.size();
    }
    tagR = diffLforOneTripType[jmLeftItem]->tag()[1];
    if (!tagL)
      tagL = tagR;
    if ((tagR - tagL) == 1)
    {
      jmRightItem = jmLeftItem;
    }
  }
  return jmRightItem;
}

//*******************************************************************************************
//                Here the two sectors are consolidated in the one.
//*******************************************************************************************
DifferentialValidator::ReturnValues
DifferentialValidator::consolidateTwoDifferentialSectors(
    const std::vector<DifferentialData*>& diffLforOneTripType,
    const std::vector<Differentials*>& differList,
    unsigned int& jmLeftItem,
    unsigned int& jmRightItem,
    DifferentialData*& diffItem,
    const bool& theSameSequence,
    const bool& consecFound)
{
  const Itin& itin = *itinerary();
  Indicator tagL = Indicator(0);
  Indicator tagR = Indicator(0);
  bool retVal = false;
  DifferentialData* diffLeft = nullptr;
  DifferentialData* diffRight = nullptr;
  const FareMarket* fm = nullptr;
  DifferentialData::FareTypeDesignators tagCombined(DifferentialData::BLANK_FTD);

  diffLeft = diffLforOneTripType[jmLeftItem];
  diffRight = diffLforOneTripType[jmRightItem];

  if (!diffLeft || !diffRight)
  {
    LOG4CXX_ERROR(logger, "3) DIFF DATA HAS NULL PNTR IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
    return FATAL_ERROR;
  }

  if (diffLeft->status() == DifferentialData::SC_ADJACENT_FAIL ||
      diffRight->status() == DifferentialData::SC_ADJACENT_FAIL)

    return STOP_PROCESSING;

  // Check that jmRight is not pointing past the vector limits... If it is, we done.

  if (jmRightItem >= diffLforOneTripType.size())
  {
    return STOP_PROCESSING;
  }

  // The letters in the 2-nd position of the diffTag string of the two Diff Sectors should be the
  // same,
  // otherwise break the sequence and start over for the new sequence and continue till the end is
  // reached:

  if (theSameSequence && diffLeft->tag()[1] != diffRight->tag()[1])
  {
    jmLeftItem = jmRightItem;

    return END_OF_SEQUENCE;
  }
  if (diffLeft->fareMarket().empty() || diffRight->fareMarket().empty())
  {
    LOG4CXX_ERROR(
        logger,
        "4) DIFF DATA HAS EMPTY FARE MARKET VECTOR IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
    return FATAL_ERROR;
  }

  FareMarket* fmL = *(diffLeft->fareMarket().begin());
  FareMarket* fmR = *(diffRight->fareMarket().begin());

  if (!fmL || !fmR)
  {
    LOG4CXX_ERROR(
        logger,
        "4) DIFF DATA HAS EMPTY FARE MARKET VECTOR IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
    return FATAL_ERROR;
  }
  std::string::size_type ltSize = diffLeft->tag().length();

  std::string::size_type rtSize = diffRight->tag().length();

  if ((ltSize != REGULAR_SECTOR && ltSize != CONSEC_SECTOR) ||
      (rtSize != REGULAR_SECTOR && rtSize != CONSEC_SECTOR))
  {
    LOG4CXX_ERROR(logger, "5) INCORRECT TAG LENGTH IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
    return FATAL_ERROR;
  }
  tagL = _toupper(diffLeft->tag()[ltSize - 1]);
  tagR = _toupper(diffRight->tag()[rtSize - 1]);

  if (!checkTagFareType(tagL) || !checkTagFareType(tagR)) // Check that tags are valid
  {
    LOG4CXX_ERROR(logger, "6) INCORRECT TAG IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
    return FATAL_ERROR;
  }
  DifferentialData::FareTypeDesignators tl =
      static_cast<DifferentialData::FareTypeDesignators>(tagL);
  DifferentialData::FareTypeDesignators tr =
      static_cast<DifferentialData::FareTypeDesignators>(tagR);

  tagCombined = findCombinedFareType(tl, tr, consecFound);
  { // Do a real work RIGHT here...
    uint16_t startSegNum = _itin->segmentOrder(fmL->travelSeg().front());
    uint16_t endSegNum = _itin->segmentOrder(fmR->travelSeg().back());

    std::vector<TravelSeg*> trvlSeg;
    std::vector<TravelSeg*>::const_iterator itinTrvlSeg = itin.travelSeg().begin();

    const AirSeg* airSeg = nullptr;
    const AirSeg* airSegFirst = nullptr;
    bool sameCarrier =
        true; // True if it is the same carrier on all segments of the differential sector

    for (bool found = false; itinTrvlSeg != itin.travelSeg().end(); itinTrvlSeg++)
    {
      if (!found && _itin->segmentOrder(*itinTrvlSeg) != startSegNum)
        continue;
      found = true;
      trvlSeg.push_back(*itinTrvlSeg);
      airSeg = dynamic_cast<const AirSeg*>(*itinTrvlSeg);
      if (airSeg)
      {
        if (!airSegFirst)
          airSegFirst = airSeg;
        if (sameCarrier && (getCarrierCode(*airSeg, nullptr) != getCarrierCode(*airSegFirst, nullptr)))
          sameCarrier = false;
      }
      if (_itin->segmentOrder(*itinTrvlSeg) == endSegNum)
        break;
    }
    if (trvlSeg.empty())
    {
      LOG4CXX_ERROR(
          logger,
          "10) NO TRAVEL SEGMENTS FOUND FOR THE ITINERY IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
      return FATAL_ERROR;
    }

    //  Find governing carrier(s) for a new fare market:
    //  At first check for the Governing Carrier Override for YY,
    //  second, if not YY check for Gov Cxr Override
    //  third, if none of the above gets through Governing CXR

    std::set<CarrierCode> govCXRs;
    std::vector<CarrierCode> govCXR; // for the Governing carrier Override methods

    if (FareMarketUtil::isYYOverride(*_diffTrx, trvlSeg)) // Command Pricing Gov Cxr Over
    {
      govCXRs.insert(INDUSTRY_CARRIER);
    }
    else
    {
      FareMarketUtil::getGovCxrOverride(*_diffTrx, trvlSeg, govCXR);
      GoverningCarrier govCxrUtil(_diffTrx);
      govCxrUtil.getGoverningCarrier(trvlSeg, govCXRs, _mkt->direction());
    }
    if (govCXRs.empty())
    {
      LOG4CXX_ERROR(logger,
                    "11) NO GOVERNING CARRIER FOUND IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
      return FATAL_ERROR;
    }
    diffTrx()->dataHandle().get(diffItem); // Create a new Differential object
    if (!diffItem)
    {
      LOG4CXX_ERROR(logger,
                    "8) DATA HANDLE RETURNS NULL PNTR IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
      return FATAL_ERROR;
    }

    for (const auto& govCXR : govCXRs)
    {
      fm = getFareMarket(trvlSeg, &govCXR);

      if (!fm)
      {
        LOG4CXX_ERROR(logger,
                      "7) FARE MARKET: " << trvlSeg.front()->boardMultiCity() << "-" << govCXR
                                         << "-" << trvlSeg.back()->offMultiCity()
                                         << " NOT FOUND IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
        return FATAL_ERROR;
      }

      if (fm == _mkt) // Differential sector can't be as big as a whole trip.
        return STOP_PROCESSING;

      diffItem->fareMarketCurrent().push_back(const_cast<FareMarket*>(fm));
      diffItem->carrierCurrent().push_back(govCXR);
    }
    for (const auto& carrierCode : govCXR)
    {
      fm = getFareMarket(trvlSeg, &carrierCode);
      if (!fm)
      {
        LOG4CXX_ERROR(logger,
                      "7) FARE MARKET: " << trvlSeg.front()->boardMultiCity() << "- OVERRIDE "
                                         << carrierCode << "-" << trvlSeg.back()->offMultiCity()
                                         << " NOT FOUND IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
        return FATAL_ERROR;
      }

      if (fm == _mkt) // Differential sector can't be as big as a whole trip.
        return STOP_PROCESSING;

      diffItem->fareMarket().push_back(const_cast<FareMarket*>(fm));
      diffItem->carrier().push_back(carrierCode);
    }
    if (diffItem->fareMarket().empty() && !diffItem->fareMarketCurrent().empty())
    {
      swap(diffItem->fareMarketCurrent(), diffItem->fareMarket());
      swap(diffItem->carrierCurrent(), diffItem->carrier());
    }
    diffItem->origin() = const_cast<Loc*>(diffLeft->origin());
    diffItem->destination() = const_cast<Loc*>(diffRight->destination());
    diffItem->throughFare() = throughFare();
    diffItem->tripType() = diffLeft->tripType();
    diffItem->tag() = diffLeft->tag();
    diffItem->cabin() = tagCombined;
    diffItem->setSameCarrier() = sameCarrier;
    diffItem->travelSeg().assign(trvlSeg.begin(), trvlSeg.end());
    retVal = diffSectorValidation(diffItem, differList);

    if (diffItem->status() != DifferentialData::SC_FAILED)
    {
      retVal = getHip(*diffItem);

      if (diffItem->amount() < 0.0)
        diffItem->amount() = 0.0;

      return (retVal) ? SUCCEEDED : FAILED;
    }
  }
  return FAILED;
}

//*******************************************************************************************
//               Does the sequence contain the consecutive Area 1 sectors?
//*******************************************************************************************

bool
DifferentialValidator::consecutiveSectorsFoundInSequence(
    const std::vector<DifferentialData*>& diffLforOneTripType,
    unsigned int& diffStart,
    bool& consecFound) const
{
  uint8_t ltS = 0;
  uint8_t rtS = 0;

  unsigned int diffVecSize = diffLforOneTripType.size();
  unsigned int diffL = diffStart;
  unsigned int diffR = diffStart;

  for (++diffR; diffR < diffVecSize; ++diffR)
  {
    if (diffLforOneTripType[diffL]->status() == DifferentialData::SC_ADJACENT_FAIL ||
        diffLforOneTripType[diffR]->status() == DifferentialData::SC_ADJACENT_FAIL)
      return false;

    ltS = diffLforOneTripType[diffL]->tag().length();
    rtS = diffLforOneTripType[diffR]->tag().length();

    if ((ltS != REGULAR_SECTOR && ltS != CONSEC_SECTOR) ||
        (rtS != REGULAR_SECTOR && rtS != CONSEC_SECTOR))
    {
      LOG4CXX_ERROR(logger, "9) INCORRECT TAG LENGTH IN \"CONSOLIDATE DIFF SECTORS PROCESS\"");
      return false;
    }
    if (diffLforOneTripType[diffL]->tag()[1] != diffLforOneTripType[diffR]->tag()[1])
    {
      diffStart = --diffR;
      break;
    }
    if (ltS == CONSEC_SECTOR || rtS == CONSEC_SECTOR)
    {
      consecFound = true;
      break; //  just one enough to find
    }
  }
  return true;
}

//*******************************************************************************************
//                   Check that the tag type is a valid one.
//*******************************************************************************************
bool
DifferentialValidator::checkTagFareType(const Indicator& tagType) const
{
  return (tagType == DifferentialData::ECONOMY_FTD ||
          tagType == DifferentialData::PREMIUM_ECONOMY_FTD ||
          tagType == DifferentialData::BUSINESS_FTD ||
          tagType == DifferentialData::PREMIUM_BUSINESS_FTD_NEW ||
          tagType == DifferentialData::FIRST_FTD || tagType == DifferentialData::PREMIUM_FIRST_FTD);
}

//*******************************************************************************************
//         Determine to what fare type family the consolidation should be done.
//*******************************************************************************************
DifferentialData::FareTypeDesignators
DifferentialValidator::findCombinedFareType(const DifferentialData::FareTypeDesignators& tagL,
                                            const DifferentialData::FareTypeDesignators& tagR,
                                            const bool& consecFound) const
{
  // --NR--3. If the consolidation of Differentials involves two Fare type designator (s)
  // ...
  //--NR--    3.1. The consolidation to  the higher Fare type designator will be attempted; 1AB, 2AF
  //would attempt
  //               to consolidate  into a  through F  differential only if this is less  than the
  //               sum of 1AB + 2AF.
  //--NR--    3.2. If those Fare type designator (s) are tagged FB (free F upgrade Area 1), then the
  //consolidation
  //               will be  attempted at the through lower Fare type  designator. 1AFB, 2AB and  3AB
  //               would attempt
  //               a consolidation  of a through  Business  Fare Family  differential if  the result
  //               is less  than
  //               1AFB + 2AB + 3AB.

  std::string fareTypes;

  fareTypes =
      std::string(1, DifferentialData::ECONOMY_FTD) + std::string(1, DifferentialData::PREMIUM_ECONOMY_FTD) +
      std::string(1, DifferentialData::BUSINESS_FTD) +
      std::string(1, DifferentialData::PREMIUM_BUSINESS_FTD_NEW) + std::string(1, DifferentialData::FIRST_FTD) +
      std::string(1, DifferentialData::PREMIUM_FIRST_FTD);

  std::string::size_type lpos = fareTypes.find(tagL);
  std::string::size_type rpos = fareTypes.find(tagR);

  if (consecFound)
  {
    return (lpos < rpos) ? tagL : tagR;
  }
  else
  {
    return (lpos > rpos) ? tagL : tagR;
  }
}

//*******************************************************************************************
//                 Retrieve the PaxTypeFares for the known Pax Type
//*******************************************************************************************
bool
DifferentialValidator::getPaxTypeFareVec(PaxTypeFarePtrVec& paxTypeFareVec,
                                         const FareMarket& mkt) const // FM for diff segment
{
  const PaxType& reqPaxType = *requestedPaxType();
  PaxTypeBucketVec paxTypeCortegeVec = mkt.paxTypeCortege();

  for (const auto& paxTypeCortege : paxTypeCortegeVec)
  {
    if (paxTypeCortege.requestedPaxType()->paxType() == reqPaxType.paxType())
    {
      paxTypeFareVec = paxTypeCortege.paxTypeFare();
      break;
    }
  }
  return (!paxTypeFareVec.empty());
}

//*******************************************************************************************
//              This is an accessor to the Differential Data container
//*******************************************************************************************
std::vector<DifferentialData*>&
DifferentialValidator::differential(void) // throw (string &)
{
  return fareUsage()->differentialPlusUp();
}

//*******************************************************************************************
//            Differential HIP determination & apply mileage surcharge process
//*******************************************************************************************
bool
DifferentialValidator::getHip(DifferentialData& diffItem) const throw(std::string&)
{
  FareUsage tmpFareUsageH; // Temp fareUsage for the MinFare processing
  FareUsage tmpFareUsageL; // Temp fareUsage for the MinFare processing

  if (_fareUsage->hipExemptInd() == DifferentialData::NO_HIP) // no HIP exempt
  {
    MoneyAmount highHip = 0.f;

    MoneyAmount lowHip = 0.f;

    HIPMinimumFare hip(*diffTrx());

    // Hip validation for the Low fare type
    tmpFareUsageH.paxTypeFare() = diffItem.fareHigh();

    if (_pricingUnit && _farePath)

      hip.process(tmpFareUsageH, *_pricingUnit, *_farePath);

    const MinFarePlusUpItem* hipInfo = tmpFareUsageH.minFarePlusUp().getPlusUp(HIP);

    if (hipInfo) // Get detail hip information for High fare
    {
      if (hipInfo->baseAmount < 0.f)
        throw std::string("1) AN ERROR OCCURED WHEN HIP WAS CALCULATED");
      if (hipInfo->baseAmount != 0.f)
      {
        highHip = hipInfo->baseAmount;
        diffItem.hipHighOrigin() = hipInfo->boardPoint;
        diffItem.hipHighDestination() = hipInfo->offPoint;
      }
    }
    // Hip validation for the Low fare type

    tmpFareUsageL.paxTypeFare() = diffItem.fareLow();
    if (_pricingUnit && _farePath)
      hip.process(tmpFareUsageL, *_pricingUnit, *_farePath);

    hipInfo = tmpFareUsageL.minFarePlusUp().getPlusUp(HIP);

    if (hipInfo) // Get detail hip information for Low fare
    {
      if (hipInfo->baseAmount < 0.f)
        throw std::string("1) AN ERROR OCCURED WHEN HIP WAS CALCULATED");

      if (hipInfo->baseAmount != 0.f)
      {
        lowHip = hipInfo->baseAmount;
        diffItem.hipLowOrigin() = hipInfo->boardPoint;
        diffItem.hipLowDestination() = hipInfo->offPoint;
      }
    }
    if (highHip == 0.f && lowHip == 0.f)
    {
      if (diffItem.fareHigh()->mileageSurchargePctg() > 1)
      {
        diffItem.amountFareClassHigh() += diffItem.fareHigh()->mileageSurchargeAmt();
        diffItem.amountFareClassLow() += diffItem.fareLow()->mileageSurchargeAmt();
      }
      diffItem.amount() = diffItem.amountFareClassHigh() - diffItem.amountFareClassLow();
    }
    else if (highHip == 0.f) //  Only low hip is found
    {
      if (diffItem.fareHigh()->mileageSurchargePctg() > 1)
      {
        diffItem.amountFareClassHigh() += diffItem.fareHigh()->mileageSurchargeAmt();
      }
      if (diffItem.amountFareClassHigh() - lowHip <= 0.f)
        return false;
      diffItem.hipAmount() = diffItem.amountFareClassHigh() - lowHip;
      defineLowFareCabin(diffItem);
    }
    else if (lowHip == 0.f) //  Only high hip is found
    {
      if (diffItem.fareHigh()->mileageSurchargePctg() > 1)
      {

        diffItem.amountFareClassLow() += diffItem.fareLow()->mileageSurchargeAmt();
      }
      if (highHip - diffItem.amountFareClassLow() <= 0.f)
        return false;
      diffItem.hipAmount() = highHip - diffItem.amountFareClassLow();
      defineHighFareCabin(diffItem);
    }
    else // both hip are found
    {
      if (highHip - lowHip < 0.f)
      {
        LOG4CXX_ERROR(logger, "LOW HIP AMOUNT HIGHER THAN HIGH HIP AMOUNT IN \"GET HIP\"");
        return false;
      }

      diffItem.hipAmount() = highHip - lowHip;

      if (diffItem.hipHighOrigin() != diffItem.hipLowOrigin() ||
          diffItem.hipHighDestination() != diffItem.hipLowDestination())
      {
        defineHighFareCabin(diffItem);
        defineLowFareCabin(diffItem);
      }
    }
    diffItem.hipAmtFareClassHigh() = highHip;
    diffItem.hipAmtFareClassLow() = lowHip;
  }
  else // HIP Exempt applied
  {
    if (diffItem.fareHigh()->mileageSurchargePctg() > 1)
    {
      diffItem.amountFareClassHigh() += diffItem.fareHigh()->mileageSurchargeAmt();
      diffItem.amountFareClassLow() += diffItem.fareLow()->mileageSurchargeAmt();
    }
    diffItem.amount() = diffItem.amountFareClassHigh() - diffItem.amountFareClassLow();
  }
  return true;
}

//*******************************************************************************************
//                         For a low cabin save its type
//*******************************************************************************************
void
DifferentialValidator::defineLowFareCabin(DifferentialData& diffItem) const
{
  diffItem.hipCabinLow() = cabinTypeToHip(diffItem.fareLow()->cabin());
}

//*******************************************************************************************
//                          For a high cabin save its type
//*******************************************************************************************
void
DifferentialValidator::defineHighFareCabin(DifferentialData& diffItem) const
{
  diffItem.hipCabinHigh() = cabinTypeToHip(diffItem.fareHigh()->cabin());
}

//*******************************************************************************************
//     It's been used for WPNC entries only. Find a rebooked value of the Booking Code.
//*******************************************************************************************
bool
DifferentialValidator::getBookingCode(const TravelSeg* ts, const AirSeg& airSeg, BookingCode& bc)
    const
{
  if (diffTrx()->getRequest()->isLowFareRequested()) // Is it a WPNC entry?
  {
    const FareUsage* fu = fareUsage();

    TravelSegPtrVec tsv = _mkt->travelSeg();
    TravelSegPtrVecI tsi = find(tsv.begin(), tsv.end(), ts);

    if (tsi == tsv.end())
    {
      LOG4CXX_ERROR(logger, "TRAVEL SEGMENT WAS NOT FOUND IN \"getBookingCode\" method");
      return false;
    }
    uint32_t ii = distance(tsv.begin(), tsi);

    SegmentStatusVec segStat = fu->segmentStatus();

    if (segStat[ii]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)) // Was it rebooked?
      bc = segStat[ii]._bkgCodeReBook;
    else
      bc = airSeg.getBookingCode();
  }
  else
    bc = airSeg.getBookingCode();
  return true;
}
//**********************************************************************************
// Match/Nomatch the geographic board or off point of the Differential Sectors
// Loc1AType & Loc1A; Loc2AType & Loc2A; AND Loc1BType & Loc1B; Loc2BType & Loc2B;
//**********************************************************************************
bool
DifferentialValidator::countStopsCarriers(DifferentialData& diffSeg, const Differentials& diffTable)
    const
{
  CarrierCode cxr;

  diffSeg.stops() = false; // Initialize stops count

  diffSeg.setSameCarrier() = true; // initiale same carrier

  // intermediate points 1A/2A

  LocKey loc1ak = diffTable.intermedLoc1a();
  LocCode loc1ac = diffTable.intermedLoc1a().loc();
  LocKey loc2ak = diffTable.intermedLoc2a();

  LocCode loc2ac = diffTable.intermedLoc2a().loc();

  bool stops_1A_2A = false;
  bool sameCxr_1A_2A = true;

  // intermediate points 1B/2B
  LocKey loc1bk = diffTable.intermedLoc1b();
  LocCode loc1bc = diffTable.intermedLoc1b().loc();
  LocKey loc2bk = diffTable.intermedLoc2b();
  LocCode loc2bc = diffTable.intermedLoc2b().loc();

  bool stops_1B_2B = false;
  bool sameCxr_1B_2B = true;

  TravelSegPtrVecIC lastPoint = diffSeg.travelSeg().end();
  TravelSegPtrVecIC firstPoint = diffSeg.travelSeg().end();

  if (diffSeg.travelSeg().empty())
    return false;

  int16_t endNumber = _itin->segmentOrder(diffSeg.travelSeg().back()); // Number of the Last segment

  TravelSegPtrVecIC itTvl = diffSeg.travelSeg().begin();

  for (; itTvl != diffSeg.travelSeg().end(); itTvl++) // Loop for all TravelSegment
  {
    if (_itin->segmentOrder(*itTvl) <= endNumber)
    {
      if (matchGeo(loc1ak, loc1ac, (*(diffSeg.origin()))))
      {
        firstPoint = itTvl;
        break;
      }
    }
  } // for loop
  if (firstPoint != diffSeg.travelSeg().end())
  {
    for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
         itTvl++) // Loop for all TravelSegment
    {
      if (_itin->segmentOrder(*itTvl) <= endNumber)
      {
        if (matchGeo(loc2ak, loc2ac, (*(diffSeg.destination()))))
        {
          lastPoint = itTvl;
        }
      }
    } // for loop
  } // firstPoint != diffSeg.travelSeg().end()
  if (firstPoint == diffSeg.travelSeg().end() || lastPoint == diffSeg.travelSeg().end())
    return false;

  if (_itin->segmentOrder(*lastPoint) < _itin->segmentOrder(*firstPoint))
    return false;

  // Match Intermediate parameters in the Loc1A/2A, or Loc1B/2B , if present

  if ((firstPoint + 1 != lastPoint) ||
      ((firstPoint + 1 == lastPoint) && (!(*firstPoint)->hiddenStops().empty())))
  {
    stops_1A_2A = true;
  }

  cxr = "";

  for (; firstPoint != lastPoint; firstPoint++) // Loop for part of  TravelSeg vector
  {
    if ((*firstPoint)->segmentType() == Open || (*firstPoint)->segmentType() == Air)
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(*firstPoint);

      if (!airSeg)
        continue;
      if (cxr.empty())
      {
        cxr = getCarrierCode(*airSeg, nullptr);
      }
      if (getCarrierCode(*airSeg, nullptr) != cxr)
      {
        sameCxr_1A_2A = false;
      }
    }
  }
  if (diffTable.intermedLoc1b().locType() == BLANK)
  {
    diffSeg.stops() = stops_1A_2A;
    diffSeg.setSameCarrier() = sameCxr_1A_2A;
    return true;
  }

  // Intermediate 1B/2B

  lastPoint = diffSeg.travelSeg().end();
  firstPoint = diffSeg.travelSeg().end();
  for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
       itTvl++) // Loop for all TravelSegment
  {
    if (_itin->segmentOrder(*itTvl) <= endNumber)
    {
      if (matchGeo(loc1bk, loc1bc, (*(diffSeg.origin()))))
      {
        firstPoint = itTvl;
        break;
      }
    }
  } // for loop
  if (firstPoint != diffSeg.travelSeg().end())
  {
    for (itTvl = diffSeg.travelSeg().begin(); itTvl != diffSeg.travelSeg().end();
         itTvl++) // Loop for all TravelSegment
    {

      if (_itin->segmentOrder(*itTvl) <= endNumber)
      {
        if (matchGeo(loc2bk, loc2bc, (*(diffSeg.destination()))))
        {
          lastPoint = itTvl;
        }
      }
    } // for loop
  } // firstPoint != diffSeg.travelSeg().end()
  if (firstPoint == diffSeg.travelSeg().end() || lastPoint == diffSeg.travelSeg().end())
    return false;

  if (_itin->segmentOrder(*lastPoint) < _itin->segmentOrder(*firstPoint))
    return false;

  // Match Intermediate parameters in the Loc1A/2A, or Loc1B/2B , if present

  if ((firstPoint + 1 != lastPoint) ||
      ((firstPoint + 1 == lastPoint) && (!(*firstPoint)->hiddenStops().empty())))
  {
    stops_1B_2B = true;
  }
  for (; firstPoint != lastPoint; firstPoint++) // Loop for part of TravelSeg vector
  {
    if ((*firstPoint)->segmentType() == Open || (*firstPoint)->segmentType() == Air)
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(*firstPoint);

      if (!airSeg)
        continue;

      if (cxr.empty())
      {
        cxr = getCarrierCode(*airSeg, nullptr);
      }
      if (getCarrierCode(*airSeg, nullptr) != cxr)
      {
        sameCxr_1B_2B = false;
      }
    }
  }
  diffSeg.setSameCarrier() = (sameCxr_1A_2A && sameCxr_1B_2B);
  if (!stops_1A_2A && !stops_1B_2B)
    diffSeg.stops() = false;
  else
    diffSeg.stops() = true;
  return true;
}

//**********************************************************************************
//     Does differential contain at least one item with valid High/Low fares?
//**********************************************************************************
bool
DifferentialValidator::highAndLowFound(void)
{
  std::vector<DifferentialData*>& differV = differential();
  std::vector<DifferentialData*>::const_iterator diff = differV.begin();
  uint16_t singleDiffNum = 0;
  for (; diff != differV.end(); ++diff)
  {
    if ((**diff).thruNumber())
      singleDiffNum++; // accumulate differential sectors
  }

  diff = differV.begin();
  for (; diff != differV.end(); ++diff)
  {
    DifferentialData& diffItem = **diff;

    if ( diffItem.status() == DifferentialData::SC_CONSOLIDATED_PASS &&
         (diffItem.fareHigh() != nullptr ) && (diffItem.fareLow() != nullptr ) )
    {
      // Check All Diff sectors not included in Consolidated for PASS/FAIL status
      // to make sure we are Passing Consolidated to Pricing correctly. If any of
      // single sector or consolidated Fail the whole Differential combination must FAIL.

      uint16_t numDif =
          diffItem.inConsolDiffData().size(); // number of Diff segmnets in Consolidated

      if(diffItem.isItemConsolidated())
        numDif = countThruNumbers(diffItem);

      uint16_t arrayCount = numDif;
      size_t m = std::max(numDif, singleDiffNum);
      uint16_t i[m];

      size_t k = std::min(numDif, singleDiffNum);

      std::vector<DifferentialData*>& vec = diffItem.inConsolDiffData();
      uint16_t v=0;
      for (uint16_t j = 0; j < k && v < vec.size(); ++j, ++v)
      {
        if(vec[v]->thruNumber())
        {
          i[j] = vec[v]->thruNumber();
        }
        else
        {
          collectThruNumbers(i, j, *(vec[v]));
          --j;
        }
      }

      std::vector<DifferentialData*>::const_iterator cidiV = differV.begin();
      for (; cidiV != differV.end(); ++cidiV)
      {
        DifferentialData& dI = **cidiV;
        if (dI.thruNumber()) // number exist -> simple Diff segment
        {
          bool found = false;
          for (uint16_t j = 0; j < arrayCount; j++)
          {
            if (i[j] == dI.thruNumber())
            {
              found = true;
            }
          }
          if (!found && (dI.status() == DifferentialData::SC_PASSED))
          {
            i[arrayCount] = dI.thruNumber();
            arrayCount++;
          }
        }
        else // Consolidated sector
            if ((dI.status() == DifferentialData::SC_CONSOLIDATED_PASS) &&
                dI.inConsolDiffData().size())
        {
          findNumbers(i, arrayCount, dI);
        }
      }
      if (arrayCount != singleDiffNum)
      {
        diffItem.status() = DifferentialData::SC_COMBINATION_FAIL;
      }
    }
  }

  diff = differV.begin();

  for (; diff != differV.end(); ++diff)
  {
    DifferentialData& diffItem = **diff;
    if ((diffItem.status() == DifferentialData::SC_CONSOLIDATED_PASS) &&
        (diffItem.fareHigh() != nullptr) && (diffItem.fareLow() != nullptr))
      return true;
  }

  diff = differV.begin();
  for (; diff != differV.end(); ++diff)
  {
    DifferentialData& diffItem = **diff;

    if ((diffItem.status() != DifferentialData::SC_PASSED) &&
        (diffItem.status() != DifferentialData::SC_CONSOLIDATED_FAIL) &&
        (diffItem.status() != DifferentialData::SC_CONSOLIDATED_PASS))
      return false;
  }
  return true;
}

//**********************************************************************************
//  CollectThruNumbers contains in each consolidated Differential Item
//**********************************************************************************
void
DifferentialValidator::collectThruNumbers(uint16_t i[], uint16_t& index, DifferentialData& dd) const
{
  std::vector<DifferentialData*>& vecDI = dd.inConsolDiffData();
  std::vector<DifferentialData*>::const_iterator idiV = vecDI.begin();
  std::vector<DifferentialData*>::const_iterator iediV = vecDI.end();

  for (; idiV != iediV; ++idiV)
  {
    DifferentialData& dII = **idiV;
    if (dII.thruNumber()) // number exist -> simple Diff segment
    {
      i[index] = dII.thruNumber();
      index++;
    }
    else if (dII.inConsolDiffData().size())
    {
      collectThruNumbers(i, index, dII);
    }
  }
  return;
}

//**********************************************************************************
//  CountThruNumbers contains in the consolidated Differential Item
//**********************************************************************************
uint16_t
DifferentialValidator::countThruNumbers(DifferentialData& dd) const
{
   uint16_t numOfDif = 0;
   std::vector<DifferentialData*>& vecDiff = dd.inConsolDiffData();
   std::vector<DifferentialData*>::const_iterator iddi = vecDiff.begin();
   std::vector<DifferentialData*>::const_iterator idde = vecDiff.end();

   for (; iddi != idde; ++iddi)
   {
     DifferentialData& dII = **iddi;
     if (dII.thruNumber()) // number exist -> simple Diff segment
     {
        numOfDif++;
     }
     else
     {
        numOfDif += countThruNumbers(dII);
     }
   }
   return numOfDif;
}

//**********************************************************************************
// Call diagnostic
//**********************************************************************************
void
DifferentialValidator::processDiag(void)
{
  createDiag();
  doDiag();
  endDiag();
}

//**********************************************************************************
// Create Diagnostic object and populate a new header if Fare Market is not the same
//**********************************************************************************
void
DifferentialValidator::createDiag(void)
{
  PricingTrx& trx = *diffTrx();
  DCFactory* factory = DCFactory::instance();

  if (!_diag)
    _diag = factory->create(trx);
  _diag->lineSkip(0);

  if (trx.diagnostic().diagnosticType() == Diagnostic413)
  {
    _diag->enable(Diagnostic413);
  }
  std::stringstream oStr;
  const std::vector<FareUsage*> fuV = _pricingUnit->fareUsage();
  std::vector<FareUsage*>::const_iterator fui = fuV.begin();

  oStr << "\n                                                                                      "
          "                   ";
  oStr << "\n***************************************************************";
  oStr << "\nPRICING UNIT: ";
  for (; fui != fuV.end(); ++fui)
  {
    oStr << std::setw(3) << (*fui)->paxTypeFare()->fareMarket()->boardMultiCity() << "-"
         << std::setw(2) << (*fui)->paxTypeFare()->fareMarket()->governingCarrier() << "-"
         << std::setw(3) << (*fui)->paxTypeFare()->fareMarket()->offMultiCity() << "  ";
  }
  oStr << "\nREQ PAX TYPE: " << requestedPaxType()->paxType() << "    PU TYPE: ";
  if (_pricingUnit->puType() == PricingUnit::Type::ONEWAY)
    oStr << "ONEWAY       ";
  else if (_pricingUnit->puType() == PricingUnit::Type::UNKNOWN)
    oStr << "UNKNOWN      ";
  else if (_pricingUnit->puType() == PricingUnit::Type::OPENJAW)
    oStr << "OPENJAW      ";
  else if (_pricingUnit->puType() == PricingUnit::Type::ROUNDTRIP)
    oStr << "ROUNDTRIP    ";
  else if (_pricingUnit->puType() == PricingUnit::Type::CIRCLETRIP)
    oStr << "CIRCLETRIP   ";
  else
    oStr << "             ";

  oStr << std::endl;

  *_diag << oStr.str();

  if (_diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX))
    MinimumFare::printRexDatesAdjustment(*_diag, _diffTrx->dataHandle().ticketDate());

  *_diag << *_mkt;
  _diag->lineSkip(1); // skip one line =  "\n" ;
}

//**********************************************************************************
//                           Do Diagnostic 413 here
//**********************************************************************************
void
DifferentialValidator::doDiag(void)
{
  PricingTrx& trx = *diffTrx();
  if (!trx.diagnostic().isActive() || trx.diagnostic().diagnosticType() != Diagnostic413)
    return;

  DiagParamMapVecI endD = trx.diagnostic().diagParamMap().end();
  DiagParamMapVecI beginD = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);

  if (beginD != endD)
  {
    size_t len = ((*beginD).second).size();
    if (len != 0)
    {
      if (((*beginD).second).substr(0, len) != _paxTfare->fareClass())
      {
        std::stringstream oStr;
        oStr << "\n " << _paxTfare->fareClass() << " FARE IN THIS PU, NOT  "
             << ((*beginD).second).substr(0, len) << " FARE";
        *_diag << oStr.str();

        return;
      }
    }
  }
  *_diag << *fareUsage();
}

//**********************************************************************************
//                Flush buffers and reclaim resources

//**********************************************************************************
void
DifferentialValidator::endDiag(void)
{
  PricingTrx& trx = *diffTrx();
  if (trx.diagnostic().diagnosticType() != Diagnostic413)
    return;
  _diag->lineSkip(2); // skip two lines =  "\n\n" ;
  _diag->flushMsg();

  _diag->disable(Diagnostic413);
  _diag = nullptr;
}

//**********************************************************************************
//      Determination of the adjacent sectors on the Fare Market.
//**********************************************************************************
bool
DifferentialValidator::adjacentSectorDiff(DifferentialData& diffData,
                                          int tSN,
                                          const int8_t& increment,
                                          const Indicator& diffNumber,
                                          const Indicator& diffLetter,
                                          uint16_t& thruNumber)
{
  DifferentialData::FareTypeDesignators cabin(DifferentialData::BLANK_FTD);
  CabinType cabinType;
  bool found = false;
  int index = tSN + increment;

  uint16_t tsSize = _mkt->travelSeg().size();

  // ERROR - Business logic error
  if (tSN >= tsSize)
  {
    LOG4CXX_ERROR(logger, "Leaving DifferentialValidator::adjacentSectorDiff (): wrong size");
    return false;
  }

  TravelSeg* tvl = _mkt->travelSeg()[tSN];

  TravelSeg* tvlAny = _mkt->travelSeg()[index];

  AirSeg* airSegPtr = dynamic_cast<AirSeg*>(tvl);

  if (!airSegPtr)
    return false;

  AirSeg& airSeg = *airSegPtr;

  std::vector<TravelSeg*>::const_iterator itTvl = _mkt->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator itTvlEnd = _mkt->travelSeg().end();

  DifferFailBkgCodeEnum diffCandid = analyzeFailBkgCode(index, cabin);

  // Cabin of previous/next flight sector is Higher than cabin of through Fare.
  // Are board/off points of the adjacent DIFF sector are the same as
  // the board/off points of the through fare component ?  1.3.1.1 / 1.3.1.2

  cabinTypeFromFTD(cabin, cabinType);

  if (diffCandid == CABIN_HIGHER)
  {
    if (increment == -1)
    {
      if ((tvlAny->boardMultiCity() == FareMarketUtil::getBoardMultiCity(*_mkt, **itTvl)) &&
          (tvl->offMultiCity() == FareMarketUtil::getOffMultiCity(*_mkt, **(itTvlEnd - 1))))
      {
        return false;
      }
    }
    else
    {
      if ((tvl->boardMultiCity() == FareMarketUtil::getBoardMultiCity(*_mkt, **itTvl)) &&
          (tvlAny->offMultiCity() == FareMarketUtil::getOffMultiCity(*_mkt, **(itTvlEnd - 1))))
      {
        return false;
      }
    }

    uint16_t startSegNum = 0;
    uint16_t endSegNum = 0;

    if (increment == -1)
    {

      startSegNum = _itin->segmentOrder(tvlAny);
      endSegNum = _itin->segmentOrder(tvl);
    }
    else
    {
      startSegNum = _itin->segmentOrder(tvl);
      endSegNum = _itin->segmentOrder(tvlAny);
    }

    std::vector<TravelSeg*>::const_iterator itinTrvlSeg = _itin->travelSeg().begin();

    std::vector<TravelSeg*> segOrderVec;

    for (bool inHere = false; itinTrvlSeg != _itin->travelSeg().end(); itinTrvlSeg++)
    {
      if (!inHere && _itin->segmentOrder(*itinTrvlSeg) != startSegNum)
        continue;
      inHere = true;

      segOrderVec.push_back(*itinTrvlSeg);

      if (_itin->segmentOrder(*itinTrvlSeg) == endSegNum)
        break;
    }

    AirSeg* airSegPtrAdj = dynamic_cast<AirSeg*>(tvlAny);
    AirSeg& airSegAdj = *airSegPtrAdj;
    const FareMarket* pFM = nullptr;

    //  Find governing carrier(s) for a new fare market:
    //  At first check for the Governing Carrier Override for YY,
    //  second, if not YY check for Gov Cxr Override
    //  third, if none of the above gets through Governing CXR

    std::set<CarrierCode> govCXRs;
    std::vector<CarrierCode> govCXR; // for the Governing carrier Override methods

    if (FareMarketUtil::isYYOverride(*_diffTrx, segOrderVec)) // Command Pricing Gov Cxr Over
    {
      govCXRs.insert(INDUSTRY_CARRIER);
    }
    else
    {
      FareMarketUtil::getGovCxrOverride(*_diffTrx, segOrderVec, govCXR);
      GoverningCarrier govCxrUtil(_diffTrx);
      govCxrUtil.getGoverningCarrier(segOrderVec, govCXRs, _mkt->direction());
    }

    if (govCXRs.empty())
    {
      LOG4CXX_ERROR(logger, "11) NO GOVERNING CARRIER FOUND IN \"ADJACENT DIFF SECTORS PROCESS\"");
      return false;
    }

    for (const auto& govCXR : govCXRs)
    {
      pFM = getFareMarket(segOrderVec, &govCXR);

      if (!pFM)
      {
        LOG4CXX_ERROR(logger,
                      "7) FARE MARKET: " << segOrderVec.front()->boardMultiCity() << "-" << govCXR
                                         << "-" << segOrderVec.back()->offMultiCity()
                                         << " NOT FOUND IN \"ADJACENT DIFF SECTORS PROCESS\"");
        return false;
      }

      if (pFM == _mkt) // Differential sector can't be as big as a whole trip.
        return false;

      diffData.fareMarketCurrent().push_back(const_cast<FareMarket*>(pFM));
      diffData.carrierCurrent().push_back(govCXR);
    }

    for (const auto& carrierCode : govCXR)
    {
      pFM = getFareMarket(segOrderVec, &carrierCode);

      if (!pFM)
      {
        LOG4CXX_ERROR(logger,
                      "7) FARE MARKET: " << segOrderVec.front()->boardMultiCity() << "- OVERRIDE "
                                         << carrierCode << "-" << segOrderVec.back()->offMultiCity()
                                         << " NOT FOUND IN \"ADJACENT DIFF SECTORS PROCESS\"");
        return false;
      }

      if (pFM == _mkt) // Differential sector can't be as big as a whole trip.
        return false;

      diffData.fareMarket().push_back(const_cast<FareMarket*>(pFM));
      diffData.carrier().push_back(carrierCode);
    }
    if (diffData.fareMarket().empty() && !diffData.fareMarketCurrent().empty())
    {
      swap(diffData.fareMarketCurrent(), diffData.fareMarket());
      swap(diffData.carrierCurrent(), diffData.carrier());
    }

    // populate differential item by a new data

    diffData.origin() = const_cast<Loc*>(airSegAdj.origin());
    diffData.destination() = const_cast<Loc*>(airSeg.destination());
    diffData.travelSeg().push_back(tvlAny);
    diffData.travelSeg().push_back(tvl);
    diffData.tag() += (diffNumber);
    diffData.tag() += (diffLetter);
    diffData.tag() += (cabin);
    diffData.cabin() = cabin;
    diffData.tripType() =
        (_pricingUnit->puType() == PricingUnit::Type::ONEWAY) ? ONE_WAY_TRIP : ROUND_TRIP;
    diffData.thruNumber() = thruNumber;
    thruNumber++;

    //  Just get FareMarket for the adjacent DIFF sectors need to validated
    //  Now it's time to go thru all PaxTypeFares to find the First Class fares
    //  and both sectors should pass RBD validation and Rule validation.

    bool fail = false; // default BKG fail RBD

    PaxTypeFarePtrVec paxTypeFareVec;
    getPaxTypeFareVec(paxTypeFareVec, *pFM);

    for (PaxTypeFarePtrVecIC itPTFare = paxTypeFareVec.begin(); itPTFare != paxTypeFareVec.end();
         itPTFare++) // Big LOOP for all PaxTypeFares
    {
      PaxTypeFare& ptf = **itPTFare;
      //  Initialize Iterator for the major loop through all Fares for the ONE PaxType
      if (ptf.isNormal() && // Is this is a Normal Fare?
          (cabinType == ptf.cabin())) // same as through fare
      {
        if (!ptf.isValidNoBookingCode())
          continue;

        if (ptf.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL)) // Did Fare already FAIL?
          continue;

        if (ptf.bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED)) // Not processed
        {
          if (!isBkgCodeStatusPass(ptf))
          {
            continue;
          }
        }
        SegmentStatusVecCI itSegStat = ptf.segmentStatus().begin();

        for (; itSegStat != ptf.segmentStatus().end(); itSegStat++)
        {
          const PaxTypeFare::SegmentStatus& segStat = *itSegStat;
          if (!segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
          {
            fail = true;
            break;
          }
        } // for ( ; itSegStat)

        if (fail)
        {
          fail = false;
          continue;
        }
        found = true; // First class fare with two passed RBD is found
      } // if (   paxTFare.isNormal() &&
      if (UNLIKELY(found))
        break;
    } // for PaxType Fare Loop

    diffData.stops() = true; // for future Non-stop/direct validation
    diffData.setSameCarrier() = (getCarrierCode(airSegAdj, nullptr) == getCarrierCode(airSeg, nullptr));

    diffData.bookingCode() = airSegAdj.getBookingCode();
    wpncFindFareForDiffSector(diffData, increment);

    // move diff item to the vector in FareUsage
    std::vector<DifferentialData*>& differV = differential();

    differV.push_back(&diffData);

    if (!found)
      diffData.status() = DifferentialData::SC_ADJACENT_FAIL;

  } // if ( diffCandid == CABIN_HIGHER )
  return found;
}

//**********************************************************************************
//          Add Money to the Pricing Unit and to the  Fare Usage.
//**********************************************************************************

void
DifferentialValidator::copyDifferentialData()
{
  PricingUnit& prU = *pricingUnit();
  FareUsage& fu = *fareUsage();

  bool roundTrip = (prU.puType() == PricingUnit::Type::ROUNDTRIP);
  bool onewayTrip = (prU.puType() == PricingUnit::Type::ONEWAY);

  bool openjaw = (prU.puType() == PricingUnit::Type::OPENJAW);
  bool circleTrip = (prU.puType() == PricingUnit::Type::CIRCLETRIP);

  if (!roundTrip && !onewayTrip && !circleTrip && !openjaw)
    return;

  fu.differentialAmt() = 0.0;
  for (DifferentialData* const diffData : differential())
  {
    if (diffData == nullptr)
      continue;

    bool isDPorPPentry;
    if (TrxUtil::newDiscountLogic(*diffTrx()))
      isDPorPPentry = _diffTrx->getRequest()->isDPorPPentry();
    else
      isDPorPPentry = _diffTrx->getRequest()->isDPEntry();

    if (isDPorPPentry && // Is DP qualifier in the entry ?
        !(diffData->status() == DifferentialData::SC_FAILED))
      calculateDPercentage(*diffData);

    if (diffData->status() == DifferentialData::SC_FAILED ||
        diffData->status() == DifferentialData::SC_CONSOLIDATED_FAIL ||
        diffData->status() == DifferentialData::SC_ADJACENT_FAIL)
      continue;

    const FMDirection& diffDir = diffData->throughFare()->fareMarket()->direction();
    if (diffDir == FMDirection::INBOUND && !diffData->isSetSwap())
      diffData->swap();

    if (((roundTrip || circleTrip || openjaw) && diffData->tripType() == ROUND_TRIP) ||
        (onewayTrip && diffData->tripType() == ONE_WAY_TRIP))
    {
      prU.setTotalPuNucAmount(prU.getTotalPuNucAmount() + diffData->amount());
      fu.differentialAmt() +=
          (diffData->hipAmount() != 0.f) ? diffData->hipAmount() : diffData->amount();
    }
  }
}

//**********************************************************************************
//        Find if the summ of diff sectors is as big as a Fare Component.
//**********************************************************************************
bool
DifferentialValidator::checkDifferentialVector(void)
{
  std::vector<DifferentialData*>& differentialDataVect = differential();
  std::vector<DifferentialData*>::iterator it = differentialDataVect.begin();
  std::vector<DifferentialData*>::iterator itEnd = differentialDataVect.end();
  uint16_t numberDiffSect = 0;
  bool bothOWRT = false;

  for (Indicator tripT = (*it)->tripType(); it != itEnd; ++it)
  {
    DifferentialData& diffData = **it;

    if (diffData.status() == DifferentialData::SC_FAILED ||
        diffData.status() == DifferentialData::SC_CONSOLIDATED_FAIL)
      continue;

    if (!bothOWRT && tripT != diffData.tripType())
      bothOWRT = true;
    tripT = diffData.tripType();
    numberDiffSect++;
  }
  if (!fareUsage() && bothOWRT)
    numberDiffSect /= 2;
  uint16_t numberTrvlSeg = 0;

  const std::vector<TravelSeg*>& tsv = _mkt->travelSeg();
  std::vector<TravelSeg*>::const_iterator its = tsv.begin();
  std::vector<TravelSeg*>::const_iterator itsEnd = tsv.end();

  for (; its != itsEnd; ++its)
  {
    TravelSeg* trSeg = *its;
    if (!dynamic_cast<AirSeg*>(trSeg))
      continue; // Surface segment;
    numberTrvlSeg++;
  }

  if (numberTrvlSeg == numberDiffSect)
  {
    for (it = differentialDataVect.begin(); it != itEnd; ++it)
    {
      (*it)->status() = DifferentialData::SC_FAILED;
    }
    return false;
  }
  return true;
}

//**********************************************************************************
//        Check the booking code status for the fare, if it's PASS return TRUE
//        otherwise return FALSE except NP status - needs to validate Booking Code.
//**********************************************************************************
const bool
DifferentialValidator::isBkgCodeStatusPass(PaxTypeFare& paxTF, bool updateFU) const
{
  if (paxTF.isBookingCodePass())
    return true;
  if (!paxTF.bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
    return false;

  //@@
  const FareMarket& fm = *paxTF.fareMarket();
  FareUsage fu;

  fu.paxTypeFare() = &paxTF;

  fu.travelSeg().insert(fu.travelSeg().end(), fm.travelSeg().begin(), fm.travelSeg().end());

  PaxTypeFare::SegmentStatus segStat;

  for (TravelSegPtrVecI iterTvl = fu.travelSeg().begin(); iterTvl != fu.travelSeg().end();
       iterTvl++) // Initialize
  {
    segStat._bkgCodeSegStatus.setNull();
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

    if (airSeg != nullptr)
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
    }
    else
    {
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
    }

    fu.segmentStatus().push_back(segStat);

    fu.inbound() = (fm.direction() == FMDirection::INBOUND);
  }
  //@@

  if (_fBCValidator->validate(fu))
  {
    if (fu.isBookingCodePass())
    {
      // Move rebooked bkg codes if exists from dummy FareUsage to real FareUsage
      // to corresponding TravelSeg for the WPNC entry only
      if (diffTrx()->getRequest()->isLowFareRequested() && updateFU)
      {
        int indexD = 0;

        TravelSegPtrVecI itRealFu, itDummyFu;
        SegmentStatusVecI itRealStatFu, itDummyStatFu;

        // TODO: commented out on 10/2/2008 SPR 112275
        // Collect rebooked bkg codes for the same fareClass and Fare Market
        //
        //        if((paxTF.fareClass()  != fareUsage()->paxTypeFare()->fareClass())&&
        //           (paxTF.fareMarket() != fareUsage()->paxTypeFare()->fareMarket()  ))
        //        {
        //           return true;
        //        }

        itDummyFu = fu.travelSeg().begin();
        itDummyStatFu = fu.segmentStatus().begin();

        for (; itDummyFu != fu.travelSeg().end() && itDummyStatFu != fu.segmentStatus().end();
             ++itDummyFu, ++itDummyStatFu)
        {
          PaxTypeFare::SegmentStatus& segStatDummy = *itDummyStatFu;
          if (segStatDummy._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)) // Was it rebooked?
          {
            indexD = itinerary()->segmentOrder(*itDummyFu) - 1;

            itRealFu = fareUsage()->travelSeg().begin();
            itRealStatFu = fareUsage()->segmentStatus().begin();

            for (; itRealFu != fareUsage()->travelSeg().end() &&
                       itRealStatFu != fareUsage()->segmentStatus().end();
                 ++itRealFu, ++itRealStatFu)
            {
              if (indexD != (itinerary()->segmentOrder(*itRealFu) - 1))
                continue;

              PaxTypeFare::SegmentStatus& segStatReal = *itRealStatFu;
              segStatReal = segStatDummy;
            }
          }
        }
      }
      return true;
    }
  }
  return false;
}

//**********************************************************************************
// Need to check the passed fare for NP (not processed) or SP (soft-pass) rule status, if so, then
// send it to PricingRuleController to validate or re-validate the rule(s) for the fare
// (part of IMAY105 deliverable)
//**********************************************************************************
bool
DifferentialValidator::validateRules(const PaxTypeFare& paxTF) const
{
  std::vector<uint16_t> puCategories;
  std::vector<uint16_t> fcCategories;
  bool pU = true;
  bool fC = true;
  bool cat14 = false;
  bool cat2 = false;
  // PU scope & FC scope
  if (paxTF.isCategorySoftPassed(RuleConst::DAY_TIME_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::DAY_TIME_RULE))
    cat2 = true;
  if (paxTF.isCategorySoftPassed(RuleConst::SEASONAL_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::SEASONAL_RULE))
    puCategories.push_back(RuleConst::SEASONAL_RULE);
  if (paxTF.isCategorySoftPassed(RuleConst::FLIGHT_APPLICATION_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE))
    puCategories.push_back(RuleConst::FLIGHT_APPLICATION_RULE);
  if (paxTF.isCategorySoftPassed(RuleConst::ADVANCE_RESERVATION_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::ADVANCE_RESERVATION_RULE))
    puCategories.push_back(RuleConst::ADVANCE_RESERVATION_RULE);
  if (paxTF.isCategorySoftPassed(RuleConst::STOPOVER_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::STOPOVER_RULE))
    fcCategories.push_back(RuleConst::STOPOVER_RULE);
  if (paxTF.isCategorySoftPassed(RuleConst::TRANSFER_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::TRANSFER_RULE))
    fcCategories.push_back(RuleConst::TRANSFER_RULE);
  if (paxTF.isCategorySoftPassed(RuleConst::BLACKOUTS_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::BLACKOUTS_RULE))
    puCategories.push_back(RuleConst::BLACKOUTS_RULE);
  if (paxTF.isCategorySoftPassed(RuleConst::TRAVEL_RESTRICTIONS_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::TRAVEL_RESTRICTIONS_RULE))
    cat14 = true;
  if (paxTF.isCategorySoftPassed(RuleConst::SALE_RESTRICTIONS_RULE) ||
      !paxTF.isCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE))
    puCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);

  if (UNLIKELY(paxTF.isCategorySoftPassed(RuleConst::MISC_FARE_TAG) ||
      !paxTF.isCategoryProcessed(RuleConst::MISC_FARE_TAG)))
    puCategories.push_back(RuleConst::MISC_FARE_TAG);

  if (UNLIKELY(diffTrx()->getOptions()->isNoPenalties()))
    fcCategories.push_back(RuleConst::PENALTIES_RULE);

  if (puCategories.empty() && fcCategories.empty() && !cat14 && !cat2)
    return true;
  // PU scope rule revalidation
  if (!puCategories.empty())
  {
    if (!validatePuRules(paxTF, puCategories))
      pU = false;
  }

  if (!fcCategories.empty())
  {
    RuleControllerWithChancelor<FareMarketRuleController> ruleController(
        DynamicValidation, fcCategories, _diffTrx);
    if (!ruleController.validate(
            *_diffTrx, *(const_cast<Itin*>(_itin)), *(const_cast<PaxTypeFare*>(&paxTF))))
    {
      fC = false;
    }
  }

  if (cat14 || cat2)
  {
    pU = false;
    puCategories.clear();
    if (cat2)
      puCategories.push_back(RuleConst::DAY_TIME_RULE);
    if (cat14)
      puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    if (validatePuRules(paxTF, puCategories, true))
      pU = true;
  }
  if (pU && fC)
    return true;

  return false;
}

bool
DifferentialValidator::validatePuRules(const PaxTypeFare& paxTF,
                                       std::vector<uint16_t> puCategories,
                                       bool catPU) const
{
  const FareMarket& fm = *paxTF.fareMarket();

  // PU scope rule revalidation
  PricingUnit pu;
  if (catPU)
    pu = *_farePath->pricingUnit().front();
  FareUsage fu;
  fu.paxTypeFare() = const_cast<PaxTypeFare*>(&paxTF);

  if (!catPU)
  {
    pu.fareUsage().push_back(&fu);
    pu.travelSeg().insert(pu.travelSeg().end(), fm.travelSeg().begin(), fm.travelSeg().end());

    pu.puType() = _pricingUnit->puType();
    pu.paxType() = _pricingUnit->paxType();
  }

  fu.travelSeg().insert(fu.travelSeg().end(), fm.travelSeg().begin(), fm.travelSeg().end());

  if (LIKELY(_fareUsage != nullptr))
  {
    fu.inbound() = _fareUsage->inbound();
  }
  else
  {
    fu.inbound() = (fm.direction() == FMDirection::INBOUND);
  }

  pu.dynamicValidationPhase() = true;

  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(
      DynamicValidation, puCategories, _diffTrx);

  bool result(false);
  result = ruleController.validate(*_diffTrx, *(const_cast<FarePath*>(_farePath)), pu, fu);
  pu.dynamicValidationPhase() = false;

  if (result)
  {
    return true;
  }

  return false;
}

//**********************************************************************************
// Calculate discount percentage for the Differential Item
// (part of IMAY105 deliverable)
//**********************************************************************************
void
DifferentialValidator::calculateDPercentage(DifferentialData& diffData) const
{
  Percent perCent =
      _diffTrx->getRequest()->discountPercentage(diffData.travelSeg().front()->segmentOrder());
  if (perCent != 0 && diffData.amount() > EPSILON)
  {
    diffData.amount() *= (1.f - perCent / 100.f); // take off percent
  }
  return;
}

//**********************************************************************************
//  Sort Differential Items before display them in Fare Calc Line
//**********************************************************************************
bool
DifferentialValidator::LowerSegmentNumber::
operator()(const DifferentialData* item1, const DifferentialData* item2)
{
  return (item1->travelSeg().size() > 0 && item2->travelSeg().size() > 0 &&
          _itin->segmentOrder(item1->travelSeg().front()) <
              _itin->segmentOrder(item2->travelSeg().front()));
}

//**********************************************************************************
// Return carrier code depending on Override indicators
//**********************************************************************************
CarrierCode
DifferentialValidator::getCarrierCode(const AirSeg& airSeg, std::vector<TravelSeg*>* tv) const
{
  CarrierCode carrier = airSeg.carrier();

  // Governing Carrier Override related code below
  // At first check for the C-YY override option
  // In second, if not YY, check for the C-XX override
  if (_diffTrx->getRequest()->isIndustryFareOverrideEntry())
  {
    if (_diffTrx->getRequest()->industryFareOverride(airSeg.segmentOrder()))
    {
      carrier = INDUSTRY_CARRIER;
    }
  }
  else if (_diffTrx->getRequest()->isGoverningCarrierOverrideEntry())
  {
    carrier = _diffTrx->getRequest()->governingCarrierOverride(airSeg.segmentOrder());
    if (tv)
    {
      std::set<CarrierCode> govCXRs;
      FareMarketUtil::getParticipatingCarrier(*tv, govCXRs);
      if (!FareMarketUtil::isParticipatingCarrier(govCXRs, carrier))
      {
        carrier = airSeg.carrier();
      }
    }
  }
  else if (_diffTrx->getOptions()->isIataFares())
  {
    carrier = INDUSTRY_CARRIER;
  }

  if (carrier.empty())
  {
    carrier = airSeg.carrier();
  }

  return carrier;
}

//**********************************************************************************
// Match/Nomatch validation for the Intermediate sector
//**********************************************************************************
void
DifferentialValidator::findNumbers(uint16_t i[], uint16_t arrayCount, DifferentialData& dI) const
{
  std::vector<DifferentialData*>& vecDI = dI.inConsolDiffData();
  std::vector<DifferentialData*>::const_iterator idiV = vecDI.begin();
  std::vector<DifferentialData*>::const_iterator iediV = vecDI.end();

  for (; idiV != iediV; ++idiV)
  {
    DifferentialData& dII = **idiV;
    if (dII.thruNumber()) // number exist -> simple Diff segment
    {
      bool found = false;
      for (uint16_t j = 0; j < arrayCount; j++)
      {
        if (i[j] == dII.thruNumber())
        {
          found = true;
        }
      }
      if (!found)
      {
        i[arrayCount] = dII.thruNumber();
        arrayCount++;
      }
    }
    else if (dII.inConsolDiffData().size())
    {
      findNumbers(i, arrayCount, dII);
    }
  }
}

//**********************************************************************************
// populate carrier and Faremarket info in Diff sector
//**********************************************************************************
void
DifferentialValidator::moveCarrierAndMarket(DifferentialData* diffItem,
                                            AirSeg& airSeg,
                                            std::vector<TravelSeg*>& segOrderVec)
{
  diffItem->carrier().clear();

  diffItem->origin() = const_cast<Loc*>(airSeg.origin());
  diffItem->destination() = const_cast<Loc*>(airSeg.destination());

  CarrierCode carrier = getCarrierCode(airSeg, &segOrderVec);

  const FareMarket* fm = getFareMarket(segOrderVec, &carrier);

  if (fm)
  {
    diffItem->fareMarket().push_back(const_cast<FareMarket*>(fm));
  }
  diffItem->carrier().push_back(carrier);

  if (carrier != airSeg.carrier())
  {
    const FareMarket* fm = getFareMarket(segOrderVec, &airSeg.carrier());

    diffItem->fareMarketCurrent().push_back(const_cast<FareMarket*>(fm));
    diffItem->carrierCurrent().push_back(airSeg.carrier());
  }

  if (diffItem->fareMarket().empty() && !diffItem->fareMarketCurrent().empty())
  {
    swap(diffItem->fareMarketCurrent(), diffItem->fareMarket());
    swap(diffItem->carrierCurrent(), diffItem->carrier());
  }
  return;
}

//**********************************************************************************
// gets availability for the specified travel sector
//**********************************************************************************
std::vector<ClassOfService*>*
DifferentialValidator::getAvailability(const int tSN) const
{
  std::vector<ClassOfService*>* cosVec = nullptr;

  const FareUsage* fu = fareUsage();

  uint16_t tsSize = _mkt->travelSeg().size();
  TravelSeg* tvl = _mkt->travelSeg()[tSN];
  AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);
  if (!airSeg)
    return cosVec;

  SegmentStatusVec ssv = fu->segmentStatus();

  std::vector<TravelSeg*> segOrderVec;

  // Check the cases when BKSS_AVAIL_BREAK is 'T'
  // if current and one up are 'T', then use cos availability from the current travel segment
  // if current is 'T' and one up is 'F', then check one more up for 'T' or 'F'
  // Based on the result use cos availability from the FareMarket( retrieved for 'T' and up to two
  // 'F')
  // for current segment
  if (!tSN && ssv[tSN]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
  {
    return (&(tvl->classOfService()));
  }

  if (tSN && ssv[tSN]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
  {
    // up one segment
    if (ssv[tSN - 1]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
    {
      return (&(tvl->classOfService()));
    }
    else
    {
      if ((tSN - 2 >= 0) && !ssv[tSN - 2]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
      {
        segOrderVec.push_back(_mkt->travelSeg()[tSN - 2]); // up two segment
      }
      segOrderVec.push_back(_mkt->travelSeg()[tSN - 1]);
      segOrderVec.push_back(_mkt->travelSeg()[tSN]);
      if (tsSize == segOrderVec.size())
      {
        return (_mkt->classOfServiceVec()[tSN]);
      }
      return getCOS(tSN, segOrderVec);
    }
  }

  // Check the cases when BKSS_AVAIL_BREAK is 'F'
  // if current is 'F' check the following sector (up to two) for 'T',
  // Based on the result use cos availability from the FareMarket( retrieved for 'T' and up to two
  // 'F')
  // for current segment

  if (!tSN && !ssv[tSN]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
  {
    segOrderVec.push_back(_mkt->travelSeg()[tSN]);
    segOrderVec.push_back(_mkt->travelSeg()[tSN + 1]);
    // down one segment
    if (!ssv[tSN + 1]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK) && tSN + 2 < tsSize)
    {
      segOrderVec.push_back(_mkt->travelSeg()[tSN + 2]);
    }
    if (tsSize == segOrderVec.size())
    {
      return (_mkt->classOfServiceVec()[tSN]);
    }
    return getCOS(tSN, segOrderVec);
  }

  if (tSN && !ssv[tSN]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
  {
    // up one segment
    if (!ssv[tSN - 1]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
    {
      segOrderVec.push_back(_mkt->travelSeg()[tSN - 1]);
    }
    segOrderVec.push_back(_mkt->travelSeg()[tSN]);
    if (tSN + 1 < tsSize)
    {
      // down one segment
      segOrderVec.push_back(_mkt->travelSeg()[tSN + 1]);

      if (!ssv[tSN + 1]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK) &&
          (segOrderVec.size() < 3 && (tSN + 2) < tsSize))
      {
        // down two segments
        segOrderVec.push_back(_mkt->travelSeg()[tSN + 2]);
      }
    }
    if (tsSize == segOrderVec.size())
    {
      return (_mkt->classOfServiceVec()[tSN]);
    }
    return getCOS(tSN, segOrderVec);
  }

  return cosVec;
}

//**********************************************************************************
// Journey - find the correct fare market
//           and gets availability for the specified travel sector
//**********************************************************************************
std::vector<ClassOfService*>*
DifferentialValidator::getCOS(const int tSN, std::vector<TravelSeg*>& segOrderVec) const
{
  TravelSeg* tvl = _mkt->travelSeg()[tSN];
  AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);
  if (!airSeg)
  {
    return nullptr;
  }

  CarrierCode carrier = getCarrierCode(*airSeg, &segOrderVec);
  const FareMarket* fm = getFareMarket(segOrderVec, &carrier, &airSeg->carrier());

  /*TrxUtil::getFareMarket(*diffTrx(),
                                            carrier,
                                            segOrderVec,
                                            _paxTfare->retrievalDate(),
                                            _itin);
if(!fm && (carrier  != airSeg->carrier()))
{
 fm = TrxUtil::getFareMarket(*diffTrx(),
                             airSeg->carrier(),
                             segOrderVec,
                             _paxTfare->retrievalDate(),
                             _itin);
}*/
  if (!fm)
  {
    return nullptr;
  }

  const std::vector<TravelSeg*>& travelSeg = fm->travelSeg();
  for (uint16_t num = 0; num < travelSeg.size(); num++)
  {
    if (travelSeg[num] == tvl)
      return (fm->classOfServiceVec()[num]);
  }
  return nullptr;
}

bool
DifferentialValidator::validateThruFare(Differentials& di) const
{
  const PaxTypeFare& paxTfare = *throughFare();

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // First part of the validation - Validate THROUGH Fare data vs Differential Table
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // DIRECTIONALITY

  if (di.directionality() != BLANK)
  {
    if (!thruFareDirectionality(di))
      return false; // continue
  } // directionality is BLANK

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MATCH VIA LOCATION
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if (UNLIKELY(di.viaLoc().locType() != BLANK))
  {
    if (!matchLocation(di.viaLoc(), // LocKey
                       di.viaLoc().loc())) // LocCode
    {
      return false; // continue;
    }
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MATCH GLOBAL DIRECTION
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if (di.globalDir() != GlobalDirection::ZZ)
  {
    if (_mkt->getGlobalDirection() != di.globalDir())
      return false; // continue;
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MATCH Through Fare Type Code/Generic
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if (!di.fareType().empty())
  {
    if (!RuleUtil::matchFareType(di.fareType(), paxTfare.fcaFareType()))
    {
      return false; // continue;
    }
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Thru Fare Type Code is not empty, validate Thru BookingCode
  // or/and Thru Fare Class
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if (di.fareType().empty())
  {
    if (!di.bookingCode().empty())
    {
      // Match the BookingCode in all Travel Segments with "PASS" status

      if (!matchThroughBkgCode(di.bookingCode()))
      {
        return false; // continue;      // not match
      }
    }
    if (!di.fareClass().empty())
    {
      // Match the Fare Class of the Thru fare vs. Differential Table
      if (di.fareClass() != paxTfare.fareClass())
      {
        if (!RuleUtil::matchFareClass(di.fareClass().c_str(), paxTfare.fareClass().c_str()))
        {
          return false; // continue;         // not match
        }
      }
    } // FareClass
  } // FareType

  // At this point all Match for the Through Fare is done
  return true;
}

//**********************************************************************************
// WPNC, the fare is valid for Diff calculation if Diff bkg = Fare Bkg
//**********************************************************************************
bool
DifferentialValidator::wpncMatchBkgCodes(PaxTypeFare& df, DifferentialData& diffDP) const
{
  if (!_diffTrx->getRequest()->isLowFareRequested())
    return true;
  if (!diffDP.isRebookedBkg())
    return true;

  std::vector<TravelSeg*>& travelSeg = df.fareMarket()->travelSeg();
  std::vector<PaxTypeFare::SegmentStatus>& segmentStatus = df.segmentStatus();

  uint16_t tsSize = travelSeg.size();
  uint16_t segStSize = segmentStatus.size();
  // ERROR - Business logic error
  if (segStSize != tsSize)
  {
    LOG4CXX_ERROR(logger,
                  "Pass Diff fare: Leaving "
                  "DifferentialValidator::findHighFareForDiffSectorWPNC(): wrong size");
    return false; // Do not use the fare w/o status
  }

  for (uint16_t i = 0; i < segStSize; i++)
  {
    TravelSeg* tvl = travelSeg[i];
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);
    if (airSeg == nullptr)
      continue;

    if (!segmentStatus[i]._bkgCodeReBook.empty() &&
        segmentStatus[i]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      if (segmentStatus[i]._bkgCodeReBook == diffDP.bookingCode())
        return true;
    }
    else
    {
      if (travelSeg[i]->getBookingCode() == diffDP.bookingCode())
        return true;
    }
  }
  return false;
}

//**********************************************************************************
// If WPNC then try to determine the High Differential fare
//**********************************************************************************
void
DifferentialValidator::wpncFindFareForDiffSector(DifferentialData& diffDP, const int8_t increment)
    const
{
  if (!diffTrx()->getRequest()->isLowFareRequested())
    return;
  bool rv = true;

  rv = wpncFindHighFareForDiffSector(diffDP, diffDP.fareMarket(), diffDP.carrier(), increment);

  // Validate it for the current Carriers, before it was done for the Gov Cxr Override
  if (!rv && !diffDP.fareMarketCurrent().empty())
  {
    rv = wpncFindHighFareForDiffSector(
        diffDP, diffDP.fareMarketCurrent(), diffDP.carrierCurrent(), increment, false);
  }

  return;
}

//**********************************************************************************
// If WPNC then try to determine the High Differential fare
// for this Diff sector and use fare's bkg code in future fare selection
//**********************************************************************************
bool
DifferentialValidator::wpncFindHighFareForDiffSector(DifferentialData& diffDP,
                                                     std::vector<FareMarket*>& fareMarket,
                                                     std::vector<CarrierCode>& carrier,
                                                     const int8_t increment,
                                                     bool gOver) const
{
  // Differential data for 1 segment
  if (fareMarket.empty() || carrier.empty() || fareMarket.size() != carrier.size())
  {
    LOG4CXX_ERROR(logger, "WPNC FARE SELECT: \"fareMarket\" OR \"carrier\" CONTAINER IS CORRUPTED");
    return false; // Shouldn't ever happen, but it's better to check
  }

  bool rv = false;
  const PaxTypeFare& through = *throughFare(); // Through fare

  CabinType throughFTD = through.cabin();

  if (throughFTD.isPremiumFirstClass())
    return false; // The highest type fare

  size_t posThrough = throughFTD.index();

  if (posThrough == std::string::npos)
    return false; // Shouldn't ever happen, but it's better to check

  DiffDataFMI fmi = fareMarket.begin();
  DiffDataFMI fmiEnd = fareMarket.end();

  DiffDataCxrI carr = carrier.begin();

  CabinType failedFareCabin;
  cabinTypeFromFTD(diffDP.cabin(), failedFareCabin);

  // Multiple Governing carriers are allowed now...
  for (; fmi != fmiEnd; fmi++, carr++)
  { // This is valid only for consolidation.
    rv = false;
    FareMarket* fmiDiff = *fmi;
    if (fmiDiff == nullptr)
      continue;

    FareSelectionDataHelper fsdm(*diffTrx(), diffDP);

    CarrierCode govCXR = *carr;
    PaxTypeFarePtrVec paxTypeFareVec;
    if (!getPaxTypeFareVec(fsdm.paxTypeFareVec(),
                           *fmiDiff)) // Retrieve PaxTypeFares for the given PaxType
    {
      return false;
    }
    // The Carrier Application For Industry YY Pricing Table shall be interrogated to determine if
    // the governing carrier of DIFF sector permits using industry fares (YY) in a primary pricing.
    Indicator prPricePr = LOWEST_FARE;

    if (!getIndustryPrimePricingPrecedence(diffDP, govCXR, gOver, fmi, fmiDiff, prPricePr))
      return false;

    const uint8_t MAX_NUM_CARRIERS = (prPricePr == PREFER_CXR_FARE) ? 2 : 1;

    fsdm.globalDirection() = fmiDiff->getGlobalDirection();
    fsdm.prPricePr() = prPricePr;
    fsdm.slideAllowed() = allowPremiumCabinSlide(*carr);
    fsdm.govCXR() = govCXR;
    fsdm.throughFTD() = throughFTD;
    fsdm.ilow() = false;
    fsdm.failedFareCabin() = failedFareCabin;
    fsdm.throughRealFTD() = throughFTD;
    diffDP.setSlideAllow() = fsdm.slideAllowed();

    // If we went through this process once and didn't find the LOW fare we should repeat it one
    // more time
    // and try to find the LOW fare among the Economy class fares...
    // only under condition that the through fare is the Business fare

    bool cont = true; // It would be cleaner to use the goto operator instead

    govCXR = *carr; //  05/23/05

    // We'll try two times if the Carrier Application table holds value "C" == PREFER_CXR_FARE,
    // which means that Carrier fares take precedence over Industry fares.

    for (uint8_t iCnt = 0; cont && iCnt < MAX_NUM_CARRIERS; // Max of 2 iterations:
         iCnt++, govCXR = INDUSTRY_CARRIER) // the 1-st is for the real Governing carrier
    // and the 2-nd is for the Industry carrier ... only if needed
    {
      rv = processWPNCFareSelection(fsdm);
      if (rv)
        cont = false;
    } // for loop ---> ict

    if (rv)
    {
      FareSelectionData fsd;
      fsdm.getValue(fsd);
      uint16_t tsSize = fsd.fareHigh()->fareMarket()->travelSeg().size();
      uint16_t segStSize = fsd.fareHigh()->segmentStatus().size();
      // ERROR - Business logic error
      if (segStSize != tsSize)
      {
        LOG4CXX_ERROR(logger,
                      "Pass Diff fare: Leaving "
                      "DifferentialValidator::wpncFindHighFareForDiffSector(): wrong "
                      "size");
        return false; // Do not use the fare w/o status
      }
      int indexL = 0; // default, 1st sector in higher cabin
      if (increment == 1)
        indexL = increment; // 2nd sector in higher cabin

      for (int8_t i = 0; i < segStSize; i++)
      {
        if (indexL == i)
        {
          const PaxTypeFare::SegmentStatus& segStat = fsd.fareHigh()->segmentStatus()[i];
          TravelSeg* tvl = fsd.fareHigh()->fareMarket()->travelSeg()[i];
          AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);
          if (airSeg == nullptr)
            continue;

          if (!(segStat._bkgCodeReBook.empty()) &&
              segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
          {
            diffDP.bookingCode() = segStat._bkgCodeReBook;
            diffDP.setRebookedBkg() = true;
          }
          break;
        }
      }
    }
  }
  return rv;
}
//**********************************************************************************
// Convert FareTypeDesignator to CabinType
//**********************************************************************************
void
DifferentialValidator::cabinTypeFromFTD(const DifferentialData::FareTypeDesignators& cabin,
                                        CabinType& cabinType) const
{
  switch (cabin)
  {
  case DifferentialData::PREMIUM_FIRST_FTD:
    cabinType.setPremiumFirstClass();
    break;
  case DifferentialData::FIRST_FTD:
    cabinType.setFirstClass();
    break;
  case DifferentialData::PREMIUM_BUSINESS_FTD_NEW:
    cabinType.setPremiumBusinessClass();
    break;
  case DifferentialData::BUSINESS_FTD:
    cabinType.setBusinessClass();
    break;
  case DifferentialData::PREMIUM_ECONOMY_FTD:
    cabinType.setPremiumEconomyClass();
    break;
  default: // cabin == DifferentialData::ECONOMY_FTD || cabin == DifferentialData::BLANK_FTD
    cabinType.setEconomyClass();
    break;
  }
}
//**********************************************************************************
// Convert CabinType to FareTypeDesignator
//**********************************************************************************
DifferentialData::FareTypeDesignators
DifferentialValidator::cabinTypeToFTD(const CabinType& cabinType) const
{
  if (cabinType.isPremiumFirstClass())
    return DifferentialData::PREMIUM_FIRST_FTD;
  else if (cabinType.isFirstClass())
    return DifferentialData::FIRST_FTD;
  else if (cabinType.isPremiumBusinessClass())
  {
    return DifferentialData::PREMIUM_BUSINESS_FTD_NEW;
  }
  else if (cabinType.isBusinessClass())
    return DifferentialData::BUSINESS_FTD;
  else if (cabinType.isPremiumEconomyClass())
    return DifferentialData::PREMIUM_ECONOMY_FTD;
  //    else if(cabinType.isEconomyClass())
  //        return DifferentialData::ECONOMY_FTD;
  return DifferentialData::BLANK_FTD;
}
//**********************************************************************************
// Convert HIP to CabinType
//**********************************************************************************
void
DifferentialValidator::cabinTypeFromHip(const DifferentialData::HipRelated hip,
                                        CabinType& cabinType) const
{
  switch (hip)
  {
  case DifferentialData::PREMIUM_FIRST_HIP:
    cabinType.setPremiumFirstClass();
    break;
  case DifferentialData::PREMIUM_FIRST_HIP_ANSWER:
    cabinType.setPremiumFirstClass();
    break;
  case DifferentialData::FIRST_HIP:
    cabinType.setFirstClass();
    break;
  case DifferentialData::PREMIUM_BUSINESS_HIP:
    cabinType.setPremiumBusinessClass();
    break;
  case DifferentialData::BUSINESS_HIP:
    cabinType.setBusinessClass();
    break;
  case DifferentialData::PREMIUM_ECONOMY_HIP:
    cabinType.setPremiumEconomyClass();
    break;
  case DifferentialData::PREMIUM_ECONOMY_HIP_ANSWER:
    cabinType.setPremiumEconomyClass();
    break;
  case DifferentialData::ECONOMY_HIP:
    cabinType.setEconomyClass();
    break;
  default:
    cabinType.setInvalidClass();
  }
}
//**********************************************************************************
// Convert CabinType to HIP
//**********************************************************************************
DifferentialData::HipRelated
DifferentialValidator::cabinTypeToHip(const CabinType& cabin) const
{
  if (cabin.isPremiumFirstClass())
  {
    if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*diffTrx()))
      return DifferentialData::PREMIUM_FIRST_HIP_ANSWER;
    else
      return DifferentialData::PREMIUM_FIRST_HIP;
  }
  else if (cabin.isFirstClass())
    return DifferentialData::FIRST_HIP;
  else if (cabin.isPremiumBusinessClass())
    return DifferentialData::PREMIUM_BUSINESS_HIP;
  else if (cabin.isBusinessClass())
    return DifferentialData::BUSINESS_HIP;
  else if (cabin.isPremiumEconomyClass())
  {
    if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*diffTrx()))
      return DifferentialData::PREMIUM_ECONOMY_HIP_ANSWER;
    else
      return DifferentialData::PREMIUM_ECONOMY_HIP;
  }
  else if (cabin.isEconomyClass())
    return DifferentialData::ECONOMY_HIP;
  return DifferentialData::NO_HIP;
}
//**********************************************************************************
// Access carrier preference table to check if carrier allow differential calulation
// between premium economy and economy class
//**********************************************************************************
bool
DifferentialValidator::applyPremEconCabinDiffCalc(const CarrierCode& cxr) const
{
  const CarrierPreference* cxrPref =
      diffTrx()->dataHandle().getCarrierPreference(cxr, diffTrx()->ticketingDate());
  if (cxrPref)
    return cxrPref->applyPremEconCabinDiffCalc() == 'Y';
  return false;
}
//**********************************************************************************
// Access carriere preference table to check if carrier allow differential calulation
// between premium business and economy business
//**********************************************************************************
bool
DifferentialValidator::applyPremBusCabinDiffCalc(const CarrierCode& cxr) const
{
  if (!_paxTfare->cabin().isPremiumBusinessClass() && !_paxTfare->cabin().isPremiumEconomyClass())
    return true;
  const CarrierPreference* cxrPref =
      diffTrx()->dataHandle().getCarrierPreference(cxr, diffTrx()->ticketingDate());
  if (cxrPref)
    return cxrPref->applyPremBusCabinDiffCalc() == 'Y';
  return false;
}
//**********************************************************************************
// Access carriere preference table to check if carrier allow
// slide to a non-Premium cabin fare
//**********************************************************************************
bool
DifferentialValidator::allowPremiumCabinSlide(const CarrierCode& cxr) const
{
  const CarrierPreference* cxrPref =
      diffTrx()->dataHandle().getCarrierPreference(cxr, diffTrx()->ticketingDate());
  if (cxrPref)
    return cxrPref->noApplySlideToNonPremium() == 'N';
  return false;
}
//***********************************************************************************
// go thrue tarvel sectors and try to find matching IndystryPricingApplication
// record. When it's matched, Prime Pricing Precedence is returned
//***********************************************************************************
bool
DifferentialValidator::getIndustryPrimePricingPrecedence(DifferentialData& diffDP,
                                                         const CarrierCode& govCXR,
                                                         bool gOver,
                                                         const DiffDataFMI& fmi,
                                                         FareMarket*& fmiDiff,
                                                         Indicator& prPricePr) const
{
  // set to default value
  prPricePr = LOWEST_FARE;
  std::vector<TravelSeg*> trSeg = diffDP.travelSeg();

  std::vector<TravelSeg*>::const_iterator iB = trSeg.begin();
  std::vector<TravelSeg*>::const_iterator iE = trSeg.end();
  for (; iB != iE; iB++)
  { // Find travel date for the Diff segment:
    AirSeg* as = dynamic_cast<AirSeg*>(*iB);

    std::vector<TravelSeg*> segOrderVec;
    segOrderVec.push_back(*iB);

    if (!as)
      continue; // skip a surface sector

    if (!gOver || (gOver && !_diffTrx->getRequest()->industryFareOverride(as->segmentOrder())))
    {
      CarrierCode cxr = govCXR;
      GlobalDirection gdir = fmiDiff->getGlobalDirection();

      for (uint8_t i = 0; i < 2; ++i, cxr = "", gdir = GlobalDirection::ZZ)
      {
        if (diffDP.isItemConsolidated()) // 12.01.06 - skip for Normal Diff sector
        {
          CarrierCode carrier = getCarrierCode(*as, &segOrderVec);
          const FareMarket* fm = getFareMarket(segOrderVec, &carrier, &as->carrier());

          if (*fmi != fm)
            continue;

          fmiDiff = *fmi;
        }
        // virtual "getIndustryPricingAppl" wrapper created only to be overridden in CPP UNIT TEST
        const DateTime& travelDate = _diffTrx && (_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX)
                                         ? _travelDate
                                         : as->departureDT();

        const std::vector<const IndustryPricingAppl*>& indPrc =
            getIndustryPricingAppl(cxr, gdir, travelDate);
        Directionality direct;
        Indicator directDiff;

        std::vector<const IndustryPricingAppl*>::const_iterator il = indPrc.begin();
        std::vector<const IndustryPricingAppl*>::const_iterator ile = indPrc.end();
        for (; il != ile; il++)
        {
          const IndustryPricingAppl& ilr = **il;
          direct = ilr.directionality();

          if (direct == FROM || direct == TO)
            directDiff = DF_FROM;
          else if (direct == BETWEEN || direct == BOTH)
            directDiff = DF_BETWEEN;
          else if (direct == WITHIN)
            directDiff = DF_WITHIN;
          else
            directDiff = DF_ORIGIN;

          if (i ||
              matchLocation(
                  ilr.loc1(), ilr.loc1().loc(), ilr.loc2(), ilr.loc2().loc(), *fmiDiff, directDiff))
          {
            prPricePr = _toupper(ilr.primePricingAppl()); // Prime Pricing Precedence
            if (prPricePr != LOWEST_FARE && prPricePr != PREFER_CXR_FARE)
            {
              LOG4CXX_ERROR(logger,
                            "1) FARE SELECT: INVALID VALUE IN \"PRIME PRICING PRECEDENCE\" FIELD");
              return false;
            }
            // we matched pmime pricing appl, so we're done
            return true;
          }
        }
      }
      break;
    }
  }
  return true;
}
//***********************************************************************************
// go thrue tarvel sectors and try to find matching IndystryPricingApplication
// record. When it's matched, Prime Pricing Precedence is returned
//***********************************************************************************
bool
DifferentialValidator::getLowerCabin(CabinType& cabin) const
{
  if (cabin.isPremiumFirstClass())
    cabin.setFirstClass();
  else if (cabin.isFirstClass())
    cabin.setPremiumBusinessClass();
  else if (cabin.isPremiumBusinessClass())
    cabin.setBusinessClass();
  else if (cabin.isBusinessClass())
    cabin.setPremiumEconomyClass();
  else if (cabin.isPremiumEconomyClass())
    cabin.setEconomyClass();
  else
    return false;
  return true;
}
//***********************************************************************************
// Check if fare is eligible for fare selection
//***********************************************************************************
bool
DifferentialValidator::isFareValidForFareSelection(PaxTypeFarePtrVecIC& ptFare,
                                                   DifferentialData& diffDP,
                                                   Indicator prPricePr,
                                                   const CarrierCode& govCXR) const
{
  if (UNLIKELY(!(*ptFare)))
    return false;
  if ((**ptFare).isFareByRule() || (**ptFare).tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    return false;

  if ((**ptFare).bookingCodeStatus().isSet(PaxTypeFare::BKS_REQ_LOWER_CABIN))
    return false;

  if ((**ptFare).bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N))
    return false;

  const CarrierCode cc = (**ptFare).carrier();
  bool validCXR = (prPricePr == PREFER_CXR_FARE && govCXR == cc) ||
                  (prPricePr == LOWEST_FARE && (govCXR == cc || INDUSTRY_CARRIER == cc));
  if (!validCXR)
    return false;

  if (UNLIKELY((_diffTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
       _diffTrx->excTrxType() == PricingTrx::AF_EXC_TRX) &&
      !_farePath->rebookClassesExists() &&
      (*ptFare)->getRebookedClassesStatus() == PaxTypeFare::BKSS_REBOOKED))
    return false;

  if (!_farePath->validatingCarriers().empty())
  {
    const CarrierCode cxr = _farePath->validatingCarriers()[0];
    if (std::none_of((**ptFare).validatingCarriers().cbegin(),
                     (**ptFare).validatingCarriers().cend(),
                     [cxr](const CarrierCode valCarrier)
                     { return valCarrier == cxr; }))
      return false;
  }

  return checkDifferentialFare(**ptFare, diffDP);
}
//***********************************************************************************
// go thrue tarvel sectors and try to find matching IndystryPricingApplication
// record. When it's matched, Prime Pricing Precedence is returned
//***********************************************************************************
bool
DifferentialValidator::processNormalFareSelection(FareSelectionDataHelper& fsdm) const
{
  const Indicator diffCalculation = _toupper(fsdm.diffDP().calculationIndicator());
  const std::size_t RESTRICT_NUM =
      (diffCalculation == SAME_TYPE || diffCalculation == NOT_FOUND_IN_TABLE) ? 2 : 1;

  CabinType throughFTD = fsdm.throughFTD();
  CabinType diffFTD;

  size_t posThrough = throughFTD.index();
  size_t posL_or_HF = 0;
  const FareType throughFT =
      throughFare()->fcaFareType(); // Indicates either it is restricted or not

  // This loop is needed only if diffCalculation == 'S' to discard the requirement imposed on both
  // through and diff fares
  // to be simultaneously of the same Fare type (Restricted or not) in the case if no fares found.

  for (size_t ii = 0; ii < RESTRICT_NUM;
       ii++) // Depending on "diffCalculation" do this 1 or 2 times
  {
    PaxTypeFarePtrVecIC diffFare = fsdm.paxTypeFareVec().begin();
    for (; diffFare != fsdm.paxTypeFareVec().end(); diffFare++)
    {
      if (!isFareValidForFareSelection(diffFare, fsdm.diffDP(), fsdm.prPricePr(), fsdm.govCXR()))
        continue;

      PaxTypeFare& df = **diffFare;
      diffFTD = df.cabin();

      if (LIKELY(!fsdm.ilow())) // if it's a second run we are always looking for a LOW fares only
      {
        posL_or_HF = diffFTD.index();
        if (posL_or_HF == std::string::npos || posL_or_HF < posThrough)
          continue;
        // check failed RBD for high or through fare cabin for low
        if (diffFTD != fsdm.failedFareCabin() && diffFTD != throughFTD)
          continue;
      }
      else if (diffFTD < throughFTD)
        continue;

      if (!fsdm.matchSlide(diffFTD))
        continue;

      FareSelectionData* fsd = fsdm[diffFTD];
      if (!fsd)
        continue;

      PaxTypeCode rptc = requestedPaxType()->paxType();
      PaxTypeCode aptcL =
          (fsdm.diffDP().fareLow()) ? fsdm.diffDP().fareLow()->actualPaxType()->paxType() : "";
      PaxTypeCode aptcH =
          (fsdm.diffDP().fareHigh()) ? fsdm.diffDP().fareHigh()->actualPaxType()->paxType() : "";
      switch (diffCalculation)
      {
      case NOT_FOUND_IN_TABLE: // A fall through case
      case SAME_TYPE:
        if (fsdm.checkLowHigh(posL_or_HF, posThrough)) // Low
        {
          if (!fsd->foundLow()) // don't overwrite the values for the genuine CXR
          {
            if (checkFareType(throughFT, df, ii > 0) // Both should be simultaneously
                // Restricted or not
                &&
                (fsdm.diffDP().amountFareClassLow() == 0.F || (aptcL == ADULT && rptc == CHILD) ||
                 fsdm.diffDP().amountFareClassLow() > df.nucFareAmount()))
            {
              setDiffLowHigh(*fsd, df, LOW);
              fsd->foundLow() = true;
            }
          }
        }
        else if (isBkgCodeStatusPass(df, true)) // High
        {
          if (!fsd->foundHigh()) // don't overwrite the values for the genuine CXR
          {
            if (!validateHighFareForPremiumEconomyCabin(fsdm.diffDP(), df))
              continue;
            if (checkFareType(throughFT, df, ii > 0) // Both should be simultaneously
                // Restricted or not
                &&
                (fsdm.diffDP().amountFareClassHigh() == 0.F || (aptcH == ADULT && rptc == CHILD) ||
                 fsdm.diffDP().amountFareClassHigh() > df.nucFareAmount()) &&
                wpncMatchBkgCodes(df, fsdm.diffDP()))
            {
              setDiffLowHigh(*fsd, df, HIGH);
              fsd->foundHigh() = true;
            }
          }
        }
        break;
      case LOW_TYPE:
        if (fsdm.checkLowHigh(posL_or_HF, posThrough)) // Low
        {
          if (!fsd->foundLow()) // don't overwrite the values for the genuine CXR
          {
            if (fsdm.diffDP().amountFareClassLow() == 0.F || (aptcL == ADULT && rptc == CHILD) ||
                fsdm.diffDP().amountFareClassLow() > df.nucFareAmount())
            {
              setDiffLowHigh(*fsd, df, LOW);
              fsd->foundLow() = true;
            }
          }
          else if ((fsdm.diffDP().amountFareClassLow() == 0.F ||
                    (aptcL == ADULT && rptc == CHILD)) &&
                   fsd->amountFareClassLow() > df.nucFareAmount() &&
                   ((fsdm.prPricePr() == PREFER_CXR_FARE && !df.fare()->isIndustry()) ||
                    fsdm.prPricePr() == LOWEST_FARE))
          {
            setDiffLowHigh(*fsd, df, LOW);
            fsd->foundLow() = true;
          }
        }
        else if (isBkgCodeStatusPass(df, true)) // High
        {
          if (!validateHighFareForPremiumEconomyCabin(fsdm.diffDP(), df))
            continue;
          if (!fsd->foundHigh()) // don't overwrite the values for the genuine CXR
          {
            if ((fsdm.diffDP().amountFareClassHigh() == 0.F || (aptcH == ADULT && rptc == CHILD) ||
                 fsdm.diffDP().amountFareClassHigh() > df.nucFareAmount()) &&
                wpncMatchBkgCodes(df, fsdm.diffDP()))
            {
              setDiffLowHigh(*fsd, df, HIGH);
              fsd->foundHigh() = true;
            }
          }
          else if ((fsdm.diffDP().amountFareClassHigh() == 0.F ||
                    (aptcH == ADULT && rptc == CHILD)) &&
                   fsd->amountFareClassHigh() > df.nucFareAmount() &&
                   ((fsdm.prPricePr() == PREFER_CXR_FARE && !df.fare()->isIndustry()) ||
                    fsdm.prPricePr() == LOWEST_FARE) &&
                   wpncMatchBkgCodes(df, fsdm.diffDP()))
          {
            setDiffLowHigh(*fsd, df, HIGH);
            fsd->foundHigh() = true;
          }
        }
        break;
      case HIGH_TYPE:
        if (fsdm.checkLowHigh(posL_or_HF, posThrough)) // Low
        {
          if (!fsd->foundLow()) // don't overwrite the values for the genuine CXR
          {
            if ((fsdm.globalDirection() == df.globalDirection() ||
                 df.globalDirection() == GlobalDirection::ZZ) &&
                (fsdm.diffDP().amountFareClassLow() == 0.F || (aptcL == ADULT && rptc == CHILD) ||
                 fsdm.diffDP().amountFareClassLow() < df.nucFareAmount()))
            {
              setDiffLowHigh(*fsd, df, LOW);
              fsd->foundLow() = true;
            }
          }
          else if ((fsdm.globalDirection() == df.globalDirection() ||
                    df.globalDirection() == GlobalDirection::ZZ) &&
                   ((fsdm.diffDP().amountFareClassLow() == 0.F ||
                     (aptcL == ADULT && rptc == CHILD)) &&
                    fsd->amountFareClassLow() < df.nucFareAmount()) &&
                   ((fsdm.prPricePr() == PREFER_CXR_FARE && !df.fare()->isIndustry()) ||
                    fsdm.prPricePr() == LOWEST_FARE))
          {
            setDiffLowHigh(*fsd, df, LOW);
            fsd->foundLow() = true;
          }
        }
        else if (isBkgCodeStatusPass(df, true)) // High
        {
          if (!validateHighFareForPremiumEconomyCabin(fsdm.diffDP(), df))
            continue;
          if (!fsd->foundHigh()) // don't overwrite the values for the genuine CXR
          {
            if ((fsdm.globalDirection() == df.globalDirection() ||
                 df.globalDirection() == GlobalDirection::ZZ) &&
                (fsdm.diffDP().amountFareClassLow() == 0.F || (aptcH == ADULT && rptc == CHILD) ||
                 fsdm.diffDP().amountFareClassLow() < df.nucFareAmount()) &&
                wpncMatchBkgCodes(df, fsdm.diffDP()))
            {
              setDiffLowHigh(*fsd, df, HIGH);
              fsd->foundHigh() = true;
            }
          }
          else
          {
            if ((fsdm.globalDirection() == df.globalDirection() ||
                 df.globalDirection() == GlobalDirection::ZZ) &&
                ((fsdm.diffDP().amountFareClassLow() == 0.F || (aptcH == ADULT && rptc == CHILD)) &&
                 fsd->amountFareClassHigh() < df.nucFareAmount()) &&
                ((fsdm.prPricePr() == PREFER_CXR_FARE && !df.fare()->isIndustry()) ||
                 fsdm.prPricePr() == LOWEST_FARE))
            {
              setDiffLowHigh(*fsd, df, HIGH);
              fsd->foundHigh() = true;
            }
          }
        }
        break;
      default:
        break;
      } // endswitch - diff calc
    } // endfor - all diffFare

    if (fsdm.foundLow() && fsdm.foundHigh())
    {
      return true;
    }
  } // for loop ---> ii
  return false;
}
bool
DifferentialValidator::processWPNCFareSelection(FareSelectionDataHelper& fsdm) const
{
  const size_t RESTRICT_NUM = 2;
  CabinType throughFTD = fsdm.throughFTD();
  CabinType diffFTD;

  size_t posThrough = throughFTD.index();
  size_t posL_or_HF = 0;
  // This loop is needed only if diffCalculation == 'S' to discard the requirement imposed on both
  // through and diff fares
  // to be simultaneously of the same Fare type (Restricted or not) in the case if no fares found.

  for (size_t ii = 0; ii < RESTRICT_NUM; ii++) // Depending on - do this 1 or 2 times
  {
    PaxTypeFarePtrVecIC diffFare = fsdm.paxTypeFareVec().begin();
    for (; diffFare != fsdm.paxTypeFareVec().end(); diffFare++)
    {
      if (!isFareValidForFareSelection(diffFare, fsdm.diffDP(), fsdm.prPricePr(), fsdm.govCXR()))
        continue;

      PaxTypeFare& df = **diffFare;

      diffFTD = df.cabin();
      posL_or_HF = diffFTD.index();
      if (posL_or_HF == std::string::npos || posL_or_HF < posThrough)
        continue;

      if (diffFTD != fsdm.failedFareCabin())
        continue;

      if (!fsdm.matchSlide(diffFTD))
        continue;

      FareSelectionData* fsd = fsdm[diffFTD];
      if (!fsd)
        continue;

      PaxTypeCode rptc = requestedPaxType()->paxType();
      PaxTypeCode aptcH =
          (fsdm.diffDP().fareHigh()) ? fsdm.diffDP().fareHigh()->actualPaxType()->paxType() : "";

      if (isBkgCodeStatusPass(df, true)) // Check & sets up the High fare only
      {
        if (!validateHighFareForPremiumEconomyCabin(fsdm.diffDP(), df))
          continue;
        if (!fsd->foundHigh()) // don't overwrite the values for the genuine CXR
        {
          if (fsdm.diffDP().amountFareClassHigh() == 0.F || (aptcH == ADULT && rptc == CHILD) ||
              fsdm.diffDP().amountFareClassHigh() > df.nucFareAmount())
          {
            setDiffLowHigh(*fsd, df, HIGH);
            fsd->foundHigh() = true;
          }
        }
        else if ((fsdm.diffDP().amountFareClassHigh() == 0.F ||
                  (aptcH == ADULT && rptc == CHILD)) &&
                 fsd->amountFareClassHigh() > df.nucFareAmount() &&
                 ((fsdm.prPricePr() == PREFER_CXR_FARE && !df.fare()->isIndustry()) ||
                  fsdm.prPricePr() == LOWEST_FARE))
        {
          setDiffLowHigh(*fsd, df, HIGH);
          fsd->foundHigh() = true;
        }
      }
    } // endfor - all diffFare

    if (fsdm.foundHigh())
    {
      return true;
    }
  } // for loop ---> ii
  return false;
}
// Helper class used during fare selection process

DifferentialValidator::FareSelectionDataHelper::FareSelectionDataHelper(PricingTrx& trx,
                                                                        DifferentialData& diffDP)
  : _prPricePr(LOWEST_FARE), _diffDP(diffDP)
{ // initialize fare selection map
  for (size_t i = 0; i < CabinType::size(); i++)
  {
    FareSelectionData* ptr = nullptr;
    trx.dataHandle().get(ptr);
    _fareSelectionMap.insert(std::pair<size_t, FareSelectionData*>(i, ptr));
  }
}
// *****************************************************************************
// operator [], return pointer to fare selection data
// *****************************************************************************
DifferentialValidator::FareSelectionData*
DifferentialValidator::FareSelectionDataHelper::
operator[](const CabinType& c)
{
  size_t index = c.index();
  if (index == std::string::npos)
    return nullptr;
  return _fareSelectionMap[index];
}
// *****************************************************************************
// check if any of fare selection data items have foundLow set
// *****************************************************************************
bool
DifferentialValidator::FareSelectionDataHelper::foundLow() const
{
  return std::any_of(_fareSelectionMap.cbegin(), _fareSelectionMap.cend(),
                     [](const std::pair<size_t, FareSelectionData*>& fareSelection)
                     { return fareSelection.second->foundLow(); });
}
// *****************************************************************************
// check if any of fare selection data items have foundHigh set
// *****************************************************************************
bool
DifferentialValidator::FareSelectionDataHelper::foundHigh() const
{
  return std::any_of(_fareSelectionMap.cbegin(), _fareSelectionMap.cend(),
                     [](const std::pair<size_t, FareSelectionData*>& fareSelection)
                     { return fareSelection.second->foundHigh(); });
}
// *****************************************************************************
// Go thrue all FareSelectiondata items and see it there is a match on low or high fare,
// if yes, high/low data is copied
// *****************************************************************************
void
DifferentialValidator::FareSelectionDataHelper::getValue(FareSelectionData& fsdGl) const
{
  // from economy to premium first
  for (std::pair<size_t, FareSelectionData*> fareSelection : _fareSelectionMap)
  {
    FareSelectionData& fsd = *(fareSelection.second);
    if (fsd.foundLow()) // copy low data
    {
      fsdGl.fareLow() = fsd.fareLow();
      fsdGl.fareClassLow() = fsd.fareClassLow();
      fsdGl.amountFareClassLow() = fsd.amountFareClassLow();
      fsdGl.fareLowOW() = fsd.fareLowOW();
      fsdGl.fareClassLowOW() = fsd.fareClassLowOW();
      fsdGl.amountFareClassLowOW() = fsd.amountFareClassLowOW();
      fsdGl.foundLow() = fsd.foundLow();
    }
    if (fsd.foundHigh()) // copy high data
    {
      fsdGl.fareHigh() = fsd.fareHigh();
      fsdGl.fareClassHigh() = fsd.fareClassHigh();
      fsdGl.amountFareClassHigh() = fsd.amountFareClassHigh();
      fsdGl.fareHighOW() = fsd.fareHighOW();
      fsdGl.fareClassHighOW() = fsd.fareClassHighOW();
      fsdGl.amountFareClassHighOW() = fsd.amountFareClassHighOW();
      fsdGl.foundHigh() = fsd.foundHigh();
      fsdGl.diffCxr() = govCXR();
    }
  }
}
//*******************************************************************************************
//         Save intermidiate Low/High money amount depending on their differences
//*******************************************************************************************
void
DifferentialValidator::FareSelectionDataHelper::saveTempFSData(FareSelectionData& fsdGl) const
{
  FareSelectionData fsd;
  getValue(fsd);
  MoneyAmount highFsdGl = (fsdGl.amountFareClassHigh() != 0.f) ? (fsdGl.amountFareClassHigh())
                                                               : (fsdGl.amountFareClassHighOW());
  MoneyAmount lowFsdGl = (fsdGl.amountFareClassLow() != 0.f) ? (fsdGl.amountFareClassLow())
                                                             : (fsdGl.amountFareClassLowOW());
  MoneyAmount highFsd = (fsd.amountFareClassHigh() != 0.f) ? (fsd.amountFareClassHigh())
                                                           : (fsd.amountFareClassHighOW());
  MoneyAmount lowFsd =
      (fsd.amountFareClassLow() != 0.f) ? (fsd.amountFareClassLow()) : (fsd.amountFareClassLowOW());
  if (highFsd - lowFsd < highFsdGl - lowFsdGl)
    fsdGl = fsd;
}
//*******************************************************************************************
// Return true if this is a low fare (Fare Type Designator of the Differential Fare equal to
// the FTD of the through Fare).
//*******************************************************************************************
bool
DifferentialValidator::FareSelectionDataHelper::checkLowHigh(const size_t& posL_or_HF,
                                                             const size_t& posThrough) const
{
  return _ilow || (posL_or_HF == posThrough);
}
//*******************************************************************************************
// Return true if this is a low fare (Fare Type Designator of the Differential Fare equal to
// the FTD of the through Fare).
//*******************************************************************************************
bool
DifferentialValidator::FareSelectionDataHelper::matchSlide(const CabinType& c) const
{
  if (_throughRealFTD.isPremiumBusinessClass() && c.isBusinessClass())
    return _slideAllowed;

  if (_throughRealFTD.isPremiumEconomyClass() && c.isEconomyClass())
    return _slideAllowed;

  return true;
}

RepricingTrx*
DifferentialValidator::getRepricingTrx(const std::vector<TravelSeg*>& tvlSeg,
                                       FMDirection fmDirectionOverride) const
{
  RepricingTrx* rpTrx = nullptr;

  FareMarket::RetrievalInfo* retrievalInfo = nullptr;
  if (diffTrx()->excTrxType() == PricingTrx::AR_EXC_TRX)
    retrievalInfo = _paxTfare->retrievalInfo();

  try
  {
    rpTrx = TrxUtil::reprice(*diffTrx(),
                             tvlSeg,
                             fmDirectionOverride,
                             false,
                             nullptr,
                             nullptr,
                             "",
                             false,
                             false,
                             'I',
                             0,
                             false,
                             false,
                             false,
                             retrievalInfo);
  }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger,
                  "DifferentialValidator: Exception during repricing with " << ex.code() << " - "
                                                                            << ex.message());
    return nullptr;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "DifferentialValidator: Unknown exception during repricing");
    return nullptr;
  }

  //  rpTrx->redirectedDiagnostic() = &trx.diagnostic();

  return rpTrx;
}

const FareMarket*
DifferentialValidator::getFareMarket(const std::vector<TravelSeg*>& segOrderVec,
                                     const CarrierCode* carrier,
                                     const CarrierCode* altCarrier,
                                     bool* gOver) const
{
  const FareMarket* fm =
      TrxUtil::getFareMarket(*diffTrx(), *carrier, segOrderVec, _paxTfare->retrievalDate(), _itin);
  if (!fm)
  {
    if (altCarrier && (*carrier != *altCarrier))
    {
      if (gOver)
        *gOver = false;
      fm = TrxUtil::getFareMarket(
          *diffTrx(), *altCarrier, segOrderVec, _paxTfare->retrievalDate(), _itin);
    }
  }

  if (!fm)
  {
    // try repricing
    RepricingTrx* reTrx = getRepricingTrx(
        segOrderVec, _paxTfare->directionality() == TO ? FMDirection::INBOUND : FMDirection::UNKNOWN);

    if (reTrx)
    {
      fm = TrxUtil::getFareMarket(*reTrx, *carrier, segOrderVec, _paxTfare->retrievalDate(), _itin);

      if (!fm)
      {
        if (altCarrier && (*carrier != *altCarrier))
        {
          if (gOver)
            *gOver = false;
          fm = TrxUtil::getFareMarket(
              *reTrx, *altCarrier, segOrderVec, _paxTfare->retrievalDate(), _itin);
        }
      }
    }
  }

  return fm;
}
}
