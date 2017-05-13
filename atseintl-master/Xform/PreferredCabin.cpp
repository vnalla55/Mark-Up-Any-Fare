#include "Xform/PreferredCabin.h"

#include "Common/FallbackUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseUtil.h"
#include "Common/TrxUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag908Collector.h"
#include "ItinAnalyzer/FareMarketBuilder.h"

#include <boost/cast.hpp>
#include <memory>

namespace tse
{
FALLBACK_DECL(segmentAttributesRefactor)

Logger
PreferredCabin::_logger("Xform.PreferredCabin");

PreferredCabin::PreferredCabin()
  : _trx(nullptr), _preferredCabin(), _bkkPreferredCabinInd(CabinType::UNDEFINED_CLASS),
    _legId(999)
{
  _preferredCabin.setEconomyClass();
}

void
PreferredCabin::selectCabin()
{
  if (_bkkNodes.empty() || !_trx)
  {
    if (_bkkPreferredCabinInd != CabinType::UNDEFINED_CLASS)
    {
      _preferredCabin.setClass(_bkkPreferredCabinInd);
    }
    return;
  }

  const DateTime travelDate = TseUtil::getTravelDate(_trx->travelSeg());

  std::vector<std::shared_ptr<TravelSeg>> tvlSegsPointers;
  std::vector<TravelSeg*> tvlSegs;
  uint32_t index = 0;

  std::shared_ptr<Itin> itin(new Itin());
  std::shared_ptr<FareMarket> fareMarket(new FareMarket());

  for (const std::vector<BkkNode>::value_type& bkk : _bkkNodes)
  {
    std::shared_ptr<TravelSeg> tvlSeg(new AirSeg());
    tvlSegsPointers.push_back(tvlSeg);
    tvlSegs.push_back(tvlSeg.get());

    AirSeg* airSeg = boost::polymorphic_downcast<AirSeg*>(tvlSeg.get());
    TravelSegUtil::setupItinerarySegment(
        _trx->dataHandle(), airSeg, travelDate, std::get<2>(bkk), std::get<3>(bkk), std::get<1>(bkk), index++);

    airSeg->setBookingCode(std::get<0>(bkk));
  }

  std::copy(
      tvlSegs.begin(), tvlSegs.end(), std::inserter(itin->travelSeg(), itin->travelSeg().begin()));

  if (fallback::segmentAttributesRefactor(_trx))
  {
    std::vector<SegmentAttributes> segmentAttributes;
    TravelSegUtil::setSegmentAttributes(tvlSegs, segmentAttributes);

    fareMarket->origin() = segmentAttributes.front().tvlSeg->origin();
    fareMarket->destination() = segmentAttributes.back().tvlSeg->destination();
    fareMarket->boardMultiCity() = segmentAttributes.front().tvlSeg->boardMultiCity();
    fareMarket->offMultiCity() = segmentAttributes.back().tvlSeg->offMultiCity();
    fareMarket->travelDate() = travelDate;

    for (const auto& segment : segmentAttributes)
    {
      fareMarket->travelSeg().push_back(segment.tvlSeg);
    }
  }
  else
  {
    fareMarket->origin() = tvlSegs.front()->origin();
    fareMarket->destination() = tvlSegs.back()->destination();
    fareMarket->boardMultiCity() = tvlSegs.front()->boardMultiCity();
    fareMarket->offMultiCity() = tvlSegs.back()->offMultiCity();
    fareMarket->travelDate() = travelDate;
    fareMarket->travelSeg() = tvlSegs;
  }

  TravelSegAnalysis tvlSegAnalysis;

  Boundary tvlBoundary = tvlSegAnalysis.selectTravelBoundary(tvlSegs);
  ItinUtil::setGeoTravelType(tvlBoundary, *fareMarket);

  FareMarket* fm = fareMarket.get();

  FareMarketBuilder::setBreakIndicator(fm, itin.get(), *_trx);

  GoverningCarrier govCxr(_trx);
  govCxr.process(*fareMarket.get());

  TravelSeg* primarySeg = fareMarket->primarySector();

  if (!primarySeg)
  {
    if (!govCxr.getGovCxrSpecialCases(*fareMarket.get()) || !fareMarket->primarySector())
    {
      LOG4CXX_ERROR(_logger, "Error while processing primary travel segment");
      return;
    }

    primarySeg = fareMarket->primarySector();
  }
  const Cabin* cabin = getCabin(primarySeg);

  if (cabin != nullptr)
  {
    _preferredCabin.setClass(cabin->cabin().getCabinIndicator());

    if (_trx->diagnostic().diagnosticType() == Diagnostic908)
    {
      DCFactory* factory = DCFactory::instance();
      Diag908Collector* dc = dynamic_cast<Diag908Collector*>(factory->create(*_trx));

      dc->enable(Diagnostic908);
      dc->printPrimeSeg(_legId, primarySeg, cabin->cabin().getCabinIndicator());

      dc->flushMsg();
    }

  }
  else
  {
    LOG4CXX_ERROR(_logger, "Error while processing cabin");
  }
}

const Cabin*
PreferredCabin::getCabin(TravelSeg* primarySeg)
{
  if (primarySeg || primarySeg->isAir())
  {
    if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_trx))
    {
      const AirSeg* airSeg = static_cast<const AirSeg*>(primarySeg);
      RBDByCabinUtil rbdUtil(*_trx, PREFERRED_CABIN);
      return rbdUtil.getCabinForAirseg(*airSeg);
    }
    else
    {
      return _trx->dataHandle().getCabin(boost::polymorphic_downcast<AirSeg*>(primarySeg)->carrier(),
                                        primarySeg->getBookingCode(),
                                        primarySeg->arrivalDT());
    }
  }

  return nullptr;
}

void
PreferredCabin::setup(PricingTrx* trx)
{
  _trx = trx;
  _bkkNodes.clear();
  _preferredCabin.setEconomyClass();
  _bkkPreferredCabinInd = CabinType::UNDEFINED_CLASS;
}

void
PreferredCabin::addBkkNode(const BookingCode& bkc,
                           const CarrierCode& cxr,
                           const LocCode& origin,
                           const LocCode& destination)
{
  _bkkNodes.emplace_back(bkc, cxr, origin, destination);
}

void
PreferredCabin::setPreferredCabinInd(Indicator preferredCabin)
{
  _preferredCabin.setClass(preferredCabin);
}

const CabinType&
PreferredCabin::getPreferredCabin() const
{
  return _preferredCabin;
}

Indicator
PreferredCabin::getBkkPreferredCabinInd() const
{
  return _bkkPreferredCabinInd;
}

void
PreferredCabin::setBkkPreferredCabinInd(Indicator bkkPreferredCabinInd)
{
  _bkkPreferredCabinInd = bkkPreferredCabinInd;
}

void
PreferredCabin::setClass(Indicator ch)
{
  _preferredCabin.setClass(ch);
}

void
PreferredCabin::setClassFromAlphaNum(char ch)
{
  _preferredCabin.setClassFromAlphaNum(ch);
}
}
