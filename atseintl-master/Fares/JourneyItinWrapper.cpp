//-------------------------------------------------------------------
//
//  File:        JourneyItinWrapper.h
//  Created:     Nov 02, 2011
//
//  Description: Wrapper class for journey itin used in FVO
//
//  Copyright Sabre 2011
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

#include "Fares/JourneyItinWrapper.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"

namespace tse
{

FALLBACK_DECL(fallbackSOPUsagesResize);

JourneyItinWrapper::JourneyItinWrapper(ShoppingTrx& trx,
                                       FareMarket* fareMarket,
                                       ShoppingTrx::Leg& leg,
                                       const uint32_t legId,
                                       const uint32_t carrierKey,
                                       Itin* journeyItin)
  : _solProcessingEnabled(trx.isSumOfLocalsProcessingEnabled()),
    _fareMarket(fareMarket),
    _legId(legId),
    _carrierKey(carrierKey),
    _leg(leg),
    _dataHandle(trx.dataHandle())
{
  _trx = &trx;
  if (trx.isAltDates() && journeyItin)
  {
    _journeyItin = *journeyItin;
    _duration = ShoppingAltDateUtil::getDuration(*_journeyItin.datePair());
  }
  else
    _journeyItin = *trx.journeyItin();

  _segmentLegBeginIndex = (_leg.stopOverLegFlag()) ? _leg.jumpedLegIndices().front() : _legId;
  _segmentLegEndIndex = (_leg.stopOverLegFlag()) ? _leg.jumpedLegIndices().back() + 1 : _legId + 1;
  _segmentInsertIndexStart = _segmentLegBeginIndex;
}

JourneyItinWrapper::~JourneyItinWrapper() { eraseSegments(); }

bool
JourneyItinWrapper::initializeNormal()
{
  _journeyItin.travelSeg().erase(_journeyItin.travelSeg().begin() + _segmentLegBeginIndex,
                                 _journeyItin.travelSeg().begin() + _segmentLegEndIndex);

  return true;
}

bool
JourneyItinWrapper::initializeSol(const std::vector<TravelSeg*>& sourceSegments)
{
  if (UNLIKELY(sourceSegments.empty()))
    return false;

  const TravelSeg* originSeg = *(_journeyItin.travelSeg().begin() + _segmentLegBeginIndex);
  const TravelSeg* destinationSeg = *(_journeyItin.travelSeg().begin() + _segmentLegEndIndex - 1);

  _journeyItin.travelSeg().erase(_journeyItin.travelSeg().begin() + _segmentLegBeginIndex,
                                 _journeyItin.travelSeg().begin() + _segmentLegEndIndex);

  if (_fareMarket->origin()->loc() != originSeg->origin()->loc())
  {
    _journeyItin.travelSeg().insert(_journeyItin.travelSeg().begin() + _segmentInsertIndexStart,
                                    createOriginOpenSegment(originSeg, sourceSegments.front()));
    ++_segmentInsertIndexStart;
    ++_segmentLegEndIndex;
  }

  if (_fareMarket->destination()->loc() != destinationSeg->destination()->loc())
  {
    _journeyItin.travelSeg().insert(
        _journeyItin.travelSeg().begin() + _segmentInsertIndexStart,
        createDestinationOpenSegment(sourceSegments.back(), destinationSeg));
    ++_segmentLegEndIndex;
  }

  return true;
}

bool
JourneyItinWrapper::applySegments()
{
  if (UNLIKELY(_insertedSegmentsCount != 0 && !_fmBackupSegments.empty()))
    return false;

  // do not initialize for normal path yet
  if (LIKELY(!_initialized && _solProcessingEnabled))
  {
    _initialized = initializeSol(_fareMarket->travelSeg());
  }

  if (LIKELY(_solProcessingEnabled))
  {
    _journeyItin.travelSeg().insert(_journeyItin.travelSeg().begin() + _segmentInsertIndexStart,
                                    _fareMarket->travelSeg().begin(),
                                    _fareMarket->travelSeg().end());

    _insertedSegmentsCount = _fareMarket->travelSeg().size();
  }
  else
  {
    _fareMarket->travelSeg().swap(_fmBackupSegments);
    _fareMarket->travelSeg().clear();
    _fareMarket->travelSeg().insert(_fareMarket->travelSeg().begin(),
                                    _journeyItin.travelSeg().begin() + _segmentLegBeginIndex,
                                    _journeyItin.travelSeg().begin() + _segmentLegEndIndex);

    _insertedSegmentsCount = 0;
  }

  setupTravelSegmentCarrier();

  return true;
}

bool
JourneyItinWrapper::applySegments(const Itin* itin, const uint32_t bitIndex)
{
  if (_insertedSegmentsCount != 0 || itin->isDummy())
    return false;

  if (!_initialized)
  {
    if (LIKELY(_solProcessingEnabled))
      initializeSol(itin->travelSeg());
    else
      initializeNormal();

    _initialized = true;
  }

  if (LIKELY(_solProcessingEnabled))
    return applySegmentsSol(itin, bitIndex);

  return applySegmentsNormal(itin);
}

bool
JourneyItinWrapper::applySegmentsSol(const Itin* itin, const uint32_t bitIndex)
{
  if (UNLIKELY(itin->travelSeg().empty()))
    return false;

  ApplicableSOP* applicableSops = _fareMarket->getApplicableSOPs();
  if (UNLIKELY(!applicableSops))
    return false;

  if (!fallback::fallbackSOPUsagesResize(_trx) &&
      (applicableSops->find(_carrierKey) == applicableSops->end() ||
      (*applicableSops)[_carrierKey].size()<=bitIndex))
    return false;

  const SOPUsage& sopUsage((*applicableSops)[_carrierKey][bitIndex]);
  if (!sopUsage.applicable_ || sopUsage.startSegment_ == -1 || sopUsage.endSegment_ == -1)
    return false;

  std::vector<TravelSeg*>::const_iterator itSegmentStart(itin->travelSeg().begin() +
                                                         sopUsage.startSegment_);
  std::vector<TravelSeg*>::const_iterator itSegmentEnd(itin->travelSeg().begin() +
                                                       sopUsage.endSegment_ + 1);

  _journeyItin.travelSeg().insert(
      _journeyItin.travelSeg().begin() + _segmentInsertIndexStart, itSegmentStart, itSegmentEnd);

  _insertedSegmentsCount = (sopUsage.endSegment_ - sopUsage.startSegment_ + 1);

  _fareMarket->travelSeg().swap(_fmBackupSegments);
  _fareMarket->travelSeg().resize(sopUsage.endSegment_ - sopUsage.startSegment_ + 1);
  std::copy(itSegmentStart, itSegmentEnd, _fareMarket->travelSeg().begin());

  _fareMarket->classOfServiceVec().swap(_fmBackupClassOfService);
  _fareMarket->classOfServiceVec() = sopUsage.cos_;

  _fmBackupDirection = _fareMarket->getGlobalDirection();

  return true;
}

bool
JourneyItinWrapper::applySegmentsNormal(const Itin* itin)
{
  _journeyItin.travelSeg().insert(_journeyItin.travelSeg().begin() + _segmentInsertIndexStart,
                                  itin->travelSeg().begin(),
                                  itin->travelSeg().end());

  _insertedSegmentsCount = itin->travelSeg().size();

  _fareMarket->travelSeg().swap(_fmBackupSegments);
  _fareMarket->travelSeg() = itin->travelSeg();

  // attach class of service to fare market
  std::vector<std::vector<ClassOfService*>*>& classOfServiceVec = _fareMarket->classOfServiceVec();
  classOfServiceVec.clear();
  classOfServiceVec.reserve(_fareMarket->travelSeg().size());
  for (TravelSeg* travelSeg : _fareMarket->travelSeg())
  {
    classOfServiceVec.push_back(&(travelSeg->classOfService()));
  }

  _fmBackupDirection = _fareMarket->getGlobalDirection();

  return true;
}

void
JourneyItinWrapper::eraseSegments()
{
  if (!_fmBackupSegments.empty())
  {
    _fareMarket->travelSeg().swap(_fmBackupSegments);
    _fmBackupSegments.clear();
    _fareMarket->setGlobalDirection(_fmBackupDirection);
  }

  if (!_fmBackupClassOfService.empty())
    _fareMarket->classOfServiceVec().swap(_fmBackupClassOfService);

  if (_insertedSegmentsCount == 0)
    return;

  _journeyItin.travelSeg().erase(_journeyItin.travelSeg().begin() + _segmentInsertIndexStart,
                                 _journeyItin.travelSeg().begin() +
                                     (_segmentInsertIndexStart + _insertedSegmentsCount));

  _insertedSegmentsCount = 0;
}

TravelSeg*
JourneyItinWrapper::createOriginOpenSegment(const TravelSeg* orig, const TravelSeg* dest)
{
  AirSeg* result = _dataHandle.create<AirSeg>();

  result->arrivalDT() = DateTime::emptyDate();
  result->resStatus() = "OK";
  result->segmentType() = Open;

  result->origAirport() = orig->origAirport();
  result->departureDT() = orig->departureDT();
  result->bookingDT() = orig->bookingDT();
  result->origin() = orig->origin();
  result->destAirport() = dest->origAirport();
  result->destination() = dest->origin();
  result->boardMultiCity() = orig->boardMultiCity();
  result->offMultiCity() = dest->boardMultiCity();

  result->pssDepartureDate() = orig->departureDT().dateToSqlString();

  return result;
}

TravelSeg*
JourneyItinWrapper::createDestinationOpenSegment(const TravelSeg* orig, const TravelSeg* dest)
{
  AirSeg* result = _dataHandle.create<AirSeg>();

  result->arrivalDT() = DateTime::emptyDate();
  result->resStatus() = "OK";
  result->segmentType() = Open;

  result->origAirport() = orig->destAirport();
  result->departureDT() = orig->departureDT();
  result->bookingDT() = orig->bookingDT();
  result->origin() = orig->destination();
  result->destAirport() = dest->destAirport();
  result->destination() = dest->destination();
  result->boardMultiCity() = orig->offMultiCity();
  result->offMultiCity() = dest->offMultiCity();

  result->pssDepartureDate() = orig->departureDT().dateToSqlString();

  return result;
}

void
JourneyItinWrapper::setupTravelSegmentCarrier()
{
  std::vector<TravelSeg*>::iterator it(_journeyItin.travelSeg().begin());
  for (; it != _journeyItin.travelSeg().end(); ++it)
  {
    AirSeg* airSeg(dynamic_cast<AirSeg*>(*it));
    if (LIKELY(airSeg))
      airSeg->carrier() = _fareMarket->governingCarrier();
  }
}
}
