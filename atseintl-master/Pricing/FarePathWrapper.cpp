// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Pricing/FarePathWrapper.h"

#include "Common/ItinUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"

#include <assert.h>

#include <boost/range/adaptors.hpp>

namespace tse
{

void
FarePathWrapper::buildFarePath()
{
  _farePath = _source.trx.dataHandle().create<FarePath>();

  initializeFarePath();
  populateFarePath();

  if (_source.isRoundTripEnabled())
    completeRoundTrip();
}

void
FarePathWrapper::removeMirrorFareUsage() const
{
  _source.removeMirrorFareUsage();
  std::vector<FareUsage*>& fuVec = _farePath->pricingUnit().front()->fareUsage();
  std::vector<TravelSeg*>& segments = _farePath->pricingUnit().front()->travelSeg();

  if (_origDirection == FMDirection::OUTBOUND)
  {
    fuVec.pop_back();
    segments.resize(segments.size() / 2);
  }
  else
  {
    fuVec.erase(fuVec.begin());
    segments.erase(segments.begin(), segments.begin() + _farePath->itin()->travelSeg().size() / 2);
  }

  _farePath->itin()->travelSeg() = segments;
}

void
FarePathWrapper::initializeFarePath()
{
  _source.fillWithItin(*_farePath);
  PricingUnit* pricingUnit = _source.trx.dataHandle().create<PricingUnit>();
  FareUsage* fareUsage = _source.trx.dataHandle().create<FareUsage>();

  pricingUnit->fareUsage().push_back(fareUsage);

  _farePath->pricingUnit().push_back(pricingUnit);
}

void
FarePathWrapper::populateFarePath()
{
  FareUsage& fareUsage = *_farePath->pricingUnit().front()->fareUsage().front();
  PricingUnit& pricingUnit = *_farePath->pricingUnit().front();
  Itin& itin = *_farePath->itin();

  fareUsage.paxTypeFare() = &_source.paxTypeFare;

  fareUsage.inbound() = (_source.paxTypeFare.fareMarket()->direction() == FMDirection::INBOUND);

  fareUsage.travelSeg() = _source.paxTypeFare.fareMarket()->travelSeg();
  pricingUnit.travelSeg() = _source.paxTypeFare.fareMarket()->travelSeg();

  fareUsage.segmentStatus() = _source.getSegmentStatus();

  _farePath->setTotalNUCAmount(_source.paxTypeFare.totalFareAmount());

  _farePath->paxType() = _source.getPaxType();
  pricingUnit.paxType() = _farePath->paxType();

  _farePath->baseFareCurrency() = itin.calculationCurrency();
  _farePath->calculationCurrency() = itin.calculationCurrency();
}

void
FarePathWrapper::completeRoundTrip() const
{
  PricingUnit& pricingUnit = *_farePath->pricingUnit().front();
  FareUsage& fareUsage = *pricingUnit.fareUsage().front();
  Itin& itin = *_farePath->itin();

  FareUsage& mirrorFareUsage = createMirrorFareUsage(fareUsage);

  if (_origDirection == FMDirection::OUTBOUND)
  {
    pricingUnit.fareUsage().push_back(&mirrorFareUsage);
    pricingUnit.travelSeg().insert(pricingUnit.travelSeg().end(),
                                   mirrorFareUsage.travelSeg().begin(),
                                   mirrorFareUsage.travelSeg().end());
  }
  else
  {
    pricingUnit.fareUsage().insert(pricingUnit.fareUsage().begin(), &mirrorFareUsage);
    pricingUnit.travelSeg().insert(pricingUnit.travelSeg().begin(),
                                   mirrorFareUsage.travelSeg().begin(),
                                   mirrorFareUsage.travelSeg().end());
  }
  pricingUnit.puType() = PricingUnit::Type::ROUNDTRIP;

  itin.travelSeg() = pricingUnit.travelSeg();

  {
    TravelSegAnalysis tvlSegAnalysis;
    const Boundary tvlBoundary = tvlSegAnalysis.selectTravelBoundary(itin.travelSeg());
    ItinUtil::setGeoTravelType(tvlSegAnalysis, tvlBoundary, itin);
  }
  _source.rtUpdateValidatingCarrier(itin);

  if (_origDirection == FMDirection::OUTBOUND)
    itin.fareMarket().push_back(fareUsage.paxTypeFare()->fareMarket());
  else
    itin.fareMarket().insert(itin.fareMarket().begin(), fareUsage.paxTypeFare()->fareMarket());
}

FareUsage&
FarePathWrapper::createMirrorFareUsage(FareUsage& fareUsage) const
{
  FareUsage& mirrorFareUsage = *_source.trx.dataHandle().create<FareUsage>();

  mirrorFareUsage.paxTypeFare() = fareUsage.paxTypeFare();

  const DateTime travelDate =
      (_origDirection == FMDirection::OUTBOUND ? fareUsage.travelSeg().back()->arrivalDT()
                                              : fareUsage.travelSeg().front()->departureDT());

  for (TravelSeg* segment : boost::adaptors::reverse(fareUsage.travelSeg()))
  {
    AirSeg* airSegment = segment->toAirSeg();
    if (UNLIKELY(!airSegment))
      continue;

    AirSeg* newSegment = _source.trx.dataHandle().create<AirSeg>();
    newSegment->origin() = airSegment->destination();
    newSegment->destination() = airSegment->origin();
    newSegment->boardMultiCity() = airSegment->offMultiCity();
    newSegment->offMultiCity() = airSegment->boardMultiCity();
    newSegment->departureDT() = travelDate;
    newSegment->arrivalDT() = travelDate;
    newSegment->carrier() = airSegment->carrier();
    mirrorFareUsage.travelSeg().push_back(newSegment);
  }

  mirrorFareUsage.segmentStatus().assign(fareUsage.segmentStatus().rbegin(),
                                         fareUsage.segmentStatus().rend());

  return mirrorFareUsage;
}

FarePathWrapper::FareMarketBackup::FareMarketBackup(FareMarket& fm)
  : _fareMarket(fm),
    _fareMarketTravelSegOrg(std::move(fm.travelSeg())),
    _fareMarketDirectionOrg(fm.direction())
{
}

void
FarePathWrapper::FareMarketBackup::setFareMarket(std::vector<TravelSeg*> segments,
                                                 FMDirection dir)
{
  _fareMarket.travelSeg() = std::move(segments);

  if (_fareMarket.direction() == FMDirection::UNKNOWN)
    _fareMarket.direction() = dir;
}

void
FarePathWrapper::FareMarketBackup::restoreFM()
{
  _fareMarket.travelSeg() = std::move(_fareMarketTravelSegOrg);
  _fareMarket.direction() = _fareMarketDirectionOrg;
}

FMDirection
FarePathWrapperSource::origDirection()
{
  return paxTypeFare.fareMarket()->legIndex() == 0 ? FMDirection::OUTBOUND : FMDirection::INBOUND;
}

std::vector<PaxTypeFare::SegmentStatus>
FarePathWrapperSource::getSegmentStatus()
{
  return paxTypeFare.segmentStatus();
}

PaxType*
FarePathWrapperSource::getPaxType()
{
  if (LIKELY(paxTypeFare.actualPaxType()))
    return paxTypeFare.actualPaxType();
  return trx.paxType().front();
}

void
FarePathWrapperSource::removeMirrorFareUsage()
{
  assert(_isRoundTripEnabled);
  _isRoundTripEnabled = false;
}

AltDatesFarePathWrapperSource::AltDatesFarePathWrapperSource(PricingTrx& trx,
                                                             PaxTypeFare& ptf,
                                                             const Itin& itin,
                                                             PaxType* paxType)
  : FarePathWrapperSource(trx, ptf), itin(itin), _paxType(paxType)
{
  _isRoundTripEnabled = itin.simpleTrip() && (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ||
                                              paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);
}

void
AltDatesFarePathWrapperSource::fillWithItin(FarePath& farePath)
{
  Itin* itin = trx.dataHandle().create<Itin>();
  itin->duplicate(this->itin, paxTypeFare, trx.dataHandle());

  itin->farePath().push_back(&farePath);
  farePath.itin() = itin;

  itin->ticketingCarrier() = paxTypeFare.carrier();
}

PaxType*
AltDatesFarePathWrapperSource::getPaxType()
{
  if (LIKELY(_paxType))
    return _paxType;
  return FarePathWrapperSource::getPaxType();
}

void
AltDatesFarePathWrapperSource::rtUpdateValidatingCarrier(Itin& itin)
{
  ValidatingCarrierUpdater valCxrUpdater(trx);
  valCxrUpdater.legacyProcess(itin);
}

namespace
{
bool
isThruFare(PaxTypeFare& paxTypeFare)
{
  return (paxTypeFare.fareMarket()->getFmTypeSol() != FareMarket::SOL_FM_LOCAL);
}
}

SoloFarePathWrapperSource::SoloFarePathWrapperSource(ShoppingTrx& trx,
                                                     PaxTypeFare& ptf,
                                                     SOPUsage& sopUsage,
                                                     uint32_t bitIndex)
  : FarePathWrapperSource(trx, ptf),
    sopUsage(sopUsage),
    bitIndex(bitIndex),
    _fareMarketBackup(*ptf.fareMarket())
{
  _isRoundTripEnabled = shoppingTrx().legs().size() == 2 && isThruFare(paxTypeFare) &&
                        (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ||
                         paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);

  setFareMarket();
}

ShoppingTrx&
SoloFarePathWrapperSource::shoppingTrx()
{
  return static_cast<ShoppingTrx&>(trx);
}

void
SoloFarePathWrapperSource::fillWithItin(FarePath& farePath)
{
  Itin* itin = sopUsage.itin_;

  if (_isRoundTripEnabled)
  {
    itin = trx.dataHandle().create<Itin>();
    itin->duplicate(*sopUsage.itin_, paxTypeFare, trx.dataHandle());
  }

  farePath.itin() = itin;
}

std::vector<PaxTypeFare::SegmentStatus>
SoloFarePathWrapperSource::getSegmentStatus()
{
  PaxTypeFare::FlightBit& bitMap = paxTypeFare.flightBitmap()[bitIndex];
  return bitMap._segmentStatus;
}

void
SoloFarePathWrapperSource::rtUpdateValidatingCarrier(Itin& itin)
{
  ValidatingCarrierUpdater valCxrUpdater(trx);
  valCxrUpdater.update(itin);
}

void
SoloFarePathWrapperSource::setFareMarket()
{
  std::vector<TravelSeg*> travelSegs(sopUsage.itin_->travelSeg().begin() + sopUsage.startSegment_,
                                     sopUsage.itin_->travelSeg().begin() + sopUsage.endSegment_ +
                                         1);

  const FMDirection dir =
      (paxTypeFare.directionality() == FROM ? FMDirection::INBOUND : FMDirection::OUTBOUND);

  _fareMarketBackup.setFareMarket(std::move(travelSegs), dir);
}

ShoppingFarePathWrapperSource::ShoppingFarePathWrapperSource(ShoppingTrx& trx,
                                                             PaxTypeFare& ptf,
                                                             PaxType* paxType,
                                                             uint32_t bitIndex)
  : FarePathWrapperSource(trx, ptf),
    bitIndex(bitIndex),
    _fareMarketBackup(*ptf.fareMarket()),
    _paxType(paxType)
{
  _isRoundTripEnabled = shoppingTrx().isSimpleTrip() && shoppingTrx().legs().size() == 2 &&
                        (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ||
                         paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);
}

ShoppingTrx&
ShoppingFarePathWrapperSource::shoppingTrx()
{
  return static_cast<ShoppingTrx&>(trx);
}

void
ShoppingFarePathWrapperSource::initWithCell(const ItinIndex::ItinCell& cell)
{
  _itin = trx.dataHandle().create<Itin>();
  _itin->duplicate(*cell.second, paxTypeFare, trx.dataHandle());

  initItin();

  const FMDirection dir =
      (paxTypeFare.directionality() == FROM ? FMDirection::INBOUND : FMDirection::OUTBOUND);

  _fareMarketBackup.setFareMarket(_itin->travelSeg(), dir);
}

void
ShoppingFarePathWrapperSource::initItin()
{
  if (_itin->validatingCarrier().empty())
  {
    ValidatingCarrierUpdater vcu(trx);
    vcu.update(*_itin);
  }

  if (_itin->calculationCurrency().empty())
    _itin->calculationCurrency() = shoppingTrx().journeyItin()->calculationCurrency();

  if (_itin->originationCurrency().empty())
    _itin->originationCurrency() = shoppingTrx().journeyItin()->originationCurrency();
}

void
ShoppingFarePathWrapperSource::fillWithItin(FarePath& farePath)
{
  farePath.itin() = _itin;

  _itin->farePath().push_back(&farePath);
  farePath.itin() = _itin;

  _itin->ticketingCarrier() = paxTypeFare.carrier();
}

std::vector<PaxTypeFare::SegmentStatus>
ShoppingFarePathWrapperSource::getSegmentStatus()
{
  PaxTypeFare::FlightBit& bitMap = paxTypeFare.flightBitmap()[bitIndex];
  return bitMap._segmentStatus;
}

PaxType*
ShoppingFarePathWrapperSource::getPaxType()
{
  return _paxType;
}

void
ShoppingFarePathWrapperSource::rtUpdateValidatingCarrier(Itin& itin)
{
  ValidatingCarrierUpdater valCxrUpdater(trx);
  valCxrUpdater.update(itin);
}
}
