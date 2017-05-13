#include "Pricing/Shopping/PQ/SoloPUPathCollector.h"

#include "Common/FareCalcUtil.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag600Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

namespace tse
{
namespace shpq
{

SoloPUPathCollector::SoloPUPathCollector(ShoppingTrx& trx) : _trx(trx), _fmPathCollector(trx)
{
  PUPathMatrix::trx() = &_trx;
  _fcConfig = FareCalcUtil::getFareCalcConfig(_trx);
}

SoloPUPathCollector::PUStruct
SoloPUPathCollector::buildPUStruct(const ConxRouteCxrPQItem* const pqItem,
                                   PUFactoryBucketVec& puFactoryBucketVec)
{
  PUStruct puStruct;
  if (LIKELY(pqItem))
  {
    SoloFMPathCollector::FMPathItinPair fmPathItin =
        _fmPathCollector.getOrCreateFareMarketPath(pqItem);
    puStruct._fmPath = fmPathItin.first;
    puStruct._itin = fmPathItin.second;

    if (puStruct._fmPath && puStruct._itin)
    {
      puStruct._puPath = buildPuPath(*(pqItem->getSolPattern()), puStruct._fmPath, puStruct._itin);
      if (puStruct._puPath)
      {
        PricingOrchestrator::getRec2Cat10(_trx, puStruct._fmPath->fareMarketPath());
        for (MergedFareMarket* mfm : puStruct._fmPath->fareMarketPath())
        {
          // to avoid getRec2Cat10() apply several times on existing fareMarketPath
          mfm->collectRec2Cat10() = false;
        }
      }

      updatePUFactoryBucket(puStruct, puFactoryBucketVec);
      setPUPath(puStruct._puPath);
    }

    collectDiagnostic(puStruct);
  }
  return puStruct;
}

void
SoloPUPathCollector::updatePUFactoryBucket(SoloPUPathCollector::PUStruct& puStruct,
                                           PUFactoryBucketVec& puFactoryBucketVec)
{
  if (puStruct._puPath && puStruct._itin)
  {
    PUPathMatrix::itin() = puStruct._itin;
    PUPathMatrix::updatePUFactoryBucket(*(puStruct._puPath), puFactoryBucketVec, _updatedPUSet);
    puStruct._puFactoryBucketVec = &puFactoryBucketVec;
  }
}

PUPath*
SoloPUPathCollector::buildPuPath(const SolutionPattern& solutionPattern,
                                 FareMarketPath* fmPath,
                                 Itin* itin)
{
  PUPath* puPath = nullptr;
  if (LIKELY(fmPath && itin))
  {
    PUPathMatrix::itin() = itin;
    PUPathMatrix::determineItinTravelBoundary();

    puPath = constructPUPath();
    if (!solutionPattern.createPUPath(*this, fmPath, puPath))
      return nullptr;
    if (!isPUPathComplete(*puPath, *fmPath, fmPath->fareMarketPath().size()))
      return nullptr;
  }
  return puPath;
}

void
SoloPUPathCollector::setPUPath(PUPath* puPath)
{
  if (puPath)
  {
    PUPathMatrix::createMainTripSideTripLink(*puPath);
    puPath->setTotalPU();
    puPath->countTotalFC();
    puPath->itinWithinScandinavia() = _travelWithinScandinavia;
  }
}

bool
SoloPUPathCollector::isPUPathComplete(PUPath& puPath, const FareMarketPath& fmp, size_t totalMktCnt)
{
  if (PUPathMatrix::isPUPathValid(fmp, puPath, totalMktCnt))
  {
    puPath.fareMarketPath() = (FareMarketPath*)&fmp;
    setOWPUDirectionality(puPath);
    setIsIntlCTJourneyWithOWPU(fmp, puPath);
    return true;
  }
  return false;
}
PU*
SoloPUPathCollector::getOrCreateOWPU(MergedFareMarket* mfm)
{
  if (!mfm || mfm->mergedFareMarket().empty())
    return nullptr;

  MergedFMKey mfmKey(mfm->mergedFareMarket().front());
  OWMapPair insertResult = _owPUMap.insert(OWPUMap::value_type(mfmKey, nullptr));
  if (insertResult.second) // no PU for mfmKey
    (insertResult.first)->second = buildOWPU(mfm);

  return (insertResult.first)->second;
}

PU*
SoloPUPathCollector::getOrCreateRTPU(MergedFareMarket* mfm1, MergedFareMarket* mfm2)
{
  PU* rtPU = nullptr;
  if (LIKELY(mfm1 && mfm2 && !mfm1->mergedFareMarket().empty() && !mfm2->mergedFareMarket().empty()))
  {
    SoloFMPathKey mfmPathKey(mfm1->mergedFareMarket().front(), mfm2->mergedFareMarket().front());
    PUMap::iterator it = _rtPUMap.find(mfmPathKey);
    PUMapPair insertResult = _rtPUMap.insert(PUMap::value_type(mfmPathKey, nullptr));
    if (insertResult.second) // no PU for mfmPathKey
      (insertResult.first)->second = buildRTPU(mfm1, mfm2);

    rtPU = (insertResult.first)->second;
  }
  return rtPU;
}

PU*
SoloPUPathCollector::getOrCreateOJPU(MergedFareMarket* outboundMfm, MergedFareMarket* inboundMfm)
{
  PU* ojPU = nullptr;
  if (outboundMfm && inboundMfm && !outboundMfm->mergedFareMarket().empty() &&
      !inboundMfm->mergedFareMarket().empty())
  {
    SoloFMPathKey mfmPathKey(outboundMfm->mergedFareMarket().front(),
                             inboundMfm->mergedFareMarket().front());
    PUMapPair insertResult = _ojPUMap.insert(PUMap::value_type(mfmPathKey, nullptr));
    if (insertResult.second) // no PU for mfmPathKey
    {
      MergedFMVector outboundVec(1, outboundMfm);
      MergedFMVector inboundVec(1, inboundMfm);

      (insertResult.first)->second =
          buildOJPU(outboundVec,
                    inboundVec,
                    outboundMfm->geoTravelType(),
                    outboundMfm->cxrFarePreferred() || inboundMfm->cxrFarePreferred());
    }
    ojPU = (insertResult.first)->second;
  }
  return ojPU;
}

bool
SoloPUPathCollector::createCTPUPath(PUPath& puPath, MergedFMVector& fmPath)
{
  bool ctFound = false;
  if (fmPath.size() == 4 // SP49: HRT+HRT//HRT+HRT (CT excluding showman)
      ||
      fmPath.size() == 3) // SP36, SP37: HRT//HRT+HRT (103 CT), HRT+HRT//HRT (103 CT)
  {
    bool done = false;

    PU* ctPU = constructPU();
    MergedFareMarket* startFM = fmPath[0];
    ctPU->puType() = PricingUnit::Type::CIRCLETRIP;
    ctPU->geoTravelType() = startFM->geoTravelType();
    ctPU->fareMarket().push_back(startFM);
    ctPU->fareDirectionality().push_back(FROM);
    ctPU->cxrFarePreferred() = startFM->cxrFarePreferred();
    puPath.cxrFarePreferred() = ctPU->cxrFarePreferred();

    buildCTPU(
        *ctPU, 1 /*mktIdx*/, fmPath.size() /*totalMktCnt*/, puPath, fmPath, false, ctFound, done);
  }
  return ctFound;
}

void
SoloPUPathCollector::collectDiagnostic(PUStruct& puStruct)
{
  DiagManager diag600(_trx, DiagnosticTypes::Diagnostic600);
  if (LIKELY(!diag600.isActive()))
    return;

  if (!puStruct._itin || !puStruct._fmPath)
    return;

  FareMarketPath* fmPath = puStruct._fmPath;
  Itin* itin = puStruct._itin;
  FareMarketPathMatrix fmpMatrix(_trx, *itin, fmPath->fareMarketPath());
  fmpMatrix.fareMarketPathMatrix().push_back(fmPath);
  fmpMatrix.collectDiagnostic(&fmpMatrix.itin());

  PUPathMatrix::puPathMatrix().clear();
  if (puStruct._puPath)
    PUPathMatrix::puPathMatrix().push_back(puStruct._puPath);

  Diag600Collector& dc = static_cast<Diag600Collector&>(diag600.collector());
  dc.displayPUPathMatrix((PUPathMatrix&)*this);
}
}
} // namespace tse::shpq
