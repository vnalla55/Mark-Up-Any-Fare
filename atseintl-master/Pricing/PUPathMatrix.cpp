//-------------------------------------------------------------------
// File:    PUPathMatrix.cpp
// Created: April 2004
// Authors: Mohammad Hossan
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

#include "Pricing/PUPathMatrix.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigMan.h"
#include "Common/FareCalcUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CircleTripProvision.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag600Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"

#include <algorithm>
#include <iostream>

namespace tse
{
FALLBACK_DECL(apo36040Cat6TSI5Check);
FALLBACK_DECL(reworkTrxAborter);

namespace
{
Logger logger("atseintl.Pricing.PUPathMatrix");
}

PUPathMatrix::UniqueIntlSameNationOJ
PUPathMatrix::findUniqueInternationalSameNationOJ() const
{
  UniqueIntlSameNationOJ uniqueOJs;
  for (const auto puPath : _puPathMatrix)
  {
    for (PU* pu : puPath->puPath())
      if (PricingUnit::Type::OPENJAW == pu->puType() && GeoTravelType::International == pu->geoTravelType() &&
          pu->sameNationOJ())
        uniqueOJs.insert(pu);

    // only Main-PUPath may have SideTrip
    for (const auto& sideTripPUPath : puPath->sideTripPUPath())
      for (const auto stPUPath : sideTripPUPath.second)
        for (PU* stPU : stPUPath->puPath())
          if (PricingUnit::Type::OPENJAW == stPU->puType() && GeoTravelType::International == stPU->geoTravelType() &&
              stPU->sameNationOJ())
            uniqueOJs.insert(stPU);
  }
  return uniqueOJs;
}

PU*
PUPathMatrix::buildRTPU(MergedFareMarket* fm1, MergedFareMarket* fm2)
{
  PU* rtPU = nullptr;
  if (isRoundTrip(*fm1, *fm2))
  {
    if (fm1->tag2FareIndicator() != MergedFareMarket::Tag2FareIndicator::Absent ||
        fm2->tag2FareIndicator() != MergedFareMarket::Tag2FareIndicator::Absent)
    // Even these FM can form RT, but there is not Tag2 fare in these Market,
    // Not possible to build any RT-Fare combination, needed of US/CA
    // For other market value will be  MergedFareMarket::Tag2FareIndicator::NonIssue
    {
      rtPU = constructPU();

      rtPU->puType() = PricingUnit::Type::ROUNDTRIP;
      rtPU->fcCount() = 2;
      rtPU->fareMarket().push_back(fm1);
      rtPU->fareMarket().push_back(fm2);
      rtPU->turnAroundPoint() = fm2->travelSeg().front(); // INBOUND
      rtPU->geoTravelType() = fm1->geoTravelType();
      rtPU->fareDirectionality().push_back(FROM);
      rtPU->fareDirectionality().push_back(TO);

      rtPU->cxrFarePreferred() = (fm1->cxrFarePreferred() || fm2->cxrFarePreferred());
    }
  }
  return rtPU;
}

PU*
PUPathMatrix::buildOJPU(const MergedFMVector& outboundFM,
                        const MergedFMVector& inboundFM,
                        const GeoTravelType geoTvlType,
                        const bool ojPUCxrPref)
{
  PU* ojPU = nullptr;

  PricingUnit::PUSubType ojType = PricingUnit::UNKNOWN_SUBTYPE;
  bool sameNationOJ = false;
  bool sameNationOrigSurfaceOJ = false;
  bool allowNOJInZone210 = false;
  PricingUnit::OJSurfaceStatus ojSurfaceStatus = PricingUnit::NOT_CHECKED;

  std::vector<CarrierCode> invalidCxrForTOJ;
  bool openJawBtwTwoAreas = false;
  bool inDiffCntrySameSubareaForOOJ = false;
  bool specialEuropeanDoubleOJ = false;
  bool specialOJ = false;

  if (isValidOpenJawTrip(outboundFM,
                         inboundFM,
                         geoTvlType,
                         ojType,
                         sameNationOJ,
                         sameNationOrigSurfaceOJ,
                         allowNOJInZone210,
                         ojSurfaceStatus,
                         openJawBtwTwoAreas,
                         invalidCxrForTOJ,
                         inDiffCntrySameSubareaForOOJ,
                         specialEuropeanDoubleOJ,
                         specialOJ))
  {
    ojPU = constructPU();

    ojPU->puType() = PricingUnit::Type::OPENJAW;
    ojPU->puSubType() = ojType;
    ojPU->sameNationOJ() = sameNationOJ;
    ojPU->sameNationOrigSurfaceOJ() = sameNationOrigSurfaceOJ;
    ojPU->allowNOJInZone210() = allowNOJInZone210;

    ojPU->ojSurfaceStatus() = ojSurfaceStatus;

    ojPU->geoTravelType() = geoTvlType;
    ojPU->ojLeg1FCCount() = outboundFM.size();
    ojPU->fareMarket().insert(ojPU->fareMarket().end(), outboundFM.begin(), outboundFM.end());
    ojPU->fareMarket().insert(ojPU->fareMarket().end(), inboundFM.begin(), inboundFM.end());
    ojPU->fcCount() = ojPU->fareMarket().size();

    ojPU->fareDirectionality().insert(ojPU->fareDirectionality().end(), outboundFM.size(), FROM);
    ojPU->cxrFarePreferred() = ojPUCxrPref;

    if (LIKELY(TrxUtil::isSpecialOpenJawActivated(*_trx)))
    {
      ojPU->invalidateYYForTOJ() = openJawBtwTwoAreas || specialOJ;
      ojPU->setSpecialOpenJaw(specialOJ);
    }
    else
      ojPU->invalidateYYForTOJ() = openJawBtwTwoAreas;

    ojPU->invalidCxrForOJ().insert(
        ojPU->invalidCxrForOJ().end(), invalidCxrForTOJ.begin(), invalidCxrForTOJ.end());
    ojPU->inDiffCntrySameSubareaForOOJ() = inDiffCntrySameSubareaForOOJ;
    ojPU->specialEuropeanDoubleOJ() = specialEuropeanDoubleOJ;

    // ----- determine turnAroundPoint and Inbound-Trip's Directioanlity ------
    if (geoTvlType == GeoTravelType::International) //  diff country, intl OJ can have at only 2 component
    {
      const MergedFareMarket& obFM = *outboundFM.front();
      const MergedFareMarket& ibFM = *inboundFM.front();

      if (ojType == PricingUnit::DEST_OPENJAW || // returning to same point
          isInboundToNationZone210(*obFM.origin(), ibFM) ||
          isInboundToCountry(*obFM.origin(), ibFM) ||
          isInboundToNetherlandAntilles(*obFM.origin(), ibFM))
      {
        ojPU->fareDirectionality().push_back(TO);
        ojPU->turnAroundPoint() = ibFM.travelSeg().front(); // INBOUND
      }
      else
      {
        ojPU->fareDirectionality().push_back(FROM);
        ojPU->turnAroundPoint() = ibFM.travelSeg().front();
      }
    }
    else // Not an International OJ
    {
      const uint16_t mktCount = inboundFM.size();

      ojPU->fareDirectionality().insert(ojPU->fareDirectionality().end(), mktCount - 1, FROM);
      ojPU->turnAroundPoint() = inboundFM.back()->travelSeg().front(); // INBOUND
      ojPU->fareDirectionality().push_back(TO);
    }
  }
  return ojPU;
}

PU*
PUPathMatrix::buildOWPU(MergedFareMarket* fm)
{
  PU* owPU = nullptr;
  if (fm->travelSeg().front()->segmentType() == Arunk)
  {
    // OW Failed: FareMkt Starts with ARUNK
    if (!fm->travelSeg().back()->fareCalcFareAmt().empty() && fm->mergedFareMarket().size() == 1)
    {
      // merged fare market with only dummyFare, let built
    }
    else
      return owPU;
  }
  owPU = constructPU();

  owPU->puType() = PricingUnit::Type::ONEWAY;
  owPU->fcCount() = 1;
  owPU->fareMarket().push_back(fm);
  owPU->geoTravelType() = fm->geoTravelType();
  owPU->fareDirectionality().push_back(FROM); // OUTBOUND
  owPU->cxrFarePreferred() = fm->cxrFarePreferred();

  return owPU;
}

// -- follown 2 methods are for DEBUG only --
// void debugOnlyDisplay(FareMarketPath& fmp);
// void debugOnlyDisplayPUPath(const PUPath& puPath);
// void debugOnlyDisplayMainTripSTLink(const PUPath& puPath);

//------------------------------------------------------------------
// Build All PUPath
//------------------------------------------------------------------
bool
PUPathMatrix::buildAllPUPath(FareMarketPathMatrix& fmpMatrix,
                             std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  return buildAllPUPath(fmpMatrix.fareMarketPathMatrix(), puFactoryBucketVect);
}

//------------------------------------------------------------------
// Build All PUPath
//------------------------------------------------------------------
bool
PUPathMatrix::buildAllPUPath(std::vector<FareMarketPath*>& matrix,
                             std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  checkTrxAborted(*_trx);

  if (matrix.empty())
    return false;

  limitFareMarketPath(matrix);

  _fareMarketPathCount = matrix.size();
  _maxPUPathPerFMP = static_cast<uint32_t>(ceil(FIVE_K / (double)_fareMarketPathCount));

  LOG4CXX_DEBUG(logger, "FareMarketPathMatrix SIZE = " << _fareMarketPathCount);

  _fcConfig = FareCalcUtil::getFareCalcConfig(*_trx);

  // To support PU-Template building effort
  determineItinTravelBoundary();

  int remainingPaths = _fareMarketPathCount;
  std::vector<PUPathBuildTask> thrInputVect;
  thrInputVect.resize(remainingPaths);

  TseRunnableExecutor pooledExecutor(_taskId);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

  // iterate for each FareMarketPath int Matrix
  std::vector<FareMarketPath*>::iterator fmpIt = matrix.begin();
  const std::vector<FareMarketPath*>::iterator fmpItEnd = matrix.end();

  std::vector<PUPathBuildTask>::iterator threadTaskIt = thrInputVect.begin();

  for (; fmpIt != fmpItEnd; ++fmpIt, --remainingPaths, ++threadTaskIt)
  {
    // debugOnlyDisplay(**fmpIt);

    PUPathMatrix::PUPathBuildTask& puPathBuildTask = (*threadTaskIt);

    puPathBuildTask.trx(_trx);
    puPathBuildTask._puPathMatrix = this;
    puPathBuildTask._fareMarketPath = (*fmpIt);

    TseRunnableExecutor& taskExecutor = (remainingPaths > 1) ? pooledExecutor : synchronousExecutor;

    taskExecutor.execute(puPathBuildTask);
  } // for each FareMarketPath

  // wait for the threads to finish
  pooledExecutor.wait();

  UniqueIntlSameNationOJ uniqueOJs = findUniqueInternationalSameNationOJ();
  LOG4CXX_DEBUG(logger, "NUMBER OF UNIQUE OJ" << uniqueOJs.size());
  for (const auto pu : uniqueOJs)
  {
    if (fallback::reworkTrxAborter(_trx))
      checkTrxAborted(*_trx);
    else
      _trx->checkTrxAborted();
    // intl OJ has only 2 FM
    const MergedFareMarket* fm1(pu->fareMarket().front());
    const MergedFareMarket* fm2(pu->fareMarket().back());
    markIntlOWfromBrokenOJ(pu, fm1, fm2);
  }

  std::set<PU*, PU::PUPtrCmp> alreadySeenPUs;

  for (const auto puPath : _puPathMatrix)
  {
    updatePUFactoryBucket(*puPath, puFactoryBucketVect, alreadySeenPUs);

    createMainTripSideTripLink(*puPath);

    // debugOnlyDisplayMainTripSTLink(*puPath);

    puPath->setTotalPU();
    puPath->countTotalFC();
    puPath->itinWithinScandinavia() = _travelWithinScandinavia;
  }

  // import to trx DataHandle from the local one
  _trx->dataHandle().import(_dataHandle);

  LOG4CXX_DEBUG(logger, "PUPathMatrix SIZE = " << _puPathMatrix.size());

  if (_trx->isTestRequest())
  {
    PUPathMatrix::PUPathPtrCmp cmp;
    std::sort(_puPathMatrix.begin(), _puPathMatrix.end(), cmp);
  }

  DiagManager diag600(*_trx, DiagnosticTypes::Diagnostic600);
  if (UNLIKELY(diag600.isActive()))
  {
    Diag600Collector& dc = static_cast<Diag600Collector&>(diag600.collector());
    dc.displayPUPathMatrix(*this);
  }

  return true;
}

//------------------------------------------------------------------
// Build PUPath
//------------------------------------------------------------------
bool
PUPathMatrix::buildPUPath(const FareMarketPath& fmp,
                          const uint16_t mktIdx,
                          const uint16_t totalMktCnt,
                          PUPath& puPath,
                          std::vector<MergedFareMarket*>& fmPath,
                          std::vector<PUPath*>& puPathVect,
                          bool& done,
                          bool onlyOwFares)
{
  if (UNLIKELY(_trx->getOptions()->isRtw()))
  {
    puPath.puPath().push_back(&buildRW(*fmPath.front()));
    puPath.cxrFarePreferred() = puPath.puPath().front()->cxrFarePreferred();
    puPathVect.push_back(&puPath);
    puPath.fareMarketPath() = const_cast<FareMarketPath*>(&fmp);
    return true;
  }

  if (mktIdx >= totalMktCnt)
  {
    // check whether the path is complete
    if (!done && isPUPathValid(fmp, puPath, totalMktCnt))
    {
      // PUPath Valid
      puPath.fareMarketPath() = (FareMarketPath*)&fmp;
      setOWPUDirectionality(puPath);
      setIsIntlCTJourneyWithOWPU(fmp, puPath);
      puPathVect.push_back(&puPath);
      done = isBuildPUPathComplete(puPathVect);
      return true;
    }
    else
    {
      // PUPath is not Valid
      return false;
    }
  }
  if (UNLIKELY(done))
  {
    // To trim Big PNR
    return true;
  }

  checkTrxAborted(*_trx);

  //----- mktIdx < totalMktCnt ----

  if (LIKELY(!onlyOwFares))
  {
    //--------- Try Round Trip, need 2 components to be RT -----------------
    // will make copy of puPath inside
    buildRT(fmp, mktIdx, totalMktCnt, puPath, fmPath, puPathVect, done);

    //----- Try Circle Trip, 2 components can be CT with CT Provision -------------
    // will make copy of puPath inside
    buildCT(fmp, mktIdx, totalMktCnt, puPath, fmPath, puPathVect, done);

    //---------------- Try OpenJaw Trip, minimum is 2 component OJ ----------------
    uint16_t maxOJcomp = MAX_OJ_COMP;
    if (UNLIKELY(_fareMarketPathCount > FMP_THRESHOLD))
    {
      maxOJcomp = OPT_OJ_COMP;
    }
    for (uint16_t compCount = 2; compCount <= (totalMktCnt - mktIdx) && compCount <= maxOJcomp;
         ++compCount)
    {
      for (uint16_t obCount = 1; obCount < compCount; ++obCount)
      {
        const uint16_t ibCount = compCount - obCount;
        for (uint16_t ibMktIdx = mktIdx + obCount; ibMktIdx + ibCount <= totalMktCnt; ++ibMktIdx)
        {
          // will make copy of puPath inside
          buildOJ(fmp,
                  obCount,
                  ibCount,
                  mktIdx,
                  ibMktIdx,
                  totalMktCnt,
                  puPath,
                  fmPath,
                  puPathVect,
                  done);
        }
      }
    }
  } // !onlyOwFares

  //---------------- Try OneWay Trip ----------------
  // make no copy of puPath, use the one passed in
  buildOW(fmp, mktIdx, totalMktCnt, puPath, fmPath, puPathVect, done);

  return true;
}

//------------------------------------------------------------------
// Build ROUND TRIP
//------------------------------------------------------------------
bool
PUPathMatrix::buildRT(const FareMarketPath& fmp,
                      const uint16_t mktIdx,
                      const uint16_t totalMktCnt,
                      PUPath& puPath,
                      std::vector<MergedFareMarket*>& fmPath,
                      std::vector<PUPath*>& puPathVect,
                      bool& done)
{
  if (UNLIKELY(done))
  {
    // To trim Big PNR
    return true;
  }

  if (mktIdx >= totalMktCnt)
  {
    return true;
  }

  MergedFareMarket* fm1 = fmPath[mktIdx];

  if (puPath.isMarketAssigned(fm1))
  {
    buildRT(fmp, mktIdx + 1, totalMktCnt, puPath, fmPath, puPathVect, done);
    return true;
  }

  if (fm1->travelSeg().front()->isArunk())
  {
    // RT Failed: FareMkt Starts with ARUNK
    return true;
  }

  uint16_t nextInMktIdx = mktIdx + 1;
  while (nextInMktIdx < totalMktCnt)
  {
    for (uint16_t rtMktIdx = nextInMktIdx; rtMktIdx < totalMktCnt; ++rtMktIdx)
    {
      MergedFareMarket* fm2 = fmPath[rtMktIdx];

      if (puPath.isMarketAssigned(fm2))
      {
        continue;
      }
      PU* rtPU = buildRTPU(fm1, fm2);
      if (!rtPU)
        continue;

      PUPath* rtPath = constructPUPath();
      copyPUPath(puPath, *rtPath);
      rtPath->puPath().push_back(rtPU);
      rtPath->cxrFarePreferred() = (rtPath->cxrFarePreferred() || rtPU->cxrFarePreferred());

      buildPUPath(fmp, mktIdx + 1, totalMktCnt, *rtPath, fmPath, puPathVect, done);

      nextInMktIdx = rtMktIdx;
      break;
    }
    ++nextInMktIdx;
  }

  return true;
}

//----------------------------------------------------------------

bool
PUPathMatrix::isRoundTrip(const MergedFareMarket& fm1, const MergedFareMarket& fm2)
{
  if (!LocUtil::isSamePoint(
          *fm1.origin(), fm1.boardMultiCity(), *fm2.destination(), fm2.offMultiCity()) ||
      !LocUtil::isSamePoint(
          *fm1.destination(), fm1.offMultiCity(), *fm2.origin(), fm2.boardMultiCity()))
  {
    // RT failed: mismatch orig or dest
    return false;
  }

  if (fm1.globalDirection() != fm2.globalDirection())
  {
    // RT failed: mismatch globalDirection
    return false;
  }

  return true;
}

//------------------------------------------------------------------
// Build CIRCLE TRIP
//------------------------------------------------------------------
bool
PUPathMatrix::buildCT(const FareMarketPath& fmp,
                      const uint16_t mktIdx,
                      const uint16_t totalMktCnt,
                      PUPath& puPath,
                      std::vector<MergedFareMarket*>& fmPath,
                      std::vector<PUPath*>& puPathVect,
                      bool& done)
{
  if (UNLIKELY(done))
  {
    // To trim Big PNR
    return true;
  }

  if (mktIdx >= totalMktCnt)
  {
    PUPath* ctPath = constructPUPath();
    copyPUPath(puPath, *ctPath);
    buildPUPath(fmp, mktIdx, totalMktCnt, *ctPath, fmPath, puPathVect, done);
    return true;
  }

  MergedFareMarket* startFM = fmPath[mktIdx];

  if (puPath.isMarketAssigned(startFM))
  {
    buildCT(fmp, mktIdx + 1, totalMktCnt, puPath, fmPath, puPathVect, done);
    return true;
  }

  if (startFM->travelSeg().front()->isArunk())
  {
    // CT Failed: FareMkt Starts with ARUNK
    return true;
  }

  // new PU
  PU* ctPU = constructPU();
  ctPU->puType() = PricingUnit::Type::CIRCLETRIP;
  ctPU->geoTravelType() = startFM->geoTravelType();
  ctPU->fareMarket().push_back(startFM);
  ctPU->fareDirectionality().push_back(FROM);
  ctPU->cxrFarePreferred() = startFM->cxrFarePreferred();

  bool ctFound = false;

  PUPath* ctPath = constructPUPath();
  copyPUPath(puPath, *ctPath);
  ctPath->cxrFarePreferred() = ctPU->cxrFarePreferred();
  buildCTPU(*ctPU, mktIdx + 1, totalMktCnt, *ctPath, fmPath, false, ctFound, done);

  if (!ctFound)
  {
    return true;
  }

  buildPUPath(fmp, mktIdx + 1, totalMktCnt, *ctPath, fmPath, puPathVect, done);

  return true;
}

//-----------------------------------------------------------------------
// Conditions for a FareMarket to be a part of CT:
// 1. Needs to be contiguous
// 2. Can not return to an intermediate point
//
//-----------------------------------------------------------------------

bool
PUPathMatrix::isValidFCforCT(PU& ctPu,
                             std::vector<MergedFareMarket*>& fmktVect,
                             const MergedFareMarket& fm,
                             Directionality& dir,
                             bool& passedCTProvision,
                             bool& closed)
{

  const MergedFareMarket* fmfirst = fmktVect.front();
  const MergedFareMarket* fmLast = fmktVect.back();

  DataHandle dataHandle(_trx->ticketingDate());
  const Loc& puOrig = *fmfirst->origin();
  const LocCode& puOrigCity = fmfirst->boardMultiCity();

  const Loc& puDest = *fmLast->destination();
  const LocCode& puDestCity = fmLast->offMultiCity();

  const Loc& orig = *fm.origin();
  const Loc& dest = *fm.destination();
  const LocCode& origCity = fm.boardMultiCity();
  const LocCode& destCity = fm.offMultiCity();

  bool doNotChkCTProvisionTbl = _itin && (_itin->geoTravelType() != GeoTravelType::International);

  //-------------- Is Contiguous ? ------------------------
  //
  if (!LocUtil::isSamePoint(puDest, puDestCity, orig, origCity))
  {
    // Not Contiguous A--B, C-X
    // Try CT Provision Table
    //
    if (doNotChkCTProvisionTbl)
      return false;

    bool validCTProvision = false;
    const uint16_t segNum1 = _itin->segmentOrder(fmLast->travelSeg().back());
    const uint16_t segNum2 = _itin->segmentOrder(fm.travelSeg().front());
    if (segNum1 + 2 == segNum2) // is there an arunk in between
    {
      const TravelSeg* arkSeg = _itin->travelSeg()[segNum1];
      if (arkSeg->isArunk())
      {
        const CircleTripProvision* ctp = dataHandle.getCircleTripProvision(
            arkSeg->boardMultiCity(), arkSeg->offMultiCity(), arkSeg->bookingDT());
        if (ctp)
        {
          // CONTIGUOUS for entry in CTProvision Table
          validCTProvision = true;
          passedCTProvision = true;
        }
      }
    }

    if (LIKELY(!validCTProvision))
    {
      return false; // A--B, C-X : Can not form CT
    }
  }

  // at this point Contiguous A--B, B-X

  //-------------- Returning to Origin ? ------------------------
  //
  if (LocUtil::isSamePoint(puOrig, puOrigCity, dest, destCity))
  {
    // Returning to Origin A-....-D, D-A

    closed = true; // Returning to Origin: A-...-D, D-A
    dir = TO; // INBOUND
    return true;
  }
  else if (!doNotChkCTProvisionTbl)
  {
    // Not Returning to Origin A-...-D, D-X
    // Try CT Provision Table
    //

    const TravelSeg& tvlSeg = *fm.travelSeg().back();
    const LocCode& lastCity = tvlSeg.offMultiCity();
    const DateTime& lastBkDt = fm.travelSeg().back()->bookingDT();

    const CircleTripProvision* ctp =
        dataHandle.getCircleTripProvision(lastCity, puOrigCity, lastBkDt);

    if (ctp)
    {
      // Returning to Orig for entry in CTProvision Table
      passedCTProvision = true;
      closed = true; // Returning to Origin: A-...-D, D-A
      if (isInboundToCountry(puOrig, orig, dest))
      {
        LOG4CXX_DEBUG(logger, "isInboundToCountry :" << __LINE__)
        dir = TO; // INBOUND
      }
      return true;
    }
  }

  // ----------- Returning to Intermediate Point? ------------
  //
  // Since this FareMarket is Not returning to PU Origin
  // Check whether it returns to an intermediate point of this pu
  // which is not allowed for CT
  //

  std::vector<MergedFareMarket*>::iterator iter = fmktVect.begin();
  const std::vector<MergedFareMarket*>::iterator iterEnd = fmktVect.end();
  ++iter;
  for (; iter != iterEnd; ++iter)
  {
    MergedFareMarket& puFmkt = *(*iter);
    if (LocUtil::isSamePoint(*puFmkt.origin(), puFmkt.boardMultiCity(), dest, destCity))
    {
      // Fall in Origin of prev FareMarket
      return false; // returning to a intermediate point
    }
  }

  //--------- Set INBOUND Directionality -----------------------
  // by default all are set to OUTBOUND
  //
  if (isInboundToCountry(puOrig, orig, dest))
  {
    LOG4CXX_DEBUG(logger, "isInboundToCountry :" << __LINE__)
    dir = TO; // INBOUND
  }

  return true;
}

//------------------------------------------------------------------
bool
PUPathMatrix::isValidCT(PU& ctPU, const bool passedCTProvision)
{
  if (passedCTProvision && (ctPU.geoTravelType() != GeoTravelType::International))
  {
    return false;
  }

  const uint16_t ctMktCount = ctPU.fareMarket().size();
  // CT must have more than 2 FareMarket
  if (ctMktCount > 2)
  {
    ctPU.fcCount() = ctMktCount;
    return true;
  }
  else if (LIKELY(ctMktCount == 2))
  {
    if (ctPU.fareMarket()[0]->globalDirection() != ctPU.fareMarket()[1]->globalDirection())
    {
      // CT: diff globalDirection
      ctPU.fcCount() = ctMktCount;
      return true;
    }
    else if (passedCTProvision)
    {
      // CT: Because of Arunk, it is CT, and can not be RT
      ctPU.fcCount() = ctMktCount;
      return true;
    }
  }

  return false;
}

//------------------------------------------------------------------
// Build OPEN JAW TRIP
//------------------------------------------------------------------
bool
PUPathMatrix::buildOJ(const FareMarketPath& fmp,
                      const uint16_t obCount,
                      const uint16_t ibCount,
                      const uint16_t mktIdx,
                      const uint16_t ibMktIdx,
                      const uint16_t totalMktCnt,
                      PUPath& puPath,
                      std::vector<MergedFareMarket*>& fmPath,
                      std::vector<PUPath*>& puPathVect,
                      bool& done)
{
  if (done)
  {
    // To trim Big PNR
    return true;
  }
  // Number or Fare Component of an OJ is controled by outside for loop
  // Therefore, it is little bit different interms of recursive calls and
  // mem-release then CT, RT etc.
  //
  const uint16_t fcCount = obCount + ibCount;
  if (mktIdx + fcCount > totalMktCnt)
  {
    return false;
  }

  //----------  Get OutBound Side of OJ --------------------
  //
  MergedFareMarket* fm = fmPath[mktIdx];
  GeoTravelType obGeoTvlType = fm->geoTravelType();
  if ((obGeoTvlType == GeoTravelType::International) && (fcCount > 2))
  {
    return false;
  }

  if (puPath.isMarketAssigned(fm))
  {
    buildOJ(
        fmp, obCount, ibCount, mktIdx + 1, ibMktIdx, totalMktCnt, puPath, fmPath, puPathVect, done);
    return true;
  }

  if (fm->travelSeg().front()->isArunk())
  {
    // 1st segment of pu cannot be arunk
    return false;
  }

  bool ojPUCxrPref = false;
  std::vector<MergedFareMarket*> obFareMarket;
  obFareMarket.push_back(fm);
  const MergedFareMarket& origFMkt = *fm;

  const MergedFareMarket* prevMkt = fm;
  for (uint16_t i = mktIdx + 1; i < mktIdx + obCount; ++i)
  {
    MergedFareMarket* nextMfm = fmPath[i];
    determineGeoTravelType(*nextMfm, obGeoTvlType);
    if ((obGeoTvlType == GeoTravelType::International) && (fcCount > 2))
    {
      return false;
    }
    if (!LocUtil::isSamePoint(*prevMkt->destination(),
                              prevMkt->offMultiCity(),
                              *nextMfm->origin(),
                              nextMfm->boardMultiCity()))
    {
      return false;
    }

    if (!isValidFCforOJLeg(origFMkt, *nextMfm) || puPath.isMarketAssigned(nextMfm))
    {
      return false;
    }

    if (fmReturnToOJLeg(*nextMfm, obFareMarket))
    {
      // can not return to the same leg
      return false;
    }

    ojPUCxrPref = (ojPUCxrPref || nextMfm->cxrFarePreferred());
    obFareMarket.push_back(nextMfm);
    prevMkt = nextMfm;
  }

  //----------  Get InBound Side of OJ [after Turn Around point] -------------
  //
  std::vector<MergedFareMarket*> ibFareMarket;

  for (uint16_t ojMktIdx = ibMktIdx; ojMktIdx < totalMktCnt; ++ojMktIdx)
  {
    if (UNLIKELY(ojMktIdx + ibCount > totalMktCnt))
    {
      return false;
    }

    MergedFareMarket* mfm = fmPath[ojMktIdx];
    GeoTravelType geoTvlType = obGeoTvlType;
    determineGeoTravelType(*mfm, geoTvlType);
    if ((geoTvlType == GeoTravelType::International) && (fcCount > 2))
    {
      return false;
    }

    if (puPath.isMarketAssigned(mfm))
    {
      return false;
    }
    ojPUCxrPref = (ojPUCxrPref || mfm->cxrFarePreferred());
    ibFareMarket.push_back(mfm);

    const MergedFareMarket* prevMfMkt = mfm;
    for (uint16_t i = ojMktIdx + 1, j = 1; j < ibCount && i < totalMktCnt; ++i, ++j)
    {
      MergedFareMarket* nextMfm = fmPath[i];
      determineGeoTravelType(*nextMfm, geoTvlType);
      if ((geoTvlType == GeoTravelType::International) && (fcCount > 2))
      {
        return false;
      }
      if (!LocUtil::isSamePoint(*prevMfMkt->destination(),
                                prevMfMkt->offMultiCity(),
                                *nextMfm->origin(),
                                nextMfm->boardMultiCity()))
      {
        // not continuous
        return false;
      }
      if (!isValidFCforOJLeg(origFMkt, *nextMfm) || puPath.isMarketAssigned(nextMfm))
      {
        return false;
      }
      if (fmReturnToOJLeg(*nextMfm, ibFareMarket))
      {
        return false;
      }

      ojPUCxrPref = (ojPUCxrPref || nextMfm->cxrFarePreferred());
      ibFareMarket.push_back(nextMfm);
      prevMfMkt = nextMfm;
    }

    PU* ojPU = buildOJPU(obFareMarket, ibFareMarket, geoTvlType, ojPUCxrPref);
    if (!ojPU)
      return false;

    PUPath* ojPath = constructPUPath();
    copyPUPath(puPath, *ojPath);
    ojPath->puPath().push_back(ojPU);
    ojPath->cxrFarePreferred() = (ojPath->cxrFarePreferred() || ojPU->cxrFarePreferred());

    /******************* For Test ***********************>>>>>>
     //
     string testStr;
     LOG4CXX_DEBUG(logger, "Valid OJ :")
     LOG4CXX_DEBUG(logger, "mktIdx="<< mktIdx <<
                            ", obCount="<< obCount<<
                                ", ibCount="<<ibCount)
         LOG4CXX_DEBUG(logger, ", ibMktIdx="<<ibMktIdx<<
                                ", ojMktIdx="<<ojMktIdx<<
                ", totalMktCnt="<<totalMktCnt)
     std::vector<MergedFareMarket*>::iterator it = obFareMarket.begin();
     std::vector<MergedFareMarket*>::iterator itEnd = obFareMarket.end();
     for(; it != itEnd; ++it)
     {
        testStr += string(" ") +(*it)->boardMultiCity()
                + string("-") + (*it)->offMultiCity();
     }
     LOG4CXX_DEBUG(logger, "OutBound FareMkt "<<testStr)
     testStr = "";
     it = ibFareMarket.begin();
     itEnd = ibFareMarket.end();
     for(; it != itEnd; ++it)
     {
        testStr += string(" ") +(*it)->boardMultiCity()
                + string("-") + (*it)->offMultiCity();
     }
     LOG4CXX_DEBUG(logger, "InBound FareMkt "<<testStr)
     //
     <<<<<< ************* For Test ***************/

    buildPUPath(fmp, mktIdx + 1, totalMktCnt, *ojPath, fmPath, puPathVect, done);
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------
// Valid OJ FareComponent:
//	1. Two Intl FC
//	2. One Intl FC + One Domestic (US/CA) FC
//      3. Up to 9 Domestic or Foreign Domestic
//--------------------------------------------------------------------------------
bool
PUPathMatrix::isValidFCforOJLeg(const MergedFareMarket& origFMkt, const MergedFareMarket& fm)
{

  const GeoTravelType itinTvlType = _itin->geoTravelType();
  const GeoTravelType geoTvlType = origFMkt.geoTravelType();
  const GeoTravelType fmTvlType = fm.geoTravelType();

  if (UNLIKELY(fmTvlType == GeoTravelType::International || geoTvlType == GeoTravelType::International))
  {
    // can not have more than one FC in one Leg for Intl-PU
    return false;
  }
  else if (itinTvlType == GeoTravelType::Transborder)
  {
    // Now US/CA is diff country
    if (fmTvlType == GeoTravelType::Transborder || geoTvlType == GeoTravelType::Transborder)
    {
      // can not have more than one FC in one Leg for Intl-PU
      return false;
    }
  }

  if (geoTvlType != fmTvlType)
  {
    // all needs to be Domestic or ForeignDomestic
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Templete Scope:
// Valid OJ FareComponent:
//	1. Two Intl FC
//	2. One Intl FC + One Domestic (US/CA) FC
//      3. Up to 9 Domestic or Foreign Domestic

// Orig-OJ-Surface:
//       If IATA-Logic Pass
// TurnAround-OJ-Surface:
//       If IATA-Logic Pass
//---------------------------------------------
// PU-Factory Scope
// Orig-OJ-Surface:
//       Must be within Same Country (NL-PU)
// TurnAround-OJ-Surface:
//      Diff country if Cxr-Pref Mileage check, BUT: if started from USCA and
//      Surface within sub-area 21 (NL), no mileage-check needed
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
bool
PUPathMatrix::isValidOpenJawTrip(const std::vector<MergedFareMarket*>& obLeg,
                                 const std::vector<MergedFareMarket*>& ibLeg,
                                 const GeoTravelType geoTvlType,
                                 PricingUnit::PUSubType& ojType,
                                 bool& sameNationOJ,
                                 bool& sameNationOrigSurfaceOJ,
                                 bool& allowNOJInZone210,
                                 PricingUnit::OJSurfaceStatus& ojSurfaceStatus,
                                 bool& openJawBtwTwoAreas,
                                 std::vector<CarrierCode>& invalidCxrForTOJ,
                                 bool& inDiffCntrySameSubareaForOOJ,
                                 bool& specialEuropeanDoubleOJ,
                                 bool& specialOJ)
{
  // --- Two Leg can not Intersect  ------
  //
  if (intersectOJLegs(obLeg, ibLeg))
  {
    return false;
  }

  const MergedFareMarket& obOrigFM = *obLeg.front();
  const MergedFareMarket& obDestFM = *obLeg.back();
  const MergedFareMarket& ibOrigFM = *ibLeg.front();
  const MergedFareMarket& ibDestFM = *ibLeg.back();

  DateTime travelDate = obOrigFM.mergedFareMarket().front()->travelDate();

  if (geoTvlType == GeoTravelType::International)
  {
    // Valid:   1 Intl  + 1 Domestic (US/CA)
    // Invalid: 1 Intl  + 1 F. Domestic
    //
    if (obOrigFM.geoTravelType() == GeoTravelType::ForeignDomestic || ibOrigFM.geoTravelType() == GeoTravelType::ForeignDomestic)
    {
      return false;
    }
  }

  // --- Must need at leas 1 Tag-2 Fare in US/CA Market ---
  //
  if ((geoTvlType == GeoTravelType::Domestic || geoTvlType == GeoTravelType::Transborder) && !checkTag2FareIndicator(obLeg) &&
      !checkTag2FareIndicator(ibLeg))
  {
    // No Tag-2 fare in these Market
    return false;
  }

  const Loc& obOrig = *obOrigFM.origin();
  const Loc& obDest = *obDestFM.destination();
  const Loc& ibOrig = *ibOrigFM.origin();
  const Loc& ibDest = *ibDestFM.destination();

  bool origMatch = obOrigFM.boardMultiCity() == ibDestFM.offMultiCity();
  bool destMatch = obDestFM.offMultiCity() == ibOrigFM.boardMultiCity();

  if (origMatch && destMatch)
  {
    // OJ failed: It is RT
    return false;
  }

  if (UNLIKELY(isSamePointInFcc(origMatch, obOrigFM, ibDestFM, destMatch, obDestFM, ibOrigFM)))
  {
    // OJ failed: It is RT
    return false;
  }

  if (!origMatch && !destMatch && obOrigFM.geoTravelType() == GeoTravelType::International &&
      ibOrigFM.geoTravelType() == GeoTravelType::International)
  { // check special European Double OJ condition:
    //    all international and (A - B /- A - C or . A - B /- C - B)
    if (isSameAnyOneXPoint(obOrigFM, ibDestFM, obDestFM, ibOrigFM) &&
        isSpecialEuropeanDoubleOJ(obOrig, obDest, ibOrig, ibDest))
      specialEuropeanDoubleOJ = true;
  }

  if (!specialEuropeanDoubleOJ)
  {
    if (LocUtil::isSamePoint(obOrig,
                             obOrigFM.travelSeg().front()->boardMultiCity(),
                             ibOrig,
                             ibOrigFM.travelSeg().front()->boardMultiCity()) ||
        LocUtil::isSamePoint(obDest,
                             obDestFM.travelSeg().back()->offMultiCity(),
                             ibDest,
                             ibDestFM.travelSeg().back()->offMultiCity()))
    {
      // DFW->ORD DFW->ORD  can not be OJ
      // DFW->LON DFW->PAR or DFW->PAR ORD->PAR can not be OJ
      return false;
    }
  }

  // It could be Open Jaw at this point, if milage permits
  //
  //-------------------- Determine OpenJaw Type  -----------------------
  const bool origDiffArea = (obOrig.area() != ibDest.area()) ? true : false;

  if (origDiffArea &&
      !(LocUtil::isRussianGroup(obOrig) &&
        LocUtil::isRussianGroup(ibDest))) // Russia: RU and XU belongs to area 2 & 3
  {
    if (!TrxUtil::isdiffCntyNMLOpenJawActivated(*_trx))
    {
      if (TrxUtil::isSpecialOpenJawActivated(*_trx))
      {
        specialOJ = true;
      }
      else
      {
        // Different IATAArea: OJ Failed
        return false;
      }
    }
    else
      openJawBtwTwoAreas = true;
  }
  // --- Set OJ type and check if surface is within the Same Nation ---
  //
  if (origMatch)
  {
    ojType = PricingUnit::DEST_OPENJAW;
    if (LocUtil::isWithinSameCountry(
            _itin->geoTravelType(), _travelWithinScandinavia, obDest, ibOrig, obOrig, ibDest))
    {
      sameNationOJ = true;
      allowNOJInZone210 = true; // sameNationOJ, NOJ automatically allowed
    }
  }
  else if (destMatch)
  {
    ojType = PricingUnit::ORIG_OPENJAW;
    if (LocUtil::isWithinSameCountry(
            _itin->geoTravelType(), _travelWithinScandinavia, obOrig, ibDest, obDest, ibOrig))
    {
      sameNationOJ = true;
      sameNationOrigSurfaceOJ = true;
      allowNOJInZone210 = true; // sameNationOJ, NOJ automatically allowed
    }
  }
  else
  {
    ojType = PricingUnit::DOUBLE_OPENJAW;
    if (LocUtil::isWithinSameCountry(
            _itin->geoTravelType(), _travelWithinScandinavia, obOrig, ibDest, obDest, ibOrig))
    {
      sameNationOrigSurfaceOJ = true;
    }

    if (sameNationOrigSurfaceOJ &&
        LocUtil::isWithinSameCountry(
            _itin->geoTravelType(), _travelWithinScandinavia, obDest, ibOrig, obOrig, ibDest))
    {
      // dest surface also within a country
      sameNationOJ = true;
      allowNOJInZone210 = true; // sameNationOJ, NOJ automatically allowed
    }
  }

  if (specialEuropeanDoubleOJ)
  {
    // do not check subarea restriction for DoubleOJ in Europe
    // call checkOJMileageRestriction to setup ojSurfaceStatus
    if (!checkOJMileageRestriction(obLeg,
                                   ibLeg,
                                   geoTvlType,
                                   ojType,
                                   ojSurfaceStatus,
                                   travelDate,
                                   openJawBtwTwoAreas,
                                   invalidCxrForTOJ))
    {
      return false; // some mileage error
    }

    return checkCarrierPrefForSPEuropeanDoubleOJ(obLeg, ibLeg, invalidCxrForTOJ);
  }

  //@TODO add NationCode in TseConst.h for Chile

  //-------------------- Check Mileage -----------------------
  //
  bool failOnMileage = false;
  if (!sameNationOJ)
  {
    bool invalidOJ = false;
    if (!checkOJIataArea(obOrig,
                         obDest,
                         obOrigFM.globalDirection(),
                         ibOrig,
                         ibDest,
                         ibOrigFM.globalDirection(),
                         ojType,
                         invalidOJ,
                         openJawBtwTwoAreas,
                         failOnMileage))
    {
      if (ojType == PricingUnit::DEST_OPENJAW)
      {
        // NL Dest-OJ allow surface on diff-country if mileage check permits and
        // does not care about IATA area.
        // But after we build this NL-PU we need to check subcat 101
        // if it allow this type of OJ
        //
        // To allow for NL Dest-OJ like: MIL-SIN, BAH-MIL, to solve PL#8548

        if (!invalidOJ && checkOJMileageRestriction(obLeg,
                                                    ibLeg,
                                                    geoTvlType,
                                                    ojType,
                                                    ojSurfaceStatus,
                                                    travelDate,
                                                    openJawBtwTwoAreas,
                                                    invalidCxrForTOJ,
                                                    true))
        {
          return true;
        }
      }
      return false;
    }
    // setup inDiffCntrySameSubareaForOOJ for the International OOJ when:
    //  -Country of orig (1st leg) and Country of dest (2nd leg)
    //   are different and within the same subarea.
    //  -The same global direction for the 1st and 2nd legs.
    // Price NL fares when it allows in CarrierPreference tbl.
    if (ojType == PricingUnit::ORIG_OPENJAW && !sameNationOrigSurfaceOJ)
    {
      if (!allowNOJInZone210 &&
          (obOrig.subarea() == ibDest.subarea())) // Per Darrin: do not check same Global
        inDiffCntrySameSubareaForOOJ = true;
    }
  }

  if (checkOJMileageRestriction(obLeg,
                                ibLeg,
                                geoTvlType,
                                ojType,
                                ojSurfaceStatus,
                                travelDate,
                                openJawBtwTwoAreas || specialOJ,
                                invalidCxrForTOJ,
                                failOnMileage))
  {
    // Only for domestic/trasnboarder/F.Domestic it may fail mileage check
    // for Intl it will not fail yet.
    //
    return true;
  }

  return false;
}

bool
PUPathMatrix::checkCarrierPrefForSPEuropeanDoubleOJ(const std::vector<MergedFareMarket*>& obLeg,
                                                    const std::vector<MergedFareMarket*>& ibLeg,
                                                    std::vector<CarrierCode>& invalidCxrForTOJ)
{
  return (checkCarrierPrefForSPEuropeanDoubleOJPerLeg(obLeg, invalidCxrForTOJ) &&
          checkCarrierPrefForSPEuropeanDoubleOJPerLeg(ibLeg, invalidCxrForTOJ));
}

// ----------------------------------------------------------------------------
bool
PUPathMatrix::checkCarrierPrefForSPEuropeanDoubleOJPerLeg(
    const std::vector<MergedFareMarket*>& leg, std::vector<CarrierCode>& invalidCxrForTOJ)
{
  std::vector<MergedFareMarket*>::const_iterator fmIt = leg.begin();
  const MergedFareMarket& mfm = *(*fmIt);

  bool passSPEuropeanDOJ = false;
  for (const auto fm : mfm.mergedFareMarket())
  {
    const CarrierPreference* cxrPref = fm->governingCarrierPref();
    if (cxrPref && (cxrPref->applySpclDOJEurope() == 'Y'))
      passSPEuropeanDOJ = true; // pass this carrier
    else
    { // multi_governing_carrier logic:
      const auto governingCarrierCode = fm->governingCarrier();
      if (std::none_of(invalidCxrForTOJ.cbegin(),
                       invalidCxrForTOJ.cend(),
                       [governingCarrierCode](const CarrierCode& cc)
                       { return cc == governingCarrierCode; }))
        invalidCxrForTOJ.push_back(governingCarrierCode);
    }
  }
  return passSPEuropeanDOJ;
}

//--------------------------------------------------------------------

bool
PUPathMatrix::checkOJMileageRestriction(const std::vector<MergedFareMarket*>& obLeg,
                                        const std::vector<MergedFareMarket*>& ibLeg,
                                        const GeoTravelType geoTvlType,
                                        const PricingUnit::PUSubType& ojType,
                                        PricingUnit::OJSurfaceStatus& ojSurfaceStatus,
                                        DateTime travelDate,
                                        const bool openJawBtwTwoAreas,
                                        std::vector<CarrierCode>& invalidCxrForTOJ,
                                        bool failOnMileage)
{
  uint32_t ob_mileage = getOJLegMileage(obLeg, geoTvlType, travelDate);
  uint32_t ib_mileage = getOJLegMileage(ibLeg, geoTvlType, travelDate);

  const MergedFareMarket& obOrigFM = *obLeg.front();
  const MergedFareMarket& obDestFM = *obLeg.back();
  const MergedFareMarket& ibOrigFM = *ibLeg.front();
  const MergedFareMarket& ibDestFM = *ibLeg.back();

  const Loc& obOrig = *obOrigFM.origin();
  const Loc& obDest = *obDestFM.destination();
  const Loc& ibOrig = *ibOrigFM.origin();
  const Loc& ibDest = *ibDestFM.destination();

  uint32_t surface1_mileage = 0;
  uint32_t surface2_mileage = 0;

  switch (ojType)
  {
  case PricingUnit::DEST_OPENJAW:
    surface1_mileage = getSurfaceMileage(obDest, ibOrig, travelDate);
    break;

  case PricingUnit::ORIG_OPENJAW:
    surface1_mileage = getSurfaceMileage(obOrig, ibDest, travelDate);
    break;

  case PricingUnit::DOUBLE_OPENJAW:
    surface1_mileage = getSurfaceMileage(obOrig, ibDest, travelDate);
    surface2_mileage = getSurfaceMileage(obDest, ibOrig, travelDate);
    break;

  default:
    break;
  }

  uint32_t smaller_mileage = ob_mileage;
  uint32_t larger_mileage = ib_mileage;
  if (ib_mileage < ob_mileage)
  {
    smaller_mileage = ib_mileage;
    larger_mileage = ob_mileage;
  }


  if (LIKELY(TrxUtil::isSpecialOpenJawActivated(*_trx)))
  {
    if ((openJawBtwTwoAreas) &&
        (ojType == PricingUnit::ORIG_OPENJAW ||
              ojType == PricingUnit::DOUBLE_OPENJAW) &&
              surface1_mileage > larger_mileage)
    {
      return false;
    }
  }

  if (openJawBtwTwoAreas &&
      (ojType == PricingUnit::DOUBLE_OPENJAW || ojType == PricingUnit::PricingUnit::DEST_OPENJAW))
  {
    uint32_t surfaceTOJ_mileage = surface1_mileage;
    if (ojType == PricingUnit::DOUBLE_OPENJAW)
      surfaceTOJ_mileage = surface2_mileage;
    // check the carrierPref for shorter/longerFC when TOJ is across 2 Areas (only for
    // Itnernational)
    if (!checkMileageForTOJAcross2Areas(
            obLeg, ibLeg, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ))
      return false;
  }

  if (surface1_mileage <= smaller_mileage && surface2_mileage <= smaller_mileage)
  {
    //
    ojSurfaceStatus = PricingUnit::SURFACE_SHORTEST;
    return true;
  }
  if (surface1_mileage <= larger_mileage && surface2_mileage <= larger_mileage)
  {
    ojSurfaceStatus = PricingUnit::SURFACE_NOT_SHORTEST;
    return true;
  }
  if (failOnMileage)
  {
    // this flag is set only when IATA-Subarea check has failed but we want to
    // see if we should alow based on mileage
    return false;
  }

  // New OJ logic: do not fail OJ PU for the ForeignDomestic
  if (geoTvlType != GeoTravelType::International && geoTvlType != GeoTravelType::ForeignDomestic)
  {
    return false;
  }

  if (surface1_mileage < 1.25 * larger_mileage && surface2_mileage < 1.25 * larger_mileage)
  {
    ojSurfaceStatus = PricingUnit::SURFACE_125LARGER;
  }
  else
  {
    ojSurfaceStatus = PricingUnit::SURFACE_VERYLARGE;
  }

  return true;
}

// ----------------------------------------------------------------------------
bool
PUPathMatrix::checkMileageForTOJAcross2Areas(const std::vector<MergedFareMarket*>& obLeg,
                                             const std::vector<MergedFareMarket*>& ibLeg,
                                             const uint32_t smaller_mileage,
                                             const uint32_t larger_mileage,
                                             const uint32_t surfaceTOJ_mileage,
                                             std::vector<CarrierCode>& invalidCxrForTOJ)
{
  return (checkMileageForTOJAcross2AreasPerLeg(
              obLeg, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ) &&
          checkMileageForTOJAcross2AreasPerLeg(
              ibLeg, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ));
}

// ----------------------------------------------------------------------------
bool
PUPathMatrix::checkMileageForTOJAcross2AreasPerLeg(const std::vector<MergedFareMarket*>& leg,
                                                   const uint32_t smaller_mileage,
                                                   const uint32_t larger_mileage,
                                                   const uint32_t surfaceTOJ_mileage,
                                                   std::vector<CarrierCode>& invalidCxrForTOJ)
{
  std::vector<MergedFareMarket*>::const_iterator fmIt = leg.begin();
  const MergedFareMarket& mfm = *(*fmIt);

  bool passShorterOrLonger = false;
  for (const auto fm : mfm.mergedFareMarket())
  {
    const CarrierPreference* cxrPref = fm->governingCarrierPref();

    if (cxrPref && (cxrPref->applySingleTOJBetwAreasShorterFC() == 'Y') &&
        (surfaceTOJ_mileage <= smaller_mileage))
      passShorterOrLonger = true; // pass this carrier
    else if (cxrPref && (cxrPref->applySingleTOJBetwAreasLongerFC() == 'Y') &&
             (surfaceTOJ_mileage <= larger_mileage))
      passShorterOrLonger = true; // pass this carrier
    else
    {
      const auto governingCarrierCode = fm->governingCarrier();
      if (std::none_of(invalidCxrForTOJ.cbegin(),
                       invalidCxrForTOJ.cend(),
                       [governingCarrierCode](const CarrierCode& cc)
                       { return cc == governingCarrierCode; }))
        invalidCxrForTOJ.push_back(governingCarrierCode);
    }
  }
  return passShorterOrLonger;
}

// ----------------------------------------------------------------------------
uint32_t
PUPathMatrix::getOJLegMileage(const std::vector<MergedFareMarket*>& leg,
                              const GeoTravelType geoTvlType,
                              DateTime travelDate)
{
  if (geoTvlType != GeoTravelType::International)
  {
    const MergedFareMarket& origFM = *leg.front();
    const MergedFareMarket& destFM = *leg.back();

    const Loc& orig = *origFM.origin();
    const Loc& dest = *destFM.destination();
    return getSurfaceMileage(orig, dest, travelDate);
  }

  uint32_t legMileage = 0;

  // Intl and F. Domestic is calculated by adding each flown TvlSeg
  GlobalDirection gd;
  for (MergedFareMarket* mfm : leg)
  {
    FareMarket* fareMarket = mfm->mergedFareMarket().front();
    for (TravelSeg* travelSeg : mfm->travelSeg())
    {
      const Loc& loc1 = *travelSeg->origin();
      const Loc& loc2 = *travelSeg->destination();
      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          _trx, fareMarket->travelDate(), *travelSeg, gd);
      legMileage += LocUtil::getTPM(loc1, loc2, gd, fareMarket->travelDate(), _trx->dataHandle());
    }
  }

  return legMileage;
}

//--------------------------------------------------------------------

bool
PUPathMatrix::checkOJIataArea(const Loc& obOrig,
                              const Loc& obDest,
                              const GlobalDirection obGD,
                              const Loc& ibOrig,
                              const Loc& ibDest,
                              const GlobalDirection ibGD,
                              const PricingUnit::PUSubType& ojType,
                              bool& invalidOJ,
                              bool& openJawBtwTwoAreas,
                              bool& failOnMileage)
{

  const IATAAreaCode& obOrigArea = obOrig.area();
  const IATAAreaCode& obDestArea = obDest.area();
  const IATAAreaCode& ibOrigArea = ibOrig.area();
  const IATAAreaCode& ibDestArea = ibDest.area();

  const IATASubAreaCode& obOrigSubArea = obOrig.subarea();
  const IATASubAreaCode& obDestSubArea = obDest.subarea();
  const IATASubAreaCode& ibOrigSubArea = ibOrig.subarea();
  const IATASubAreaCode& ibDestSubArea = ibDest.subarea();

  bool allAreaInvolved = false;

  if (_itinBoundary == ALL_IATA)
  {
    bool area1 = (obOrigArea == IATA_AREA1) || (obDestArea == IATA_AREA1) ||
                 (ibOrigArea == IATA_AREA1) || (ibDestArea == IATA_AREA1);
    bool area2 = (obOrigArea == IATA_AREA2) || (obDestArea == IATA_AREA2) ||
                 (ibOrigArea == IATA_AREA2) || (ibDestArea == IATA_AREA2);
    bool area3 = (obOrigArea == IATA_AREA3) || (obDestArea == IATA_AREA3) ||
                 (ibOrigArea == IATA_AREA3) || (ibDestArea == IATA_AREA3);

    if (area1 && area2 && area3)
    {
      // do not fail 3 IATA Areas over Pacific --. new openJaw logic
      allAreaInvolved = true;
    }
  }

  if (ojType == PricingUnit::ORIG_OPENJAW)
  {
    if (allAreaInvolved)
    {
      if ((obOrigArea == IATA_AREA2 && obDestArea == IATA_AREA1) &&
          (ibOrigArea == IATA_AREA1 && ibDestArea == IATA_AREA3))
      {
        // Travel Area Path: 2 --> 1 --> 3
        // LOG4CXX_ERROR(logger, "3-IATA OJ VALID LN#" <<__LINE__)
        return true;
      }
      else if ((obOrigArea == IATA_AREA3 && obDestArea == IATA_AREA1) &&
               (ibOrigArea == IATA_AREA1 && ibDestArea == IATA_AREA2))
      {
        // Travel Area Path: 3 --> 1 --> 2
        return true;
      }
      else if ((obOrigArea == IATA_AREA2 && obDestArea == IATA_AREA3) &&
               (ibOrigArea == IATA_AREA3 && ibDestArea == IATA_AREA1))
      {
        // Travel Area Path: 2 --> 3 --> 1
        return true;
      }
      else if ((obOrigArea == IATA_AREA1 && obDestArea == IATA_AREA3) &&
               (ibOrigArea == IATA_AREA3 && ibDestArea == IATA_AREA2))
      {
        // Travel Area Path: 1 --> 3 --> 2
        return true;
      }
      else if (TrxUtil::isSpecialOpenJawActivated(*_trx) &&
              (obOrigArea == IATA_AREA1 && obDestArea == IATA_AREA2) &&
              (ibOrigArea == IATA_AREA2 && ibDestArea == IATA_AREA3))
      {
        // Travel Area Path: 1 --> 2 --> 3
        return true;
      }
    }
    else
    {
      if (obOrigSubArea == ibDestSubArea)
      {
        // Origin Surface in same IataSub area
        return true;
      }
      else
      {
        if (UNLIKELY(!TrxUtil::isSpecialOpenJawActivated(*_trx)))
        {
          if (LocUtil::isRussianGroup(obOrig) && LocUtil::isRussianGroup(ibDest))
          {
            // only Russia belongs to two diff area. Nation code for these two
            // parts are RU and XU but allow such OJ
            return true;
          }

          if (obOrigArea != ibDestArea) // diff IATA area
          {
            invalidOJ = true;
            return false;
          }
        }
        if (obOrigSubArea == obDestSubArea || ibDestSubArea == obDestSubArea)
        {
          // surface end point(s) is in the sub-area of Turnaround point
          failOnMileage = true;
          return true;
        }
        return true;
      }
    }
  }
  else if (ojType == PricingUnit::DEST_OPENJAW)
  {
    if (allAreaInvolved)
    {
      // do not fail 3 IATA Areas 2 --> 3 // 1 --> 2  (new openJaw logic)
      openJawBtwTwoAreas = true;
      return true;
    }
    else
    {
      if (obDestSubArea == ibOrigSubArea)
      {
        // Turnaround Surface is in same IataSub area
        return true;
      }
      else
      {
        if (LocUtil::isRussianGroup(obDest) && LocUtil::isRussianGroup(ibOrig))
        {
          // only Russia belongs to two diff area. Nation code for these two
          // parts are RU and XU but allow such OJ
          return true;
        }
        if (obDestArea != ibOrigArea) // diff IATA area
        {
          // need to check a mileage for 'shorted/longerFC'  (new openJaw logic)
          openJawBtwTwoAreas = true;
          return true;
        }
        if (obDestSubArea == obOrigSubArea || ibOrigSubArea == obOrigSubArea)
        {
          // surface end point(s) is in the sub-area of Origing point
          return false;
        }
        return true;
      }
    }
  }
  else if (LIKELY(ojType == PricingUnit::DOUBLE_OPENJAW))
  {
    if (allAreaInvolved)
    {
      openJawBtwTwoAreas = true;
      return true;
    }
    else
    {
      if ((obDestSubArea == ibOrigSubArea) && (ibDestSubArea == obOrigSubArea))
      {
        // Surfaces are within same IataSub area
        return true;
      }
      else
      {
        const bool destSurfaceInRussia =
            LocUtil::isRussianGroup(obDest) && LocUtil::isRussianGroup(ibOrig);

        if (obDestArea != ibOrigArea && !destSurfaceInRussia) // diff IATA area
        {
          openJawBtwTwoAreas = true;
          return true;
        }
        const bool origSurfaceInRussia =
            LocUtil::isRussianGroup(obOrig) && LocUtil::isRussianGroup(ibDest);

        if (!TrxUtil::isSpecialOpenJawActivated(*_trx))
        {
          if (obOrigArea != ibDestArea && !origSurfaceInRussia) // diff IATA area
          {
            invalidOJ = true;
            return false;
          }
        }

        if ((!origSurfaceInRussia && !destSurfaceInRussia) &&
            (obDestSubArea == obOrigSubArea || obDestSubArea == ibDestSubArea ||
             ibOrigSubArea == obOrigSubArea || ibOrigSubArea == ibDestSubArea))
        {
          // Origin surface end point(s) is in the sub-area of Turn around points
          // OR
          // Turn around surface end point(s) is in the sub-area of Origin points
          failOnMileage = true;
          return true;
        }
        return true;
      }
    }
  }
  return false;
}

//--------------------------------------------------------------------
bool
PUPathMatrix::isSamePointInFcc(bool& origMatch,
                               const MergedFareMarket& obOrigFM,
                               const MergedFareMarket& ibDestFM,
                               bool& destMatch,
                               const MergedFareMarket& obDestFM,
                               const MergedFareMarket& ibOrigFM)
{
  const std::vector<FareCalcConfigSeg*>& fcSegs = _fcConfig->segs();
  if (LIKELY(fcSegs.empty()))
  {
    return false;
  }

  if (!origMatch)
  {
    origMatch = FareCalcUtil::isSameDisplayLoc(*_fcConfig,
                                        obOrigFM.travelSeg().front()->origAirport(),
                                        obOrigFM.boardMultiCity(),
                                        ibDestFM.travelSeg().back()->destAirport(),
                                        ibDestFM.offMultiCity());
  }
  if (!destMatch)
  {
    destMatch = FareCalcUtil::isSameDisplayLoc(*_fcConfig,
                                        obDestFM.travelSeg().back()->destAirport(),
                                        obDestFM.offMultiCity(),
                                        ibOrigFM.travelSeg().front()->origAirport(),
                                        ibOrigFM.boardMultiCity());
  }
  if (origMatch && destMatch)
  {
    return true;
  }
  return false;
}

//------------------------------------------------------------------
// Build ONE WAY TRIP
//------------------------------------------------------------------
bool
PUPathMatrix::buildOW(const FareMarketPath& fmp,
                      const uint16_t mktIdx,
                      const uint16_t totalMktCnt,
                      PUPath& puPath,
                      std::vector<MergedFareMarket*>& fmPath,
                      std::vector<PUPath*>& puPathVect,
                      bool& done)
{
  if (done)
  {
    // To trim Big PNR
    return true;
  }

  if (mktIdx >= totalMktCnt)
  {
    return true;
  }

  MergedFareMarket* fm = fmPath[mktIdx];

  if (puPath.isMarketAssigned(fm))
  {
    buildOW(fmp, mktIdx + 1, totalMktCnt, puPath, fmPath, puPathVect, done);
    return true;
  }

  PU* owPU = buildOWPU(fm);

  if (!owPU)
    return true;

  puPath.puPath().push_back(owPU);
  puPath.cxrFarePreferred() = (puPath.cxrFarePreferred() || owPU->cxrFarePreferred());

  buildPUPath(fmp, mktIdx + 1, totalMktCnt, puPath, fmPath, puPathVect, done);
  return true;
}

//--------------------------------------------------------------------------------
void
PUPathMatrix::updatePUFactoryBucket(PUPath& puPath,
                                    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                                    std::set<PU*, PU::PUPtrCmp>& alreadySeen)
{
  if (UNLIKELY(puFactoryBucketVect.empty()))
    return;

  for (auto& pu : puPath.puPath())
    addPUToPUFactoryBucket(pu, puFactoryBucketVect, alreadySeen);

  if (puPath.puPath().size() == 1)
  {
    // only one PU completes the MainTrip, set the flag
    // to be used by the combinabiltiy
    puPath.puPath().front()->noPUToEOE() = true;
    if (puPath.sideTripPUPath().empty())
    {
      puPath.puPath().front()->isCompleteJourney() = true;
      puPath.puPath().front()->noPUToEOE() = true; // can be removed with fallback code
      return;
    }
  }

  if (puPath.sideTripPUPath().empty())
    return;

  for (const auto& stPUPath : puPath.sideTripPUPath())
  {
    for (const auto puPath : stPUPath.second)
    {
      for (auto& pu : puPath->puPath())
      {
        if (!pu->possibleSideTripPU())
          pu->possibleSideTripPU() = true;

        addPUToPUFactoryBucket(pu, puFactoryBucketVect, alreadySeen);
      }
    }
  }
}

//--------------------------------------------------------------------------------
void
PUPathMatrix::addPUToPUFactoryBucket(PU*& pu,
                                     std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                                     std::set<PU*, PU::PUPtrCmp>& alreadySeen)
{
  std::pair<std::set<PU*, PU::PUPtrCmp>::iterator, bool> seen = alreadySeen.insert(pu);

  // if we couldn't insert it, it means we've already seen this PU.
  // return without processing
  if (seen.second == false)
  {
    if (pu->possibleSideTripPU() != (*seen.first)->possibleSideTripPU())
      (*seen.first)->possibleSideTripPU() = true;

    pu = *seen.first; // same PU is shared by all PUPath
    return;
  }

  // new PU to PricingUnitFactoryBucket
  pu->setFCCount();
  pu->itinWithinScandinavia() = _travelWithinScandinavia;
  pu->itinGeoTvlType() = _itin->geoTravelType();
  setPUGovCarrier(*pu);

  // add this PU into the Bucket for each PaxType
  for (const auto puFactoryBucket : puFactoryBucketVect)
  {
    PricingUnitFactory* puFactory = nullptr;
    _trx->dataHandle().get(puFactory);
    puFactory->pu() = pu;
    puFactory->fcCount() = pu->fcCount();
    puFactory->trx() = _trx;
    puFactory->itin() = _itin;
    puFactory->isISPhase() = _trx->isShopping();
    puFactory->done() = false;
    puFactory->useCxrPreference() = useCxrPreference();
    puFactory->paxType() = puFactoryBucket->paxType();
    puFactory->combinations() = puFactoryBucket->combinations();
    puFactory->journeyPULowerBound() = puFactoryBucket->journeyPULowerBound();
    puFactoryBucket->puFactoryBucket().insert(std::make_pair(pu, puFactory));
  }

  return;
}

//-------------------------------------------------------------------------------------------------
// Build Side Trip PUPath
//
// Requirement is to support at most 2 SideTrip for a FareMarket. But this algorithm does not have
// any hard coded limit of 2. SideTrip out of a SideTrip is not a requirement
//
//-------------------------------------------------------------------------------------------------
bool
PUPathMatrix::buildSideTripPUPath(
    FareMarketPath& fareMarketPath,
    std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > >& pathPUPathCombVect)

{
  std::map<MergedFareMarket*, std::vector<FareMarketPath*> >& sideTrips =
      fareMarketPath.sideTrips();

  // Each element in the map is to hold all the PUPath of all the ST
  // for a FareMarket
  std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > > pathPUPathAll;

  bool stPUPathFailed = false;
  for (const auto& sideTrip : sideTrips) // Requirement: at most 2 ST
  {
    std::vector<std::vector<PUPath*> > mktPUPathVect;
    MergedFareMarket* fm = sideTrip.first;
    for (const auto stFmp : sideTrip.second)
    {
      std::vector<MergedFareMarket*>& fmp = stFmp->fareMarketPath();
      PUPath* stPuPath = constructPUPath();
      const uint16_t idx = 0;
      const uint16_t mktCount = fmp.size();

      std::vector<PUPath*> stPUPathVect;
      bool done = false;
      buildPUPath(fareMarketPath, idx, mktCount, *stPuPath, fmp, stPUPathVect, done);

      if (!stPUPathVect.empty())
        mktPUPathVect.push_back(stPUPathVect); // one entry per ST of this Mkt
      else
      {
        LOG4CXX_INFO(logger, "stPUPathVect empty")
        stPUPathFailed = true;
        break;
      }
      stPUPathVect.clear();
    }

    if (stPUPathFailed)
    {
      break;
    }
    std::vector<std::vector<PUPath*> > mktPUPathCombVect;

    // --------- Build Combinations: Recursive Algorithm ---------
    // Find all combination of sideTrip PUPath for a FareMarket
    //
    std::vector<PUPath*> newPUPathVect;
    uint16_t stCount = mktPUPathVect.size();
    genMktPUPathCombination(newPUPathVect, 0, stCount, mktPUPathVect, mktPUPathCombVect);

    pathPUPathAll.insert(STV_VALUE_TYPE(fm, mktPUPathCombVect));

    mktPUPathCombVect.clear();
    mktPUPathVect.clear();

  } // for each ST

  // --------- Build Combinations: Recursive Algorithm ---------
  if (stPUPathFailed)
  {
    return false;
  }
  std::map<MergedFareMarket*, std::vector<PUPath*> > newPUPathComb;
  std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >::iterator it =
      pathPUPathAll.begin();
  genPathPUPathCombination(newPUPathComb, it, pathPUPathAll, pathPUPathCombVect);

  pathPUPathAll.clear();

  return true;
}

//--------------------------------------------------------------------------------
void
PUPathMatrix::setOWPUDirectionality(PUPath& puPath)
{
  // When a pricing unit for a one way subjourney terminates in a country FROM which
  // a previous international pricing unit has been assessed, the pricing unit for
  // that one way subjourney will be Inbound.

  std::vector<PU*>::iterator puIter = puPath.puPath().begin();
  const std::vector<PU*>::iterator puIterEnd = puPath.puPath().end();
  ++puIter;

  for (; puIter != puIterEnd; ++puIter)
  {
    if ((*puIter)->puType() != PricingUnit::Type::ONEWAY)
    {
      continue;
    }

    if (_itin && (_itin->geoTravelType() == GeoTravelType::Domestic || _itin->geoTravelType() == GeoTravelType::Transborder))
    {
      continue; // if journey is wholly within US/CA, OW PU should always be seen as Outbound
    }

    if ((*puIter)->geoTravelType() == GeoTravelType::ForeignDomestic ||
        (*puIter)->geoTravelType() == GeoTravelType::Transborder || (*puIter)->geoTravelType() == GeoTravelType::Domestic)
    {
      continue;
    }

    MergedFareMarket& currPUFM = *(*puIter)->fareMarket().front();
    std::vector<PU*>::iterator it = puPath.puPath().begin();
    for (; it != puIter; ++it)
    {
      PU& prevPU = *(*it);
      const MergedFareMarket& prevPUFM = *prevPU.fareMarket().front();
      const Loc& prevPUOrigin = *prevPUFM.origin();

      // check following two situations to give InBound directionality:
      //  1. Is OW PU returning to the same country (of a previous PU-Origin and
      //     that PU has an Intl outbound FC).
      //  2. Is OW PU returning to the same city (of a previous PU-Origin with a outbound FC).
      //     It involves domestic/same country OW.

      if (prevPUOrigin.nation() != currPUFM.origin()->nation() && // COULD BE diff country
          (!LocUtil::isRussianGroup(prevPUOrigin) || !LocUtil::isRussianGroup(*currPUFM.origin())))
      {
        // is returning to the country of origin Or US/CA, or Scandinavia?
        if (isInboundToCountry(prevPUOrigin, currPUFM))
        {
          std::vector<MergedFareMarket*>::const_iterator it = prevPU.fareMarket().begin();
          const std::vector<MergedFareMarket*>::const_iterator itEnd = prevPU.fareMarket().end();
          for (uint16_t mktIdx = 0; it != itEnd; ++it, ++mktIdx)
          {
            const MergedFareMarket& fm = *(*it);
            if (isOutBoundFromCountry(prevPUOrigin, fm))
            {
              // This is the 1st Intl component, Is it outbound?
              if (prevPU.fareDirectionality()[mktIdx] == FROM)
              {
                // Returning to Country: OW Trip set to INBOUND
                (*puIter)->fareDirectionality()[0] = TO; // OW: only one fare Component
                break;
              }
            }
          }
        }
      }
      else if (LocUtil::isSamePoint(prevPUOrigin,
                                    prevPUFM.boardMultiCity(),
                                    *currPUFM.destination(),
                                    currPUFM.offMultiCity()))
      {
        if (prevPU.fareDirectionality().front() == FROM)
        {
          // Returning to Origin: OW Trip set to INBOUND
          (*puIter)->fareDirectionality()[0] = TO;
          break;
        }
      }
    }
  }

  return;
}
//--------------------------------------------------------------------------------
// isIntlCTJourneyWithOWPU flag is only to optimize search for NL/SP fare loop
// for those PUPath/FarePath where it forms a loop/CT with OW PU
// This optimization is not for the ones with SideTrip
//--------------------------------------------------------------------------------

void
PUPathMatrix::setIsIntlCTJourneyWithOWPU(const FareMarketPath& fmp, PUPath& puPath)
{
  if (fmp.fareMarketPath().size() < 2)
    return;

  if (UNLIKELY(!puPath.sideTripPUPath().empty()))
  {
    return;
  }

  bool owExists = false;
  for (const auto pu : puPath.puPath())
  {
    // if OWs are Domestic or Transborder we would not Fail the
    // FarePath. Let 104 decide the validity, per discussion on 04/02/2007.
    //
    if (pu->puType() == PricingUnit::Type::ONEWAY &&
        (pu->geoTravelType() == GeoTravelType::International || pu->geoTravelType() == GeoTravelType::ForeignDomestic))
    {
      if (pu->fareMarket().front()->origin()->nation() == JAPAN &&
          (pu->fareMarket().front()->destination()->nation() == JAPAN ||
           pu->fareDirectionality().front() == FROM))
      {
        // need to wait for fare selection, SP-Fare is allowed but NL-Fare is not
        return;
      }

      owExists = true;
      break;
    }
  }
  if (!owExists)
  {
    return;
  }

  std::vector<MergedFareMarket*>::const_iterator it = fmp.fareMarketPath().begin();
  std::vector<MergedFareMarket*>::const_iterator itEnd = fmp.fareMarketPath().end();

  const MergedFareMarket& firstMFM = **it;
  const Loc& orig = *(firstMFM.origin());
  const LocCode& origCity = firstMFM.boardMultiCity();
  const Loc* dest = firstMFM.destination();
  LocCode destCity = firstMFM.offMultiCity();

  bool intlJourney = false;

  ++it;
  for (; it != itEnd; ++it)
  {
    const MergedFareMarket& mfm = **it;
    if (LocUtil::isSamePoint(*dest, destCity, *mfm.origin(), mfm.boardMultiCity()))
    {
      // LOG4CXX_INFO(logger, "Continuous ")
      dest = mfm.destination();
      destCity = mfm.offMultiCity();
      std::vector<MergedFareMarket*>::const_iterator i = fmp.fareMarketPath().begin();
      for (; i != it; ++i)
      {
        if (LocUtil::isSamePoint(*(*i)->destination(), (*i)->offMultiCity(), *dest, destCity))
        {
          return; // returning to a intermediate point
        }
      }
    }
    else
    {
      // LOG4CXX_INFO(logger, "Not Continuous ")
      return;
    }

    if (mfm.geoTravelType() == GeoTravelType::International || mfm.geoTravelType() == GeoTravelType::ForeignDomestic)
    {
      intlJourney = true;
    }
  }

  if (!intlJourney)
  {
    return;
  }

  if (LocUtil::isSamePoint(orig, origCity, *dest, destCity) == false)
  {
    LOG4CXX_INFO(logger, "Not closing the Loop, Not CT journey")
    return;
  }

  puPath.isIntlCTJourneyWithOWPU() = true;
}

//--------------------------------------------------------------------------------
void
PUPathMatrix::setPUGovCarrier(PU& pu)
{

  std::vector<MergedFareMarket*>::iterator it = pu.fareMarket().begin();
  const std::vector<MergedFareMarket*>::iterator itEnd = pu.fareMarket().end();
  const MergedFareMarket& mfm = *(*it);
  if (mfm.governingCarrier().size() > 1)
  {
    pu.puGovCarrier() = PU::UNKNOWN;
    return;
  }

  const CarrierCode& cxr = mfm.governingCarrier().front();

  ++it;
  for (; it != itEnd; ++it)
  {
    const MergedFareMarket& mfm = *(*it);
    if (mfm.governingCarrier().size() > 1)
    {
      pu.puGovCarrier() = PU::UNKNOWN;
      return;
    }
    else if (mfm.governingCarrier().front() != cxr)
    {
      pu.puGovCarrier() = PU::MULTI_CARRIER;
      return;
    }
  }

  if (cxr == SPECIAL_CARRIER_AA)
  {
    pu.puGovCarrier() = PU::ALL_AA_CARRIER;
  }
  else
  {
    pu.puGovCarrier() = PU::SAME_CARRIER;
  }
}

//--------------------------------------------------------------------------------
void
PUPathMatrix::copyPUPath(PUPath& oldPath, PUPath& newPath)
{
  for (const auto oldPu : oldPath.puPath())
  {
    PU* newPU = constructPU();
    copyPU(*oldPu, *newPU);
    newPath.puPath().push_back(newPU);
  }

  newPath.cxrFarePreferred() = oldPath.cxrFarePreferred();
  newPath.fareMarketPath() = oldPath.fareMarketPath();

  // copy all the sideTrips
  if (UNLIKELY(!oldPath.sideTripPUPath().empty()))
  {
    for (const auto& mktSTIt : oldPath.sideTripPUPath())
    {
      std::vector<PUPath*> newPuPathV;
      for (const auto oldPuPath : mktSTIt.second)
      {
        PUPath* pup = constructPUPath();
        copyPUPath(*oldPuPath, *pup);
        newPuPathV.push_back(pup);
      }

      newPath.sideTripPUPath().insert(ST_VALUE_TYPE(mktSTIt.first, newPuPathV));
    }
  }
}

//--------------------------------------------------------------------------------
void
PUPathMatrix::copyPU(PU& oldPU, PU& newPU)
{
  newPU.fareMarket() = oldPU.fareMarket();
  newPU.fareDirectionality() = oldPU.fareDirectionality();
  newPU.puType() = oldPU.puType();
  newPU.puSubType() = oldPU.puSubType();
  newPU.sameNationOJ() = oldPU.sameNationOJ();
  newPU.sameNationOrigSurfaceOJ() = oldPU.sameNationOrigSurfaceOJ();
  newPU.allowNOJInZone210() = oldPU.allowNOJInZone210();
  newPU.ojSurfaceStatus() = oldPU.ojSurfaceStatus();
  newPU.puGovCarrier() = oldPU.puGovCarrier();
  newPU.geoTravelType() = oldPU.geoTravelType();
  newPU.turnAroundPoint() = oldPU.turnAroundPoint();
  newPU.hasSideTrip() = oldPU.hasSideTrip();
  newPU.isCompleteJourney() = oldPU.isCompleteJourney();
  newPU.noPUToEOE() = oldPU.noPUToEOE();
  newPU.fcCount() = oldPU.fcCount();
  newPU.ojLeg1FCCount() = oldPU.ojLeg1FCCount();
  newPU.itinGeoTvlType() = oldPU.itinGeoTvlType();
  newPU.itinWithinScandinavia() = oldPU.itinWithinScandinavia();
  newPU.cxrFarePreferred() = oldPU.cxrFarePreferred();
  newPU.invalidateYYForTOJ() = oldPU.invalidateYYForTOJ();
  newPU.invalidCxrForOJ().insert(newPU.invalidCxrForOJ().end(),
                                 oldPU.invalidCxrForOJ().begin(),
                                 oldPU.invalidCxrForOJ().end());
  newPU.possibleSideTripPU() = oldPU.possibleSideTripPU();
  newPU.inDiffCntrySameSubareaForOOJ() = oldPU.inDiffCntrySameSubareaForOOJ();
  newPU.specialEuropeanDoubleOJ() = oldPU.specialEuropeanDoubleOJ();
  newPU.setSpecialOpenJaw(oldPU.isSpecialOpenJaw());
}

uint32_t
PUPathMatrix::getSurfaceMileage(const Loc& loc1,
                                const Loc& loc2,
                                DateTime travelDate)
{
  GlobalDirection gd;

  std::vector<TravelSeg*> tvlSegs;
  AirSeg tvlSeg;
  tvlSeg.origin() = &loc1;
  tvlSeg.destination() = &loc2;
  tvlSegs.push_back(&tvlSeg);

  GlobalDirectionFinderV2Adapter::getGlobalDirection(_trx, travelDate, tvlSegs, gd);

  std::string str;
  globalDirectionToStr(str, gd);

  return LocUtil::getTPM(loc1, loc2, gd, travelDate, _trx->dataHandle());
}

//--------------------------------------------------------------------------------
bool
PUPathMatrix::isPUPathValid(const FareMarketPath& fmp, PUPath& puPath, const uint16_t totalMktCnt)
{
  if (isOWoutOfIntlRT(fmp, puPath))
  {
    // 2-OW-PU is not allow for Intl/ForeignDomestic RT travel
    // except when whole journey is like A-B-A
    return false;
  }

  const bool useReducedConstructions = _trx->getRequest()->isUseReducedConstructions();
  const int maxFcCount = std::max(static_cast<int>(_itin->itinLegs().size()), 4);
  int fcCount = 0;

  uint16_t mktCount = 0;
  for (const PU* pu : puPath.puPath())
  {
    if (UNLIKELY(useReducedConstructions))
    {
      fcCount += pu->fcCount();
      if (fcCount > maxFcCount)
        return false;

      switch (pu->puType())
      {
      case PricingUnit::Type::OPENJAW:
        if (pu->fcCount() > 2)
          return false;
        break;
      case PricingUnit::Type::CIRCLETRIP:
        if (pu->fcCount() > 3)
          return false;
        break;
      default:
        break;
      }
    }

    mktCount += pu->fareMarket().size();
  }

  return (mktCount == totalMktCnt);
}

//--------------------------------------------------------------------------------
// 2-OW PU is not valid when it is a RT - Intl travel
//--------------------------------------------------------------------------------
bool
PUPathMatrix::isOWoutOfIntlRT(const FareMarketPath& fmp, PUPath& puPath)
{
  if (fmp.fareMarketPath().size() <= 2 && fmp.sideTrips().empty())
  {
    if (puPath.puPath().size() == 2 &&
        twoOWFormRT(*puPath.puPath().front(), *puPath.puPath().back()))
    {
      // A-B-A Journey with 2 OW PU
      puPath.abaTripWithOWPU() = true;
    }

    // if Trip is A-B-A, then we allow 2 OW PU also, PL#12863
    return false;
  }

  std::vector<PU*>::const_iterator i = puPath.puPath().begin();
  const std::vector<PU*>::const_iterator iEnd = puPath.puPath().end();
  for (; i != iEnd; ++i)
  {
    PU& pu1 = *(*i);
    if (owPUtoBlock(pu1) == false)
    {
      continue;
    }

    for (std::vector<PU*>::const_iterator j = i + 1; j != iEnd; ++j)
    {
      PU& pu2 = *(*j);
      if (owPUtoBlock(pu2) == false)
      {
        continue;
      }

      if (twoOWFormRT(pu1, pu2))
      {
        // Intl- RoundTrip travel as 2-OW PU
        return true;
      }
    }
  }
  return false;
}

bool
PUPathMatrix::owPUtoBlock(const PU& pu)
{
  if (pu.puType() != PricingUnit::Type::ONEWAY)
  {
    return false;
  }

  if (pu.geoTravelType() == GeoTravelType::Domestic || pu.geoTravelType() == GeoTravelType::Transborder)
  {
    // for Domestic and Transborder we will try both RT and 2-OW
    return false;
  }

  if ((pu.geoTravelType() == GeoTravelType::ForeignDomestic) &&
      (pu.fareMarket().front()->origin()->area() == IATA_AREA3) &&
      (pu.fareMarket().front()->destination()->area() == IATA_AREA3))
  {
    // For Foreign domestic in Area-3, we will try both RT and 2-OW
    return false;
  }

  return true;
}

bool
PUPathMatrix::twoOWFormRT(const PU& owPU1, const PU& owPU2)
{
  const Loc& pu1Orig = *(owPU1.fareMarket().front()->origin());
  const LocCode& pu1OrigCity = owPU1.fareMarket().front()->boardMultiCity();
  const Loc& pu1Dest = *(owPU1.fareMarket().front()->destination());
  const LocCode& pu1DestCity = owPU1.fareMarket().front()->offMultiCity();

  const Loc& pu2Orig = *(owPU2.fareMarket().front()->origin());
  const LocCode& pu2OrigCity = owPU2.fareMarket().front()->boardMultiCity();
  const Loc& pu2Dest = *(owPU2.fareMarket().front()->destination());
  const LocCode& pu2DestCity = owPU2.fareMarket().front()->offMultiCity();

  return (LocUtil::isSamePoint(pu1Orig, pu1OrigCity, pu2Dest, pu2DestCity) &&
          LocUtil::isSamePoint(pu1Dest, pu1DestCity, pu2Orig, pu2OrigCity));
}

bool
PUPathMatrix::genMktPUPathCombination(std::vector<PUPath*>& puPathVect,
                                      const uint16_t stIdx,
                                      const uint16_t stCount,
                                      std::vector<std::vector<PUPath*> >& mktPUPathVect,
                                      std::vector<std::vector<PUPath*> >& mktPUPathCombVect)
{

  if (stIdx >= stCount) // idx start from 0
  {
    mktPUPathCombVect.push_back(puPathVect);
    return true;
  }

  std::vector<PUPath*>& vect = mktPUPathVect[stIdx];
  uint16_t sz = vect.size();
  for (uint16_t i = 0; i < sz - 1; ++i)
  {
    std::vector<PUPath*> newPathVect;
    if (!puPathVect.empty())
      newPathVect.insert(newPathVect.end(), puPathVect.begin(), puPathVect.end());
    PUPath* path = vect[i];
    newPathVect.push_back(path);
    genMktPUPathCombination(newPathVect, stIdx + 1, stCount, mktPUPathVect, mktPUPathCombVect);
  }
  if (sz > 0)
  {
    PUPath* path = vect[sz - 1];
    puPathVect.push_back(path);
  }

  genMktPUPathCombination(puPathVect, stIdx + 1, stCount, mktPUPathVect, mktPUPathCombVect);

  return true;
}

bool
PUPathMatrix::genPathPUPathCombination(
    std::map<MergedFareMarket*, std::vector<PUPath*> >& puPathComb,
    std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >::iterator it,
    std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >& pathPUPathAll,
    std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > >& pathPUPathCombVect)
{

  if (it == pathPUPathAll.end())
  {
    pathPUPathCombVect.push_back(puPathComb);
    return true;
  }

  std::vector<std::vector<PUPath*> >::iterator pupvIt = it->second.begin();
  const std::vector<std::vector<PUPath*> >::iterator pupvItLast = it->second.end() - 1;
  const std::vector<std::vector<PUPath*> >::iterator pupvItEnd = it->second.end();
  for (; pupvIt != pupvItLast && pupvIt != pupvItEnd; ++pupvIt)
  {
    std::map<MergedFareMarket*, std::vector<PUPath*> > newPUPathComb(puPathComb.begin(),
                                                                     puPathComb.end());
    newPUPathComb.insert(ST_VALUE_TYPE(it->first, *pupvIt));

    std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >::iterator nextIt = it;
    ++nextIt;

    genPathPUPathCombination(newPUPathComb, nextIt, pathPUPathAll, pathPUPathCombVect);
  }
  if (it != pathPUPathAll.end() && !it->second.empty())
  {
    puPathComb.insert(ST_VALUE_TYPE(it->first, *pupvIt));
    std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >::iterator nextIt = it;
    ++nextIt;
    genPathPUPathCombination(puPathComb, nextIt, pathPUPathAll, pathPUPathCombVect);
  }

  return true;
}

//--------------------------------------------------------------------------------

void
PUPathMatrix::addPUPathsToMainMatrix(std::vector<PUPath*>& combinedPUPaths)
{
  boost::lock_guard<boost::mutex> guard(_puPathMatrixMutex);

  _puPathMatrix.insert(_puPathMatrix.end(), combinedPUPaths.begin(), combinedPUPaths.end());
}

void
PUPathMatrix::PUPathBuildTask::combineSTPUpath(
    std::vector<PUPath*>& mainPathMatrix,
    std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > >& pathPUPathCombVect,
    std::vector<PUPath*>& combinedPUPaths)
{
  if (mainPathMatrix.empty())
    return;

  if (pathPUPathCombVect.empty()) // no side trip to combine with the main trip
    combinedPUPaths = mainPathMatrix;
  else
  {
    for (const auto& stMap : pathPUPathCombVect)
    {
      for (const auto oldPath : mainPathMatrix)
      {
        PUPath* puPath = _puPathMatrix->constructPUPath();
        _puPathMatrix->copyPUPath(*oldPath, *puPath);

        puPath->sideTripPUPath() = stMap; // assign SideTrip

        // Following method is For Debug Only
        // debugOnlyDisplayPUPath(*puPath);

        combinedPUPaths.push_back(puPath);
      }
    }
  }
}
//--------------------------------------------------------------------------------

bool
PUPathMatrix::isBuildPUPathComplete(const std::vector<PUPath*>& puPathVect)
{
  return (puPathVect.size() >= _maxPUPathPerFMP);
}

//--------------------------------------------------------------------------------

void
PUPathMatrix::limitFareMarketPath(std::vector<FareMarketPath*>& matrix)
{
  if (matrix.empty() || (matrix.size() <= FIVE_K))
    return;

  LOG4CXX_DEBUG(logger, " init _fareMarketPathCount= " << matrix.size())

  std::multimap<uint16_t, FareMarketPath*> fareBreakFMP;

  std::set<uint16_t> fbSet;
  for (const auto fmp : matrix)
  {
    uint16_t fmCount = fareBreakCount(*fmp);
    fbSet.insert(fmCount);
    fareBreakFMP.insert(std::multimap<uint16_t, FareMarketPath*>::value_type(fmCount, fmp));
  }
  const uint16_t average = FIVE_K / fbSet.size();

  LOG4CXX_DEBUG(logger, " fareBreakFMP.size=" << fareBreakFMP.size())
  LOG4CXX_DEBUG(logger, " fbSet.size=" << fbSet.size())
  LOG4CXX_DEBUG(logger, " average=" << average)

  matrix.clear();
  for (const auto fbCount : fbSet)
  {
    std::pair<std::multimap<uint16_t, FareMarketPath*>::iterator,
              std::multimap<uint16_t, FareMarketPath*>::iterator> p =
        fareBreakFMP.equal_range(fbCount);
    std::multimap<uint16_t, FareMarketPath*>::iterator it = p.first;
    uint16_t j = 0;
    for (; it != p.second; ++it)
    {
      matrix.push_back(it->second);
      if (++j >= average)
        break;
    }
    LOG4CXX_INFO(logger, " j=" << j << " fbCount=" << fbCount)
  }
  matrix.shrink_to_fit();

  LOG4CXX_INFO(logger, "New _fareMarketPathCount= " << matrix.size())
}

uint16_t
PUPathMatrix::fareBreakCount(FareMarketPath& fmp)
{
  uint16_t fmCount = fmp.fareMarketPath().size();

  for (const auto& sideTrip : fmp.sideTrips())
    for (const auto pathIt : sideTrip.second)
      fmCount += pathIt->fareMarketPath().size();

  return fmCount;
}

//--------------------------------------------------------------------------------
bool
PUPathMatrix::isInboundToCountry(const Loc& puOrig, const Loc& fmOrig, const Loc& fmDest)
{
  if (_itin->geoTravelType() != GeoTravelType::International && _itin->geoTravelType() != GeoTravelType::Transborder)
  {
    return false;
  }

  if (LocUtil::isWithinSameCountry(
          _itin->geoTravelType(), _travelWithinScandinavia, puOrig, fmOrig))
  {
    // fm started from the country of puOrig, Can not be InBound
    return false;
  }

  if (LocUtil::isWithinSameCountry(
          _itin->geoTravelType(), _travelWithinScandinavia, puOrig, fmDest))
  {
    // fm returning to the country of puOrig, It is a InBound FM
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------
bool
PUPathMatrix::isOutBoundFromCountry(const Loc& puOrig, const Loc& fmOrig, const Loc& fmDest)
{
  if (UNLIKELY(_itin->geoTravelType() != GeoTravelType::International && _itin->geoTravelType() != GeoTravelType::Transborder))
  {
    return false;
  }

  if (LocUtil::isWithinSameCountry(
          _itin->geoTravelType(), _travelWithinScandinavia, puOrig, fmOrig))
  {
    // fm started from the country of puOrig
    if (!LocUtil::isWithinSameCountry(
            _itin->geoTravelType(), _travelWithinScandinavia, fmOrig, fmDest))
    {
      // fm dest is out side of the country
      return true;
    }
  }
  return false;
}

//--------------------------------------------------------------------------------

bool
PUPathMatrix::isTravelWithinScandinavia()
{
  for (const auto travelSeg : _itin->travelSeg())
  {
    const Loc* loc = travelSeg->origin();
    if (!LocUtil::isScandinavia(*loc))
      return false;

    loc = travelSeg->destination();
    if (!LocUtil::isScandinavia(*loc))
      return false;
  }

  return true;
}

void
PUPathMatrix::determineItinTravelBoundary()
{
  bool area1 = false;
  bool area2 = false;
  bool area3 = false;

  for (const auto travelSeg : _itin->travelSeg())
  {
    const Loc& orig = *travelSeg->origin();
    const Loc& dest = *travelSeg->destination();
    if (orig.area() == IATA_AREA1 || dest.area() == IATA_AREA1)
    {
      area1 = true;
    }
    if (orig.area() == IATA_AREA2 || dest.area() == IATA_AREA2)
    {
      area2 = true;
    }
    if (orig.area() == IATA_AREA3 || dest.area() == IATA_AREA3)
    {
      area3 = true;
    }
  }

  if (area1 && area2 && area3)
  {
    _itinBoundary = ALL_IATA;
    return;
  }
  else if ((area1 && area2) || (area1 && area3) || (area2 && area3))
  {
    _itinBoundary = TWO_IATA;
  }
  else
  {
    _itinBoundary = ONE_IATA;
  }

  TSE_ASSERT(!_itin->travelSeg().empty());

  const IATASubAreaCode& subArea = _itin->travelSeg().front()->origin()->subarea();

  const bool sameSubArea = std::all_of(_itin->travelSeg().cbegin(),
                                       _itin->travelSeg().cend(),
                                       [&subArea](const TravelSeg* const travelSeg)
                                       { return subArea == travelSeg->destination()->subarea(); });

  if (sameSubArea)
  {
    _itinBoundary = ONE_SUB_IATA;
    if (subArea == IATA_SUB_AREA_21())
    {
      _travelWithinScandinavia = isTravelWithinScandinavia();
    }
  }
}

bool
PUPathMatrix::isInboundToNationZone210(const Loc& origin, const MergedFareMarket& fm)
{
  //-- INBOUND when returning to Europe (Zone 210)  from diff sub area
  //
  if (origin.nation() != fm.origin()->nation()) // diff country
  {
    const static Zone zone("210");
    const tse::DateTime& ticketingDate = _trx->getRequest()->ticketingDT();

    if (LocUtil::isInLoc(origin,
                         LOCTYPE_ZONE,
                         zone,
                         Vendor::ATPCO,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         ticketingDate) &&
        !LocUtil::isInLoc(*fm.origin(),
                          LOCTYPE_ZONE,
                          zone,
                          Vendor::ATPCO,
                          RESERVED,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          ticketingDate) &&
        LocUtil::isInLoc(*fm.destination(),
                         LOCTYPE_ZONE,
                         zone,
                         Vendor::ATPCO,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         ticketingDate))
    {
      // Returning to Europe 210: INBOUND
      return true;
    }
  }

  return false;
}

bool
PUPathMatrix::isInboundToNetherlandAntilles(const Loc& origin, const MergedFareMarket& fm)
{
  if (origin.nation() != fm.origin()->nation()) // diff country
  {
    if (LocUtil::oneOfNetherlandAntilles(origin) &&
        !LocUtil::oneOfNetherlandAntilles(*fm.origin()) &&
        LocUtil::oneOfNetherlandAntilles(*fm.destination()))
    {
      // std::cerr << __FUNCTION__ << std::endl;
      return true;
    }
  }
  return false;
}

bool
PUPathMatrix::determineGeoTravelType(const MergedFareMarket& fm, GeoTravelType& geoTvlType)
{
  //---------  Determine GeoTravelType -------------------
  if (fm.geoTravelType() == geoTvlType)
    return true;

  if (geoTvlType == GeoTravelType::International)
    return true;

  if (fm.geoTravelType() == GeoTravelType::International)
  {
    geoTvlType = GeoTravelType::International;
  }
  else if (fm.geoTravelType() == GeoTravelType::ForeignDomestic)
  {
    geoTvlType = GeoTravelType::ForeignDomestic;
  }
  else if ((fm.geoTravelType() == GeoTravelType::Transborder) && (geoTvlType != GeoTravelType::ForeignDomestic))
  {
    geoTvlType = GeoTravelType::Transborder;
  }

  return true;
}

//------------------------------------------------------------------------
// Destination of the faremarket in consideration can not return to any of the
// origin/destination of the OpenJaw Leg
//-------------------------------------------------------------------------
bool
PUPathMatrix::fmReturnToOJLeg(const MergedFareMarket& fm,
                              const std::vector<MergedFareMarket*>& ojLeg)
{

  if (UNLIKELY(ojLeg.empty()))
    return false;

  const Loc& fmDest = *fm.destination();
  const LocCode& fmDestCity = fm.travelSeg().back()->offMultiCity();

  for (const auto it : ojLeg)
  {
    const bool origMatch = LocUtil::isSamePoint(
        fmDest, fmDestCity, *it->origin(), it->travelSeg().front()->boardMultiCity());
    const bool destMatch = LocUtil::isSamePoint(
        fmDest, fmDestCity, *it->destination(), it->travelSeg().back()->offMultiCity());

    if (origMatch || destMatch)
      return true;
  }
  return false;
}

//------------------------------------------------------------------------
// Destination of the faremarket in consideration can not fall in any of the
// origin/destination of the OutBound OpenJaw Leg
//-------------------------------------------------------------------------
bool
PUPathMatrix::intersectOJLegs(const std::vector<MergedFareMarket*>& obLeg,
                              const std::vector<MergedFareMarket*>& ibLeg)
{
  std::vector<MergedFareMarket*>::const_iterator ibIt = ibLeg.begin();
  const std::vector<MergedFareMarket*>::const_iterator ibItEnd = ibLeg.end();

  bool firstEndpoint = true;
  bool lastEndpoint = false;
  for (; ibIt != ibItEnd; ++ibIt)
  {
    MergedFareMarket& mfm = *(*ibIt);

    // last parameter to mean the end point
    if (isInLeg(*mfm.origin(), mfm.boardMultiCity(), obLeg, firstEndpoint))
      return true;

    firstEndpoint = false;

    if (ibIt == ibItEnd - 1)
      lastEndpoint = true;

    if (isInLeg(*mfm.destination(), mfm.offMultiCity(), obLeg, lastEndpoint))
      return true;
  }

  return false;
}

bool
PUPathMatrix::isInLeg(const Loc& loc,
                      const LocCode& city,
                      const std::vector<MergedFareMarket*>& ojLeg,
                      const bool endPoint)
{
  // If loc/city is a endPoint of a leg which can match the endPoint of other
  // leg, therefore do not try this endPoint with the other endPoint
  // of the leg

  std::vector<MergedFareMarket*>::const_iterator it = ojLeg.begin();
  const std::vector<MergedFareMarket*>::const_iterator itEnd = ojLeg.end();

  bool first = true;
  bool last = false;
  for (; it != itEnd; ++it)
  {
    MergedFareMarket& mfm = *(*it);

    if (!first || !endPoint)
    {
      if (LocUtil::isSamePoint(loc, city, *mfm.origin(), mfm.boardMultiCity()))
        return true;
    }

    first = false;

    if (endPoint && it == itEnd - 1)
      last = true;

    if (!last || !endPoint)
    {
      if (LocUtil::isSamePoint(loc, city, *mfm.destination(), mfm.offMultiCity()))
        return true;
    }
  }

  return false;
}

void
PUPathMatrix::buildCTPU(PU& ctPU,
                        const uint16_t nextIdx,
                        const uint16_t totalMktCnt,
                        PUPath& puPath,
                        std::vector<MergedFareMarket*>& fmPath,
                        bool passedCTProvision,
                        bool& ctFound,
                        bool& done)
{
  if (UNLIKELY(done))
  {
    // To trim Big PNR
    return;
  }

  if (UNLIKELY(ctFound))
    return;

  if (nextIdx >= totalMktCnt)
  {
    return;
  }

  PU* pu = constructPU();
  pu->puType() = PricingUnit::Type::CIRCLETRIP;
  pu->geoTravelType() = ctPU.geoTravelType();
  pu->cxrFarePreferred() = ctPU.cxrFarePreferred();
  pu->fareMarket().insert(
      pu->fareMarket().end(), ctPU.fareMarket().begin(), ctPU.fareMarket().end());
  pu->fareDirectionality().insert(pu->fareDirectionality().end(),
                                  ctPU.fareDirectionality().begin(),
                                  ctPU.fareDirectionality().end());
  bool found = false;

  // lint -e{413}
  buildCTPU(*pu, (nextIdx + 1), totalMktCnt, puPath, fmPath, passedCTProvision, found, done);

  if (found)
  {
    // CT complete
    ctFound = true;
    return;
  }

  MergedFareMarket* fm = fmPath[nextIdx];
  if (puPath.isMarketAssigned(fm))
  {
    return;
  }

  Directionality dir = FROM;
  bool closed = false;
  if (!isValidFCforCT(ctPU, ctPU.fareMarket(), *fm, dir, passedCTProvision, closed))
  {
    return;
  }

  ctPU.fareMarket().push_back(fm);
  ctPU.cxrFarePreferred() = (ctPU.cxrFarePreferred() || fm->cxrFarePreferred());
  ctPU.fareDirectionality().push_back(dir);
  if (dir == TO && ctPU.turnAroundPoint() == nullptr)
  {
    //---- set turnAroundPoint ----
    // 1st TvlSeg of the 1st INBOUND fare market is the turnAroundPoint
    ctPU.turnAroundPoint() = fm->travelSeg().front();
  }

  //---------  Determine GeoTravelType -------------------
  determineGeoTravelType(*fm, ctPU.geoTravelType());

  if (closed)
  {
    // Circle is Complete, Now we can insert into ctPU
    //
    if (isValidCT(ctPU, passedCTProvision))
    {
      if (!checkTag2FareIndicator(ctPU))
      {
        // no Tag2Fare Exist in any of the fareMarket. Restriction only for US/CA
        // LOG4CXX_DEBUG(logger, "InValid CT: tag2FareIndicator")

        return;
      }
      if (ctPU.puType() == PricingUnit::Type::CIRCLETRIP && ctPU.geoTravelType() == GeoTravelType::International &&
          (ctPU.fareMarket().back()->geoTravelType() == GeoTravelType::ForeignDomestic ||
           ctPU.fareMarket().back()->geoTravelType() == GeoTravelType::Domestic ||
           ctPU.fareMarket().back()->geoTravelType() == GeoTravelType::Transborder))
      {
        ctPU.fareDirectionality().back() = FROM;
      }
      ctPU.fcCount() = ctPU.fareMarket().size();
      ctFound = true;
      puPath.puPath().push_back(&ctPU);
      puPath.cxrFarePreferred() = (puPath.cxrFarePreferred() || ctPU.cxrFarePreferred());
      if (!fallback::apo36040Cat6TSI5Check(_trx))
      {
         //APO36040:For ct itins,set pu turnaround to furthest farebreak.
         setCTPUTurnAroundPoint(ctPU);
      }
      return;
    }
    return;
  }
  buildCTPU(ctPU, nextIdx + 1, totalMktCnt, puPath, fmPath, passedCTProvision, ctFound, done);
}

PU&
PUPathMatrix::buildRW(MergedFareMarket& mfm)
{
  PU* rwPU = constructPU();
  rwPU->puType() = _trx->itin().front()->tripCharacteristics().isSet(Itin::RW_SFC)
                       ? PricingUnit::Type::ROUNDTHEWORLD_SFC
                       : PricingUnit::Type::CIRCLETRIP_SFC;
  rwPU->geoTravelType() = mfm.geoTravelType();
  rwPU->fareMarket().push_back(&mfm);
  rwPU->fareDirectionality().push_back(FROM);
  rwPU->cxrFarePreferred() = mfm.cxrFarePreferred();
  rwPU->turnAroundPoint() = ItinUtil::getFurthestPoint(_trx->itin().front());
  rwPU->fcCount() = 1;

  return *rwPU;
}

void
PUPathMatrix::createMainTripSideTripLink(PUPath& puPath)
{
  std::vector<PU*>& allPU = puPath.allPU();

  allPU.insert(allPU.end(), puPath.puPath().begin(), puPath.puPath().end());
  LOG4CXX_INFO(logger, "Main Trip PU count= " << allPU.size());

  uint16_t totalPU = allPU.size();

  std::map<MergedFareMarket*, std::vector<PUPath*> >& stPUPathMap = puPath.sideTripPUPath();
  if (stPUPathMap.empty())
  {
    if (totalPU > 1)
      puPath.eoePUAvailable().insert(puPath.eoePUAvailable().end(), totalPU, true);

    return;
  }

  puPath.eoePUAvailable().insert(
      puPath.eoePUAvailable().end(), totalPU, totalPU > 1 ? true : false);

  // for each fu of each pu in the main trip find side trip pu vector
  // this side trip pu vector is vector of index of pu in  allPU vector
  //
  std::vector<PU*>::iterator puIt = puPath.puPath().begin();
  const std::vector<PU*>::iterator puItEnd = puPath.puPath().end();
  for (uint16_t puIdx = 0; puIt != puItEnd; ++puIt, ++puIdx)
  {
    // key is the index of fu in the PU
    PUPath::FUToPULinkMap fuToPUlinkMap;

    std::vector<MergedFareMarket*>::iterator fmIt = (*puIt)->fareMarket().begin();
    const std::vector<MergedFareMarket*>::iterator fmItEnd = (*puIt)->fareMarket().end();
    for (uint16_t fuIdx = 0; fmIt != fmItEnd; ++fmIt, ++fuIdx)
    {
      std::map<MergedFareMarket*, std::vector<PUPath*> >::iterator it = stPUPathMap.find(*fmIt);

      if (it != stPUPathMap.end()) // side trip exists for this fareMarket?
      {
        std::vector<std::vector<uint8_t> > fuStPuIdxVect; // up to 2 ST per FU

        for (const PUPath* path : it->second)
        {
          const bool eoeAvailable = path->puPath().size() > 1 ? true : false;
          std::vector<uint8_t> puIdxVect; // puIdx of one ST

          for (PU* stPU : path->puPath())
          {
            stPU->possibleSideTripPU() = true;
            allPU.push_back(stPU);
            puPath.eoePUAvailable().push_back(eoeAvailable); // for ST PU
            LOG4CXX_INFO(logger,
                         "ST PU IDX= " << (int)totalPU << " eoeAvailable=" << eoeAvailable);
            puIdxVect.push_back(totalPU++);
          }

          fuStPuIdxVect.push_back(puIdxVect);
        }

        fuToPUlinkMap[fuIdx] = fuStPuIdxVect;
        (*puIt)->hasSideTrip() = true;
      }
    }

    if (!fuToPUlinkMap.empty())
    {
      _itin->setHasSideTrip(true);
      puPath.mainTripSideTripLink().insert(
          PUPath::MainTripSideTripLink::value_type(puIdx, fuToPUlinkMap));
    }
  }
}

void
PUPathMatrix::PUPathBuildTask::performTask()
{
  try
  {
    //--------- Build Main Trip ----------------------------
    std::vector<PUPath*> mainPathMatrix;

    std::vector<MergedFareMarket*>& fmPath = _fareMarketPath->fareMarketPath();
    const uint16_t totalMktCnt = fmPath.size();

    PUPath* puPath = _puPathMatrix->constructPUPath();

    const uint16_t mktIdx = 0;
    bool done = false;
    _puPathMatrix->buildPUPath(
        *_fareMarketPath, mktIdx, totalMktCnt, *puPath, fmPath, mainPathMatrix, done);

    //----------------- Build SideTrip ----------------------------
    // one FareMarket has ==> vector or ST, which is vect of TravelSeg*
    // one ST gives ==> vector of FareMarketPath
    // one FareMarketPath gives ==> vector of PUPath

    std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > > pathPUPathCombVect;

    bool stFlag = true;
    if (!_fareMarketPath->sideTrips().empty())
      stFlag = _puPathMatrix->buildSideTripPUPath(*_fareMarketPath, pathPUPathCombVect);

    LOG4CXX_INFO(logger, "mainPathMatrix size = " << mainPathMatrix.size());
    LOG4CXX_INFO(logger, "pathPUPathCombVect size = " << pathPUPathCombVect.size())
    LOG4CXX_DEBUG(
        logger,
        "Before combineSTPUpath _puPathMatrix size = " << _puPathMatrix->puPathMatrix().size())

    //--- Generate combinations of MainTrip and SideTrips --------//
    if (LIKELY(stFlag))
    {
      // At this point SideTrip PUPath (if there is any)  building succeeded.
      // When SideTrip building fails, we discard the main trip also
      combineSTPUpath(mainPathMatrix, pathPUPathCombVect, _fareMarketPath->puPaths());
      _puPathMatrix->addPUPathsToMainMatrix(_fareMarketPath->puPaths());
    }

    LOG4CXX_DEBUG(
        logger,
        "After combineSTPUpath _puPathMatrix size = " << _puPathMatrix->puPathMatrix().size())
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger, "Exception:" << ex.message() << " - Build PUPath failed");
    throw;
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(logger, "Exception:" << e.what() << " - Build PUPath failed");
    throw;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "UNKNOWN Exception Caught: Build PUPath failed");
    throw;
  }
}

bool
PUPathMatrix::checkTag2FareIndicator(const PU& pu)
{

  if (pu.geoTravelType() != GeoTravelType::Domestic && pu.geoTravelType() != GeoTravelType::Transborder)
  {
    // for Intl and F. Domestic we don't need at least 1 Tag-2 fare as must
    return true;
  }

  if (checkTag2FareIndicator(pu.fareMarket()))
    return true;

  return false;
}

bool
PUPathMatrix::checkTag2FareIndicator(const std::vector<MergedFareMarket*>& fareMarketVect)
{
  return std::any_of(fareMarketVect.cbegin(),
                     fareMarketVect.cend(),
                     [](const MergedFareMarket* const mfm)
                     { return mfm->tag2FareIndicator() == MergedFareMarket::Tag2FareIndicator::Present; });
}

void
PUPathMatrix::markIntlOWfromBrokenOJ(PU* oj,
                                     const MergedFareMarket* fm1,
                                     const MergedFareMarket* fm2) const
{
  const LocCode& board1(fm1->boardMultiCity());
  const LocCode& off1(fm1->offMultiCity());
  const LocCode& board2(fm2->boardMultiCity());
  const LocCode& off2(fm2->offMultiCity());

  for (const PUPath* puPath : _puPathMatrix)
  {
    for (PU* pu : puPath->puPath())
    {
      if (PricingUnit::Type::ONEWAY == pu->puType() && GeoTravelType::International == pu->geoTravelType())
      {
        const MergedFareMarket* fm(pu->fareMarket().front());
        if ((board1 == fm->boardMultiCity() && off1 == fm->offMultiCity()) ||
            (board2 == fm->boardMultiCity() && off2 == fm->offMultiCity()))
        {
          // std::cerr << __FUNCTION__ << std::endl;
          pu->intlOJToOW().insert(oj);
        }
      }
    }
  }
}

bool
PUPathMatrix::isSpecialEuropeanDoubleOJ(const Loc& obOrig,
                                        const Loc& obDest,
                                        const Loc& ibOrig,
                                        const Loc& ibDest)
{
  // both (originsurface and destinationsurface) should be in the different country
  if (LocUtil::isWithinSameCountry(
          _itin->geoTravelType(), _travelWithinScandinavia, obDest, ibOrig) ||
      LocUtil::isWithinSameCountry(
          _itin->geoTravelType(), _travelWithinScandinavia, obOrig, ibDest))
    return false; // this is not SpecialEuropeanDoubleOJ

  // check  (A - B /- A - C or A - B /- C - B)

  const static Zone zone("210");
  const tse::DateTime& ticketingDate = _trx->getRequest()->ticketingDT();
  if (!LocUtil::isInLoc(obOrig,
                        LOCTYPE_ZONE,
                        zone,
                        Vendor::ATPCO,
                        RESERVED,
                        LocUtil::OTHER,
                        GeoTravelType::International,
                        EMPTY_STRING(),
                        ticketingDate) ||
      !LocUtil::isInLoc(obDest,
                        LOCTYPE_ZONE,
                        zone,
                        Vendor::ATPCO,
                        RESERVED,
                        LocUtil::OTHER,
                        GeoTravelType::International,
                        EMPTY_STRING(),
                        ticketingDate) ||
      !LocUtil::isInLoc(ibOrig,
                        LOCTYPE_ZONE,
                        zone,
                        Vendor::ATPCO,
                        RESERVED,
                        LocUtil::OTHER,
                        GeoTravelType::International,
                        EMPTY_STRING(),
                        ticketingDate) ||
      !LocUtil::isInLoc(ibDest,
                        LOCTYPE_ZONE,
                        zone,
                        Vendor::ATPCO,
                        RESERVED,
                        LocUtil::OTHER,
                        GeoTravelType::International,
                        EMPTY_STRING(),
                        ticketingDate))
    return false; // this is not SpecialEuropeanDoubleOJ

  return true;
}

//------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
bool
PUPathMatrix::isSameAnyOneXPoint(const MergedFareMarket& obOrigFM,
                                 const MergedFareMarket& ibDestFM,
                                 const MergedFareMarket& obDestFM,
                                 const MergedFareMarket& ibOrigFM)
{
  // check (A - B /- A - C or . A - B /- C - B) --> pass
  bool match1 = (obOrigFM.boardMultiCity() == ibOrigFM.boardMultiCity());
  bool match2 = (obDestFM.offMultiCity() == ibDestFM.offMultiCity());
  if ((match1 && match2) || (!match1 && !match2))
    return false;
  return true;
}

//APO36040:Set turnaround point on ct pu to furthest fare break point
void
PUPathMatrix::setCTPUTurnAroundPoint(PU& ctPU)
{
   TravelSeg* furthestTvlSeg = 0;
   uint32_t highestGcmInPu = 0;
   //compute furthest fare break point from pu origin
   const Loc*&  puOrigin = ctPU.fareMarket().front()->origin();
   for (const MergedFareMarket* mfmP : ctPU.fareMarket())
   {
      const Loc*&  fmOrigin = mfmP->travelSeg().front()->origin();
      const uint32_t gcm = TseUtil::greatCircleMiles(*puOrigin, *fmOrigin);
      if (gcm > highestGcmInPu)
      {
          highestGcmInPu = gcm;
          furthestTvlSeg = mfmP->travelSeg().front() ;
      }
   }
   if (furthestTvlSeg)
   {
       ctPU.turnAroundPoint() = furthestTvlSeg;
       LOG4CXX_INFO(logger, "CircleTripPU turnaround point: " << furthestTvlSeg->origin()->city() );
   }
   return;
}

}
