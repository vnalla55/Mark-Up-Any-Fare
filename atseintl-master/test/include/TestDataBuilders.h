#pragma once

#include "Common/CabinType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"

#include "test/include/TestMemHandle.h"
#include "test/include/ATSEv2Test.h"

namespace tse
{
template <class T>
class GenericBuilder
{
public:
  GenericBuilder(TestMemHandle& mh) : _mh(mh) { clear(); }
  void clear() { _ptr = _mh.create<T>(); }
  T* build()
  {
    T* ret = _ptr;
    clear();
    return ret;
  }

protected:
  T* _ptr;
  TestMemHandle& _mh;
};

class LocBuilder : public GenericBuilder<Loc>
{
  typedef GenericBuilder<Loc> Base;

public:
  using Base::Base;

  LocBuilder& withArea(const IATAAreaCode& area, const IATASubAreaCode& subarea = "")
  {
    _ptr->area() = area;
    _ptr->subarea() = subarea;
    return *this;
  }

  LocBuilder& withNation(const NationCode& nation)
  {
    _ptr->nation() = nation;
    return *this;
  }

  LocBuilder& withLoc(const LocCode& loc)
  {
    _ptr->loc() = loc;
    return *this;
  }
};

template <class SegType>
class SegBuilder : public GenericBuilder<SegType>
{
  typedef GenericBuilder<SegType> Base;

public:
  using Base::Base;

  SegBuilder& withLocs(const Loc& o, const Loc& d)
  {
    Base::_ptr->origAirport() = o.loc();
    Base::_ptr->origin() = &o;
    Base::_ptr->destAirport() = d.loc();
    Base::_ptr->destination() = &d;
    return *this;
  }

  SegBuilder& withLocs(const LocCode& o, const LocCode& d)
  {
    Base::_ptr->boardMultiCity() = o;
    Base::_ptr->offMultiCity() = d;
    LocBuilder bld(Base::_mh);
    return withLocs(*bld.withLoc(o).build(), *bld.withLoc(d).build());
  }

  SegBuilder& withCxr(const CarrierCode cxr) { return withCxr(cxr, cxr); }
  SegBuilder& withCxr(const CarrierCode mkt, const CarrierCode oper)
  {
    Base::_ptr->carrier() = mkt;
    Base::_ptr->setOperatingCarrierCode(oper);
    return *this;
  }

  SegBuilder& withLegId(const int16_t legId)
  {
    Base::_ptr->legId() = legId;
    return *this;
  }

  SegBuilder& withCabin(const Indicator ind)
  {
    Base::_ptr->bookedCabin().setClass(ind);
    return *this;
  }
};

using AirSegBuilder = SegBuilder<AirSeg>;
using ArunkSegBuilder = SegBuilder<ArunkSeg>;

class FareMarketBuilder : public GenericBuilder<FareMarket>
{
  typedef GenericBuilder<FareMarket> Base;

public:
  using Base::Base;

  template <class It>
  FareMarketBuilder& withSegs(It first, It last)
  {
    _ptr->travelSeg().assign(first, last);

    if (first != last)
    {
      --last;
      _ptr->origin() = (*first)->origin();
      _ptr->boardMultiCity() = (*first)->boardMultiCity();
      _ptr->destination() = (*last)->destination();
      _ptr->offMultiCity() = (*last)->offMultiCity();
    }

    return *this;
  }

  FareMarketBuilder& withSegs(const std::vector<TravelSeg*>& segs)
  {
    return withSegs(segs.begin(), segs.end());
  }
};

class ItinBuilder : public GenericBuilder<Itin>
{
  using Base = GenericBuilder<Itin>;

public:
  using Base::Base;

  ItinBuilder& withGeoTravelType(GeoTravelType gtt)
  {
    _ptr->geoTravelType() = gtt;
    return *this;
  }

  template <class It>
  ItinBuilder& withSegs(It first, It last)
  {
    _ptr->travelSeg().assign(first, last);
    return *this;
  }

  ItinBuilder& withSegs(const std::vector<TravelSeg*>& segs)
  {
    return withSegs(segs.begin(), segs.end());
  }
};

class TrxBuilder : public GenericBuilder<PricingTrx>
{
  typedef GenericBuilder<PricingTrx> Base;

public:
  using Base::Base;

  TrxBuilder& withPaxTypes(std::initializer_list<PaxTypeCode> paxes)
  {
    for (const PaxTypeCode pax : paxes)
    {
      PaxType* pt = _mh.create<PaxType>();
      pt->paxType() = pax;
      _ptr->paxType().push_back(pt);
    }
    return *this;
  }
};

class ItinBuilder2
{
public:
   ItinBuilder2(TestMemHandle& memHandle) : _memHandle(memHandle)
  {
    _pricingUnit = _memHandle.create<PricingUnit>();
  }

  ItinBuilder2&
  addSegment(const Loc* origin, const Loc* destination, CarrierCode carrierCode)
  {
   ++segmentNum;
    _pricingUnit->fareUsage().push_back(ATSEv2Test::createFareUsage(_memHandle,
                                                                   origin,
                                                                   destination,
                                                                   carrierCode));
    return *this;
  }

  ItinBuilder2&
  addArunkSegment(const Loc* origin, const Loc* destination)
  {
    ++segmentNum;
    arunks[segmentNum] = ATSEv2Test::createArunkSeg(_memHandle, origin, destination);
    return *this;
  }

  ItinBuilder2&
  setPuType(PricingUnit::Type type)
  {
    _pricingUnit->puType() = type;
    return *this;
  }

  Itin* build()
  {
    std::vector<PricingUnit*> pricingUnits = {_pricingUnit};

    Itin* itin = _memHandle.create<Itin>();

    FarePath *farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;
    farePath->pricingUnit() = pricingUnits;

    itin->farePath().push_back(farePath);

    for (PricingUnit* pricingUnit : farePath->pricingUnit())
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
        for (TravelSeg* travelSeg : fareUsage->travelSeg())
          itin->travelSeg().push_back(travelSeg);

    for (auto& arunkPair : arunks)
      itin->travelSeg().insert(itin->travelSeg().begin() + arunkPair.first- 1, arunkPair.second);

    return itin;
  }

private:
  uint32_t segmentNum = 0;
  std::map<uint32_t, TravelSeg*> arunks;
  TestMemHandle& _memHandle;
  PricingUnit* _pricingUnit;
};


class PUPathBuilder
{
public:
  PUPathBuilder(TestMemHandle& memHandle) : _memHandle(memHandle)
  {
    _puPath = _memHandle.create<PUPath>();
    _fareMarketPath = _memHandle.create<FareMarketPath>();
    _puPath->fareMarketPath() = _fareMarketPath;
  }

  PUPathBuilder&
  addSegment(const Loc* origin, const Loc* destination, CarrierCode carrierCode)
  {
    _segs.push_back(ATSEv2Test::createAirSeg(_memHandle,
                                             origin,
                                             destination,
                                             carrierCode));
    return *this;
  }

  PUPathBuilder&
  addArunkSegment(const Loc* origin, const Loc* destination, CarrierCode carrierCode)
  {
    _segs.push_back(ATSEv2Test::createArunkSeg(_memHandle, origin, destination));
    return *this;
  }

  PUPathBuilder&
  addFareMarket(CarrierCode carrierCode)
  {
    const Loc* origin = _segs.front()->origin();
    const Loc* destination = _segs.back()->destination();

    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->origin() = origin;
    fm->destination() = destination;
    fm->governingCarrier() = carrierCode;

    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    mfm->mergedFareMarket().push_back(fm);
    if (mfm->boardMultiCity().empty())
      mfm->boardMultiCity() = origin->loc();
    mfm->offMultiCity() = destination->loc();
    mfm->governingCarrier().push_back(carrierCode);
    for (auto seg : _segs)
      mfm->travelSeg().push_back(seg);
    _segs.clear();

    _mfms.push_back(mfm);
    _puPath->fareMarketPath()->fareMarketPath().push_back(mfm);
    return *this;
  }

  PUPathBuilder&
  setPuType(PricingUnit::Type type)
  {
    PU* pu = _memHandle.create<PU>();
    pu->puType() = type;

    pu->fareMarket() = _mfms;
    _mfms.clear();
    _puPath->puPath().push_back(pu);
    return *this;
  }

  PUPath*
  build()
  {
    return _puPath;
  }

private:
  std::vector<MergedFareMarket*> _mfms;
  PUPath* _puPath = nullptr;
  FareMarketPath* _fareMarketPath = nullptr;
  std::vector<TravelSeg*> _segs;
  TestMemHandle& _memHandle;
};

}
