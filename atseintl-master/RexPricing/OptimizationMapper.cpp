#include "RexPricing/OptimizationMapper.h"
#include "Diagnostic/DiagCollector.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"

namespace tse
{

void
OptimizationMapper::processMapping(const ExcItin& excItin,
                                   const Itin& newItin,
                                   DiagCollector* dc)
{
  const std::vector<FareCompInfo*>& fareCompVec = excItin.fareComponent();
  const std::vector<TravelSeg*>& newSegs = newItin.travelSeg();
  _matchedFCs.resize(fareCompVec.size());
  _matchedTvlSegs.resize(newSegs.size());
  _dc = dc;

  uint16_t i = 0;
  for (const TravelSeg* seg : newSegs)
  {
    _tvlPtrToId.emplace(seg, i++);
  }

  mapWith(CityMapper(), excItin, newItin);
  auto fcMatchingCheck = [](bool value) { return value; };

  printMap("City Mapper:");
  bool finished = std::all_of(_matchedFCs.begin(), _matchedFCs.end(), fcMatchingCheck);

  if (!finished)
  {
    mapWith(CountryMapper(), excItin, newItin);
    printMap("Country Mapper:");
    finished = std::all_of(_matchedFCs.begin(), _matchedFCs.end(), fcMatchingCheck);
  }

  if (!finished)
  {
    mapWith(SubAreaMapper(), excItin, newItin);
    printMap("SubArea Mapper:");
    finished = std::all_of(_matchedFCs.begin(), _matchedFCs.end(), fcMatchingCheck);
  }

  printUnmapped(fareCompVec, newSegs);
}

void
OptimizationMapper::mapWith(const MapperBase& mapper,
                            const ExcItin& excItin,
                            const Itin& newItin)
{
  uint16_t counter = 0;
  for (const FareCompInfo* fci : excItin.fareComponent())
  {
    if (_matchedFCs[counter++])
      continue;

    Direction fcDirection = getDirection(excItin.furthestPointSegmentOrder(), fci);

    bool fciMatched = false;
    for (auto origIter = newItin.travelSeg().begin(); origIter != newItin.travelSeg().end(); ++origIter)
    {
      if (_matchedTvlSegs[_tvlPtrToId.at(*origIter)])
        continue;

      Direction segDirection = getDirection(newItin.furthestPointSegmentOrder(),
                                            _tvlPtrToId.at(*origIter) + 1);

      if (mapper(fci->fareMarket()->origin(), (*origIter)->origin()) &&
          isDirectionMatched(fcDirection, segDirection))
      {
        for (auto destIter = origIter; destIter != newItin.travelSeg().end(); ++destIter)
        {
          if (_matchedTvlSegs[_tvlPtrToId.at(*destIter)])
            continue;

          segDirection = getDirection(newItin.furthestPointSegmentOrder(),
                                      _tvlPtrToId.at(*destIter) + 1);

          if (mapper(fci->fareMarket()->destination(), (*destIter)->destination())
              && isDirectionMatched(fcDirection, segDirection))
          {
            std::vector<TravelSeg*> matchedSegs(origIter, destIter + 1);

            for (const TravelSeg* seg : matchedSegs)
            {
              _matchedTvlSegs[_tvlPtrToId.at(seg)] = true;
            }

            _fcToSegs.emplace(fci, matchedSegs);
            fciMatched = true;
            break;
          }
        }
      }
    }
    _matchedFCs[counter - 1] = fciMatched;
  }
}

OptimizationMapper::Direction
OptimizationMapper::getDirection(uint16_t furthestPoint, uint16_t curPos) const
{
  return curPos <= furthestPoint ? OUTBOUND : INBOUND;
}

OptimizationMapper::Direction
OptimizationMapper::getDirection(uint16_t furthestPoint, const FareCompInfo* fci) const
{
  const std::vector<TravelSeg*> tvlSegs = fci->fareMarket()->travelSeg();
  if (tvlSegs.front()->pnrSegment() <= furthestPoint &&
      tvlSegs.back()->pnrSegment() <= furthestPoint)
  {
    return OUTBOUND;
  }
  else if(tvlSegs.front()->pnrSegment() > furthestPoint &&
          tvlSegs.back()->pnrSegment() > furthestPoint)
  {
    return INBOUND;
  }
  else
  {
    return BIDIRECTIONAL;
  }
}

bool
OptimizationMapper::isDirectionMatched(Direction fcDirection, Direction segDirection) const
{
  return fcDirection == BIDIRECTIONAL || fcDirection == segDirection;
}


void
OptimizationMapper::addCarrierRestriction(const FareCompInfo* fci,
                                          std::set<CarrierCode> carriers)
{
  auto posIter = _fcToSegs.find(fci);

  if (posIter != _fcToSegs.end())
  {
    if (_dc)
    {
      *_dc << "ADDING CARRIER RESTRICTIONS FOR FC" << fci->fareCompNumber() << "\n";
      std::for_each(carriers.begin(), carriers.end(), [&](CarrierCode cxr){ *_dc << "  " << cxr; });
      *_dc << "\nIN TRAVEL SEGMENTS:\n";
      std::for_each(posIter->second.begin(), posIter->second.end(),
                    [&](const TravelSeg* seg){ *_dc << "    " << seg->origin()->loc()
                                                    << "-" << seg->destination()->loc() << "\n";});
    }

    for (const TravelSeg* seg : posIter->second)
    {
      _newTvlSegToCxr[seg] = carriers;
    }
  }
}

void
OptimizationMapper::addTariffRestriction(const FareCompInfo* fci, const PaxTypeFare* ptf)
{
  auto posIter = _fcToSegs.find(fci);

  if (posIter != _fcToSegs.end())
  {
    bool privateOnly = (ptf->tcrTariffCat() == PRIVATE_TARIFF);
    if (_dc)
    {
      *_dc << "ADDING TARIFF RESTRICTIONS FOR FC" << fci->fareCompNumber() << " - "
           << ( privateOnly ? "PRIVATE" : "PUBLIC") << " TAFFIC AVAILABLE\n";
      *_dc << "\nIN TRAVEL SEGMENTS:\n";
      std::for_each(posIter->second.begin(), posIter->second.end(),
                    [&](const TravelSeg* seg){ *_dc << "    " << seg->origin()->loc()
                                                    << "-" << seg->destination()->loc() << "\n";});
    }

    for (const TravelSeg* seg : posIter->second)
    {
      _newTvlSegToTariffType[seg] = privateOnly;
    }
  }
}

bool
OptimizationMapper::isFareMarketAllowed(const FareMarket* fm) const
{
  CarrierCode govCxr = fm->governingCarrier();

  for (const TravelSeg* seg : fm->travelSeg())
  {
    auto posIter = _newTvlSegToCxr.find(seg);

    if (posIter != _newTvlSegToCxr.end())
    {
      return posIter->second.count(govCxr);
    }
  }
  return true;
}

OptimizationMapper::TariffRestr
OptimizationMapper::isFMRestrToPrivOrPub(const FareMarket* fm) const
{
  for (const TravelSeg* seg : fm->travelSeg())
  {
    auto posIter = _newTvlSegToTariffType.find(seg);

    if (posIter != _newTvlSegToTariffType.end())
    {
      return posIter->second ? PRIVATE : PUBLIC;
    }
  }
  return UNRESTRICTED;
}

void
OptimizationMapper::printMap(std::string header) const
{
  if (!_dc)
    return;

  *_dc << "\n" << header << "\n";
  for (auto item : _fcToSegs)
  {
    *_dc << "FC" << item.first->fareCompNumber() << " "
         << item.first->fareMarket()->toString() << "\n";

    for (auto tvlSeg : item.second)
    {
      *_dc << "   " << tvlSeg->origin()->loc() << "-"
           << tvlSeg->destination()->loc() << "\n";
    }
  }
}

void
OptimizationMapper::printUnmapped(const std::vector<FareCompInfo*>& fcInfos,
                                  const std::vector<TravelSeg*>& newSegs) const
{
  if (!_dc)
    return;

  *_dc << "UNMAPPED FARE COMPONENTS:\n";
  for (uint32_t i = 0; i < fcInfos.size(); ++i)
  {
    if (!_matchedFCs[i])
      *_dc << "FC" << fcInfos[i]->fareCompNumber() << " "
           << fcInfos[i]->fareMarket()->toString() << "\n";
  }

  *_dc << "\nUNMAPPED TRAVEL SEGMENTS:\n";
  for (uint32_t i = 0; i < newSegs.size(); ++i)
  {
    if (!_matchedTvlSegs[i])
      *_dc << "   " << newSegs[i]->origin()->loc() << "-"
           << newSegs[i]->destination()->loc() << "\n";
  }
}


}
