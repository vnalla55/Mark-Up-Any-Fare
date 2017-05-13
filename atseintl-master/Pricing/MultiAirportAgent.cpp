#include "Pricing/MultiAirportAgent.h"

#include "Common/Logger.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/ShoppingPQ.h"

namespace tse
{
namespace
{
Logger
logger("atseintl.PricingOrchestrator.MultiAirportAgent");
}

void
MultiAirportAgent::perform()
{
  for (const auto& pq : _trx->shoppingPQVector())
    if (!(*pq).isInterline())
      if (!existsAirportSolution(*pq))
        if (possibleAirportSolution((*pq).carrier()))
          addAirportsolution(*pq);
}

bool
MultiAirportAgent::existsAirportSolution(ShoppingPQ& pq) const
{
  LOG4CXX_DEBUG(logger, (pq.carrier() ? *pq.carrier() : "NULL"));
  logDebugInfoMatrix(pq);
  const ShoppingTrx::FlightMatrix& solutions = pq.getFlightMatrix();
  for (const auto& solution : solutions)
  {
    const SopIdVec& sopVec = solution.first;

    if (isAirportSolution(sopVec))
    {
      LOG4CXX_DEBUG(logger, (pq.carrier() ? *pq.carrier() : "NULL") + " exists solution");
      return true;
    }
  }

  LOG4CXX_DEBUG(logger, (pq.carrier() ? *pq.carrier() : "NULL") << " does not exist solution");
  return false;
}

bool
MultiAirportAgent::isAirportSolution(const SopIdVec& sopVec) const
{
  bool result = isAirportSolution(getSop(sopVec, 0), false);
  if (result && (sopVec.size() == 2))
    result = isAirportSolution(getSop(sopVec, 1), true);
  return result;
}

bool
MultiAirportAgent::possibleAirportSolution(const CarrierCode* const carrier) const
{
  bool result = true;
  if (!carrier)
  {
    LOG4CXX_DEBUG(logger, "possibleAirportSolution, carrier NULL, return false")
    return false;
  }

  const std::vector<ShoppingTrx::Leg>& legVec = _trx->legs();
  for (size_t i = 0; i < legVec.size() && result; ++i)
    result &= possibleAirportSolution(carrier, legVec[i], (i == 1));

  LOG4CXX_DEBUG(logger, *carrier << " possible solution: " << (result ? "true" : "false"));
  return result;
}

bool
MultiAirportAgent::possibleAirportSolution(const CarrierCode* const carrier,
                                           const ShoppingTrx::Leg& leg,
                                           const bool& oppositePoint) const
{
  const std::vector<ShoppingTrx::SchedulingOption>& sopVec = leg.sop();

  for (const auto& sop : sopVec)
  {
    if ((sop.governingCarrier() == *carrier) && (isAirportSolution(sop, oppositePoint)))
      return true;
  }
  return false;
}

void
MultiAirportAgent::addAirportsolution(ShoppingPQ& pq) const
{
  LOG4CXX_DEBUG(logger, *(pq.carrier()) << " addAirportsolution")
  ShoppingTrx::FlightMatrix::const_iterator minFam;
  ShoppingTrx::FlightMatrix::const_iterator maxFam;
  bool minFound = false;
  bool maxFound = false;
  findMinMaxFamily(pq, minFam, maxFam, minFound, maxFound);

  if (minFound)
  {
    // data for debug info
    const int fms = pq.getFlightMatrix().size();
    const int efms = pq.getEstimateMatrix().size();
    // add solutions
    pq.setMultiAirportAgent(this);
    pq.processSolution(minFam->second, true);
    pq.setMultiAirportAgent(nullptr);

    // replace the most expensive solutions with new ones if enough has been
    // found
    if (maxFound)
    {
      const int addedSolutionsSize =
          pq.getFlightMatrix().size() + pq.getEstimateMatrix().size() - fms - efms;
      const int maxFamilySize = getFamilySize(pq, maxFam);
      LOG4CXX_DEBUG(logger,
                    "addedSolutionsSize = " << addedSolutionsSize
                                            << ", maxFamilySize = " << maxFamilySize);
      LOG4CXX_DEBUG(logger, "maxFamily head:");
      logDebugInfoSolution(maxFam->first, maxFam->second);

      if (addedSolutionsSize >= maxFamilySize)
        removeFamily(pq, *maxFam);
    }

    // log debug info
    logDebugInfo(pq, fms, efms);
  }
  else
    LOG4CXX_DEBUG(logger, "no solutions for PQ");
}

void
MultiAirportAgent::removeFamily(ShoppingPQ& pq, const ShoppingTrx::FlightMatrix::value_type& v)
    const
{
  LOG4CXX_DEBUG(logger, "removeFamily");
  // delete from estimated soluitons
  ShoppingTrx::EstimateMatrix::iterator it = pq.estimateMatrix().begin();

  while (it != pq.estimateMatrix().end())
    if (it->second.first == v.first)
      pq.estimateMatrix().erase(it++);
    else
      ++it;

  // delete family's head
  pq.flightMatrix().erase(v.first);
}

int
MultiAirportAgent::getFamilySize(const ShoppingPQ& pq,
                                 const ShoppingTrx::FlightMatrix::const_iterator fit) const
{
  int size = 1; // fit is 1
  // count all solutions with fit as a parent
  for (const auto& estimateMatrixElement : pq.getEstimateMatrix())
    if (estimateMatrixElement.second.first == fit->first)
      ++size;

  return size;
}

void
MultiAirportAgent::findMinMaxFamily(const ShoppingPQ& pq,
                                    ShoppingTrx::FlightMatrix::const_iterator& minFam,
                                    ShoppingTrx::FlightMatrix::const_iterator& maxFam,
                                    bool& minFound,
                                    bool& maxFound) const
{
  minFound = false;
  maxFound = false;
  ShoppingTrx::FlightMatrix::const_iterator fit = pq.getFlightMatrix().begin();
  const ShoppingTrx::FlightMatrix::const_iterator fitEnd = pq.getFlightMatrix().end();

  for (; fit != fitEnd; ++fit)
  {
    if (fit->second == nullptr)
      continue;

    // look for min
    if ((fit->second->getTotalNUCAmount() != 0) && (fit->second->getTotalNUCAmount() != 1e6))
    {
      if (!minFound) // min haven't been found yet
      {
        minFam = fit;
        minFound = true;
      }
      else // update min if cheaper found
      {
        if (minFam->second->getTotalNUCAmount() > fit->second->getTotalNUCAmount())
          minFam = fit;
      }
    }

    // look for max
    if (fit->second->getTotalNUCAmount() != 0)
    {
      if (!maxFound) // max haven't been found yet
      {
        maxFam = fit;
        maxFound = true;
      }
      else // update max if more expensive or equal found
      {
        if (maxFam->second->getTotalNUCAmount() <= fit->second->getTotalNUCAmount())
          maxFam = fit;
      }
    }
  }
}

ShoppingTrx::SchedulingOption&
MultiAirportAgent::getSop(const SopIdVec& sopVec, const int i) const
{
  return _trx->legs()[i].sop()[sopVec[i]];
}

const std::vector<TravelSeg*>&
MultiAirportAgent::getTSvec(const ShoppingTrx::SchedulingOption& sop) const
{
  return sop.itin()->travelSeg();
}

bool
MultiAirportAgent::isFrontAirport(const std::vector<TravelSeg*>& tsVec) const
{
  return isAirportSolution(tsVec.front()->origin());
}

bool
MultiAirportAgent::isBackAirport(const std::vector<TravelSeg*>& tsVec) const
{
  return isAirportSolution(tsVec.back()->destination());
}

bool
MultiAirportAgent::isAirportSolution(const ShoppingTrx::SchedulingOption& sop,
                                     const bool& oppositePoint) const
{
  if (_atOrigin ^ oppositePoint) // Bitwise XOR (exclusive or)
    return isFrontAirport(getTSvec(sop));
  else
    return isBackAirport(getTSvec(sop));
}

bool
MultiAirportAgent::isAirportSolution(const Loc* const loc) const
{
  return loc->loc() == _airport;
}

void
MultiAirportAgent::init(ShoppingTrx* trx,
                        const LocCode& city,
                        const LocCode& airport,
                        const bool& atOrigin)
{
  _trx = trx;
  _city = city;
  _airport = airport;
  _atOrigin = atOrigin;
}

void
MultiAirportAgent::logDebugInfo(const ShoppingPQ& pq, const int& fms, const int& efms) const
{
  LOG4CXX_DEBUG(logger, *(pq.carrier()) << " Flight Matrix size before   : " << fms);
  LOG4CXX_DEBUG(
      logger, *(pq.carrier()) << " Flight Matrix size after    : " << pq.getFlightMatrix().size());
  LOG4CXX_DEBUG(logger, *(pq.carrier()) << " Estimated Matrix size before: " << efms);
  LOG4CXX_DEBUG(
      logger,
      *(pq.carrier()) << " Estimated Matrix size after : " << pq.getEstimateMatrix().size());
  logDebugInfoMatrix(pq);
}

void
MultiAirportAgent::logDebugInfoMatrix(const ShoppingPQ& pq) const
{
  if (!IS_DEBUG_ENABLED(logger))
    return;

  LOG4CXX_DEBUG(logger, "Flight Matrix:");
  logDebugInfoFlightMatrix(pq.getFlightMatrix());
  LOG4CXX_DEBUG(logger, "Estimated Matrix:");
  logDebugInfoEstimateMatrix(pq.getEstimateMatrix());
}

void
MultiAirportAgent::logDebugInfoFlightMatrix(const ShoppingTrx::FlightMatrix& matrix) const
{
  for (const auto& solution : matrix)
    logDebugInfoSolution(solution.first, solution.second);
}

void
MultiAirportAgent::logDebugInfoEstimateMatrix(const ShoppingTrx::EstimateMatrix& matrix) const
{
  ShoppingTrx::EstimateMatrix::const_iterator solIt = matrix.begin();
  const ShoppingTrx::EstimateMatrix::const_iterator solItEnd = matrix.end();

  for (int i = 0; solIt != solItEnd; ++solIt, ++i)
    logDebugInfoSolution(solIt->first, solIt->second.second, &solIt->second.first);
}

void
MultiAirportAgent::logDebugInfoSolution(const SopIdVec& sopVec,
                                        GroupFarePath* const gfp,
                                        const SopIdVec* const parentSopVec) const
{
  if (!IS_DEBUG_ENABLED(logger))
    return;

  std::stringstream s;

  // parent sop info for Estimated Solution
  if (parentSopVec)
    s << getSopVecInfo(*parentSopVec) << " ";

  // sop info for Solution
  s << getSopVecInfo(sopVec);
  // carrier + flightNumber + origin + destination
  // eg. KL4606DFWATL
  s << " out: " << getSopDebugInfo(getSop(sopVec, 0));

  if (sopVec.size() == 2)
    s << " in: " << getSopDebugInfo(getSop(sopVec, 1));

  // eg. gfp_nuc: 1399 fp: HHW7AP 714.5USD HHX7AP 684.5USD
  if (gfp)
    s << " gfp_nuc: " << getGFPinfo(gfp);

  LOG4CXX_DEBUG(logger, s.str());
}

std::string
MultiAirportAgent::getSopVecInfo(const SopIdVec& sopVec) const
{
  std::stringstream s;
  // sop indices
  s << "(" << sopVec[0];

  if (sopVec.size() == 2)
    s << "," << sopVec[1];

  s << ")";
  return s.str();
}

std::string
MultiAirportAgent::getGFPinfo(GroupFarePath* const gfp) const
{
  std::stringstream result;
  result << gfp->getTotalNUCAmount();
  for (const auto fppqItem : gfp->groupFPPQItem())
    result << " fp: " << getFPinfo(fppqItem);

  return result.str();
}

std::string
MultiAirportAgent::getFPinfo(FPPQItem* const item) const
{
  std::stringstream result;

  for (const auto pricingUnit : item->farePath()->pricingUnit())
    result << getFPinfo(pricingUnit) << " ";

  return result.str();
}

std::string
MultiAirportAgent::getFPinfo(PricingUnit* const pu) const
{
  std::stringstream result;
  for (const auto fareUsage : pu->fareUsage())
    result << fareUsage->paxTypeFare()->fare()->fareClass() << " "
           << fareUsage->paxTypeFare()->fareAmount() << fareUsage->paxTypeFare()->currency() << " ";

  return result.str();
}

std::string
MultiAirportAgent::getSopDebugInfo(const ShoppingTrx::SchedulingOption& sop) const
{
  std::stringstream result;
  for (const auto travelSegment : sop.itin()->travelSeg())
  {
    if (travelSegment->isAir())
    {
      const AirSeg* as = travelSegment->toAirSeg();
      if (as)
      {
        result << as->carrier();
        result << as->flightNumber();
      }
      else
        LOG4CXX_ERROR(logger, "AirSeg* is null");
    }

    result << travelSegment->origin()->loc();
    result << travelSegment->destination()->loc();
    result << " ";
  }

  return result.str();
}

} // tse
