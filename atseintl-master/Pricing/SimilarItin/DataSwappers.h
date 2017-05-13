/*---------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Common/TseCodeTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"

#include <forward_list>

namespace tse
{
class FarePath;
class Itin;

class FMAvail
{
public:
  explicit FMAvail(FareUsage* fu)
    : _fm(fu->paxTypeFare()->fareMarket()),
      _primarySector(_fm->primarySector()),
      _availBreaks(std::move(_fm->availBreaks())),
      _travelBoundary(_fm->travelBoundary())

  {
  }

  FMAvail(const FMAvail&) = delete;

  ~FMAvail()
  {
    _fm->primarySector() = _primarySector;
    _fm->availBreaks() = std::move(_availBreaks);
    _fm->travelBoundary() = _travelBoundary;
  }

private:
  FareMarket* _fm;
  TravelSeg* _primarySector;
  std::vector<bool> _availBreaks;
  SmallBitSet<uint8_t, FMTravelBoundary> _travelBoundary;
};

class FMContext
{
public:
  explicit FMContext(FareUsage* fu) : _fu(fu), _fmAvail(fu)
  {
    PaxTypeFare& ptf = *fu->paxTypeFare();
    _bookingCodeStatus = ptf.bookingCodeStatus();
    _segmentStatus = std::move(ptf.segmentStatus());
    _segmentStatusRule2 = std::move(ptf.segmentStatusRule2());

    FareMarket& fm = *ptf.fareMarket();
    _cosVec = std::move(fm.classOfServiceVec());
    _govCxr = fm.governingCarrier();
    _travelSeg = std::move(fm.travelSeg());

    fm.travelSeg() = fu->travelSeg();
    fm.classOfServiceVec().clear();

    ptf.segmentStatus().clear();
    ptf.segmentStatusRule2().clear();
    fu->segmentStatus().clear();
    fu->segmentStatusRule2().clear();

    ptf.bookingCodeStatus() = fu->bookingCodeStatus() = PaxTypeFare::BKS_NOT_YET_PROCESSED;
  }

  FMContext(const FMContext&) = delete;

  ~FMContext()
  {
    PaxTypeFare& ptf = *_fu->paxTypeFare();
    ptf.segmentStatus() = std::move(_segmentStatus);
    ptf.segmentStatusRule2() = std::move(_segmentStatusRule2);
    ptf.bookingCodeStatus() = _bookingCodeStatus;

    FareMarket& fm = *ptf.fareMarket();
    fm.travelSeg() = std::move(_travelSeg);
    fm.classOfServiceVec() = std::move(_cosVec);
    fm.governingCarrier() = _govCxr;
  }

private:
  FareUsage* _fu;
  FMAvail _fmAvail;

  std::vector<TravelSeg*> _travelSeg;
  std::vector<std::vector<ClassOfService*>*> _cosVec;
  std::vector<PaxTypeFare::SegmentStatus> _segmentStatus;
  std::vector<PaxTypeFare::SegmentStatus> _segmentStatusRule2;
  PaxTypeFare::BookingCodeStatus _bookingCodeStatus;
  CarrierCode _govCxr;
};

template <typename T>
class FareMarketDataResetter
{
public:
  FareMarketDataResetter(FarePath& fp)
  {
    for (auto* const pu : fp.pricingUnit())
    {
      for (auto* const fu : pu->fareUsage())
        _data.emplace_front(fu);
    }
  }

private:
  std::forward_list<T> _data;
};
}
