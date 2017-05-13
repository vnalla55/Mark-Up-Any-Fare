//-------------------------------------------------------------------
// File:    FareMarketPathMatrix.cpp
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

#include "Pricing/FareMarketPathMatrix.h"

#include "Common/ErrorResponseException.h"
#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IbfAvailabilityTools.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag600Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Util/Algorithm/Container.h"

#include <iostream>
#include <ostream>

namespace tse
{

log4cxx::LoggerPtr
FareMarketPathMatrix::_logger(log4cxx::Logger::getLogger("atseintl.Pricing.FareMarketPathMatrix"));

FALLBACK_DECL(simpleFareMarketPaths);
FALLBACK_DECL(throughFarePrecedenceStopoverFix);

// FOR DEBUG ONLY
// void
// forDebugOnlyDisplayItin(tse::Itin& itin);
// void
// forDebugOnlyDisplayFMP(std::vector<FareMarketPath*>& fmpv);

FareMarketPath*
FareMarketPathMatrix::constructFareMarketPath()
{
  FareMarketPath* newPath;
  _dataHandle.get(newPath);

  return newPath;
}

static void
removeComplexFareMarketPaths(const PricingTrx& trx,
                             const Itin& itin,
                             std::vector<FareMarketPath*>& fmpv)
{
  if (!trx.isMip() || trx.getRequest()->getMaxFCsPerLeg() == 0 || !trx.getRequest()->isSimpleShoppingRQ())
    return;

  const size_t maxFc = std::max(size_t(3), trx.getRequest()->getMaxFCsPerLeg() * ShoppingUtil::getNumLegs(itin));

  alg::erase_if(fmpv,
                [=](FareMarketPath* fmp)
                { return fmp->fareMarketPath().size() > maxFc; });
}

bool
FareMarketPathMatrix::buildAllFareMarketPath()
{
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&_trx);

  if (_itin.fareMarket().empty())
  {
    LOG4CXX_WARN(_logger, "NO FareMarket in the Itin");
    return false;
  }

  if (_mergedFareMarketVect.empty())
  {
    if (!(noPNRTrx != nullptr && noPNRTrx->isNoMatch()))
      LOG4CXX_WARN(_logger, "NO MergedFareMarket in the _mergedFareMarketVect");

    collectDiagnostic(&_itin);

    //normal soldout calculations for itin/legs are done for _fareMarketPathMatrix
    //in this case this matrix is empty (no availablily = no possible fare paths)
    //so we need to calculate it for all possible fare paths.
    if ((_trx.getRequest()->isBrandedFaresRequest()) && (_trx.getTrxType() == PricingTrx::MIP_TRX))
    {
      IbfAvailabilityTools::updateIbfAvailabilitySoldoutsForItin(&_itin, _brandCode);
    }

    return false;
  }

  // forDebugOnlyDisplayItin(_itin);

  // 3rd param false, doing MainTrip
  buildFareMarketPath(_itin.travelSeg(), _fareMarketPathMatrix, false);

  if (!fallback::simpleFareMarketPaths(&_trx))
    removeComplexFareMarketPaths(_trx, _itin, _fareMarketPathMatrix);

  buildSideTrip();

  markUsedMergedFareMarket();

  if (_trx.paxType().size() > 1)
  {
    // For Multi-Pax Tuning we may need to do Same-FareBreak matching
    // Matching with FareMarketPath pointer will be easier
    // Therefore, save the objects until the end of the trx
    _trx.dataHandle().import(_dataHandle);
  }

  collectDiagnostic(&_itin);

  if ((_trx.getRequest()->isBrandedFaresRequest()) && (_trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    if (!_fareMarketPathMatrix.empty()) {
      std::map<LegId, IbfErrorMessage> finalPerLeg;
      const IbfErrorMessage itinStatus =
          IbfAvailabilityTools::calculateAllStatusesForMatrix(finalPerLeg, _fareMarketPathMatrix);
      _itin.getMutableIbfAvailabilityTracker().setStatus(_brandCode, itinStatus);

      for (const auto& legErrorElement : finalPerLeg)
      {
        _itin.getMutableIbfAvailabilityTracker().setStatusForLeg(
            _brandCode, legErrorElement.first, legErrorElement.second);
      }

      removeInvalidFareMarketPaths();
    }
    else
    {
      //normal soldout calculations for itin/legs are done for _fareMarketPathMatrix
      //in this case this matrix is empty (no availablily = no possible fare paths)
      //so we need to calculate it for all possible fare paths.
      IbfAvailabilityTools::updateIbfAvailabilitySoldoutsForItin(&_itin, _brandCode);
    }
  }

  for (FareMarketPath* fmp : _fareMarketPathMatrix)
  {
    if (_trx.delayXpn())
      fmp->calculateFirstFareAmt(_trx.paxType());

    if (_itin.isThroughFarePrecedence())
    {
      // Temporary condition until project activation.
      if (!fallback::throughFarePrecedenceStopoverFix(&_trx))
        fmp->calculateThroughFarePrecedenceRank();
      else
        fmp->calculateThroughFarePrecedenceRankOld();
    }
  }

  return true;
}

void
FareMarketPathMatrix::removeInvalidFareMarketPaths()
{
  std::vector<FareMarketPath*> tmpFareMarketPaths;

  for (FareMarketPath* fmp : _fareMarketPathMatrix)
  if (fmp->isValidForIbf())
    tmpFareMarketPaths.push_back(fmp);

  _fareMarketPathMatrix = std::move(tmpFareMarketPaths);
}

//---------------------------------------------------------------------
// build FareMarket Paths for the travelSegment vector
//

bool
FareMarketPathMatrix::buildFareMarketPath(std::vector<TravelSeg*>& travelSegs,
                                          std::vector<FareMarketPath*>& targetMatrix,
                                          const bool processSideTrip)
{
  uint16_t pathStartSN = _itin.segmentOrder(travelSegs.front());
  uint16_t pathEndSN = _itin.segmentOrder(travelSegs.back());
  uint16_t curSegNum = pathStartSN;

  // LOG4CXX_DEBUG(_logger, "buildFareMarketPath: pathStartSN="<< pathStartSN
  //                                         <<", pathEndSN="<< pathEndSN
  //                                         <<", curSegNum="<< curSegNum);

  for (const auto mergedFareMarket : _mergedFareMarketVect)
  {
    if (processSideTrip && !mergedFareMarket->sideTripTravelSeg().empty())
    {
      // don't allow SideTrip out of SideTrip
      continue;
    }

    uint16_t fmktSN = mergedFareMarket->getStartSegNum();

    if (fmktSN == pathStartSN ||
        (fmktSN == pathStartSN + 1 && dynamic_cast<ArunkSeg*>(travelSegs.front())))
    {
      FareMarketPath* newPath = constructFareMarketPath();
      newPath->fareMarketPath().push_back(mergedFareMarket);
      if(_trx.getRequest()->isBrandedFaresRequest())
          newPath->setBrandCode(_brandCode);

      buildPath(travelSegs,
                _mergedFareMarketVect,
                curSegNum + 1,
                pathStartSN,
                pathEndSN,
                *newPath,
                targetMatrix,
                processSideTrip);
    }
  }

  return true;
}


//---------------------------------------------------------------------
// Recursive Algorithm
//---------------------------------------------------------------------
void
FareMarketPathMatrix::buildPath(std::vector<TravelSeg*>& travelSegs,
                                std::vector<MergedFareMarket*>& fareMarketVect,
                                const uint16_t curSegNum,
                                const uint16_t pathStartSN,
                                const uint16_t pathEndSN,
                                FareMarketPath& fmktPath,
                                std::vector<FareMarketPath*>& targetMatrix,
                                const bool processSideTrip)
{

  uint16_t lastSegNum = fmktPath.fareMarketPath().back()->getEndSegNum();

  LOG4CXX_INFO(_logger,
               "buildPath: curSegNum="
                   << curSegNum << ", lastSegNum=" << lastSegNum << ", pathStartSN=" << pathStartSN
                   << ", pathEndSN=" << pathEndSN << ", ST travelSegs size=" << travelSegs.size());

  if (lastSegNum == pathEndSN ||
      (lastSegNum == pathEndSN - 1 && dynamic_cast<ArunkSeg*>(travelSegs.back())))
  {
    targetMatrix.push_back(&fmktPath);
    return;
  }

  if (curSegNum > pathEndSN || (curSegNum > (lastSegNum + 2))) // not possible with arunk even
    return;

  bool arunkSeg = false;
  // lint -e{530}
  if (travelSegs.size() > (size_t)(lastSegNum - pathStartSN + 1))
  {
    TravelSeg* tvlSeg = travelSegs[lastSegNum - pathStartSN + 1];
    if (dynamic_cast<ArunkSeg*>(tvlSeg))
    {
      arunkSeg = true;
    }
    else
    {
      arunkSeg = false;
    }
  }

  std::vector<MergedFareMarket*>::iterator fmIt = fareMarketVect.begin();
  std::vector<MergedFareMarket*>::iterator fmItEnd = fareMarketVect.end();
  std::vector<MergedFareMarket*>::iterator fmLastIt = fareMarketVect.end() - 1;
  for (; fmIt != fmItEnd && fmIt != fmLastIt; ++fmIt)
  {
    MergedFareMarket* fareMarket = *fmIt;
    if (processSideTrip && !fareMarket->sideTripTravelSeg().empty())
    {
      // don't allow SideTrip out of SideTrip
      continue;
    }

    // get the start & end segment numbers for this fare market
    //
    uint16_t mktStartSegNum = fareMarket->getStartSegNum();

    if ((lastSegNum + 1 == mktStartSegNum) || (arunkSeg && (lastSegNum + 2 == mktStartSegNum)))
    {
      // lint --e{530}
      FareMarketPath* newPath = constructFareMarketPath();
      newPath->fareMarketPath().insert(newPath->fareMarketPath().end(),
                                       fmktPath.fareMarketPath().begin(),
                                       fmktPath.fareMarketPath().end());
      newPath->fareMarketPath().push_back(fareMarket);
      if (UNLIKELY(_trx.getRequest()->isBrandedFaresRequest()))
        newPath->setBrandCode(_brandCode);

      buildPath(travelSegs,
                fareMarketVect,
                curSegNum + 1,
                pathStartSN,
                pathEndSN,
                *newPath,
                targetMatrix,
                processSideTrip);
    }
  }
  if (LIKELY(!fareMarketVect.empty()))
  {
    MergedFareMarket* fareMarket = fareMarketVect.back();
    uint16_t mktStartSegNum = fareMarket->getStartSegNum();
    if (((lastSegNum + 1 == mktStartSegNum) || (arunkSeg && (lastSegNum + 2 == mktStartSegNum))) &&
        (!processSideTrip || fareMarket->sideTripTravelSeg().empty()))
    {
      fmktPath.fareMarketPath().push_back(fareMarket);
      buildPath(travelSegs,
                fareMarketVect,
                curSegNum + 1,
                pathStartSN,
                pathEndSN,
                fmktPath,
                targetMatrix,
                processSideTrip);
    }
  }
}

//---------------------------------------------------------------------
// One FareMarket ===> vector of SideTrip, which is vector<TravelSeg*>
// One SideTrip ===> vector of FareMarketPath
// Therefore, we have vector of vector

bool
FareMarketPathMatrix::buildSideTrip()
{

  std::vector<FareMarketPath*> tmpMatrix;
  std::vector<FareMarketPath*>::iterator pathIt = _fareMarketPathMatrix.begin();
  while (pathIt != _fareMarketPathMatrix.end())
  {
    // to hold the combination of all the ST of all the FareMarkets
    std::vector<std::map<MergedFareMarket*, FareMarketPathVect> > pathSideTripCombVect;

    std::vector<std::vector<FareMarketPathVect> > pathSideTripVect;

    FareMarketPath* fmPath = *pathIt;

    // uint16_t mktIdx = 0;
    bool validFMP = true;
    for (const auto mergedFareMarket : fmPath->fareMarketPath())
    {
      std::vector<std::vector<TravelSeg*>>& stTvlSegVect = mergedFareMarket->sideTripTravelSeg();
      std::vector<FareMarketPathVect> mktSideTripsCombVect;
      if (!stTvlSegVect.empty())
      {
        // each elem (vect of FareMarketPath*) of mktSideTrips contains one
        // combination of FareMarketPath of all the ST of this mkt
        //
        LOG4CXX_DEBUG(_logger, "Side Trip Exists");
        std::vector<FareMarketPathVect> mktSideTripVect;

        for (auto& sideTripTravelSegments : stTvlSegVect)
        {
          FareMarketPathVect sideTripVect; // n number of paths for one side trip
          buildFareMarketPath(
              sideTripTravelSegments, sideTripVect, true); // 3rd param true, for doing SideTrip

          // LOG4CXX_DEBUG(_logger, "sideTripVect SIZE= "<<sideTripVect.size());
          // forDebugOnlyDisplayFMP(_oldlogger, sideTripVect);

          if (!sideTripVect.empty())
          {
            mktSideTripVect.push_back(sideTripVect);
          }
          else
          {
            validFMP = false; // Could not complete fareMarketPath of the SideTrip,
            // Therefore, MainTrip FareMareketPath is invalide
            break;
          }
        }
        if (!validFMP)
        {
          break;
        }

        // if no. of side trip in one FareMarket is m, it will create (n*m) combinations
        FareMarketPathVect newPathVect;
        uint16_t stCount = mktSideTripVect.size();
        genMarketSideTripCombination(
            newPathVect, 0, stCount, mktSideTripVect, mktSideTripsCombVect);

      } // if there is a side trip

      // If there is no SideTrip for this fMkt, an empty mktSideTripsCombVect is
      // pushed back, therefore, index in pathSideTripVect corresponsds to the
      // index of FareMarket in FareMarketPath
      //
      pathSideTripVect.push_back(mktSideTripsCombVect);
      //++mktIdx;

    } // for each mkt

    if (!validFMP)
    {
      pathIt = _fareMarketPathMatrix.erase(pathIt);
      continue;
    }

    std::map<MergedFareMarket*, FareMarketPathVect> pathSideTripMap;
    uint16_t mktCount = pathSideTripVect.size();
    genPathSideTripCombination(fmPath->fareMarketPath(),
                               pathSideTripMap,
                               0,
                               mktCount,
                               pathSideTripVect,
                               pathSideTripCombVect);

    // LOG4CXX_DEBUG(_logger, "pathSideTripCombVect Size= "<<pathSideTripCombVect.size());

    //------ Combine SideTrip with current MainTrip: Cross Product ----
    std::vector<std::map<MergedFareMarket*, FareMarketPathVect> >::iterator stIt =
        pathSideTripCombVect.begin();
    std::vector<std::map<MergedFareMarket*, FareMarketPathVect> >::iterator stItEnd =
        pathSideTripCombVect.end();
    std::vector<std::map<MergedFareMarket*, FareMarketPathVect> >::iterator stLastIt =
        pathSideTripCombVect.end() - 1;
    for (; stIt != stItEnd && stIt != stLastIt; ++stIt)
    {
      // create a new copy for (N-1) items
      FareMarketPath* newFMPath = copyFareMarketPath(*fmPath);

      for (const auto& sideTrip : *stIt)
      {
        newFMPath->sideTrips()[sideTrip.first] = sideTrip.second;
      }
      tmpMatrix.push_back(newFMPath);
    }

    if (LIKELY(!pathSideTripCombVect.empty()))
    {
      // use the Nth item
      for (const auto& sideTrip : *stIt)
      {
        fmPath->sideTrips()[sideTrip.first] = sideTrip.second;
      }
    }

    ++pathIt;

  } // for each FareMarketPath in _fareMarketPathMatrix

  _fareMarketPathMatrix.insert(_fareMarketPathMatrix.end(), tmpMatrix.begin(), tmpMatrix.end());

  return true;
}

bool
FareMarketPathMatrix::genMarketSideTripCombination(
    FareMarketPathVect& pathVect,
    uint16_t stIdx,
    uint16_t stCount,
    std::vector<FareMarketPathVect>& mktSideTripVect,
    std::vector<FareMarketPathVect>& mktSideTripsCombVect)
{

  if (stIdx >= stCount) // idx start from 0
  {
    mktSideTripsCombVect.push_back(pathVect);
    return true;
  }

  FareMarketPathVect& vect = mktSideTripVect[stIdx];
  uint16_t sz = vect.size();
  for (uint16_t i = 0; i < sz - 1; ++i)
  {
    FareMarketPathVect newPathVect;
    copyFareMarketPathVect(pathVect, newPathVect);
    FareMarketPath* path = vect[i];
    newPathVect.push_back(path);
    genMarketSideTripCombination(
        newPathVect, stIdx + 1, stCount, mktSideTripVect, mktSideTripsCombVect);
  }
  if (sz > 0)
  {
    FareMarketPath* path = vect[sz - 1];
    pathVect.push_back(path);
  }

  genMarketSideTripCombination(pathVect, stIdx + 1, stCount, mktSideTripVect, mktSideTripsCombVect);

  return true;
}

bool
FareMarketPathMatrix::genPathSideTripCombination(
    std::vector<MergedFareMarket*>& fareMarketPath,
    std::map<MergedFareMarket*, FareMarketPathVect>& pathSideTripMap,
    uint16_t mktIdx,
    uint16_t mktCount,
    std::vector<std::vector<FareMarketPathVect> >& pathSideTripVect,
    std::vector<std::map<MergedFareMarket*, FareMarketPathVect> >& pathSideTripCombVect)
{
  if (mktIdx >= mktCount) // idx start from 0
  {
    pathSideTripCombVect.push_back(pathSideTripMap);
    return true;
  }

  std::vector<FareMarketPathVect>& vect = pathSideTripVect[mktIdx];
  uint16_t sz = vect.size();
  for (uint16_t i = 0; i < sz - 1; ++i)
  {
    std::map<MergedFareMarket*, FareMarketPathVect> newMktPathMap;
    copyFareMarketPathMap(pathSideTripMap, newMktPathMap);
    newMktPathMap[fareMarketPath[mktIdx]] = vect[i];
    genPathSideTripCombination(fareMarketPath,
                               newMktPathMap,
                               mktIdx + 1,
                               mktCount,
                               pathSideTripVect,
                               pathSideTripCombVect);
  }
  if (sz > 0)
  {
    pathSideTripMap[fareMarketPath[mktIdx]] = vect[sz - 1];
  }

  genPathSideTripCombination(fareMarketPath,
                             pathSideTripMap,
                             mktIdx + 1,
                             mktCount,
                             pathSideTripVect,
                             pathSideTripCombVect);

  return true;
}

//---------------------------------------------------------------------
FareMarketPath*
FareMarketPathMatrix::copyFareMarketPath(const FareMarketPath& oldPath)
{
  FareMarketPath* newPath = constructFareMarketPath();
  newPath->fareMarketPath().insert(newPath->fareMarketPath().end(),
                                   oldPath.fareMarketPath().begin(),
                                   oldPath.fareMarketPath().end());

  newPath->sideTrips().insert(oldPath.sideTrips().begin(), oldPath.sideTrips().end());
  if (_trx.getRequest()->isBrandedFaresRequest())
    newPath->setBrandCode(oldPath.getBrandCode());


  return newPath;
}

//---------------------------------------------------------------------
bool
FareMarketPathMatrix::copyFareMarketPathVect(const FareMarketPathVect& oldVect,
                                             FareMarketPathVect& newVect)
{
  for (const auto sourceFareMarketPath : oldVect)
  {
    FareMarketPath* newPath = constructFareMarketPath();
    newPath->fareMarketPath().insert(newPath->fareMarketPath().end(),
                                     sourceFareMarketPath->fareMarketPath().begin(),
                                     sourceFareMarketPath->fareMarketPath().end());
    newVect.push_back(newPath);
  }

  return true;
}

//---------------------------------------------------------------------
bool
FareMarketPathMatrix::copyFareMarketPathMap(
    const std::map<MergedFareMarket*, FareMarketPathVect>& oldMktPathMap,
    std::map<MergedFareMarket*, FareMarketPathVect>& newMktPathMap)
{
  for (const auto& sourceFareMarketPathMap : oldMktPathMap)
  {
    newMktPathMap[sourceFareMarketPathMap.first] = sourceFareMarketPathMap.second;
  }

  return true;
}

//---------------------------------------------------------------------
void
FareMarketPathMatrix::collectDiagnostic(Itin* itin, std::vector<FareMarketPath*>* fmps)
{
  LOG4CXX_DEBUG(_logger, "FareMarketPathMatrix SIZE = " << _fareMarketPathMatrix.size());

  DiagManager diag600(_trx, DiagnosticTypes::Diagnostic600);
  if (!diag600.isActive())
    return;

  Diag600Collector& dc = static_cast<Diag600Collector&>(diag600.collector());

  dc.displayFareMarkets(*itin, _fareMarketPathMatrix, _brandCode);
  dc.displayFMPMatrix(*this, itin, fmps);
  dc.displayFareMarkestWithoutTag2Fare(*itin, _mergedFareMarketVect);
}

//---------------------------------------------------------------------

void
FareMarketPathMatrix::markUsedMergedFareMarket()
{
  for (const auto mergedFareMarket : _mergedFareMarketVect)
  {
    if (mergedFareMarketUsedInPath(mergedFareMarket, _fareMarketPathMatrix))
    {
      mergedFareMarket->collectRec2Cat10() = true;
    }
  }
}

bool
FareMarketPathMatrix::mergedFareMarketUsedInPath(const MergedFareMarket* fareMarket,
                                                 const std::vector<FareMarketPath*>& fmpVect)
{
  for (const auto fareMarketPath : fmpVect)
  {
    const FareMarketPath& fmp = *fareMarketPath;
    if (std::find(fmp.fareMarketPath().begin(), fmp.fareMarketPath().end(), fareMarket) !=
        fmp.fareMarketPath().end())
    {
      return true;
    }
    else if (!fmp.sideTrips().empty())
    {
      for (const auto& sideTrip : fmp.sideTrips())
      {
        if (mergedFareMarketUsedInPath(fareMarket, sideTrip.second))
        {
          return true;
        }
      }
    }
  }
  return false;
}

} // namespace tse

#if 0
/****************************************************************
                      FOR DEBUG ONLY
*****************************************************************/
void forDebugOnlyDisplayFMP( std::vector<FareMarketPath*>& fmpv)
{
  log4cxx::LoggerPtr _logger(
    log4cxx::Logger::getLogger("atseintl.Pricing.forDebugOnlyDisplayFMP"));
	std::vector<FareMarketPath*>::iterator it = fmpv.begin();
	std::vector<FareMarketPath*>::iterator itEnd = fmpv.end();
        for(; it != itEnd; ++it)
        {

	    std::vector<MergedFareMarket*>::const_iterator mktIt = (*it)->fareMarketPath().begin();
	    for( ; mktIt != (*it)->fareMarketPath().end(); ++mktIt)
	    {
	      MergedFareMarket* fm = *mktIt;
	      LOG4CXX_DEBUG(_logger, fm->getStartSegNum()<<"--"
	        <<fm->getEndSegNum()<<": "<<fm->boardMultiCity()<<"--"
	         <<fm->offMultiCity());
	    }

	}
}

void forDebugOnlyDisplayItin(tse::Itin& itin)
{
  log4cxx::LoggerPtr _logger(
    log4cxx::Logger::getLogger("atseintl.Pricing.forDebugOnlyDisplayItin"));
	std::vector<FareMarket*>::iterator fmIt = itin.fareMarket().begin();
	std::vector<FareMarket*>::iterator fmItEnd = itin.fareMarket().end();
        for(; fmIt != fmItEnd; ++fmIt)
	{
	  FareMarket& fareMarket = **fmIt;
	  std::vector<TravelSeg*>::iterator it = fareMarket.travelSeg().begin();
	  std::vector<TravelSeg*>::iterator itEnd = fareMarket.travelSeg().end();
	  LOG4CXX_DEBUG(_logger, "Itinerary Passed In:");
          for(; it != itEnd; ++it)
          {
	     std::string segType = "TravelSeg";
	     if((*it)->segmentType() == Arunk)
                 segType = "ArunkSeg";
	     else if((*it)->segmentType() == Air)
                 segType = "AirSeg";
	     else if((*it)->segmentType() == Open)
                 segType = "OpenSeg";

             LocCode boardMultiCity = FareMarketUtil::getBoardMultiCity(fareMarket, **it);
             LocCode offMultiCity = FareMarketUtil::getOffMultiCity(fareMarket, **it);
	     LOG4CXX_DEBUG(_logger, (*it)->origin()->loc() <<"-"
	                            << (*it)->destination()->loc()<<" "
	                            << boardMultiCity<<"-"<< offMultiCity<< "SegNum="
	                            << itin.segmentOrder(*it)<< ", SegType="<< segType);
          }
	}
	fmIt = itin.fareMarket().begin();
	LOG4CXX_DEBUG(_logger, "FareMarket Passed In:");
        for(; fmIt != fmItEnd; ++fmIt)
	{
	  FareMarket& fareMarket = **fmIt;
	  LOG4CXX_DEBUG(_logger, "Board: "<<fareMarket.boardMultiCity()<<"  Off: "
	                                  << fareMarket.offMultiCity());
	  LOG4CXX_DEBUG(_logger, "  includes: "<<fareMarket.travelSeg().size()
	                                  <<" travel sements");
	  LOG4CXX_DEBUG(_logger, "  includes: "<<fareMarket.sideTripTravelSeg().size()
	                                       <<" side trips");
	  LOG4CXX_DEBUG(_logger, "  breakIndicator: "<<fareMarket.breakIndicator());
	  LOG4CXX_INFO(_logger, "  governingCarrier: "<<fareMarket.governingCarrier());
	}
}

#endif
