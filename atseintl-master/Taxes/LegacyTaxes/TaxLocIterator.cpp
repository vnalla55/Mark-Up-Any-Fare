#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"


namespace tse
{

void
TaxLocIterator::initialize(const FarePath& farePath)
{
  _farePath = &farePath;
  _fareBreaksFound = false;
  _turnAroundPointsFound = false;
  _travelSegs = farePath.itin()->travelSeg();
  _numOfSegs = _travelSegs.size();
  toFront();
}

uint16_t
TaxLocIterator::getNumOfHiddenStops()
{
  if (!_skipHidden)
    return _travelSegs[_segNo]->hiddenStops().size();
  else
    return 0;
}

void
TaxLocIterator::toFront()
{
  _segNo = 0;
  _subSegNo = 0;
  _numOfSubSegs = getNumOfHiddenStops();
}

bool
TaxLocIterator::hasNext() const
{
  return (_segNo < _numOfSegs);
}

bool
TaxLocIterator::hasPrevious() const
{
  return (_subSegNo > 0 || _segNo > 0);
}

bool
TaxLocIterator::isEnd() const
{
  return _segNo > _numOfSegs;
}

void
TaxLocIterator::next()
{
  if (_subSegNo < _numOfSubSegs)
  {
    ++_subSegNo;
  }
  else
  {
    ++_segNo;
    _subSegNo = 0;
    if (_segNo < _numOfSegs)
      _numOfSubSegs = getNumOfHiddenStops();
    else
      _numOfSubSegs = 0;
  }
}

void
TaxLocIterator::previous()
{
  if (_subSegNo > 0)
  {
    --_subSegNo;
  }
  else
  {
    --_segNo;
    _numOfSubSegs = getNumOfHiddenStops();
    _subSegNo = _numOfSubSegs;
  }
}

void
TaxLocIterator::toBack()
{
  _segNo = _numOfSegs;
  _numOfSubSegs = 0;
  _subSegNo = _numOfSubSegs;
}

void
TaxLocIterator::toSegmentNo(uint16_t s)
{
  _segNo = s;
  _numOfSubSegs = getNumOfHiddenStops();
  _subSegNo = 0;
}

uint16_t
TaxLocIterator::prevSegNo() const
{
  if (_subSegNo > 0)
    return _segNo;
  else
    return _segNo - 1;
}

uint16_t
TaxLocIterator::nextSegNo() const
{
  return _segNo;
}

TravelSeg*
TaxLocIterator::prevSeg() const
{
  return _travelSegs[prevSegNo()];
}

TravelSeg*
TaxLocIterator::nextSeg() const
{
  return _travelSegs[nextSegNo()];
}

uint16_t
TaxLocIterator::subSegNo() const
{
  return _subSegNo;
}

const Loc*
TaxLocIterator::loc() const
{
  if (_segNo == _numOfSegs)
    return _travelSegs[_segNo - 1]->destination();

  if (_subSegNo > 0)
    return _travelSegs[_segNo]->hiddenStops()[_subSegNo - 1];

  return _travelSegs[_segNo]->origin();
}

const Loc*
TaxLocIterator::locDeplanement() const
{
  if (_subSegNo > 0)
    return _travelSegs[_segNo]->hiddenStops()[_subSegNo - 1];

  if (_segNo == 0)
    return _travelSegs[_segNo]->origin();

  return _travelSegs[_segNo - 1]->destination();
}

bool
TaxLocIterator::isInLoc(LocTypeCode type, const LocCode& code, Indicator excl,
    PricingTrx& trx, bool deplane) const
{
  bool locMatch = LocUtil::isInLoc(deplane ? *locDeplanement() : *loc(),
                                   type,
                                   code,
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::TAXES,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   trx.getRequest()->ticketingDT());

  if (locMatch)
    return (type == LOCTYPE_ZONE || excl != YES);
  else
    return excl == YES;
}

bool
TaxLocIterator::isInLoc1(TaxCodeReg& taxCodeReg, PricingTrx& trx) const
{
    return isInLoc(taxCodeReg.loc1Type(),
      taxCodeReg.loc1(),
      taxCodeReg.loc1ExclInd(),
      trx,
      false);
}

bool
TaxLocIterator::isInLoc2(TaxCodeReg& taxCodeReg, PricingTrx& trx) const
{
    return isInLoc(taxCodeReg.loc2Type(),
      taxCodeReg.loc2(),
      taxCodeReg.loc2ExclInd(),
      trx,
      true);
}

bool
TaxLocIterator::isStop()
{
  if (!hasPrevious() || !hasNext())
    return true;

  if (_subSegNo != 0)
    return false;

  if (_surfaceAsStop && (!isNextSegAirSeg() || !isPrevSegAirSeg()))
    return true;

  return (nextSeg()->isStopOver(prevSeg(), _stopHours * 3600) || prevSeg()->isForcedStopOver()) &&
         !(prevSeg()->isForcedConx());
}

bool
TaxLocIterator::isNextSegAirSeg() const
{
  if (LIKELY(hasNext()))
  {
    if (UNLIKELY(_treatTrainBusAsNonAir
        && (nextSeg()->equipmentType() == BUS || nextSeg()->equipmentType() == TRAIN)))
      return false;

    return nextSeg()->isAir() && nextSeg()->segmentType() != Surface &&
           nextSeg()->segmentType() != Arunk;
  }

  return false;
}

bool
TaxLocIterator::isPrevSegAirSeg() const
{
  if (hasPrevious())
  {
    if (UNLIKELY(_treatTrainBusAsNonAir
        && (prevSeg()->equipmentType() == BUS || prevSeg()->equipmentType() == TRAIN)))
      return false;

    return prevSeg()->isAir() && prevSeg()->segmentType() != Surface &&
           prevSeg()->segmentType() != Arunk;
  }

  return false;
}

bool
TaxLocIterator::isFareBreak()
{
  if (!_fareBreaksFound)
  {
    _fareBreaks.clear();
    taxUtil::findFareBreaks(_fareBreaks, *_farePath);
    _fareBreaksFound = true;
  }

  if (hasPrevious() && _subSegNo == 0)
    return (_fareBreaks.find(_segNo - 1) != _fareBreaks.end());
  else
    return false;
}

bool
TaxLocIterator::isHidden()
{
  return _subSegNo > 0;
}

void
TaxLocIterator::findTurnAroundPoints()
{
  uint16_t savedSegNo = _segNo;
  uint16_t savedSubSegNo = _subSegNo;

  _turnAroundPoints.clear();

  toFront();
  const Loc* location[3] = { nullptr, nullptr, nullptr };
  location[2] = loc();
  uint32_t edge[2] = { 0, 0 };
  uint16_t i = 0;

  while (hasNext())
  {
    uint16_t prevSegNo = _segNo;

    do
      next();
    while (hasNext() && !isStop() && !isFareBreak());

    if (_turnAroundAfterSurface)
    {
      while (hasNext() && !isNextSegAirSeg())
        next();
    }

    location[0] = location[1];
    location[1] = location[2];
    location[2] = loc();

    edge[0] = edge[1];
    edge[1] = TseUtil::greatCircleMiles(*location[1], *location[2]);

    if (i > 0)
    {
      uint32_t edge2 = TseUtil::greatCircleMiles(*location[0], *location[2]);
      if (edge[0] > edge2 && edge[1] > edge2)
        _turnAroundPoints.insert(prevSegNo);
    }
    ++i;
  }

  _segNo = savedSegNo;
  _subSegNo = savedSubSegNo;
}

bool
TaxLocIterator::isTurnAround()
{
  if (!_turnAroundPointsFound)
  {
    findTurnAroundPoints();
    _turnAroundPointsFound = true;
  }

  return (_turnAroundPoints.find(_segNo) != _turnAroundPoints.end());
}

bool
TaxLocIterator::isMirror()
{
  if (!(hasNext() && hasPrevious()))
    return false;

  previous();
  const Loc* prevLoc = loc();
  next();
  next();
  const Loc* nextLoc = locDeplanement();
  previous();

  if (prevLoc == nextLoc)
    return true;

  if (loc()->nation() != prevLoc->nation() && prevLoc->nation() == nextLoc->nation())
    return true;

  return false;
}
}
