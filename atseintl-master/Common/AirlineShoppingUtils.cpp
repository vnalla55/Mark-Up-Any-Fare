#include "Common/AirlineShoppingUtils.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/FarePathCopier.h"
#include "Common/ItinUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/PUPathMatrix.h"
#include "Pricing/PaxFarePathFactoryBase.h"


namespace tse
{
namespace AirlineShoppingUtils
{
using namespace std;

namespace
{

struct NotSpecifiedLeg
{
  LegId _id;
  NotSpecifiedLeg(LegId id) : _id(id) {}

  bool operator()(const Itin* itin) const
  {
    const uint16_t legID = static_cast<uint16_t>(itin->legID().front().first);
    return legID != _id;
  }
};

struct NoSolutionFound
{
  NoSolutionFound() {}

  bool operator()(const Itin* itin) const { return itin->farePath().empty(); }
};

struct MoreSolutionFirst
{
  bool operator()(const Itin* l, const Itin* r) const
  {
    return r->farePath().size() < l->farePath().size();
  }
};

struct IsSameBrandSamePax
{
  const FarePath* _fp;

  IsSameBrandSamePax(const FarePath* fp) : _fp(fp) {}

  bool operator()(const FarePath* fp) const;
};

struct IsSamePax
{
  const FarePath* _fp;

  IsSamePax(const FarePath* fp) : _fp(fp) {}

  bool operator()(const FarePath* fp) const;
};


struct ContainedIn : public std::unary_function<const FarePath*, bool>
{
  std::set<FarePath*>& _set;
  ContainedIn(std::set<FarePath*>& set) : _set(set) {}

  bool operator()(FarePath* fp) { return _set.count(fp) != 0; }
};

bool
IsSameBrandSamePax::
operator()(const FarePath* fp) const
{
  if (fp->duplicate() || _fp->duplicate())
    return false;

  return (_fp->brandIndex() == fp->brandIndex() &&
          _fp->paxType()->paxType() == fp->paxType()->paxType());
}

bool
IsSamePax::
operator()(const FarePath* fp) const
{
  if (fp->duplicate() || _fp->duplicate())
    return false;

  return (_fp->paxType()->paxType() == fp->paxType()->paxType());
}

} // empty namespace

log4cxx::LoggerPtr
AseItinCombiner::_logger(
    log4cxx::Logger::getLogger("atseintl.AirlineShoppingUtils.AseItinCombiner"));

std::ostream& operator<<(std::ostream& os, const TravelSeg& ts)
{
  if (ts.isAir())
  {
    const AirSeg& seg(static_cast<const AirSeg&>(ts));
    os << "leg:" << seg.legId() << ' '
       //<< "orgId:" << seg.originalId() << ' '
       //<< "pnrSegment:" << seg.pnrSegment() << ' '
       << "mktgCXR:" << seg.marketingCarrierCode() << ' '
       << "oprtgCXR:" << seg.operatingCarrierCode() << ' ' << "origAirport:" << seg.origAirport()
       << ' ' << "brdMultiCity:" << seg.boardMultiCity() << ' '
       << "dstAirport:" << seg.destAirport() << ' ' << "offMultiCity:" << seg.offMultiCity() << ' '
       << "departDT:" << seg.departureDT() << ' ' << "arrvlDT:" << seg.arrivalDT() << ' '
       << "orgGmtAdj:" << seg.originGmtAdjustment() << ' '
       << "dstGmtAdj:" << seg.destinationGmtAdjustment() << ' '
       << "mktgFLT#:" << seg.marketingFlightNumber() << ' '
       << "oprtgFLT#:" << seg.operatingFlightNumber() << ' ' << "bbr:" << seg.bbrCarrier() << ' '
       << "eticket:" << seg.eticket() << ' ' << "equip:" << seg.equipmentType() << ' '
       << "bookingCode:" << seg.getBookingCode() << ' ' << "bookedCabin:" << seg.bookedCabin();
    os << ' ' << "COS:";
    for (const ClassOfService* cos : seg.classOfService())
      os << cos->cabin() << '^' << cos->bookingCode() << ' ';

    if (!seg.hiddenStops().empty())
    {
      os << ' ' << "hiddenStops:";
      for (const Loc* loc : seg.hiddenStops())
        os << loc->loc() << ' ';
    }
  }
  return os;
}
std::ostream& operator<<(std::ostream& os, const std::vector<TravelSeg*>& schedule)
{
  os << "segments:\n";
  for (const TravelSeg* seg : schedule)
    os << *seg << '\n';

  return os;
}

void
AseItinCombiner::combineOneWayItineraries()
{
  if (!_trx.isAltDates())
  {
    LOG4CXX_ERROR(_logger, "Alt dates only logic");
    if (_trx.diagnostic().diagnosticType() != DiagnosticNone)
      return;

    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }

  // Remove itins with no solution
  _trx.itin().erase(remove_if(_trx.itin().begin(), _trx.itin().end(), NoSolutionFound()),
                    _trx.itin().end());

  sort(_trx.itin().begin(), _trx.itin().end(), MoreSolutionFirst());

  remove_copy_if(_trx.itin().begin(),
                 _trx.itin().end(),
                 back_inserter(_trx.subItinVecOutbound()),
                 NotSpecifiedLeg(FIRST_LEG));

  remove_copy_if(_trx.itin().begin(),
                 _trx.itin().end(),
                 back_inserter(_trx.subItinVecInbound()),
                 NotSpecifiedLeg(SECOND_LEG));

  if (_trx.subItinVecOutbound().empty() || _trx.subItinVecInbound().empty())
  {
    LOG4CXX_ERROR(_logger, "No one way solution found");
    if (_trx.diagnostic().diagnosticType() != DiagnosticNone)
      return;
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }

  _trx.itin().clear();
  combineSolutions(_trx.subItinVecOutbound(), _trx.subItinVecInbound());

  if (_trx.itin().empty())
  {
    LOG4CXX_ERROR(_logger, "No solution found for combined itins");

    if (_trx.diagnostic().diagnosticType() != DiagnosticNone)
      return;
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }
}

void
AseItinCombiner::setupCombinedItin(PricingTrx& trx, Itin* outbound, Itin* inbound, Itin* resultItin)
{
  resultItin->calculationCurrency() = outbound->calculationCurrency();
  resultItin->originationCurrency() = outbound->originationCurrency();
  resultItin->calcCurrencyOverride() = outbound->calcCurrencyOverride();

  resultItin->travelSeg().insert(
      resultItin->travelSeg().end(), outbound->travelSeg().begin(), outbound->travelSeg().end());

  resultItin->travelSeg().insert(
      resultItin->travelSeg().end(), inbound->travelSeg().begin(), inbound->travelSeg().end());

  resultItin->setTravelDate(TseUtil::getTravelDate(resultItin->travelSeg()));
  resultItin->bookingDate() = TseUtil::getBookingDate(resultItin->travelSeg());

  updateValidatingCarrier(trx, *resultItin);

  setTripCharacteristics(resultItin);

  resultItin->legID().push_back(outbound->legID()[0]);
  resultItin->legID().push_back(inbound->legID()[0]);
  if (_trx.isAltDates())
  {
    trx.dataHandle().get(resultItin->datePair());
    *resultItin->datePair() = make_pair(outbound->datePair()->first, inbound->datePair()->first);
  }
}

void
AseItinCombiner::combineItins(PricingTrx& trx, Itin* outbound, Itin* inbound)
{
  vector<FarePath*> outbounds;
  vector<FarePath*> inbounds;

  outbounds.reserve(outbound->farePath().size());
  copy(outbound->farePath().begin(),
       outbound->farePath().end(),
       back_inserter(outbounds));

  inbounds.reserve(inbound->farePath().size());
  copy(inbound->farePath().begin(),
       inbound->farePath().end(),
       back_inserter(inbounds));

  for (FarePath* outbOption : outbounds)
  {
    FarePath* inbOption = nullptr;
    vector<FarePath*>::iterator solIt =
        find_if(inbounds.begin(), inbounds.end(), IsSameBrandSamePax(outbOption));

    if (solIt != inbounds.end()) // matches for pax and brand
    {
      inbOption = *solIt;
      inbounds.erase(solIt);
    }
    else
    {

      solIt =
          find_if(inbound->farePath().begin(), inbound->farePath().end(), IsSamePax(outbOption));

      if (solIt != inbound->farePath().end())
        inbOption = *solIt;
    }

    if (outbOption == nullptr || inbOption == nullptr)
      continue;

    combineItinSolutions(outbOption->brandIndex(), outbOption, inbOption);
  }

  // match remaining inbounds regardless of  brand
  for (FarePath* inbOption : inbounds)
  {

    vector<FarePath*>::iterator solIt =
        find_if(outbounds.begin(), outbounds.end(), IsSamePax(inbOption));

    if (solIt == outbounds.end())
    {
      LOG4CXX_DEBUG(_logger, "Inbound not matched, brand: " << inbOption->brandIndex());
      continue; // could not find matching solution
    }

    FarePath* outbOption = *solIt;
    combineItinSolutions(inbOption->brandIndex(), outbOption, inbOption);
  }

} // end combineItins

FarePath*
AseItinCombiner::combineFarePaths(PricingTrx& trx, FarePath* outbOption, FarePath* inbOption)
{
  TSE_ASSERT(outbOption);
  TSE_ASSERT(inbOption);

  FarePath* solutionPath = FarePathCopier(trx.dataHandle()).getDuplicate(*outbOption);

  solutionPath->brandIndexPair() = make_pair(outbOption->brandIndex(), inbOption->brandIndex());
  solutionPath->parentItinPair() = make_pair(outbOption->itin(), inbOption->itin());
  solutionPath->pricingUnit().clear();

  for (PricingUnit* pu : outbOption->pricingUnit())
  {
    PricingUnit* newPU = pu->clone(trx.dataHandle());
    solutionPath->pricingUnit().push_back(newPU);

    newPU->fareUsage().clear();

    for (FareUsage* fu : pu->fareUsage())
      newPU->fareUsage().push_back(fu->clone(trx.dataHandle()));
  }

  for (PricingUnit* pu : inbOption->pricingUnit())
  {
    PricingUnit* newPU = pu->clone(trx.dataHandle());
    solutionPath->pricingUnit().push_back(newPU);

    newPU->fareUsage().clear();

    for (FareUsage* fu : pu->fareUsage())
      newPU->fareUsage().push_back(fu->clone(trx.dataHandle()));
  }

  return solutionPath;
}

std::string
collectSegmentInfo(const PricingTrx& trx, const FarePath* fp)
{
  std::ostringstream dc;
  const Itin* itin = fp->itin();

  if (itin->itinNum() != -1)
    dc << "[" << itin->itinNum() << "] ";

  for (const TravelSeg* segment : itin->travelSeg())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(segment);

    if (airSeg != nullptr)
    {
      dc << airSeg->carrier() << " ";
      dc << airSeg->flightNumber() << " ";
    }
  }

  if (trx.getRequest()->getBrandedFareSize() > 1)
  {
    if (fp->brandIndexPair().first == fp->brandIndexPair().second)
    {
      dc << "\nBRAND ID:" << trx.getRequest()->brandId(fp->brandIndex());
    }
    else
    {
      dc << "\nBRAND ID:" << trx.getRequest()->brandId(fp->brandIndexPair().first) << " "
         << trx.getRequest()->brandId(fp->brandIndexPair().second);
    }
  }
  else if (!trx.validBrands().empty())
  {
    dc << "\nBRAND ID:" << fp->getBrandCode();
  }

  dc << "\n";

  if (trx.isAltDates())
  {
    const DateTime& dep = itin->datePair()->first;
    const DateTime& arr = itin->datePair()->second;

    dc << dep.dateToString(DDMMM, "");

    if (!arr.isEmptyDate())
    {
      dc << " " << arr.dateToString(DDMMM, "");
    }
  }

  return dc.str();
}

void
AseItinCombiner::combineSolutions(std::vector<Itin*>& outboundItins,
                                  std::vector<Itin*>& inboundItins)
{
  LOG4CXX_DEBUG(_logger, "*** Combining AD cheapest fares ***")

  vector<FarePath*> outbSolutions;
  vector<FarePath*> inbSolutions;

  clearCache();
  if (!getSolutions(outboundItins, outbSolutions) ||
      !getSolutions(inboundItins, inbSolutions))
  {
    LOG4CXX_WARN(_logger, "No solution on direction for cheapest fares!!!")
  }

  LOG4CXX_DEBUG(_logger,
                "SolutionMap outbounds:" << outbSolutions.size()
                                         << " inbound:" << inbSolutions.size());

  combineOutboundInboundSolutions(outbSolutions, inbSolutions);
}

void
AseItinCombiner::combineOutboundInboundSolutions(std::vector<FarePath*>& outbSolutions,
                                                 std::vector<FarePath*>& inbSolutions)
{

  LOG4CXX_INFO(_logger,
               "All Solution outbound:" << outbSolutions.size()
                                        << " inbound:" << inbSolutions.size());

  set<FarePath*> processedInb;

  for (FarePath* outbOption : outbSolutions)
  {
    std::vector<FarePath*>::iterator solIt =
        find_if(inbSolutions.begin(), inbSolutions.end(), IsSameBrandSamePax(outbOption));

    if (solIt != inbSolutions.end()) // exact matching
    {
      FarePath* inbOption = *solIt;
      processedInb.insert(inbOption);

      combineItinSolutions(outbOption->brandIndex(), outbOption, inbOption);
      LOG4CXX_DEBUG(_logger, "Matched same brand:" << outbOption->brandIndex());
    }
    else
    {
      solIt = find_if(inbSolutions.begin(), inbSolutions.end(), IsSamePax(outbOption));

      if (solIt != inbSolutions.end()) // mixed matching
      {
        FarePath* inbOption = *solIt;
        processedInb.insert(inbOption);

        combineItinSolutions(outbOption->brandIndex(), outbOption, inbOption);

        LOG4CXX_DEBUG(_logger,
                      "Mixing - matched brand:" << outbOption->brandIndex()
                                                << " and brand:" << inbOption->brandIndex());
      }
    }

  } // outbOption

  inbSolutions.erase(remove_if(inbSolutions.begin(), inbSolutions.end(), ContainedIn(processedInb)),
                     inbSolutions.end());

  LOG4CXX_DEBUG(_logger, "Remaining inbounds to be matched:" << inbSolutions.size());

  // Remaining solution are mixed
  for (FarePath* inbOption : inbSolutions)
  {
    vector<FarePath*>::iterator solIt =
        find_if(outbSolutions.begin(), outbSolutions.end(), IsSamePax(inbOption));

    if (solIt != outbSolutions.end())
    {
      FarePath* outbOption = *solIt;
      combineItinSolutions(inbOption->brandIndex(), outbOption, inbOption);

      LOG4CXX_DEBUG(_logger,
                    "Mixing - matched brand:" << outbOption->brandIndex()
                                              << " and brand:" << inbOption->brandIndex());
    }
  }
}

bool
AseItinCombiner::getSolutions(const std::vector<Itin*>& itins,
                              std::vector<FarePath*>& allSolutions) const

{
  for (const Itin* itin : itins)
  {
    // TODO: check if this is still needed
    copy(itin->farePath().begin(), itin->farePath().end(), back_inserter(allSolutions));
  }

  return !allSolutions.empty();
}

void
AseItinCombiner::combineItinSolutions(uint16_t brandIndex,
                                      FarePath* outboundSol,
                                      FarePath* inboundSol)
{
  if (outboundSol->duplicate() || inboundSol->duplicate())
    return;

  Itin* resultItin = nullptr;
  Itin* outbound = outboundSol->itin();
  Itin* inbound = inboundSol->itin();
  pair<Itin*, Itin*> key = make_pair(outbound, inbound);

  // when itin pairs already processed for different brand
  if (_outboundInboundToResultCache.count(key) == 0)
  {
    LOG4CXX_DEBUG(_logger, "Caching itin, brand:" << outboundSol->brandIndex());

    _trx.dataHandle().get(resultItin);
    _outboundInboundToResultCache.insert(make_pair(key, resultItin));

    setupCombinedItin(_trx, outbound, inbound, resultItin);

    _trx.itin().push_back(resultItin);
    _trx.primeSubItinMap()[resultItin].outboundItin = outbound;
    _trx.primeSubItinMap()[resultItin].inboundItin = inbound;
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "Using cached itin, brand:" << outboundSol->brandIndex());
    resultItin = _outboundInboundToResultCache[key];
  }

  FarePath* solutionPath = combineFarePaths(_trx, outboundSol, inboundSol);
  solutionPath->brandIndex() = brandIndex; // target brand
  solutionPath->brandIndexPair() = make_pair(outboundSol->brandIndex(), inboundSol->brandIndex());
  solutionPath->parentItinPair() = make_pair(outboundSol->itin(), inboundSol->itin());

  FarePathCopier farePathCopier(_trx.dataHandle());

  if ((outboundSol->brandIndexPair().first == INVALID_BRAND_INDEX) &&
      (outboundSol->parentItinPair().first == nullptr))
  {
    outboundSol->brandIndexPair() = solutionPath->brandIndexPair();
    outboundSol->parentItinPair() = solutionPath->parentItinPair();
  }
  else
  {
    FarePath* newOutboundSol = farePathCopier.getDuplicate(*outboundSol);

    newOutboundSol->brandIndexPair() = solutionPath->brandIndexPair();
    newOutboundSol->parentItinPair() = solutionPath->parentItinPair();
    newOutboundSol->duplicate() = true;
    outbound->farePath().push_back(newOutboundSol);
  }

  if ((inboundSol->brandIndexPair().first == INVALID_BRAND_INDEX) &&
      (inboundSol->parentItinPair().first == nullptr))
  {
    inboundSol->brandIndexPair() = solutionPath->brandIndexPair();
    inboundSol->parentItinPair() = solutionPath->parentItinPair();
  }
  else
  {
    FarePath* newInboundSol = farePathCopier.getDuplicate(*inboundSol);

    newInboundSol->brandIndexPair() = solutionPath->brandIndexPair();
    newInboundSol->parentItinPair() = solutionPath->parentItinPair();
    newInboundSol->duplicate() = true;
    inbound->farePath().push_back(newInboundSol);
  }

  // create links
  resultItin->farePath().push_back(solutionPath);
  solutionPath->itin() = resultItin;
}

void
AseItinCombiner::setTripCharacteristics(Itin* itin)
{

  std::vector<TravelSeg*> tvlSegs = itin->travelSeg();
  if (tvlSegs.empty())
    return;

  itin->tripCharacteristics().set(Itin::RoundTrip, true);

  TravelSeg* firstSeg = nullptr;
  TravelSeg* lastSeg = nullptr;
  // Trip originated in US?
  firstSeg = tvlSegs[0];
  if (firstSeg->origin()->nation() == UNITED_STATES)
    itin->tripCharacteristics().set(Itin::OriginatesUS, true);

  // Trip terminates in US?
  lastSeg = tvlSegs[tvlSegs.size() - 1];
  if (lastSeg->destination()->nation() == UNITED_STATES)
    itin->tripCharacteristics().set(Itin::TerminatesUS, true);

  // Trip originates in Canada Maritime?
  firstSeg = tvlSegs[0];
  if (firstSeg->origin()->nation() == CANADA)
    itin->tripCharacteristics().set(Itin::OriginatesCanadaMaritime, true);

  // Trip US only
  TravelSeg* tvlSeg;
  bool USonly = true;
  const size_t segsSize = tvlSegs.size();
  for (size_t i = 0; i < segsSize; i++)
  {
    tvlSeg = tvlSegs[i];
    if (tvlSeg->origin()->nation() == UNITED_STATES &&
        tvlSeg->destination()->nation() == UNITED_STATES)
    {
      continue;
    }
    else
    {
      USonly = false;
      break;
    }
  }
  itin->tripCharacteristics().set(Itin::USOnly, USonly);

  // Trip Canada only
  bool CanadaOnly = true;
  for (size_t i = 0; i < segsSize; i++)
  {
    firstSeg = tvlSegs[i];
    if (firstSeg->origin()->nation() == CANADA && firstSeg->destination()->nation() == CANADA)
      continue;
    else
    {
      CanadaOnly = false;
      break;
    }
  }
  itin->tripCharacteristics().set(Itin::CanadaOnly, CanadaOnly);

  // Set geoTravelType and stopOver
  std::vector<bool> stopOver = TravelSegUtil::calculateStopOvers(tvlSegs, itin->geoTravelType());
  for (size_t i = 0; i < segsSize; i++)
  {
    _tvlSegAnalysis.setGeoTravelType(tvlSegs[i]);
    tvlSegs[i]->stopOver() = stopOver[i];
  }

  // Travel totally in or between Russian nations RU/XU
  //
  bool RussiaOnly = true;

  if (ItinUtil::isRussian(itin))
    itin->tripCharacteristics().set(Itin::RussiaOnly, RussiaOnly);
  else
  {
    RussiaOnly = false;
    itin->tripCharacteristics().set(Itin::RussiaOnly, RussiaOnly);
  }
}

void
retrieveBrandInfo(PricingTrx& trx,
                  GroupFarePathFactory& groupFarePathFactory,
                  uint16_t& brandIndex,
                  BrandCode& brandCode)
{
  brandIndex = INVALID_BRAND_INDEX;

  if (!groupFarePathFactory.getPaxFarePathFactoryBucket().empty())
  {
    PaxFarePathFactoryBase& pfpf = *groupFarePathFactory.getPaxFarePathFactoryBucket().front();
    if (!pfpf.puPathMatrixVect().empty())
    {
      if (trx.getRequest()->brandedFareEntry() || trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing())
        brandIndex = pfpf.puPathMatrixVect().front()->brandIndex();
      else if (pfpf.puPathMatrixVect().front()->brandCode())
        brandCode = *(pfpf.puPathMatrixVect().front()->brandCode());
    }
  }
}

void
setSolutionBrandInfo(PricingTrx& trx,
                     GroupFarePathFactory& groupFarePathFactory,
                     GroupFarePath& groupFPath)
{
  if (!trx.getRequest()->brandedFareEntry() &&
      !trx.getRequest()->isBrandedFaresRequest() &&
      !trx.isBRAll() &&
      !trx.activationFlags().isSearchForBrandsPricing())
    return;

  uint16_t brandIndex;
  BrandCode brandCode;
  retrieveBrandInfo(trx, groupFarePathFactory, brandIndex, brandCode);

  std::vector<FPPQItem*>::iterator it = groupFPath.groupFPPQItem().begin();
  std::vector<FPPQItem*>::iterator itEnd = groupFPath.groupFPPQItem().end();
  for (; it != itEnd; ++it)
  {
    (*it)->farePath()->brandIndex() = brandIndex;
    (*it)->farePath()->setBrandCode(brandCode);
  }
}

std::string
brandsAsString(PricingTrx const& trx, std::set<uint16_t> const& indexes)
{
  std::ostringstream ostr;
  for (uint16_t idx : indexes)
    ostr << trx.getRequest()->brandId(idx) << " ";

  return ostr.str();
}

std::string
getBrandIndexes(PricingTrx const& trx, const FarePath& fp)
{
  std::set<uint16_t> indexes;
  for (PricingUnit* pu : fp.pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
      fu->paxTypeFare()->fare()->getBrandIndexes(indexes);

  return brandsAsString(trx, indexes);
}

std::string
getBrandIndexes(PricingTrx const& trx, const PricingUnit& pu)
{
  std::set<uint16_t> indexes;
  for (FareUsage* fu : pu.fareUsage())
    fu->paxTypeFare()->fare()->getBrandIndexes(indexes);

  return brandsAsString(trx, indexes);
}

std::string
getBrandIndexes(PricingTrx const& trx, const PaxTypeFare& ptf)
{
  std::set<uint16_t> indexes;
  ptf.fare()->getBrandIndexes(indexes);

  return brandsAsString(trx, indexes);
}

bool
enableItinLevelThreading(const PricingTrx& trx)
{
  if (trx.isAltDates() && trx.getRequest()->processingDirection() == ProcessingDirection::ONEWAY &&
      trx.getRequest()->brandedFareEntry() && trx.getRequest()->getBrandedFareSize() > 1)
    return true;

  return false;
}

} // AirlineShoppingUtils
} // tse
