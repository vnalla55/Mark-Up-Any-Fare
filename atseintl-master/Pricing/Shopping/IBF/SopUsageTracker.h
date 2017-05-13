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

#include "Pricing/Shopping/IBF/ISopUsageTracker.h"
#include "Pricing/Shopping/Utils/UsageCounter.h"

#include <boost/noncopyable.hpp>

#include <sstream>

namespace tse
{

class ShoppingTrx;


class SopUsageTracker: public ISopUsageTracker,
    public ISopUsageCounter,
    boost::noncopyable
{
public:

  typedef utils::UsageCounter<utils::SopEntry> SopUsageCounter;
  typedef SopUsageCounter::ElementSet::const_iterator UnusedSopsIterator;
  // Iterator over usage of sops with usage > 0
  typedef SopUsageCounter::UsageCountMap::const_iterator SopsUsageIterator;
  typedef std::map<unsigned int, unsigned int> SopCountPerLeg;

  SopUsageTracker(const ShoppingTrx& trx) : _trx(trx){}

  bool combinationAdded(const utils::SopCombination& comb) override;
  void combinationRemoved(const utils::SopCombination& comb) override;

  void addSopForTracking(unsigned int legId, uint32_t sopId);

  // Returns the number of SOPs tracked
  unsigned int getNbrOfSops() const override { return _sopUsage.getNbrOfElements(); }

  // Returns all tracked SOPs
  SopUsageCounter::ElementSet getTrackedSops() const
  {
    return _sopUsage.getAllElements();
  }

  // Returns mapping legId -> number of tracked SOPs
  SopCountPerLeg getTrackedSopCountPerLeg() const
  {
    const SopUsageCounter::ElementSet allSops = getTrackedSops();
    SopCountPerLeg countPerLeg;
    for (const auto& allSop : allSops)
    {
      countPerLeg[allSop.legId]++;
    }
    return countPerLeg;
  }

  // Returns the number of SOPs with usage zero
  unsigned int getNbrOfUnusedSops() const override { return _sopUsage.getNbrOfUnusedElements(); }

  // Returns the number of SOPs with usage zero
  // on the given leg
  unsigned int getNbrOfUnusedSopsOnLeg(unsigned int legId) const override;

  unsigned int getUsageCount(unsigned int legId, uint32_t sopId) const override
  {
    return _sopUsage.getUsageCount(utils::SopEntry(legId, sopId));
  }

  UnusedSopsIterator unusedSopsBegin() const { return _sopUsage.getUnusedElements().begin(); }

  UnusedSopsIterator unusedSopsEnd() const { return _sopUsage.getUnusedElements().end(); }

  SopsUsageIterator sopUsageBegin() const { return _sopUsage.getUsageCountMap().begin(); }

  SopsUsageIterator sopUsageEnd() const { return _sopUsage.getUsageCountMap().end(); }

  std::string toString() const override
  {
    std::ostringstream out;
    out << "SopUsageTrackingAppraiser (" << (getNbrOfSops() - getNbrOfUnusedSops())
        << "/" << getNbrOfSops() << " SOPs represented)";
    return out.str();
  }

  void toStream(std::ostream& out) const;

  ISopUsageCounter::SopUsages getSopUsagesOnLeg(unsigned int legId) const override
  {
    SopUsages usages;
    for (SopsUsageIterator it = sopUsageBegin(); it != sopUsageEnd(); ++it)
    {
      if (it->first.legId == legId)
      {
        usages[it->first.sopId] = it->second;
      }
    }
    return usages;
  }

private:
  // Increases/decreases usage count by one
  void increaseUsageCount(unsigned int legId, uint32_t sopId);
  void decreaseUsageCount(unsigned int legId, uint32_t sopId);

  SopUsageCounter _sopUsage;
  const ShoppingTrx& _trx;
};

std::ostream& operator<<(std::ostream& out, const SopUsageTracker& a);

class SopUsageTrackerFormatter
{
public:
  void format(const SopUsageTracker::SopUsageCounter& counter,
              std::ostream& out,
              const ShoppingTrx& trx);
};


} // namespace tse

