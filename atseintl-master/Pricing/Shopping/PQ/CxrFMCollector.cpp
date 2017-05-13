#include "Pricing/Shopping/PQ/CxrFMCollector.h"

#include "DataModel/CxrFareMarkets.h"
#include "DataModel/DirFMPath.h"
#include "DataModel/DirFMPathListCollector.h"
#include "DataModel/OwrtFareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/Loc.h"


namespace tse
{
namespace shpq
{

namespace
{

struct FareMarketsCmp
{
  FareMarketsCmp(const FareMarket* fm) : _fm(fm) {}

  bool operator()(const FareMarket* fareMarket) { return *_fm == *fareMarket; }

private:
  const FareMarket* _fm;
};

} // namespace

void
CxrFMCollector::OwHrtFMPair::insertFM(const BaseParamsInfo& paramsInfo)
{
  insertFM(paramsInfo, paramsInfo.owType());
  insertFM(paramsInfo, paramsInfo.hrtType());
}

void
CxrFMCollector::OwHrtFMPair::insertFM(const BaseParamsInfo& paramsInfo, const SolutionType sType)
{
  if (!isTypeValid(sType))
    return;

  OwrtFareMarketPtr owrtFM = OwrtFareMarket::create(
      paramsInfo.getTrx(), sType, paramsInfo.getFareMarket(), paramsInfo.getCxrIndexKey());

  CxrFareMarketsPtr& cxrFM = _cxrFM[getIndex(sType)];
  if (!cxrFM)
    cxrFM = CxrFareMarkets::create(paramsInfo.getTrx(), sType);
  cxrFM->insert(owrtFM);
}

CxrFareMarketsPtr
CxrFMCollector::OwHrtFMPair::getCxrFM(const SolutionType sType) const
{
  if (UNLIKELY(!isTypeValid(sType)))
    return CxrFareMarketsPtr();

  return _cxrFM[getIndex(sType)];
}

bool
CxrFMCollector::OwHrtFMPair::isTypeValid(const SolutionType sType) const
{
  return (sType == OW) || (sType == HRT);
}

size_t
CxrFMCollector::OwHrtFMPair::getIndex(const SolutionType sType) const
{
  // Not using sType-1 intentionally to avoid errors in case changing SolutionType enum definition
  if (sType == OW)
    return 0;
  if (LIKELY(sType == HRT))
    return 1;
  return 2;
}

CxrFMCollector::BaseParamsInfo::BaseParamsInfo(ShoppingTrx& trx,
                                               FareMarket* fm,
                                               const ItinIndex::Key& cxrIndexKey)
  : _trx(trx), _fm(fm), _owType(false), _hrtType(false), _cxrIndexKey(cxrIndexKey)
{
  if (LIKELY(fm))
  {
    _owType = (fm->ow_begin() != fm->ow_end());
    _hrtType = (fm->hrt_begin() != fm->hrt_end());
  }
}

CxrFMCollector::CxrFMCollector(ShoppingTrx& trx,
                               const TravelSeg* origin,
                               const TravelSeg* destination,
                               const CitySet& citySet)
  : _trx(trx), _citySet(citySet), _origin(origin), _destination(destination)
{
}

void
CxrFMCollector::addToDirFMPathList(DirFMPathListCollectorPtr dirFMPathListCollector,
                                   OwHrtFMPair& cxrFMPair)
{
  if (dirFMPathListCollector)
  {
    if (cxrFMPair.getCxrFM(OW))
      dirFMPathListCollector->insert(DirFMPath::create(_trx, cxrFMPair.getCxrFM(OW)));
    if (cxrFMPair.getCxrFM(HRT))
      dirFMPathListCollector->insert(DirFMPath::create(_trx, cxrFMPair.getCxrFM(HRT)));
  }
}

void
CxrFMCollector::addToDirFMPathList(DirFMPathListCollectorPtr dirFMPathListCollector,
                                   const TwoSegmentLeg& leg)
{
  if (dirFMPathListCollector)
  {
    const OwHrtFMPair& originCxrFMPair = leg._segments[TwoSegmentLeg::ORIGIN_SEG];
    const OwHrtFMPair& destCxrFMPair = leg._segments[TwoSegmentLeg::DEST_SEG];

    CxrFareMarketsPtr owOriginFM = originCxrFMPair.getCxrFM(OW);
    CxrFareMarketsPtr hrtOriginFM = originCxrFMPair.getCxrFM(HRT);
    CxrFareMarketsPtr owDestFM = destCxrFMPair.getCxrFM(OW);
    CxrFareMarketsPtr hrtDestFM = destCxrFMPair.getCxrFM(HRT);
    if (owOriginFM)
    {
      if (owDestFM)
        dirFMPathListCollector->insert(DirFMPath::create(_trx, owOriginFM, owDestFM));
      if (hrtDestFM)
        dirFMPathListCollector->insert(DirFMPath::create(_trx, owOriginFM, hrtDestFM));
    }
    if (hrtOriginFM)
    {
      if (owDestFM)
        dirFMPathListCollector->insert(DirFMPath::create(_trx, hrtOriginFM, owDestFM));
      if (hrtDestFM)
        dirFMPathListCollector->insert(DirFMPath::create(_trx, hrtOriginFM, hrtDestFM));
    }
  }
}

bool
CxrFMCollector::addFareMarket(FareMarket* fm, const ItinIndex::Key cxrIndexKey)
{
  if (UNLIKELY(!fm))
    return false;

  const BaseParamsInfo paramsInfo(_trx, fm, cxrIndexKey);
  if (!paramsInfo.isOwEnabled() && !paramsInfo.isHrtEnabled())
    return false;

  FmVector::iterator it =
      std::find_if(_addedFareMarkets.begin(), _addedFareMarkets.end(), FareMarketsCmp(fm));

  if (it != _addedFareMarkets.end())
    return false;

  if (fm->getFmTypeSol() == FareMarket::SOL_FM_THRU)
    insertThruFM(paramsInfo);
  else
  {
    bool isConnectionCityOff(_citySet.find(fm->offMultiCity()) != _citySet.end()),
        isConnectionCityBoard(_citySet.find(fm->boardMultiCity()) != _citySet.end());

    if (fm->getSolComponentDirection() == FareMarket::SOL_COMPONENT_ORIGIN)
      insertTwoSegmentsFM(
          TwoSegmentLeg::ORIGIN_SEG,
          paramsInfo,
          MultiAirportLocationPair(fm->destination(), fm->offMultiCity(), isConnectionCityOff));
    else if (LIKELY(fm->getSolComponentDirection() == FareMarket::SOL_COMPONENT_DESTINATION))
      insertTwoSegmentsFM(
          TwoSegmentLeg::DEST_SEG,
          paramsInfo,
          MultiAirportLocationPair(fm->origin(), fm->boardMultiCity(), isConnectionCityBoard));
  }

  _addedFareMarkets.push_back(fm);
  return true;
}

void
CxrFMCollector::insertThruFM(const BaseParamsInfo& paramsInfo)
{
  _thruFM.insertFM(paramsInfo);
}

void
CxrFMCollector::insertTwoSegmentsFM(uint16_t segmentIndex,
                                    const BaseParamsInfo& paramsInfo,
                                    const MultiAirportLocationPair breakPointLocationKey)
{
  if (UNLIKELY(segmentIndex >= 2))
    return;

  typedef CxrFMMap::value_type value_type;
  typedef std::pair<CxrFMMap::iterator, bool> InsertResult;

  InsertResult insertResult =
      _twoSegmentCxrFM.insert(value_type(breakPointLocationKey, TwoSegmentLeg()));
  TwoSegmentLeg& leg = (insertResult.first)->second;
  OwHrtFMPair& segCxrFM = leg._segments[segmentIndex];

  segCxrFM.insertFM(paramsInfo);
}

void
CxrFMCollector::collectDirFMPathList(DirFMPathListCollectorPtr dirFMPathListCollector)
{
  // ThruFM
  addToDirFMPathList(dirFMPathListCollector, _thruFM);

  // Two segments leg
  for (auto& elem : _twoSegmentCxrFM)
  {
    TwoSegmentLeg& leg = elem.second;
    addToDirFMPathList(dirFMPathListCollector, leg);
  }
}
}
} // namespace tse::shpq
