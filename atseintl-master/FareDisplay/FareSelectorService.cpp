//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/FareSelectorService.h"

#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DBAccess/DataHandle.h"
#include "FareDisplay/FareSelector.h"
#include "FareDisplay/FareSelectorFQ.h"
#include "FareDisplay/FareSelectorRD.h"
#include "Fares/FareController.h"
#include "Server/TseServer.h"
#include "Util/Algorithm/Container.h"

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <stdlib.h>


namespace tse
{

static Logger
logger("atseintl.FareDisplay.FareSelectorService");

static LoadableModuleRegister<Service, FareSelectorService>
_("libFareSelectorSvc.so");

FareSelectorService::FareSelectorService(const std::string& sname, TseServer& tseServer)
  : Service(sname, tseServer), _config(Global::config())
{
}

bool
FareSelectorService::initialize(int argc, char* argv[])
{
  return true;
}

inline FareSelector*
FareSelectorService::getFareSelector(FareDisplayTrx& trx, DataHandle& dh)
{
  if (trx.isShortRequest() || trx.isERD())
  {
    FareSelectorRD* p = nullptr;
    dh.get(p);
    return p;
  }

  FareSelectorFQ* p = nullptr;
  dh.get(p);
  return p;
}

bool
FareSelectorService::process(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering FareSelectorService::process()");

  DataHandle localDataHandle(trx.ticketingDate());

  // Select the fare(s)
  FareSelector* fs = getFareSelector(trx, localDataHandle);
  if (fs != nullptr)
  {
    fs->setup(trx);
    fs->selectFares(trx);
  }
  else
    return false;

  LOG4CXX_INFO(logger, "Leaving FareSelectorService::process");
  return true;
}

namespace
{
std::vector<PaxTypeFare*>
getPaxTypeFaresWithProperFareBasis(std::vector<PaxTypeFare*> paxTypeFares,
                                   const std::string& fareBasisCode,
                                   const MoneyAmount fareAmount,
                                   PricingTrx& trx)
{
  std::vector<PaxTypeFare*> filteredPaxTypeFares;
  for (PaxTypeFare* ptf : paxTypeFares)
  {
    if (ptf && ptf->isFareBasisEqual(fareBasisCode) &&
        (fareAmount <= EPSILON || std::abs(ptf->fareAmount() - fareAmount) <= EPSILON))
    {
      ptf->setRoutingValid(true);
      ptf->setRoutingProcessed(true);
      ptf->bookingCodeStatus().set(PaxTypeFare::BookingCodeState::BKS_PASS);
      ptf->bookingCodeStatus().set(PaxTypeFare::BookingCodeState::BKS_NOT_YET_PROCESSED, false);

      filteredPaxTypeFares.push_back(ptf);
    }
  }
  return filteredPaxTypeFares;
}
}

void
FareSelectorService::selectCorrectPaxTypeFaresInBuckets(FareMarket* fm, PricingTrx& trx)
{
  for (const auto* paxType : trx.paxType())
  {
    if (!fm->paxTypeCortege(paxType))
      continue;

    std::vector<PaxTypeFare*>& paxTypeFaresInBucket = fm->paxTypeCortege(paxType)->paxTypeFare();
    alg::erase_if(paxTypeFaresInBucket,
                  [&allPTF = fm->allPaxTypeFare()](const auto* paxTypeFare)
                  { return !alg::contains(allPTF, paxTypeFare); });
  }
}

void
FareSelectorService::processForSinglePassenger(StructuredRuleTrx& trx)
{
  for (FareMarket* fm : trx.fareMarket())
  {
    const std::string fareBasisFm = fm->fareCompInfo()->fareBasisCode().c_str();
    const MoneyAmount fareAmtFm = fm->fareCompInfo()->fareCalcFareAmt();

    std::vector<PaxTypeFare*> filteredPaxTypeFares =
        getPaxTypeFaresWithProperFareBasis(fm->allPaxTypeFare(), fareBasisFm, fareAmtFm, trx);
    std::swap(fm->allPaxTypeFare(), filteredPaxTypeFares);

    selectCorrectPaxTypeFaresInBuckets(fm, trx);
  }
}

void
FareSelectorService::filterAllPassengerBuckets(StructuredRuleTrx& trx)
{
  std::unordered_map<FareCompInfo*, std::vector<FareMarket*>> fcInfoWithCorrespondingFMs;
  for (FareMarket* fareMarket : trx.fareMarket())
    fcInfoWithCorrespondingFMs[fareMarket->fareCompInfo()].push_back(fareMarket);

  for (auto& mepElem : *trx.getMultiPassengerFCMapping())
  {
    const PaxType* paxType = mepElem.first;
    std::vector<FareCompInfo*>& fciVector = mepElem.second;
    for (FareCompInfo* fareCompInfo : fciVector)
    {
      const std::string fareBasis = fareCompInfo->fareBasisCode().c_str();
      const MoneyAmount fareAmt = fareCompInfo->fareCalcFareAmt();
      // In MultiPassengerSFR, FM points only to one of passenger's FareCompInfo
      // We use this correct pointer in order to fetch all FMs, even when they
      // are not covered by fareCompInfo->fareMarket() pointers
      for (FareMarket* fareMarket :
           fcInfoWithCorrespondingFMs[fareCompInfo->fareMarket()->fareCompInfo()])
      {
        PaxTypeBucket* bucket = fareMarket->paxTypeCortege(paxType);
        filterBucketSFR(bucket, fareBasis, fareAmt, trx);
      }
    }
  }
}

void
FareSelectorService::accumulateFaresFromBuckets(StructuredRuleTrx& trx)
{
  // Second step is to accumulate fares form buckets to fareMarket->allPaxTypeFare()
  for (FareMarket* fareMarket : trx.fareMarket())
  {
    std::vector<PaxTypeFare*> faresToReplace;
    for (const auto paxType : trx.paxType())
    {
      PaxTypeBucket* filteredBucket = fareMarket->paxTypeCortege(paxType);
      if (!filteredBucket)
        continue;
      if (filteredBucket->isBucketFilteredSFR())
      {
        faresToReplace.reserve(faresToReplace.size() + filteredBucket->paxTypeFare().size());

        alg::copy_if(filteredBucket->paxTypeFare(),
                     std::back_inserter(faresToReplace),
                     [&faresToReplace](const auto* ptf)
                     { return !alg::contains(faresToReplace, ptf); });
      }
      else
        filteredBucket->removeAllFares();
    }
    fareMarket->allPaxTypeFare() = faresToReplace;
    FareController fareController(trx, *trx.itin().front(), *fareMarket);
    fareController.sortPaxTypeFares();
  }
}

void
FareSelectorService::processForMultiPassenger(StructuredRuleTrx& trx)
{
  filterAllPassengerBuckets(trx);
  accumulateFaresFromBuckets(trx);
}

bool
FareSelectorService::process(StructuredRuleTrx& trx)
{
  if (trx.isMultiPassengerSFRRequestType())
  {
    processForMultiPassenger(trx);
  }
  else
  {
    processForSinglePassenger(trx);
  }
  return true;
}

void
FareSelectorService::filterBucketSFR(PaxTypeBucket* bucket,
                                     const std::string& fareBasisCode,
                                     const MoneyAmount fareAmount,
                                     StructuredRuleTrx& trx)
{
  if (!bucket)
    return;
  std::vector<PaxTypeFare*>& paxTypeFaresInBucket = bucket->paxTypeFare();
  std::vector<PaxTypeFare*> filteredPaxTypeFaresInBucket =
      getPaxTypeFaresWithProperFareBasis(paxTypeFaresInBucket, fareBasisCode, fareAmount, trx);
  std::swap(paxTypeFaresInBucket, filteredPaxTypeFaresInBucket);
  bucket->setBucketFilteredSFR();
}

uint32_t
FareSelectorService::getActiveThreads()
{
  return FareSelector::getActiveThreads();
}
} // tse
