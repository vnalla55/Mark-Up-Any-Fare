// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"
#include "Pricing/Shopping/PQ/SOPInfo.h"


#include <algorithm>
#include <climits>
#include <iomanip>

namespace tse
{

namespace
{

const DmcRequirement::Value BUCKET_FILTER_PER_LEG = DmcRequirement::NEED_LUXURY;
const DmcRequirement::Value NONSTOP_FILTER_PER_LEG = DmcRequirement::NEED_NONSTOPS |
                                                     DmcRequirement::NEED_ADDITIONAL_NONSTOPS |
                                                     DmcRequirement::NEED_NONSTOPS_CARRIERS;
const DmcRequirement::Value CARRIER_FILTER_PER_LEG = DmcRequirement::NEED_CARRIERS;
const DmcRequirement::Value ALL_FILTER_PER_LEG =
    BUCKET_FILTER_PER_LEG | NONSTOP_FILTER_PER_LEG | CARRIER_FILTER_PER_LEG;
}

DmcRequirementsFacade::DmcRequirementsFacade(ItinStatistic& stats,
                                             DiagCollector* dc,
                                             ShoppingTrx& trx)
  : DmcRequirementsSharedContext(trx.diversity(), stats, dc, trx),
    _isBrandedFaresPath(trx.getRequest()->isBrandedFaresRequest()),
    _buckets(*this),
    _carriers(*this),
    _custom(*this),
    _longConx(*this),
    _bfAllSops(*this),
    _bfRCOnlines(*this),
    _bfAllDirects(*this),
    _bfTakeAll(*this),
    _scheduleRepeatLimit(*this),
    _oneNonStop(*this),
    _moreNonStop(*this),
    _hdmNonStop(*this),
    _nsPerCxr(*this),
    _nsCommonCheck(*this)
{
}

bool
DmcRequirementsFacade::getThrowAwayCombination(shpq::SopIdxVecArg comb)
{
  return _custom.getThrowAwayCombination(comb) || _longConx.getThrowAwayCombination(comb) ||
         _scheduleRepeatLimit.getThrowAwayCombination(comb);
}

DmcRequirementsFacade::Value
DmcRequirementsFacade::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem)
{
  if (UNLIKELY(_isBrandedFaresPath))
  {
    Value result =
        (_bfAllSops.getPQItemCouldSatisfy(pqItem) | _bfRCOnlines.getPQItemCouldSatisfy(pqItem) |
         _bfTakeAll.getPQItemCouldSatisfy(pqItem));

    // in All Flights Represented Diversity Mode we don't have sop generator for direct options
    // at the beginning of the process. Hence we need to have a bucket for it
    if(_trx.getRequest()->isAllFlightsRepresented())
      result |= _bfAllDirects.getPQItemCouldSatisfy(pqItem);

    return result;
  }

  Value result = (_buckets.getPQItemCouldSatisfy(pqItem) | _carriers.getPQItemCouldSatisfy(pqItem) |
                  _custom.getPQItemCouldSatisfy(pqItem));
  Value nsResult = 0;

  /**
   * The reason of nsCommonCheck calls order is to address performance,
   * doing it from less CPU consuming check to higher ones
   */
  if (_nsCommonCheck.canPQProduceMoreNonStop())
  {
    nsResult |= _oneNonStop.getPQItemCouldSatisfy(pqItem);

    if (!_diversity.isAdditionalNonStopsEnabled())
      nsResult |= _hdmNonStop.getPQItemCouldSatisfy(pqItem);

    nsResult |= _nsPerCxr.getPQItemCouldSatisfy(pqItem);

    nsResult |= _moreNonStop.getCouldSatisfyAdjustment(result | nsResult);

    if (nsResult && !_nsCommonCheck.getPQItemCouldSatisfy(pqItem))
      nsResult = 0;
  }

  return (result | nsResult);
}

DmcRequirementsFacade::Value
DmcRequirementsFacade::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price)
{
  if (getThrowAwayCombination(comb))
    return 0;

  if (UNLIKELY(_isBrandedFaresPath))
  {
    Value result = (_bfAllSops.getCombinationCouldSatisfy(comb, price) |
                    _bfRCOnlines.getCombinationCouldSatisfy(comb, price) |
                    _bfTakeAll.getCombinationCouldSatisfy(comb, price));

    if (_trx.getRequest()->isAllFlightsRepresented())
      result |= _bfAllDirects.getCombinationCouldSatisfy(comb, price);

    return result;
  }

  Value result = (_buckets.getCombinationCouldSatisfy(comb, price) |
                  _carriers.getCombinationCouldSatisfy(comb, price) |
                  _custom.getCombinationCouldSatisfy(comb));
  Value nsResult = 0;

  /**
   * The reason of nsCommonCheck calls order is to address performance,
   * doing it from less CPU consuming check to higher ones
   */
  if (_nsCommonCheck.canPQProduceMoreNonStop())
  {
    nsResult |= _oneNonStop.getCombinationCouldSatisfy(comb, price);

    if (!_diversity.isAdditionalNonStopsEnabled())
      nsResult |= _hdmNonStop.getCombinationCouldSatisfy(comb, price);

    nsResult |= _nsPerCxr.getCombinationCouldSatisfy(comb, price);

    nsResult |= _moreNonStop.getCouldSatisfyAdjustment(result | nsResult);

    if (nsResult && !_nsCommonCheck.getCombinationCouldSatisfy(comb, nsResult))
      nsResult = 0;
  }

  return (result | nsResult);
}

bool
DmcRequirementsFacade::checkAdditionalNonStopCondition(
    const ShoppingTrx::FlightMatrix::value_type& solution)
{
  if (!_diversity.isAdditionalNonStopsEnabled())
    return false; // This logic is not enabled

  return (getCombinationCouldSatisfy(solution.first, solution.second->getTotalNUCAmount()) ==
          NEED_ADDITIONAL_NONSTOPS);
}

void
DmcRequirementsFacade::printRequirements(bool bucketsOnly)
{
  if (!_dc)
    return;

  _buckets.print();

  if (!bucketsOnly)
  {
    // Print non-stop side to buckets
    *_dc << "     N: " << std::setw(3) << _stats.getNonStopsCount();
    if (!_diversity.isHighDensityMarket())
      *_dc << "/1";
    else
      *_dc << "/" << _diversity.getMaxOnlineNonStopCount();
    *_dc << "\n";

    printCarriersRequirements();

    *_dc << "\tTOD: ";
    for (size_t i = 0; i < _diversity.getTODRanges().size(); i++)
    {
      *_dc << i << "[" << _stats.getTODBucketSize(i) << "/"
           << (size_t)(_diversity.getTODDistribution()[i] *
                       _diversity.getNumberOfOptionsToGenerate()) << "] ";
    }

    printAdditionalNSRequirements();
    printAllSopsRequirements();
    printAllDirectsRequirement();
    printRCOnlineRequirement();

    _custom.print();
  }

  *_dc << "\n";
}

void
DmcRequirementsFacade::printCarriersRequirements(bool directOnly)
{
  if (!_dc)
    return;

  if (!directOnly)
  {
    *_dc << "\t" << std::setw(_nsPerCxr.isEffective() ? 12 : 0) << "Carriers:";
    _carriers.print();
    if (_carriers.isOCOAndOCOMapAvailable())
    {
      *_dc << "\t" << std::setw(_nsPerCxr.isEffective() ? 12 : 0) << "Online Carriers:";
          _carriers.printOCOIfSet();
    }
  }

  _nsPerCxr.print();
}

void
DmcRequirementsFacade::printAdditionalNSRequirements()
{
  typedef std::pair<CarrierCode, size_t> NonStopPerCarrier;

  size_t interlineCount = 0;

  *_dc << "\n\tAdditional NS: " << _stats.getAdditionalNonStopsCount();
  *_dc << "\n\tAdditional NS per cxr:";
  for (const NonStopPerCarrier& nsCxr : _stats.getAdditionalNonStopCountPerCarrier())
  {
    if (nsCxr.first == Diversity::INTERLINE_CARRIER)
    {
      interlineCount = nsCxr.second;
      continue;
    }
    *_dc << " " << nsCxr.first << "[" << nsCxr.second << "]";
  }
  *_dc << " Interline[" << interlineCount << "]\n";
}

void
DmcRequirementsFacade::printAllSopsRequirements()
{
  if (!_dc || !_trx.getRequest()->isBrandedFaresRequest())
    return;

  *_dc << "\tUse all sops: OB [" << _stats.getNumOfUniqueSops(0) << "/"
       << _trx.legs()[0].requestSops() << "]";
  if (_trx.legs().size() > 1)
    *_dc << " IB [" << _stats.getNumOfUniqueSops(1) << "/" << _trx.legs()[1].requestSops() << "]";
  *_dc << "\n";
}

void
DmcRequirementsFacade::printRCOnlineRequirement()
{
  if (!_dc || !_trx.getRequest()->isBrandedFaresRequest())
    return;

  *_dc << "\tRequesting Carrier Online Combinations/Goal [" << _stats.getRCOnlineOptionsCount()
       << "/";
  *_dc << _stats.getRCOnlineOptionsCount() + _stats.getMissingRCOnlineOptionsCount() << "]";
  *_dc << "\n";
}

  void
DmcRequirementsFacade::printAllDirectsRequirement()
{
  if (!_dc || !_trx.getRequest()->isBrandedFaresRequest() || !_trx.getRequest()->isAllFlightsRepresented())
    return;

  *_dc << "\tDirect Combinations/Goal [" << _stats.getDirectOptionsCount()
    << "/";
  *_dc << _stats.getDirectOptionsCount() + _stats.getMissingDirectOptionsCount() << "]";
  *_dc << "\n";

}

void
DmcRequirementsFacade::setNumberSolutionsRequiredCollected(size_t numberOfSolutionsRequired,
                                                           size_t numberOfSolutionsCollected,
                                                           MoneyAmount score)
{
  _buckets.setNumberSolutionsRequiredCollected(
      numberOfSolutionsRequired, numberOfSolutionsCollected, score);
}

void
DmcRequirementsFacade::legacyAdjustCarrierRequirementsIfNeeded(size_t numberOfSolutionsRequired,
                                                               size_t numberOfSolutionsCollected)
{
  _carriers.adjustCarrierRequirementsIfNeeded(numberOfSolutionsRequired,
                                              numberOfSolutionsCollected);
}

void
DmcRequirementsFacade::setFareCutoffReached()
{
  _buckets.setFareCutoffReached();
  _custom.setFareCutoffReached();
  _carriers.setFareCutoffReached();
  _oneNonStop.setFareCutoffReached();
}

std::unique_ptr<DmcRequirementsFacade::SOPInfos>
DmcRequirementsFacade::filterSolutionsPerLeg(MoneyAmount score,
                                             const SOPInfos& sopInfos,
                                             SOPInfos* thrownSopInfos)
{
  std::unique_ptr<SOPInfos> filteredSopInfos;
  if (sopInfos.size() != 2)
  {
    return filteredSopInfos;
  }

  const Value status = getStatus();
  if (UNLIKELY(status == 0))
  {
    return filteredSopInfos;
  }

  if (status & ~ALL_FILTER_PER_LEG)
  {
    return filteredSopInfos;
  }

  SopInfosStatistics statistics(sopInfos.size(), score, status);
  for (unsigned legId = 0; legId < sopInfos.size(); ++legId)
  {
    fillSopInfosStatistics(legId, sopInfos[legId], statistics);
  }

  filteredSopInfos.reset(new SOPInfos(sopInfos.size()));
  if (UNLIKELY(thrownSopInfos))
  {
    thrownSopInfos->resize(sopInfos.size());
  }

  for (unsigned legId = 0; legId < sopInfos.size(); ++legId)
  {
    const std::vector<SOPInfo>& legSopInfos = sopInfos[legId];
    std::vector<SOPInfo>& legSopInfosFiltered = filteredSopInfos->at(legId);
    legSopInfosFiltered.reserve(legSopInfos.size());
    if (UNLIKELY(thrownSopInfos))
    {
      thrownSopInfos->at(legId).reserve(legSopInfos.size());
    }

    for (const auto& sopInfo : legSopInfos)
    {
      if (!SopCombinationUtil::isValidForCombination(_trx, legId, sopInfo))
      {
        continue;
      }

      if (getThrowAwaySop(legId, sopInfo, statistics))
        continue;

      if (getSopCouldSatisfy(legId, sopInfo, statistics))
      {
        legSopInfosFiltered.push_back(sopInfo);
      }
      else if (UNLIKELY(thrownSopInfos))
      {
        thrownSopInfos->at(legId).push_back(sopInfo);
      }
    }
  }

  return filteredSopInfos;
}

DmcRequirementsFacade::Value
DmcRequirementsFacade::getStatusImpl(bool shadowHdmNS) const
{
  if (UNLIKELY(_isBrandedFaresPath))
  {
    Value result = (_bfAllSops.getStatus() | _bfRCOnlines.getStatus() | _bfTakeAll.getStatus());

    if (_trx.getRequest()->isAllFlightsRepresented())
      result |= _bfAllDirects.getStatus();

    return result;
  }

  Value result = (_buckets.getStatus() | _carriers.getStatus() | _custom.getStatus());

  if (_nsCommonCheck.canPQProduceMoreNonStop())
  {
    result |= _oneNonStop.getStatus();

    if (_diversity.isAdditionalNonStopsEnabled())
      result |= _moreNonStop.getStatus();
    else
      result |= (shadowHdmNS ? 0 : _hdmNonStop.getStatus());

    result |= _nsPerCxr.getStatus();
  }

  return result;
}

void
DmcRequirementsFacade::fillSopInfosStatistics(unsigned legId,
                                              const std::vector<SOPInfo>& legSopInfos,
                                              SopInfosStatistics& statistics)
{
  const ShoppingTrx::Leg& leg = _trx.legs().at(legId);
  int32_t minimumFlightTimeMinutes = INT_MAX;
  for (const auto& sopInfo : legSopInfos)
  {
    if (!SopCombinationUtil::isValidForCombination(_trx, legId, sopInfo))
    {
      continue;
    }

    const ShoppingTrx::SchedulingOption& sop = leg.sop().at(sopInfo._sopIndex);
    minimumFlightTimeMinutes =
        std::min(minimumFlightTimeMinutes, SopCombinationUtil::getDuration(sop));
  }

  statistics.minimumFlightTimeMinutes.at(legId) = minimumFlightTimeMinutes;
}

bool
DmcRequirementsFacade::getSopCouldSatisfy(unsigned legId,
                                          const SOPInfo& sopInfo,
                                          const SopInfosStatistics& statistics) const
{
  return getSopCouldSatisfyBucket(legId, sopInfo, statistics) ||
         getSopCouldSatisfyNonStop(legId, sopInfo, statistics) ||
         getSopCouldSatisfyCarrier(legId, sopInfo, statistics);
}

bool
DmcRequirementsFacade::getSopCouldSatisfyBucket(unsigned legId,
                                                const SOPInfo& sopInfo,
                                                const SopInfosStatistics& statistics) const
{
  if (statistics.status & BUCKET_FILTER_PER_LEG)
  {
    return _buckets.getSopCouldSatisfy(legId, sopInfo, statistics);
  }

  return false;
}

bool
DmcRequirementsFacade::getSopCouldSatisfyNonStop(unsigned legId,
                                                 const SOPInfo& sopInfo,
                                                 const SopInfosStatistics& statistics) const
{
  if (statistics.status & NONSTOP_FILTER_PER_LEG)
  {
    return _nsCommonCheck.getSopCouldSatisfy(legId, sopInfo, statistics);
  }

  return false;
}

bool
DmcRequirementsFacade::getSopCouldSatisfyCarrier(unsigned legId,
                                                 const SOPInfo& sopInfo,
                                                 const SopInfosStatistics& statistics) const
{
  if (statistics.status & CARRIER_FILTER_PER_LEG)
  {
    return _carriers.getSopCouldSatisfy(legId, sopInfo, statistics);
  }

  return false;
}

bool
DmcRequirementsFacade::getThrowAwaySop(std::size_t legId,
                                       const SOPInfo& sopInfo,
                                       const SopInfosStatistics& statistics) const
{
  return _longConx.getThrowAwaySop(legId, sopInfo._sopIndex);
}

} // ns tse
