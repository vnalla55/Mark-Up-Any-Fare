//-------------------------------------------------------------------
//
//  File:         TSIGateway.cpp
//  Author:       Simon Li
//  Created:      05/12/2009
//  Description:
//
//  Copyright Sabre 2009
//
//        The copyright to the computer program(s) herein
//        is the property of Sabre.
//        The program(s) may be used and/or copied only with
//        the written permission of Sabre or in accordance
//        with the terms and conditions stipulated in the
//        agreement/contract under which the program(s)
//        have been supplied.
//
//-------------------------------------------------------------------

#include "Rules/TSIGateway.h"

#include "Common/LocPredicates.h"
#include "Common/LocUtil.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

namespace tse
{

bool
TSIGateway::markGwRtw(MarkGWType markType, const std::vector<TravelSeg*>& segs)
{
  if (segs.empty())
    return false;
  _savedTvlSegs = segs;
  _tvlSegs = &_savedTvlSegs;
  _markGWType = markType;

  return markGwByPred(AreasNotEqual()) || markGwByPred(ATPReservedZonesNotEqual()) ||
         markGwByPred(NationsNotEqual());
}

template <class GwPred>
bool
TSIGateway::markGwByPred(const GwPred& gwPred)
{
  const Loc* prevLoc = _tvlSegs->front()->origin();
  uint16_t gwLocIndex = 0;

  for (const TravelSeg* ts : *_tvlSegs)
  {
    if (gwPred(prevLoc, ts->destination()))
    {
      _gwLocIndex.insert(gwLocIndex);
      _gwLocIndex.insert(gwLocIndex + 1u);
    }

    prevLoc = ts->destination();
    ++gwLocIndex;
  }

  return foundGW();
}

bool
TSIGateway::markGW(TSIGateway::MarkGWType markType, const std::vector<TravelSeg*>& tvlSegs)
{
  _tvlSegs = &tvlSegs;
  if (_tvlSegs->empty())
    return false;

  _markGWType = markType;

  const TravelSeg& orig = *_tvlSegs->front();
  const TravelSeg& dest = *_tvlSegs->back();

  const IATAAreaCode& origArea = orig.origin()->area();
  const IATAAreaCode& destArea = dest.destination()->area();

  if (origArea != destArea)
  {
    markGWBtwAreas(origArea, destArea);
    return true;
  }

  if (!LocUtil::isWithinOneATPReservedZone(*_tvlSegs))
  {
    markGWBtwZones(orig, dest);
    return true;
  }

  markGWBtwNations(orig.origin()->nation(), dest.destination()->nation());

  return foundGW();
}

void
TSIGateway::markGWBtwAreas(const IATAAreaCode& origArea, const IATAAreaCode& destArea)
{
  uint16_t gwLocIndex = 0;
  IATAAreaCode currentArea = origArea;

  std::vector<TravelSeg*>::const_iterator it = _tvlSegs->begin();
  std::vector<TravelSeg*>::const_iterator ite = _tvlSegs->end();

  for (; it != ite; ++it, ++gwLocIndex)
  {
    if (UNLIKELY(!(*it)))
      continue;

    if ((*it)->destination()->area() != currentArea)
    {
      _gwLocIndex.insert(gwLocIndex);
      _gwLocIndex.insert(gwLocIndex + 1);
      currentArea = (*it)->destination()->area();

      if (currentArea == destArea)
        break;
    }
  }
}

void
TSIGateway::markGWBtwZones(const TravelSeg& orig, const TravelSeg& dest)
{
  uint16_t gwLocIndex = 0;
  const NationCode& destNation = dest.destination()->nation();

  std::vector<TravelSeg*>::const_iterator it = _tvlSegs->begin();
  std::vector<TravelSeg*>::const_iterator ite = _tvlSegs->end();

  NationCode currentNation = orig.origin()->nation();

  for (; it != ite; ++it, ++gwLocIndex)
  {
    if (UNLIKELY(!(*it)))
      continue;

    const NationCode& tsDestNation = (*it)->destination()->nation();

    if (!LocUtil::areNationsInSameATPReservedZone(tsDestNation, currentNation))
    {
      _gwLocIndex.insert(gwLocIndex);
      _gwLocIndex.insert(gwLocIndex + 1);
      currentNation = (*it)->destination()->nation();

      if (currentNation == destNation)
        break;
    }
  }
}

void
TSIGateway::markGWBtwNations(const NationCode& origNation, const NationCode& destNation)
{
  uint16_t gwLocIndex = 0;
  std::vector<TravelSeg*>::const_iterator it = _tvlSegs->begin();
  std::vector<TravelSeg*>::const_iterator ite = _tvlSegs->end();

  NationCode currentNation = origNation;

  for (; it != ite; ++it, ++gwLocIndex)
  {
    if (UNLIKELY(!(*it)))
      continue;

    const NationCode& tsDestNation = (*it)->destination()->nation();

    if (tsDestNation != currentNation)
    {
      _gwLocIndex.insert(gwLocIndex);
      _gwLocIndex.insert(gwLocIndex + 1);
      currentNation = (*it)->destination()->nation();

      if (currentNation == destNation)
        break;
    }
  }
}

bool
TSIGateway::markGW(TSIGateway::MarkGWType markType,
                   const std::vector<TravelSeg*>& outTvlSegs,
                   const std::vector<TravelSeg*>& inTvlSegs)
{
  _savedTvlSegs.insert(_savedTvlSegs.end(), outTvlSegs.begin(), outTvlSegs.end());
  markGW(markType, _savedTvlSegs);

  TSIGateway tmpTSIGW;
  tmpTSIGW.markGW(markType, inTvlSegs);

  return this->combineInboundGW(tmpTSIGW);
}

bool
TSIGateway::combineInboundGW(const TSIGateway& inboundGW)
{
  if (inboundGW._markGWType == MARK_NONE || inboundGW._gwLocIndex.empty())
    return foundGW();

  std::vector<TravelSeg*>* tvlSegs = const_cast<std::vector<TravelSeg*>*>(_tvlSegs);
  if (inboundGW._markGWType != MARK_NONE && !tvlSegs->empty())
  {
    tvlSegs->push_back(nullptr);
  }

  const uint16_t indexShift = tvlSegs->size();
  tvlSegs->insert(tvlSegs->end(), inboundGW._tvlSegs->begin(), inboundGW._tvlSegs->end());
  std::set<uint16_t>::const_iterator inGWLocI = inboundGW._gwLocIndex.begin();
  std::set<uint16_t>::const_iterator inGWLocIEnd = inboundGW._gwLocIndex.end();

  for (; inGWLocI != inGWLocIEnd; ++inGWLocI)
  {
    _gwLocIndex.insert((*inGWLocI) + indexShift);
  }

  return foundGW();
}

bool
TSIGateway::isDepartureFromGW(const TravelSeg* ts) const
{
  if (_markGWType == MARK_ORIG_GATEWAY)
    return (ts == tsDepartFromOrigGW());

  std::set<uint16_t>::const_iterator gwI = _gwLocIndex.begin();
  const std::set<uint16_t>::const_iterator gwIEnd = _gwLocIndex.end();

  const uint16_t tvlSegSz = _tvlSegs->size();

  for (; gwI != gwIEnd; ++gwI)
  {
    if ((*gwI) >= tvlSegSz)
      break;

    if ((*_tvlSegs)[*gwI] == ts)
      return true;
  }
  return false;
}

bool
TSIGateway::isArrivalOnGW(const TravelSeg* ts) const
{
  if (UNLIKELY(_markGWType == MARK_DEST_GATEWAY))
    return (ts == tsArriveOnDestGW());

  std::set<uint16_t>::const_reverse_iterator gwI = _gwLocIndex.rbegin();
  const std::set<uint16_t>::const_reverse_iterator gwIEnd = _gwLocIndex.rend();
  for (; gwI != gwIEnd; ++gwI)
  {
    if (*gwI == 0)
      continue;

    if ((*_tvlSegs)[(*gwI) - 1] == ts)
      return true;
  }
  return false;
}

TravelSeg*
TSIGateway::tsDepartFromOrigGW() const
{
  std::set<uint16_t>::const_iterator gwI = _gwLocIndex.begin();
  if (gwI == _gwLocIndex.end())
    return nullptr;

  return ((*_tvlSegs)[*gwI]);
}

TravelSeg*
TSIGateway::tsArriveOnDestGW() const
{
  std::set<uint16_t>::const_reverse_iterator gwI = _gwLocIndex.rbegin();
  if (gwI == _gwLocIndex.rend())
    return nullptr;

  return ((*_tvlSegs)[*gwI - 1]);
}

inline bool
TSIGateway::foundGW() const
{
  return !_gwLocIndex.empty();
}

}
