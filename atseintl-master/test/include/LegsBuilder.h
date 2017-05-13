
#ifndef LEGSBUILDER_H_
#define LEGSBUILDER_H_

#include "DataModel/ShoppingTrx.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class LegsBuilder
{
public:
  struct Segment
  {
    uint32_t _legId;
    uint32_t _sopId;
    std::string _govCxr;
    std::string _orig;
    std::string _dest;
    std::string _carrier;
    // we assume that following times are at the same time zone
    // while calculating flight's duration
    DateTime _departure;
    DateTime _arrival;
  };

  LegsBuilder(ShoppingTrx& trx, TestMemHandle& memHandle, bool reuseTrx = false)
    : _trx(trx), _memHandle(memHandle)
  {
    if (!reuseTrx)
    {
      _trx.legs().clear();
      _trx.legs().resize(1);
      _trx.schedulingOptionIndices().clear();
      _trx.schedulingOptionIndices().resize(1);
      _trx.indicesToSchedulingOption().clear();
      _trx.indicesToSchedulingOption().resize(1);
    }
  }

  void addSegments(const Segment* segments, size_t count)
  {
    for (size_t i = 0; i < count; i++)
    {
      const Segment& seg = segments[i];
      ShoppingTrx::SchedulingOption& sop = getSop(seg);

      AirSeg* travelSeg =
          buildSegment(seg._orig, seg._dest, seg._carrier, seg._departure, seg._arrival);

      sop.governingCarrier() = seg._govCxr;
      sop.itin()->travelSeg().push_back(travelSeg);
    }
  }

  LegsBuilder& addSegment(uint32_t legId,
                          uint32_t sopId,
                          LocCode orig,
                          LocCode origAirport,
                          LocCode dest,
                          LocCode destAirport,
                          CarrierCode carrier,
                          CarrierCode operatingCarrier)
  {
    AirSeg* travelSeg = buildSegment(orig, dest, carrier);
    travelSeg->origAirport() = origAirport;
    travelSeg->destAirport() = destAirport;
    travelSeg->setOperatingCarrierCode(operatingCarrier);

    addSegmentToTrx(legId, sopId, travelSeg);

    return (*this);
  }

  LegsBuilder& addIBFromReversedOB(uint32_t ibSopId, uint32_t obSopId)
  {
    enum
    {
      LEG_OB,
      LEG_IB
    };

    const std::vector<TravelSeg*>& tsegs =
        _trx.legs().at(LEG_OB).sop().at(obSopId).itin()->travelSeg();

    for (std::vector<TravelSeg*>::const_reverse_iterator reverseIt = tsegs.rbegin();
         reverseIt != tsegs.rend();
         ++reverseIt)
    {
      AirSeg& orig = dynamic_cast<AirSeg&>(**reverseIt);
      AirSeg* reversed = reverse(clone(&orig));

      addSegmentToTrx(LEG_IB, ibSopId, reversed);
    }

    return (*this);
  }

  void endBuilding()
  {
    for (ShoppingTrx::Leg& leg : _trx.legs())
    {
      calcFlightTimes(leg);
    }
  }

  ShoppingTrx::SchedulingOption& getSop(const Segment& seg)
  {
    return getSop(seg._legId, seg._sopId);
  }

  ShoppingTrx::Leg& getLeg(const Segment& seg) { return getLeg(seg._legId); }

protected:
  ShoppingTrx::Leg& getLeg(uint32_t legId)
  {
    if (legId >= _trx.legs().size())
    {
      _trx.legs().resize(legId + 1);
      _trx.schedulingOptionIndices().resize(legId + 1);
      _trx.indicesToSchedulingOption().resize(legId + 1);
    }
    return _trx.legs()[legId];
  }

  ShoppingTrx::SchedulingOption& getSop(uint32_t legId, uint32_t sopId)
  {
    typedef ShoppingTrx::Leg Leg;
    typedef ShoppingTrx::SchedulingOption Sop;
    typedef std::map<uint32_t, uint32_t> OriginalToInternalSopIdMap;
    typedef std::map<uint32_t, uint32_t> InternalToOriginalSopIdMap;

    Leg& leg = getLeg(legId);

    OriginalToInternalSopIdMap& sopIdMap = _trx.schedulingOptionIndices()[legId];
    InternalToOriginalSopIdMap& intSopIdMap = _trx.indicesToSchedulingOption()[legId];
    Itin* itin = 0;

    while (sopId >= leg.sop().size())
    {
      uint32_t newSopId = static_cast<uint32_t>(leg.sop().size());

      _memHandle.get(itin);
      Sop sop(itin, newSopId, true);
      sop.sopId() = newSopId;
      leg.sop().push_back(sop);
      sopIdMap[newSopId] = newSopId;
      intSopIdMap[newSopId] = newSopId;

      leg.requestSops() = leg.sop().size();
    }

    return leg.sop()[sopId];
  }

  void calcFlightTimes(ShoppingTrx::Leg& leg)
  {
    for (ShoppingTrx::SchedulingOption& sop : leg.sop())
    {
      sop.itin()->calculateFlightTimeMinutes(0, 0);
    }
  }

  AirSeg* buildSegment(std::string origin,
                       std::string destination,
                       std::string carrier,
                       DateTime departure = DateTime::emptyDate(),
                       DateTime arrival = DateTime::emptyDate())
  {
    AirSeg* airSeg;
    _memHandle.get(airSeg);

    Loc* locorig, *locdest;
    _memHandle.get(locorig);
    _memHandle.get(locdest);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(carrier);
    airSeg->boardMultiCity() = locorig->loc();
    airSeg->offMultiCity() = locdest->loc();
    airSeg->stopOver() = false;
    return airSeg;
  };

  AirSeg* clone(AirSeg* orig)
  {
    AirSeg* cloned;
    _memHandle.get(cloned);
    *cloned = *orig;

    return cloned;
  }

  /**
   * departure / arrival dates are not handled
   */
  static AirSeg* reverse(AirSeg* seg)
  {
    using namespace std;

    swap(seg->origin(), seg->destination());
    swap(seg->origAirport(), seg->destAirport());
    swap(seg->boardMultiCity(), seg->offMultiCity());

    return seg;
  }

  void addSegmentToTrx(uint32_t legId, uint32_t sopId, AirSeg* travelSeg)
  {
    ShoppingTrx::SchedulingOption& sop = getSop(legId, sopId);
    sop.itin()->travelSeg().push_back(travelSeg);
  }

  ShoppingTrx& _trx;
  TestMemHandle& _memHandle;
};
}

#endif /* LEGSBUILDER_H_ */
