//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/IBF/ISopUsageTracker.h"
#include "Pricing/Shopping/Swapper/IObservableItemSet.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Utils/PrettyPrint.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"
#include "Pricing/Shopping/Utils/SopCombinationsGenerator.h"
#include "Pricing/Shopping/Utils/SopRankedGenerator.h"

#include <boost/unordered_map.hpp>

#include <memory>
#include <iostream>
#include <sstream>
#include <string>

namespace tse
{

namespace utils
{

struct SopPenaltyRecord
{
  size_t legId;
  size_t sopId;
  size_t usage_count;
  size_t penalty;
  size_t rank;
};

std::ostream& operator<<(std::ostream& out, const SopPenaltyRecord& r)
{
  BlockMaker n(2, false);
  BlockMaker m(3, false);
  out << "[" << r.legId << "," << n(r.sopId) << "] ";
  out << "U:" << m(r.usage_count) << ", ";
  out << "P:" << m(r.penalty) << ", ";
  out << "R:" << m(r.rank);
  return out;
}

class ISopPenaltyBank
{
public:
  virtual const SopPenaltyRecord& getPenaltyRecord(
      const utils::SopEntry& entry) const = 0;
  virtual ~ISopPenaltyBank(){}
};

//           { 0          for n == 0
// Penalty = { (n + 1)^3  for 0 < n < 50,
//           { 125000       for n >= 50,
// where n is usage_count for a Sop.

// usage_count  penalty
// 0    0
// 1    8
// 2    27
// 3    64
// 4    125
// 5    216
// 6    343
// 7    512
// 8    729
// 9    1000
// 10   1331
// ...
// 48   117649
// 49   125000
// 50   125000
class BasicPenaltyPolicy
{
public:

  SopPenaltyRecord createPenaltyRecord(const utils::SopEntry& entry,
                                       size_t usageCount,
                                       size_t returnAllFlightsLegID)
  {
    SopPenaltyRecord record;
    record.legId = entry.legId;
    record.sopId = entry.sopId;
    record.usage_count = usageCount;

    if (record.legId == returnAllFlightsLegID && usageCount > 0)
      record.penalty = MAX_PENALTY;
    else
      record.penalty = calculatePenalty(record.usage_count);

    record.rank = record.sopId + record.penalty;
    return record;
  }

private:
  size_t calculatePenalty(size_t usage_count)
  {
    if (usage_count == 0)
    {
      return 0;
    }

    if (usage_count >= THRESHOLD)
    {
      return MAX_PENALTY;
    }

    const size_t incremented = usage_count + 1;
    return incremented * incremented * incremented;
  }

  static const size_t THRESHOLD = 50;
  static const size_t MAX_PENALTY = 125000;
};


class PenaltyComparator
{
public:
  PenaltyComparator(const ISopPenaltyBank& penaltyBank, size_t legId):
    _penaltyBank(penaltyBank), _legId(legId){}

  bool operator()(size_t sopIdLeft, size_t sopIdRight)
  {
    const size_t lRank = _penaltyBank.getPenaltyRecord(
        utils::SopEntry(_legId, sopIdLeft)).rank;
    const size_t rRank = _penaltyBank.getPenaltyRecord(
        utils::SopEntry(_legId, sopIdRight)).rank;
    return lRank < rRank;
  }

private:
  const ISopPenaltyBank& _penaltyBank;
  size_t _legId;
};



template<typename PenaltyPolicyT>
class SopPenaltyGenerator:
    public BaseSopCombinationsGenerator,
    public swp::IItemSetObserver<utils::SopCombination>,
    public ISopPenaltyBank
{
public:
  typedef swp::IObservableItemSet<utils::SopCombination> ImplObservableItemSet;

  SopPenaltyGenerator(
    ImplObservableItemSet& itemSet,
    ISopUsageCounter& usageCounter,
    ILogger* logger = nullptr,
    ISopCombinationsGeneratorFactory* childGenFactory = new BasicSopRankedGeneratorFactory(),
    PenaltyPolicyT* policy = new PenaltyPolicyT(),
    ISopBank* sopBank = new SopBank()):
      BaseSopCombinationsGenerator(sopBank),
      _itemSet(itemSet),
      _usageCounter(usageCounter),
      _returnAllFlightLegID(-1)
  {
    _policy.reset(policy);
    _childGenFactory.reset(childGenFactory);
    if (logger != nullptr)
    {
      _log.install(logger);
    }
  }

  ~SopPenaltyGenerator()
  {
    _itemSet.removeItemSetObserver(this);
  }

  void itemAdded(const utils::SopCombination& item) override
  {
    if (_log->enabled(utils::LOGGER_LEVEL::INFO))
    {
      std::ostringstream tmp;
      tmp << item;
      _log->info(Fmt("Received itemAdded event for %s") % tmp.str());
    }

    updatePenaltyMapForCombination(item);
    sortSopsAndReloadChild();
  }

  void itemRemoved(const utils::SopCombination& item) override
  {
    if (_log->enabled(utils::LOGGER_LEVEL::INFO))
    {
      std::ostringstream tmp;
      tmp << item;
      _log->info(Fmt("Received itemRemoved event for %s") % tmp.str());
    }

    updatePenaltyMapForCombination(item);
    sortSopsAndReloadChild();
  }

  const SopPenaltyRecord& getPenaltyRecord(const utils::SopEntry& entry) const override
  {
    PenaltyMap::const_iterator found = _penaltyMap.find(entry);
    TSE_ASSERT(found != _penaltyMap.end());
    return found->second;
  }

  void setReturnAllFlightLegID(unsigned int legID)
  {
    _returnAllFlightLegID = legID;
  }

protected:

  void initialize() override
  {
    initPenaltyMap();
    sortSopsAndReloadChild();
    _log->info("Registering as ISopUsageObserver");
    _itemSet.addItemSetObserver(this);
  }

  SopCombination nextElement() override
  {
    return _childGenerator->next();
  }


private:

  static const size_t STATUS_COLUMN_WIDTH = 31;

  void updatePenaltyMapForCombination(const utils::SopCombination& combination)
  {
    for (size_t legId = 0; legId < combination.size(); ++legId)
    {
      SopEntry entry(legId, combination[legId]);
      _penaltyMap[entry] = makePenaltyRecordForSop(entry);
    }
  }

  SopPenaltyRecord makePenaltyRecordForSop(const SopEntry& entry)
  {
    return _policy->createPenaltyRecord(entry,
                                        _usageCounter.getUsageCount(entry.legId, entry.sopId),
                                        _returnAllFlightLegID);
  }

  void initPenaltyMap()
  {
    for (size_t legId = 0; legId < _userInputSops->getNumberOfLegs(); ++legId)
    {
      const SopCombination& sopsOnLeg = _userInputSops->getSopsOnLeg(legId);
      for (size_t j = 0; j < sopsOnLeg.size(); ++j)
      {
        const size_t sopId = sopsOnLeg[j];
        const SopEntry entry(legId, sopId);
        _penaltyMap[entry] = makePenaltyRecordForSop(entry);
      }
    }
  }

  void sortSopsAndReloadChild()
  {
    sortInputSops();
    reloadChildGenerator();
    if (_log->enabled(utils::LOGGER_LEVEL::INFO))
    {
      _log->info(statusString());
    }
  }

  void sortInputSops()
  {
    for (size_t legId = 0; legId < _userInputSops->getNumberOfLegs(); ++legId)
    {
      sortLeg(legId, _userInputSops->getSopsOnLeg(legId));
    }
  }

  void sortLeg(size_t legId, SopCombination& leg)
  {
    PenaltyComparator comp(*this, legId);
    std::stable_sort(leg.begin(), leg.end(), comp);
  }

  void reloadChildGenerator()
  {
    _childGenerator.reset(_childGenFactory->create());
    const unsigned int legsCount = _userInputSops->getNumberOfLegs();

    _childGenerator->setNumberOfLegs(legsCount);
    for (size_t legId = 0; legId < legsCount; ++legId)
    {
      const SopCombination& sopsOnLeg = _userInputSops->getSopsOnLeg(legId);
      for (size_t j = 0; j < sopsOnLeg.size(); ++j)
      {
        _childGenerator->addSop(legId, sopsOnLeg[j]);
      }
    }
  }

  std::string statusString() const
  {
    using namespace std;
    ostringstream out;
    out << "Penalty generator: top SOPs on legs:" << endl;

    const size_t legCount = _userInputSops->getNumberOfLegs();
    const size_t TOP_LENGTH = 5;

    BlockMaker c(STATUS_COLUMN_WIDTH);
    for (size_t i = 0; i < TOP_LENGTH; ++i)
    {
      for (size_t legId = 0; legId < legCount; ++legId)
      {
        const SopCombination& sopsOnLeg = _userInputSops->getSopsOnLeg(legId);
        if (i < sopsOnLeg.size())
        {
          SopEntry entry(legId, sopsOnLeg[i]);
          ostringstream tmp;
          tmp << getPenaltyRecord(entry);
          out << c(tmp.str());
        }
        else
        {
          out << c("---");
        }
      }
      out << endl;
    }
    return out.str();
  }

  typedef boost::unordered_map<utils::SopEntry, SopPenaltyRecord> PenaltyMap;
  typedef std::shared_ptr<PenaltyPolicyT> PenaltyPolicyPtr;
  typedef std::shared_ptr<ISopCombinationsGenerator> ChildGeneratorPtr;
  typedef std::shared_ptr<ISopCombinationsGeneratorFactory> ChildGeneratorFactoryPtr;
  PenaltyMap _penaltyMap;
  ImplObservableItemSet& _itemSet;
  ISopUsageCounter& _usageCounter;
  PenaltyPolicyPtr _policy;
  ChildGeneratorFactoryPtr _childGenFactory;
  ChildGeneratorPtr _childGenerator;
  utils::LoggerHandle _log;
  unsigned int _returnAllFlightLegID;
};

typedef SopPenaltyGenerator<BasicPenaltyPolicy> BasicSopPenaltyGenerator;


} // namespace utils

} // namespace tse


