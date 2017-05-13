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
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcAllDirectCombinationsRequirement.h"
#include "Pricing/Shopping/Diversity/DmcAtLeastOneNSRequirement.h"
#include "Pricing/Shopping/Diversity/DmcBrandedFaresAllSopsRequirement.h"
#include "Pricing/Shopping/Diversity/DmcBrandedFaresRCOnlineRequirement.h"
#include "Pricing/Shopping/Diversity/DmcBrandedFaresTakeAllRequirement.h"
#include "Pricing/Shopping/Diversity/DmcBucketRequirement.h"
#include "Pricing/Shopping/Diversity/DmcCarrierRequirement.h"
#include "Pricing/Shopping/Diversity/DmcCustomRequirement.h"
#include "Pricing/Shopping/Diversity/DmcHdmNSRequirement.h"
#include "Pricing/Shopping/Diversity/DmcLimitLongConxRequirement.h"
#include "Pricing/Shopping/Diversity/DmcMoreNSRequirement.h"
#include "Pricing/Shopping/Diversity/DmcNSPerCarrierRequirement.h"
#include "Pricing/Shopping/Diversity/DmcNSRequirementCommonCheck.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/Diversity/DmcScheduleRepeatLimitRequirement.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <memory>
#include <vector>

namespace tse
{
class ShoppingTrx;
struct SOPInfo;

namespace shpq
{
class SoloPQItem;
}

/**
 * This class was initially present to distinguish per combination requirements
 * after required number of options is reached.
 *
 * We are altering it with getThrowAwayCombination, which shall be also evaluated prior mentioned
 * milestone event.
 */
class DmcRequirementsFacade : public DmcRequirement, public DmcRequirementsSharedContext
{
  typedef std::vector<std::vector<SOPInfo> > SOPInfos;

public:
  DmcRequirementsFacade(ItinStatistic& stats, DiagCollector* dc, ShoppingTrx& trx);

  /**
   * The only member which gives meaningful result regardless if number of options is reached
   */
  bool getThrowAwayCombination(shpq::SopIdxVecArg comb);

  Value getStatus() const { return getStatusImpl(false); }
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem);
  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price);

  bool isHdmNSActiveOnly() const
  {
    return !getStatusImpl(true) && _hdmNonStop.getStatus() &&
           !_diversity.isAdditionalNonStopsEnabled();
  }

  bool canProduceMoreNonStop() const { return _nsCommonCheck.canPQProduceMoreNonStop(); }

  void printRequirements(bool bucketsOnly = false) override;
  void printCarriersRequirements(bool directOnly = false) override;
  void printAdditionalNSRequirements();
  void printAllSopsRequirements();
  void printRCOnlineRequirement();
  void printAllDirectsRequirement();

  void setNumberSolutionsRequiredCollected(size_t numberOfSolutionsRequired,
                                           size_t numberOfSolutionsCollected,
                                           MoneyAmount score);

  /**
   * Carriers requirements adjustment is an uncertain thing without requirements document,
   * so such implementation is to follow old path for compatibility
   */
  void legacyAdjustCarrierRequirementsIfNeeded(size_t numberOfSolutionsRequired,
                                               size_t numberOfSolutionsCollected);

  bool checkAdditionalNonStopCondition(const ShoppingTrx::FlightMatrix::value_type& solution);
  int getBucketStatus(const Diversity::BucketType bucket) const
  {
    return _buckets.getBucketStatus(bucket);
  }
  void setFareCutoffReached();

  std::unique_ptr<SOPInfos>
  filterSolutionsPerLeg(MoneyAmount score, const SOPInfos& sopInfos, SOPInfos* thrownSopInfos);

private:
  Value getStatusImpl(bool shadowHdmNS) const;

  void fillSopInfosStatistics(unsigned legId,
                              const std::vector<SOPInfo>& legSopInfos,
                              SopInfosStatistics& statistics);

  bool getSopCouldSatisfy(unsigned legId,
                          const SOPInfo& sopInfo,
                          const SopInfosStatistics& statistics) const;
  bool getSopCouldSatisfyBucket(unsigned legId,
                                const SOPInfo& sopInfo,
                                const SopInfosStatistics& statistics) const;
  bool getSopCouldSatisfyNonStop(unsigned legId,
                                 const SOPInfo& sopInfo,
                                 const SopInfosStatistics& statistics) const;
  bool getSopCouldSatisfyCarrier(unsigned legId,
                                 const SOPInfo& sopInfo,
                                 const SopInfosStatistics& statistics) const;
  bool getThrowAwaySop(std::size_t legId,
                       const SOPInfo& sopInfo,
                       const SopInfosStatistics& statistics) const;

  bool _isBrandedFaresPath;

  DmcBucketRequirement _buckets;
  DmcCarrierRequirement _carriers;
  DmcCustomRequirement _custom;
  DmcLimitLongConxRequirement _longConx;
  DmcBrandedFaresAllSopsRequirement _bfAllSops;
  DmcBrandedFaresRCOnlineRequirement _bfRCOnlines;
  DmcAllDirectCombinationsRequirement _bfAllDirects;
  DmcBrandedFaresTakeAllRequirement _bfTakeAll;
  DmcScheduleRepeatLimitRequirement _scheduleRepeatLimit;

  // Non-stop requirements
  DmcAtLeastOneNSRequirement _oneNonStop;
  DmcMoreNSRequirement _moreNonStop;
  DmcHdmNSRequirement _hdmNonStop;
  DmcNSPerCarrierRequirement _nsPerCxr;

  DmcNSRequirementCommonCheck _nsCommonCheck;
};

} // ns tse

