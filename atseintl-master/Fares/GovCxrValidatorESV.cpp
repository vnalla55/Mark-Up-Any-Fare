//----------------------------------------------------------------------------
//  File:        GovCxrValidator.cpp
//  Created:     2008-04-16
//
//  Description: Class used to validate fares for governing carrier
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Fares/GovCxrValidatorESV.h"

#include "Common/BookingCodeUtil.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MultiDimensionalPQ.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "Fares/FareValidatorESV.h"
#include "Server/TseServer.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackVisMarriedCabins);
FALLBACK_DECL(reworkTrxAborter);
namespace
{
Logger
logger("atseintl.Fares.GovCxrValidatorESV");
}

void
GovCxrValidatorESV::performTask()
{
  _checkMarriedBookingCodes = !fallback::fixed::fallbackVisMarriedCabins();
  processGovCarrier();
}

void
GovCxrValidatorESV::processGovCarrier()
{
  TSELatencyData metrics(*_trx, "FVO PROCESS GOV CARRIER");

  LOG4CXX_DEBUG(logger, "GovCxrValidatorESV::processGovCarrier()");

  // Go thorough all connection keys
  ItinIndex::ItinRowIterator rowIter((*_itinRow).begin()), rowIterEnd((*_itinRow).end());

  for (; rowIter != rowIterEnd; ++rowIter)
  {
    ItinIndex::ItinColumn& itinColumn = rowIter->second;

    // Go thorough all itin cells
    ItinIndex::ItinColumnIterator itinColumnIter(itinColumn.begin()),
        itinColumnIterEnd(itinColumn.end());

    for (; itinColumnIter != itinColumnIterEnd; ++itinColumnIter)
    {
      ItinIndex::ItinCell& itinCell = (*itinColumnIter);

      ItinIndex::ItinCellInfo& itinCellInfo = itinCell.first;

      // Skip processing of dummy itineraries
      if (true == (itinCellInfo.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT))
      {
        continue;
      }

      _flightId = itinCellInfo.sopIndex();

      _itin = itinCell.second;

      // Check if itinerary object is not NULL
      if (nullptr == _itin)
      {
        LOG4CXX_ERROR(logger, "GovCxrValidatorESV::processGovCarrier - Itinerary object is NULL.");
        continue;
      }

      processFlight(itinCellInfo);
    }
  }

  // Check timeout for transaction
  if (fallback::reworkTrxAborter(_trx))
    checkTrxAborted(*_trx);
  else
    _trx->checkTrxAborted();
}

void
GovCxrValidatorESV::processFlight(ItinIndex::ItinCellInfo& itinCellInfo)
{
  TSELatencyData metrics((*_trx), "FVO PROCESS FLIGHT ESV");

  LOG4CXX_DEBUG(logger, "GovCxrValidatorESV::processFlight(ItinIndex::ItinCellInfo&)");

  // Build fare market path matrix
  std::vector<std::vector<FareMarket*> > fareMarketPathMatrix;
  buildFareMarketPaths(fareMarketPathMatrix);

  if (true == fareMarketPathMatrix.empty())
  {
    LOG4CXX_INFO(logger, "GovCxrValidatorESV::processFlight - Fare market path matrix is empty.");
    return;
  }

  // Set correct travel segment vector in journey itin
  std::vector<TravelSeg*> tempTravelSeg;

  if (false == setupJourneyItinTravelSeg(tempTravelSeg))
  {
    LOG4CXX_ERROR(
        logger,
        "GovCxrValidatorESV::processFlight - Journey itin travel segments are not set correctly.");
    return;
  }

  // Count total penalty for SOP
  _itin->updateTotalPenalty((*_trx).esvOptions(), (*_trx).getRequest()->requestedDepartureDT());

  // Go thorough all fare market paths
  std::vector<std::vector<FareMarket*> >::const_iterator fareMarketPathIter(
      fareMarketPathMatrix.begin()),
      fareMarketPathIterEnd(fareMarketPathMatrix.end());

  for (; fareMarketPathIter != fareMarketPathIterEnd; ++fareMarketPathIter)
  {
    const std::vector<FareMarket*>& fareMarketPath = (*fareMarketPathIter);

    processFareMarketPath(itinCellInfo, fareMarketPath);
  }

  processItinJCBFlag(fareMarketPathMatrix);

  // Sort all valid fare market paths by money amount
  sortSopFareLists();

  // Restore original journey itin travel segment
  _journeyItin->travelSeg().swap(tempTravelSeg);
}

void
GovCxrValidatorESV::processFareMarketPath(ItinIndex::ItinCellInfo& itinCellInfo,
                                          const std::vector<FareMarket*>& fareMarketPath)
{
  TSELatencyData metrics((*_trx), "FVO PROCESS FARE MARKET PATH");

  LOG4CXX_DEBUG(logger,
                "GovCxrValidatorESV::processFareMarketPath(ItinIndex::ItinCellInfo&, "
                "std::vector<FareMarket*>&)");

  // Go thorough all passenger types
  std::vector<PaxType*>::const_iterator paxTypeIter((*_trx).paxType().begin()),
      paxTypeIterEnd((*_trx).paxType().end());

  for (; paxTypeIter != paxTypeIterEnd; ++paxTypeIter)
  {
    PaxType* paxType = (*paxTypeIter);

    // Check if passenger type object is not NULL
    if (nullptr == paxType)
    {
      LOG4CXX_ERROR(logger,
                    "GovCxrValidatorESV::processFareMarketPath - Passenger type object is NULL.");
      continue;
    }

    processFareMarketPathForPaxType(itinCellInfo, fareMarketPath, paxType);
  }
}

void
GovCxrValidatorESV::processFareMarketPathForPaxType(ItinIndex::ItinCellInfo& itinCellInfo,
                                                    const std::vector<FareMarket*>& fareMarketPath,
                                                    PaxType* paxType)
{
  TSELatencyData metrics((*_trx), "FVO PROCESS FARE MARKET PATH FOR PAX TYPE");

  LOG4CXX_DEBUG(logger,
                "GovCxrValidatorESV::processFareMarketPathForPaxType(ItinIndex::"
                "ItinCellInfo&, std::vector<FareMarket*>&, PaxType*)");

  std::vector<FareValidatorESV::VFHolder> vfHoldersVec;

  // Go thorough all fare markets
  for (size_t fmIdx = 0; fmIdx < fareMarketPath.size(); ++fmIdx)
  {
    FareMarket* fareMarket = fareMarketPath[fmIdx];

    // Find cheapest valid one way, round trip and open jaw fare for this
    // fare market and specified passenger type

    FareValidatorESV fareValidatorESV((*_trx),
                                      (*_tseServer),
                                      (*_leg),
                                      _journeyItin,
                                      _itin,
                                      itinCellInfo,
                                      _flightId,
                                      fareMarket,
                                      fareMarketPath.size(),
                                      paxType,
                                      (*_ruleControllersESV),
                                      (*_rule_validation_order));

    if (_checkMarriedBookingCodes)
    {
      if (fareMarket->isLocalJourneyFare())
      {
        FareValidatorESV::LocalJourneyValidatedFares* ljvf =
            FareValidatorESV::LocalJourneyValidatedFares::create(_trx->dataHandle());

        fareValidatorESV.findFirstValidFares(*ljvf);
        vfHoldersVec.push_back(FareValidatorESV::VFHolder(ljvf));
      }
      else
      {
        FareValidatorESV::ValidatedFares* vf;
        (*_trx).dataHandle().get(vf);

        fareValidatorESV.findFirstValidFares(*vf);
        vfHoldersVec.push_back(FareValidatorESV::VFHolder(vf));
      }
    }
    else
    {
      if (fareMarket->isLocalJourneyFare())
      {
        // now check neighbors
        if ((fmIdx > 0 &&
             fareMarket->governingCarrier() == fareMarketPath[fmIdx - 1]->governingCarrier()) ||
            (fmIdx < fareMarketPath.size() - 1 &&
             fareMarket->governingCarrier() == fareMarketPath[fmIdx + 1]->governingCarrier()))
        {
          FareValidatorESV::LocalJourneyValidatedFares* ljvf =
              FareValidatorESV::LocalJourneyValidatedFares::create(_trx->dataHandle());

          fareValidatorESV.findFirstValidFares(*ljvf);
          vfHoldersVec.push_back(FareValidatorESV::VFHolder(ljvf));

          continue;
        }
      }

      FareValidatorESV::ValidatedFares* vf;
      (*_trx).dataHandle().get(vf);

      fareValidatorESV.findFirstValidFares(*vf);
      vfHoldersVec.push_back(FareValidatorESV::VFHolder(vf));
    }
  }

  std::vector<PaxTypeFare*> owPaxTypeFareVec;
  std::vector<PaxTypeFare*> rtPaxTypeFareVec;
  std::vector<PaxTypeFare*> ctPaxTypeFareVec;
  std::vector<PaxTypeFare*> ojR_PaxTypeFareVec;
  std::vector<PaxTypeFare*> ojX_PaxTypeFareVec;
  std::vector<PaxTypeFare*> ojRR_PaxTypeFareVec;
  std::vector<PaxTypeFare*> ojRX_PaxTypeFareVec;
  std::vector<PaxTypeFare*> ojXR_PaxTypeFareVec;
  std::vector<PaxTypeFare*> ojXX_PaxTypeFareVec;
  std::vector<PaxTypeFare*> owPaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> rtPaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ctPaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ojR_PaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ojX_PaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ojRR_PaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ojRX_PaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ojXR_PaxTypeFareVecNoCat10;
  std::vector<PaxTypeFare*> ojXX_PaxTypeFareVecNoCat10;

  MoneyAmount owTotalAmount = 0;
  MoneyAmount rtTotalAmount = 0;
  MoneyAmount ctTotalAmount = 0;
  MoneyAmount ojR_TotalAmount = 0;
  MoneyAmount ojX_TotalAmount = 0;
  MoneyAmount ojRR_TotalAmount = 0;
  MoneyAmount ojRX_TotalAmount = 0;
  MoneyAmount ojXR_TotalAmount = 0;
  MoneyAmount ojXX_TotalAmount = 0;
  MoneyAmount owTotalAmountNoCat10 = 0;
  MoneyAmount rtTotalAmountNoCat10 = 0;
  MoneyAmount ctTotalAmountNoCat10 = 0;
  MoneyAmount ojR_TotalAmountNoCat10 = 0;
  MoneyAmount ojX_TotalAmountNoCat10 = 0;
  MoneyAmount ojRR_TotalAmountNoCat10 = 0;
  MoneyAmount ojRX_TotalAmountNoCat10 = 0;
  MoneyAmount ojXR_TotalAmountNoCat10 = 0;
  MoneyAmount ojXX_TotalAmountNoCat10 = 0;

  // We create open jaw constructions only for 2 fare markets
  if (2 != fareMarketPath.size())
  {
    ojRR_TotalAmount = -1;
    ojRX_TotalAmount = -1;
    ojXR_TotalAmount = -1;
    ojXX_TotalAmount = -1;
    ojRR_TotalAmountNoCat10 = -1;
    ojRX_TotalAmountNoCat10 = -1;
    ojXR_TotalAmountNoCat10 = -1;
    ojXX_TotalAmountNoCat10 = -1;
  }

  if (1 != fareMarketPath.size())
  {
    ojR_TotalAmount = -1;
    ojX_TotalAmount = -1;
    ojR_TotalAmountNoCat10 = -1;
    ojX_TotalAmountNoCat10 = -1;
  }

  // We create circle trip constructions max for 2 fare markets
  if (fareMarketPath.size() > 2)
  {
    ctTotalAmount = -1;
    ctTotalAmountNoCat10 = -1;
  }

  calculateTotalAmount(
      fareMarketPath, FareValidatorESV::VDF_TAG_1_3, vfHoldersVec, owTotalAmount, owPaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3_104,
                       vfHoldersVec,
                       owTotalAmountNoCat10,
                       owPaxTypeFareVecNoCat10);

  calculateTotalAmount(
      fareMarketPath, FareValidatorESV::VDF_TAG_1_2, vfHoldersVec, rtTotalAmount, rtPaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_2_102_104,
                       vfHoldersVec,
                       rtTotalAmountNoCat10,
                       rtPaxTypeFareVecNoCat10);

  calculateTotalAmount(
      fareMarketPath, FareValidatorESV::VDF_TAG_1_2, vfHoldersVec, ctTotalAmount, ctPaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_2_103,
                       vfHoldersVec,
                       ctTotalAmountNoCat10,
                       ctPaxTypeFareVecNoCat10);

  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_2,
                       vfHoldersVec,
                       ojR_TotalAmount,
                       ojR_PaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_2_104,
                       vfHoldersVec,
                       ojR_TotalAmountNoCat10,
                       ojR_PaxTypeFareVecNoCat10);

  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3,
                       vfHoldersVec,
                       ojX_TotalAmount,
                       ojX_PaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3_104,
                       vfHoldersVec,
                       ojX_TotalAmountNoCat10,
                       ojX_PaxTypeFareVecNoCat10);

  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3,
                       FareValidatorESV::VDF_TAG_2,
                       vfHoldersVec,
                       ojXR_TotalAmount,
                       ojXR_PaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3_104,
                       FareValidatorESV::VDF_TAG_2_104,
                       vfHoldersVec,
                       ojXR_TotalAmountNoCat10,
                       ojXR_PaxTypeFareVecNoCat10);

  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_2,
                       FareValidatorESV::VDF_TAG_1_3,
                       vfHoldersVec,
                       ojRX_TotalAmount,
                       ojRX_PaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_2_104,
                       FareValidatorESV::VDF_TAG_1_3_104,
                       vfHoldersVec,
                       ojRX_TotalAmountNoCat10,
                       ojRX_PaxTypeFareVecNoCat10);

  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3,
                       vfHoldersVec,
                       ojXX_TotalAmount,
                       ojXX_PaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_1_3_104,
                       vfHoldersVec,
                       ojXX_TotalAmountNoCat10,
                       ojXX_PaxTypeFareVecNoCat10);

  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_2,
                       vfHoldersVec,
                       ojRR_TotalAmount,
                       ojRR_PaxTypeFareVec);
  calculateTotalAmount(fareMarketPath,
                       FareValidatorESV::VDF_TAG_2_104,
                       vfHoldersVec,
                       ojRR_TotalAmountNoCat10,
                       ojRR_PaxTypeFareVecNoCat10);

  SOPFareList& sopFareList = _itin->paxTypeSOPFareListMap()[paxType];

  // Check if we were able to find valid one way fares for entire fare
  // market path
  addSOPFarePath(owTotalAmount,
                 owPaxTypeFareVec,
                 owPaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.owSopFarePaths());

  // Check if we were able to find valid one way fares (without cat 10
  // restrictions) for entire fare market path
  addSOPFarePath(owTotalAmountNoCat10,
                 owPaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.owSopFarePaths(),
                 false);

  // Check if we were able to find valid round trip fares for entire fare
  // market path
  addSOPFarePath(rtTotalAmount,
                 rtPaxTypeFareVec,
                 rtPaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.rtSopFarePaths());

  // Check if we were able to find valid round trip fares (without cat 10
  // restrictions) for entire fare market path
  addSOPFarePath(rtTotalAmountNoCat10,
                 rtPaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.rtSopFarePaths(),
                 false);

  // Check if we were able to find valid circle trip fares for entire fare
  // market path
  addSOPFarePath(ctTotalAmount,
                 ctPaxTypeFareVec,
                 ctPaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ctSopFarePaths());

  // Check if we were able to find valid circle trip fares (without cat 10
  // restrictions) for entire fare market path
  addSOPFarePath(ctTotalAmountNoCat10,
                 ctPaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ctSopFarePaths(),
                 false);

  // OJ R
  addSOPFarePath(ojR_TotalAmount,
                 ojR_PaxTypeFareVec,
                 ojR_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 SOPFarePath::R);
  addSOPFarePath(ojR_TotalAmountNoCat10,
                 ojR_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 false,
                 SOPFarePath::R);

  // OJ X
  addSOPFarePath(ojX_TotalAmount,
                 ojX_PaxTypeFareVec,
                 ojX_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 SOPFarePath::X);
  addSOPFarePath(ojX_TotalAmountNoCat10,
                 ojX_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 false,
                 SOPFarePath::X);

  // OJ RR
  addSOPFarePath(ojRR_TotalAmount,
                 ojRR_PaxTypeFareVec,
                 ojRR_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 SOPFarePath::RR);
  addSOPFarePath(ojRR_TotalAmountNoCat10,
                 ojRR_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 false,
                 SOPFarePath::RR);

  // OJ RX
  addSOPFarePath(ojRX_TotalAmount,
                 ojRX_PaxTypeFareVec,
                 ojRX_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 SOPFarePath::RX);
  addSOPFarePath(ojRX_TotalAmountNoCat10,
                 ojRX_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 false,
                 SOPFarePath::RX);

  // OJ XR
  addSOPFarePath(ojXR_TotalAmount,
                 ojXR_PaxTypeFareVec,
                 ojXR_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 SOPFarePath::XR);
  addSOPFarePath(ojXR_TotalAmountNoCat10,
                 ojXR_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 false,
                 SOPFarePath::XR);

  // OJ XX
  addSOPFarePath(ojXX_TotalAmount,
                 ojXX_PaxTypeFareVec,
                 ojXX_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 SOPFarePath::XX);
  addSOPFarePath(ojXX_TotalAmountNoCat10,
                 ojXX_PaxTypeFareVecNoCat10,
                 fareMarketPath,
                 sopFareList.ojSopFarePaths(),
                 false,
                 SOPFarePath::XX);
}

bool
GovCxrValidatorESV::resolveValidatedLocalJourneyFares(
    std::vector<PaxTypeFare*>& resultFaresVec,
    std::vector<FareValidatorESV::VFHolder>& vfHolder,
    const std::vector<FareValidatorESV::ValidatedFareType>& fareTypes)
{
  if (vfHolder.size() != fareTypes.size())
  {
    LOG4CXX_ERROR(logger,
                  "GovCxrValidatorESV::resolveValidatedLocalJourneyFares - wrong fare types.");
    return false;
  }

  std::vector<FareValidatorESV::ValidatedPaxTypeFares*> vptfs;

  for (size_t i = 0; i < vfHolder.size(); ++i)
  {
    if (vfHolder[i]._ljvf != nullptr)
    {
      vptfs.push_back((*vfHolder[i]._ljvf)[fareTypes[i]]);
    }
  }

  std::vector<PaxTypeFare*> paxTypeFares;

  if (_checkMarriedBookingCodes)
  {
    if (!vptfs.empty() && !resolveValidatedLocalJourneyFares(paxTypeFares, vptfs))
    {
      return false;
    }
  }
  else
  {
    if (!vptfs.empty() && !resolveValidatedLocalJourneyFaresOld(paxTypeFares, vptfs))
    {
      return false;
    }
  }

  for (size_t i = 0; i < vfHolder.size();)
  {
    if (vfHolder[i]._vf != nullptr)
    {
      resultFaresVec.push_back((*vfHolder[i]._vf)[fareTypes[i]]);
      ++i;
    }
    else if (vfHolder[i]._ljvf != nullptr)
    {
      resultFaresVec.insert(resultFaresVec.end(), paxTypeFares.begin(), paxTypeFares.end());
      i += paxTypeFares.size();
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "GovCxrValidatorESV::resolveValidatedLocalJourneyFares - no validated fares.");
      return false;
    }
  }

  return true;
}

bool
GovCxrValidatorESV::resolveValidatedLocalJourneyFares(
    std::vector<PaxTypeFare*>& paxTypeFares,
    std::vector<FareValidatorESV::ValidatedPaxTypeFares*>& vptfs)
{
  std::vector<FareValidatorESV::ValidatedPTF*> r;
  MultiDimensionalPQ<FareValidatorESV::ValidatedPTF*, double> mpq(vptfs);

  while (true)
  {
    r = mpq.next();

    if (r.empty())
    {
      return false;
    }

    // If all booking codes the same then must be flow,
    // otherwise must be local.

    BookingCode prevBkc(EMPTY_STRING());
    bool sameBookingCodes = true;

    for (size_t i = 0; i < r.size(); ++i)
    {
      PaxTypeFare* paxTypeFare = r[i]->ptFare;
      PaxTypeFare::SegmentStatusVec& segmentStatusVec =
          paxTypeFare->flightBitmapESV()[_flightId]._segmentStatus;

      for (size_t j = 0; j < segmentStatusVec.size(); ++j)
      {
        if (prevBkc == EMPTY_STRING())
        {
          prevBkc = segmentStatusVec[j]._bkgCodeReBook;
          continue;
        }

        BookingCode currBkc = segmentStatusVec[j]._bkgCodeReBook;

        if (currBkc != prevBkc)
        {
          const TravelSeg* prevSeg = _itin->travelSeg()[i + j - 1];
          const AirSeg* prevAirSeg = dynamic_cast<const AirSeg*>(prevSeg);

          if (!prevAirSeg)
          {
            LOG4CXX_ERROR(logger, "Unexpected non-air segment found");
            return false;
          }

          if (!BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, prevBkc, currBkc))
          {
            sameBookingCodes = false;
            break;
          }
        }

        prevBkc = currBkc;
      }
    }

    bool valid = true;

    for (const auto validPtf : r)
    {
      if (validPtf->flow != sameBookingCodes)
      {
        valid = false;
        break;
      }
    }

    if (valid)
    {
      paxTypeFares.resize(r.size());

      for (size_t i = 0; i < r.size(); ++i)
      {
        paxTypeFares[i] = r[i]->ptFare;
      }

      return true;
    }
  }

  return false;
}

bool
GovCxrValidatorESV::resolveValidatedLocalJourneyFaresOld(
    std::vector<PaxTypeFare*>& paxTypeFares,
    std::vector<FareValidatorESV::ValidatedPaxTypeFares*>& vptfs)
{
  std::vector<FareValidatorESV::ValidatedPTF*> r;
  MultiDimensionalPQ<FareValidatorESV::ValidatedPTF*, double> mpq(vptfs);
  for (;;)
  {
    r = mpq.next();
    if (r.empty())
    {
      return false;
    }

    const BookingCode bkc = r.front()->bkc;
    // If all booking codes the same then must be flow,
    // otherwise must be local.
    bool sameBookingCodes = true;
    for (size_t i = 1; i < r.size(); ++i)
    {
      if (r[i]->bkc != bkc)
      {
        sameBookingCodes = false;
        break;
      }
    }

    bool valid = true;
    for (const auto validPtf : r)
    {
      if (validPtf->flow != sameBookingCodes)
      {
        valid = false;
        break;
      }
    }

    if (valid)
    {
      paxTypeFares.resize(r.size());
      for (size_t i = 0; i < r.size(); ++i)
        paxTypeFares[i] = r[i]->ptFare;
      return true;
    }
  }
  return false;
}

void
GovCxrValidatorESV::calculateTotalAmount(const std::vector<FareMarket*>& fareMarketPath,
                                         FareValidatorESV::ValidatedFareType fareType,
                                         std::vector<FareValidatorESV::VFHolder>& vfHoldersVec,
                                         double& totalAmount,
                                         std::vector<PaxTypeFare*>& paxTypeFareVec)
{
  std::vector<FareValidatorESV::ValidatedFareType> fareTypes(fareMarketPath.size(), fareType);
  calculateTotalAmount(fareMarketPath, fareTypes, vfHoldersVec, totalAmount, paxTypeFareVec);
}

void
GovCxrValidatorESV::calculateTotalAmount(const std::vector<FareMarket*>& fareMarketPath,
                                         FareValidatorESV::ValidatedFareType firstFareType,
                                         FareValidatorESV::ValidatedFareType secondFareType,
                                         std::vector<FareValidatorESV::VFHolder>& vfHoldersVec,
                                         double& totalAmount,
                                         std::vector<PaxTypeFare*>& paxTypeFareVec)
{
  std::vector<FareValidatorESV::ValidatedFareType> fareTypes;
  fareTypes.push_back(firstFareType);
  fareTypes.push_back(secondFareType);
  calculateTotalAmount(fareMarketPath, fareTypes, vfHoldersVec, totalAmount, paxTypeFareVec);
}

void
GovCxrValidatorESV::calculateTotalAmount(
    const std::vector<FareMarket*>& fareMarketPath,
    const std::vector<FareValidatorESV::ValidatedFareType>& fareTypes,
    std::vector<FareValidatorESV::VFHolder>& vfHoldersVec,
    double& totalAmount,
    std::vector<PaxTypeFare*>& paxTypeFareVec)
{
  if (totalAmount != -1)
  {
    std::vector<PaxTypeFare*> resultFaresVec;

    if (resolveValidatedLocalJourneyFares(resultFaresVec, vfHoldersVec, fareTypes))
    {
      for (size_t fmIdx = 0; fmIdx < fareMarketPath.size(); ++fmIdx)
      {
        updateFarePath(resultFaresVec[fmIdx], totalAmount, paxTypeFareVec);
      }
    }
    else
    {
      totalAmount = -1;
    }
  }
}

void
GovCxrValidatorESV::updateFarePath(PaxTypeFare* paxTypeFare,
                                   MoneyAmount& amt,
                                   std::vector<PaxTypeFare*>& paxTypeFareVec)
{
  if ((nullptr != paxTypeFare) && (-1 != amt))
  {
    amt += paxTypeFare->totalFareAmount();
    paxTypeFareVec.push_back(paxTypeFare);
  }
  else
  {
    amt = -1;
  }
}

void
GovCxrValidatorESV::addSOPFarePath(MoneyAmount& totalAmount,
                                   std::vector<PaxTypeFare*>& paxTypeFareVec,
                                   std::vector<PaxTypeFare*>& paxTypeFareVecNoCat10,
                                   const std::vector<FareMarket*>& fareMarketPath,
                                   std::vector<SOPFarePath*>& sopFarePaths,
                                   SOPFarePath::CombinationType combinationType)
{
  if (("" != _itin->onlineCarrier()) && (!checkIfSameFares(paxTypeFareVec, paxTypeFareVecNoCat10)))
  {
    addSOPFarePath(
        totalAmount, paxTypeFareVec, fareMarketPath, sopFarePaths, true, combinationType);
  }
}

void
GovCxrValidatorESV::addSOPFarePath(MoneyAmount& totalAmount,
                                   std::vector<PaxTypeFare*>& paxTypeFareVec,
                                   const std::vector<FareMarket*>& fareMarketPath,
                                   std::vector<SOPFarePath*>& sopFarePaths,
                                   bool cat10Restrictions,
                                   SOPFarePath::CombinationType combinationType)
{
  if (totalAmount != -1)
  {
    SOPFarePath* sopFarePath;
    (*_trx).dataHandle().get(sopFarePath);

    sopFarePath->totalAmount() = totalAmount;
    sopFarePath->paxTypeFareVec() = paxTypeFareVec;
    sopFarePath->fareMarketPath() = fareMarketPath;
    sopFarePath->haveCat10Restrictions() = cat10Restrictions;
    sopFarePath->combinationType() = combinationType;

    sopFarePaths.push_back(sopFarePath);
  }
}

void
GovCxrValidatorESV::sortSopFareLists()
{
  TSELatencyData metrics((*_trx), "FVO SORT SOP FARE LISTS");

  LOG4CXX_DEBUG(logger, "GovCxrValidatorESV::sortSopFareLists()");

  std::vector<PaxType*>::const_iterator paxTypeIter((*_trx).paxType().begin()),
      paxTypeIterEnd((*_trx).paxType().end());

  for (; paxTypeIter != paxTypeIterEnd; ++paxTypeIter)
  {
    PaxType* paxType = (*paxTypeIter);

    // Check if passenger type object is not NULL
    if (nullptr == paxType)
    {
      LOG4CXX_ERROR(logger,
                    "GovCxrValidatorESV::sortSopFareLists - Passenger type object is NULL.");
      continue;
    }

    SOPFareList& sopFareList = _itin->paxTypeSOPFareListMap()[paxType];

    std::sort(sopFareList.owSopFarePaths().begin(),
              sopFareList.owSopFarePaths().end(),
              SOPFarePath::compare);

    // Keep only one cheapest fare market path and EOE fare market path for
    // one way fares
    removeExpensiveOwFarePaths(sopFareList.owSopFarePaths());
  }
}

void
GovCxrValidatorESV::removeExpensiveOwFarePaths(std::vector<SOPFarePath*>& owSopFarePaths)
{
  TSELatencyData metrics((*_trx), "FVO REMOVE EXPENSIVE OW FARE PATHS");

  LOG4CXX_DEBUG(logger,
                "GovCxrValidatorESV::removeExpensiveOwFarePaths(std::vector<SOPFarePath*>&)");

  std::vector<SOPFarePath*> newOwVector;

  bool farePathFound = false;
  bool farePathEoeFound = false;

  std::vector<SOPFarePath*>::const_iterator sfpIter(owSopFarePaths.begin()),
      sfpIterEnd(owSopFarePaths.end());

  for (; sfpIter != sfpIterEnd; ++sfpIter)
  {
    SOPFarePath* sopFarePath = (*sfpIter);

    if (!(sopFarePath->haveCat10Restrictions()))
    {
      if (false == farePathEoeFound)
      {
        newOwVector.push_back(sopFarePath);
        farePathEoeFound = true;
        farePathFound = true;
      }
    }
    else
    {
      if (false == farePathFound)
      {
        newOwVector.push_back(sopFarePath);
        farePathFound = true;
      }
    }

    if ((true == farePathFound) && (true == farePathEoeFound))
    {
      break;
    }
  }

  owSopFarePaths.swap(newOwVector);
}

void
GovCxrValidatorESV::buildFareMarketPaths(
    uint32_t sizeOfPath,
    std::vector<FareMarket*>& fareMarketPath,
    std::vector<std::vector<FareMarket*> >& fareMarketPathMatrix)
{
  TSELatencyData metrics((*_trx), "FVO BUILD FARE MARKET PATHS");

  LOG4CXX_DEBUG(logger,
                "GovCxrValidatorESV::buildFareMarketPaths(ShoppingTrx&, uint32_t, "
                "std::vector<FareMarket*>&, std::vector< std::vector<FareMarket*> >&)");

  // Go thorough all fare markets
  std::vector<FareMarket*>::const_iterator fareMarketIter(_itin->fareMarket().begin()),
      fareMarketIterEnd(_itin->fareMarket().end());

  for (; fareMarketIter != fareMarketIterEnd; ++fareMarketIter)
  {
    FareMarket* fareMarket = (*fareMarketIter);

    // Check if fare market object is not NULL
    if (nullptr == fareMarket)
    {
      LOG4CXX_ERROR(logger,
                    "GovCxrValidatorESV::buildFareMarketPaths - Fare market object is NULL.");
      continue;
    }

    // If fare market path is empty it means that we're starting to build
    // new fare market path and we need to find fare market which got the
    // same origin city as first segment in itinerary
    if (true == fareMarketPath.empty())
    {
      if (fareMarket->origin()->loc() != _itin->travelSeg().front()->origin()->loc())
      {
        continue;
      }
    }
    // If fare market path is not empty we're continue building fare market
    // path and we need to find fare market which got the same origin city
    // as destination of last fare market in path
    else
    {
      uint32_t segId = 0;

      std::vector<TravelSeg*>::const_iterator segIter(_itin->travelSeg().begin()),
          segIterEnd(_itin->travelSeg().end());

      for (; segIter != segIterEnd; ++segIter, ++segId)
      {
        const TravelSeg* travelSeg = (*segIter);

        if (travelSeg->destination()->loc() == fareMarketPath.back()->destination()->loc())
        {
          break;
        }
      }

      if (fareMarket->origin()->loc() != _itin->travelSeg()[segId + 1]->origin()->loc())
      {
        continue;
      }
    }

    // Create new fare market path on the basis of existing one
    std::vector<FareMarket*> newFareMarketPath;
    newFareMarketPath.insert(newFareMarketPath.end(), fareMarketPath.begin(), fareMarketPath.end());

    // Add apropriate fare market to path
    newFareMarketPath.push_back(fareMarket);

    // Ensure that we've got max 3 fares in fare market path for each leg
    if (newFareMarketPath.size() > 3)
    {
      continue;
    }

    // If destination city of currently processed fare market it's the end
    // of a trip it means that we've got complete fare market path and we
    // can add it to matrix
    if (fareMarket->destination()->loc() == _itin->travelSeg().back()->destination()->loc())
    {
      fareMarketPathMatrix.push_back(newFareMarketPath);
    }
    // Otherwise we need to check if we don't exeed the maximum size of a
    // path and continue building of fare market path
    else if (sizeOfPath < _itin->travelSeg().size())
    {
      buildFareMarketPaths(sizeOfPath + 1, newFareMarketPath, fareMarketPathMatrix);
    }
  }
}

void
GovCxrValidatorESV::buildFareMarketPaths(
    std::vector<std::vector<FareMarket*> >& fareMarketPathMatrix)
{
  uint32_t sizeOfPath = 1;
  std::vector<FareMarket*> emptyFareMarketPath;

  GovCxrValidatorESV::buildFareMarketPaths(sizeOfPath, emptyFareMarketPath, fareMarketPathMatrix);
}

void
GovCxrValidatorESV::processItinJCBFlag(std::vector<std::vector<FareMarket*> >& fareMarketPathMatrix)
{
  if (_itin == nullptr)
  {
    return;
  }

  std::vector<std::vector<FareMarket*> >::const_iterator fareMarketPathIter(
      fareMarketPathMatrix.begin()),
      fareMarketPathIterEnd(fareMarketPathMatrix.end());

  for (; fareMarketPathIter != fareMarketPathIterEnd; ++fareMarketPathIter)
  {
    const std::vector<FareMarket*>& fareMarketPath = (*fareMarketPathIter);
    std::vector<FareMarket*>::const_iterator fareMarketIter(fareMarketPath.begin()),
        fareMarketIterEnd(fareMarketPath.end());

    bool jcbFlag = true;

    for (; fareMarketIter != fareMarketIterEnd; ++fareMarketIter)
    {
      FareMarket* fareMarket = (*fareMarketIter);

      if ((fareMarket == nullptr) || (fareMarket->isJcb() == false))
      {
        jcbFlag = false;
        break;
      }
    }

    if (jcbFlag)
    {
      _itin->isJcb() = true;
      return;
    }
  }
}

bool
GovCxrValidatorESV::setupJourneyItinTravelSeg(std::vector<TravelSeg*>& tempTravelSeg)
{
  TSELatencyData metrics((*_trx), "FVO SETUP JOURNEY ITIN TRAVEL SEG");

  LOG4CXX_DEBUG(logger, "GovCxrValidatorESV::setupJourneyItinTravelSeg(std::vector<TravelSeg*>&)");

  _journeyItin->travelSeg().swap(tempTravelSeg);
  _journeyItin->travelSeg().clear();

  // Check if it's one way or other type of trip
  if (1 == (*_trx).legs().size())
  {
    // If it's one way switch journey itin travel segment with segments from
    // itinerary
    _journeyItin->travelSeg().insert(
        _journeyItin->travelSeg().end(), _itin->travelSeg().begin(), _itin->travelSeg().end());
  }
  else if (2 == (*_trx).legs().size())
  {
    // If it's other type of trip switch only one of travel segments in
    // journey itin with apropriate segments from itinerary

    // Check if it's first leg
    if (_leg == &((*_trx).legs()[0]))
    {
      _journeyItin->travelSeg().insert(
          _journeyItin->travelSeg().end(), _itin->travelSeg().begin(), _itin->travelSeg().end());
      _journeyItin->travelSeg().push_back(tempTravelSeg.back());
    }
    else
    {
      _journeyItin->travelSeg().push_back(tempTravelSeg.front());
      _journeyItin->travelSeg().insert(
          _journeyItin->travelSeg().end(), _itin->travelSeg().begin(), _itin->travelSeg().end());
    }
  }
  else
  {
    return false;
  }

  return true;
}

bool
GovCxrValidatorESV::checkIfSameFares(const std::vector<PaxTypeFare*>& fareVector1,
                                     const std::vector<PaxTypeFare*>& fareVector2)
{
  return fareVector1 == fareVector2;
}
} // tse
