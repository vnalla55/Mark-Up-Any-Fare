#include "Pricing/Shopping/PQ/SoloFmPathOrchestrator.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/DirFMPathListCollector.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/SoloFmPath.h"
#include "Pricing/Shopping/PQ/CxrFMCollector.h"

namespace tse
{

namespace shpq
{

void
SoloFmPathOrchestrator::process(SoloFmPathPtr soloFmPath)
{
  typedef std::vector<ShoppingTrx::Leg> LegVec;
  typedef std::vector<FareMarket*> FareMarketVec;

  LegVec& trxLegs = _trx.legs();
  for (auto& trxLeg : trxLegs)
  {
    Itin* itin = trxLeg.sop().begin()->itin();

    CxrFMCollector::CitySet citySet;
    CxrFMCollector cxrFmCollector(
        _trx, itin->travelSeg().front(), itin->travelSeg().back(), citySet);

    std::vector<ShoppingTrx::SchedulingOption>& sops(trxLeg.sop());

    for (auto sop : sops)
    {
      const std::vector<TravelSeg*>& tvlSegs(sop.itin()->travelSeg());

      for (const auto tvlSeg : tvlSegs)
      {
        citySet.insert(tvlSeg->boardMultiCity());
        citySet.insert(tvlSeg->offMultiCity());
      }
    }
    const ItinIndex& itinIndex = trxLeg.carrierIndex();

    ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
    ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();

    for (; iGIter != iGEndIter; ++iGIter)
    {
      ItinIndex::Key cxrIndexKey = iGIter->first;
      const ItinIndex::ItinCell* curCell =
          ShoppingUtil::retrieveDirectItin(itinIndex, cxrIndexKey, ItinIndex::CHECK_NOTHING);

      if (LIKELY(curCell))
      {
        Itin* curItin = curCell->second;
        FareMarketVec& fmVector = curItin->fareMarket();

        for (const auto fm : fmVector)
        {
          cxrFmCollector.addFareMarket(fm, cxrIndexKey);
        }
      }
    }

    DirFMPathListCollectorPtr dirFmPathList = DirFMPathListCollector::create(_trx);
    cxrFmCollector.collectDirFMPathList(dirFmPathList);
    soloFmPath->insertLeg(dirFmPathList);
  }
}
}
} // namespace tse::shpq
