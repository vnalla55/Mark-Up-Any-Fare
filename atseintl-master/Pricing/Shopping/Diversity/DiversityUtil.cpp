// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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
#include "Pricing/Shopping/Diversity/DiversityUtil.h"

#include "Common/Assert.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DiagCollector.h"

#include <tr1/tuple>

namespace tse
{

using namespace shpq;

namespace
{

/**
 * Step by marketing carrier change in std::vector<TravelSeg*>.
 * When initialized, steps by the first AirSeg.
 * While iterating over the vec, non AirSegs are skipped.
 */
template <class I>
class CxrChangeIter
{
public:
  CxrChangeIter(I begin, I end) : _curTs(begin), _endTs(end) { _curAirSeg = skipToFirstAirSeg(); }
  CxrChangeIter(I end) : _curTs(end), _endTs(end), _curAirSeg(nullptr) {}

  AirSeg& operator*() const
  {
    TSE_ASSERT(_curAirSeg);
    return *_curAirSeg;
  }

  CxrChangeIter& operator++()
  {
    TSE_ASSERT(_curTs != _endTs);

    for (++_curTs; _curTs != _endTs; ++_curTs)
    {
      if (AirSeg* nextAirSeg = skipToFirstAirSeg())
      {
        if (nextAirSeg->marketingCarrierCode() != _curAirSeg->marketingCarrierCode())
        {
          _curAirSeg = nextAirSeg;
          break;
        }
      }
      else
      {
        TSE_ASSERT(_curTs == _endTs && "Logic error");
        _curAirSeg = nullptr;
      }
    }
    return *this;
  }

  bool operator!=(const CxrChangeIter& rhs) const { return (_curTs != rhs._curTs); }

private:
  I _curTs;
  const I _endTs;
  AirSeg* _curAirSeg;

  AirSeg* skipToFirstAirSeg()
  {
    AirSeg* result = nullptr;
    while (_curTs != _endTs)
    {
      result = dynamic_cast<AirSeg*>(*_curTs);
      if (LIKELY(result))
        break;
      ++_curTs;
    }
    return result;
  }
};

bool
snowmanCxrChangeCheck(const AirSeg& obAirSeg, const AirSeg& ibAirSeg)
{
  // The same as RTValidator::isSnowman checks airport but not city
  bool sameConnectingCityAirport = (obAirSeg.origAirport() == ibAirSeg.destAirport());
  bool sameCarrier = (obAirSeg.marketingCarrierCode() == ibAirSeg.marketingCarrierCode());

  return (sameConnectingCityAirport && sameCarrier);
}

std::pair<const std::vector<TravelSeg*>*, const std::vector<TravelSeg*>*>
getSnowmanTSegs(const ShoppingTrx& trx, shpq::SopIdxVecArg sops)
{
  std::pair<const std::vector<TravelSeg*>*, const std::vector<TravelSeg*>*> result(nullptr, nullptr);
  if (sops.size() != 2)
    return result;

  const std::size_t leg1 = 0, leg2 = 1;

  const ShoppingTrx::SchedulingOption& sop1 = trx.legs()[leg1].sop()[sops[leg1]];
  const Itin& itin1 = *sop1.itin();
  result.first = &itin1.travelSeg();

  const ShoppingTrx::SchedulingOption& sop2 = trx.legs()[leg2].sop()[sops[leg2]];
  const Itin& itin2 = *sop2.itin();
  result.second = &itin2.travelSeg();

  return result;
}

class TravelSegBinaryVisitor
{
public:
  virtual ~TravelSegBinaryVisitor() {}

  virtual bool visit(const std::vector<TravelSeg*>& ts1, const std::vector<TravelSeg*>& ts2)
  {
    if (ts1.size() != ts2.size())
      return false;

    for (std::vector<TravelSeg*>::const_iterator iter1(ts1.begin()), iter2(ts2.begin());
         iter1 != ts1.end();
         ++iter1, ++iter2)
    {
      bool cont = visit(*iter1, *iter2);
      if (!cont)
        return false;
    }

    return true;
  }

  virtual bool visit(TravelSeg* ts1, TravelSeg* ts2) = 0;

  static bool apply(shpq::SopIdxVecArg sops1,
                    shpq::SopIdxVecArg sops2,
                    TravelSegBinaryVisitor& visitor,
                    const ShoppingTrx& trx)
  {
    TSE_ASSERT(sops1.size() == sops2.size()); // assume we can meet empty sops at one of the sides

    for (std::size_t leg = 0; leg < sops1.size(); ++leg)
    {
      const ShoppingTrx::SchedulingOption& sop1 = trx.legs()[leg].sop()[sops1[leg]];
      const Itin& itin1 = *sop1.itin();

      const ShoppingTrx::SchedulingOption& sop2 = trx.legs()[leg].sop()[sops2[leg]];
      const Itin& itin2 = *sop2.itin();

      bool cont = visitor.visit(itin1.travelSeg(), itin2.travelSeg());
      if (!cont)
        return false;
    }

    return true;
  }
};

class ConnectingCityCheckVisitor : public TravelSegBinaryVisitor
{
public:
  /**
   * @override
   */
  bool visit(TravelSeg* ts1, TravelSeg* ts2) override
  {
    AirSeg* air1 = dynamic_cast<AirSeg*>(ts1);
    AirSeg* air2 = dynamic_cast<AirSeg*>(ts2);

    // One/both segments are not air
    if (UNLIKELY((air1 == nullptr) && (air2 == nullptr)))
      return true;
    if (UNLIKELY((air1 == nullptr) || (air2 == nullptr)))
      return false;

    if (air1->marketingCarrierCode() != air2->marketingCarrierCode())
      return false;

    bool result =
        air1->origAirport() == air2->origAirport() && air1->destAirport() == air2->destAirport();
    return result;
  }
};

} // anon ns

DiversityUtil::CompoundCarrier::CompoundCarrier(CarrierCode ob, CarrierCode ib) : _ob(ob), _ib(ib)
{
}

bool
DiversityUtil::CompoundCarrier::isOnline() const
{
  TSE_ASSERT(!_ob.empty() || !"Not initialized");

  bool isOneWay = _ib.empty();
  if (isOneWay)
    return true;

  bool isOnline = (_ob == _ib);
  return isOnline;
}

CarrierCode
DiversityUtil::CompoundCarrier::toCarrierCode() const
{
  TSE_ASSERT(_ib.empty());
  return _ob;
}

std::ostream& operator<<(std::ostream& os, DiversityUtil::CompoundCarrier cxr)
{
  bool isInitialized = !cxr._ob.empty();
  if (!isInitialized)
  {
    os << INVALID_CARRIERCODE;
    return os;
  }

  bool isOW = cxr._ib.empty();
  if (isOW)
  {
    return os << cxr._ob;
  }

  bool isInterline = (cxr._ob != cxr._ib);
  if (isInterline)
  {
    os << cxr._ob << "/" << cxr._ib; // interline as OB/IB carrier combination
  }
  else
    os << cxr._ob;

  return os;
}

DiagCollector& operator<<(DiagCollector& dc, DiversityUtil::CompoundCarrier cxr)
{
  if (!dc.isActive())
    return dc;
  else
  {
    std::ostream& os = dc;
    os << cxr;
    return dc;
  }
}

DiversityUtil::ADSolultionKind
DiversityUtil::getSolutionKind(const ShoppingTrx& trx,
                               const FlightMatrixSolution& sol,
                               CompoundCarrier carrier)
{
  bool isSnowman = detectSnowman(trx, sol.first);
  return getSolutionKind(carrier, isSnowman);
}

DiversityUtil::ADSolultionKind
DiversityUtil::getSolutionKind(CompoundCarrier carrier, bool isSnowman)
{
  if (carrier.isOnline())
  {
    return isSnowman ? OnlineSnowman : Online;
  }
  else
  {
    return isSnowman ? InterlineSnowman : Interline;
  }
}

bool
DiversityUtil::detectSnowman(const ShoppingTrx& trx, shpq::SopIdxVecArg sops)
{
  const std::vector<TravelSeg*>* obTs, *ibTs;
  std::tr1::tie(obTs, ibTs) = getSnowmanTSegs(trx, sops);

  if (!obTs || !ibTs)
    return false;

  const bool isSnowmanEnoughSegCount = (obTs->size() > 1 && ibTs->size() > 1);
  if (!isSnowmanEnoughSegCount)
    return false;

  CxrChangeIter<std::vector<TravelSeg*>::const_iterator> obCxrChangeIt(obTs->begin(), obTs->end()),
      obEnd(obTs->end());

  CxrChangeIter<std::vector<TravelSeg*>::const_reverse_iterator> ibCxrChangeIt(ibTs->rbegin(),
                                                                        ibTs->rend()),
      ibEnd(ibTs->rend());

  TSE_ASSERT(obCxrChangeIt != obEnd && ibCxrChangeIt != ibEnd &&
             "The first AirSeg was not detected");

  // check the first AirSeg carrier and connecting city
  if (!snowmanCxrChangeCheck(*obCxrChangeIt, *ibCxrChangeIt))
    return false;

  // detect the first carrier change both for inbound and outbound
  bool detectCxrChange = (++obCxrChangeIt != obEnd && ++ibCxrChangeIt != ibEnd);
  if (!detectCxrChange)
    return false;

  // check carrier and connecting city on carrier change
  if (!snowmanCxrChangeCheck(*obCxrChangeIt, *ibCxrChangeIt))
    return false;

  // detect more than one carrier change for either inbound or outbound
  detectCxrChange = (++obCxrChangeIt != obEnd || ++ibCxrChangeIt != ibEnd);
  if (detectCxrChange)
    return false;

  return true;
}

std::ostream& operator<<(std::ostream& os, DiversityUtil::ADSolultionKind kind)
{
  switch (kind)
  {
  case DiversityUtil::Online:
    os << "Online";
    break;
  case DiversityUtil::OnlineSnowman:
    os << "Online snowman";
    break;
  case DiversityUtil::InterlineSnowman:
    os << ANY_CARRIER << " snowman";
    break;
  case DiversityUtil::Interline:
    os << ANY_CARRIER;
    break;
  default:
    os << "INVALID";
    break;
  }
  return os;
}

bool
DiversityUtil::isSolutionSimilar(const ShoppingTrx& trx,
                                 shpq::SopIdxVecArg lhSops,
                                 shpq::SopIdxVecArg rhSops)
{
  ConnectingCityCheckVisitor connCityCheck;
  bool isSimilar = TravelSegBinaryVisitor::apply(lhSops, rhSops, connCityCheck, trx);
  return isSimilar;
}

DatePair
DiversityUtil::getDatePairSops(const ShoppingTrx& trx, shpq::SopIdxVecArg sops)
{
  const int OB = 0;
  const int IB = 1;

  const LegVec& legs = trx.legs();

  const ShoppingTrx::SchedulingOption* sop1 = nullptr;
  const ShoppingTrx::SchedulingOption* sop2 = nullptr;

  switch (legs.size())
  {
  case 2:
    sop2 = &legs[IB].sop()[sops[IB]];
  case 1:
    sop1 = &legs[OB].sop()[sops[OB]];
    break;
  default:
    TSE_ASSERT(!"Unsupported number of legs");
  }

  return ShoppingAltDateUtil::getDatePairSops(sop1, sop2);
}

int32_t
DiversityUtil::getBestDuration(const ShoppingTrx& trx)
{
  const ShoppingTrx::Leg& leg0 = trx.legs()[0];
  int32_t bestDuration = leg0.sop().at(leg0.getMinDurationSopIdx()).itin()->getFlightTimeMinutes();

  if (trx.legs().size() == 2)
  {
    const ShoppingTrx::Leg& leg1 = trx.legs()[1];
    bestDuration += leg1.sop().at(leg1.getMinDurationSopIdx()).itin()->getFlightTimeMinutes();
  }

  return bestDuration;
}

} // ns tse
