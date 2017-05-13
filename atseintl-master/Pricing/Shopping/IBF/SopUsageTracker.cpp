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


#include "Pricing/Shopping/IBF/SopUsageTracker.h"

#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{

namespace
{
Logger
logger("atseintl.Pricing.IBF.SopUsageTracker");
}

bool SopUsageTracker::combinationAdded(const utils::SopCombination& comb)
{
  bool isNewSopCovered = false;
  for (unsigned int i = 0; i < comb.size(); ++i)
  {
    // e.g. i = 1, comb[i] = 6  <=>  leg 1, sop 6
    if (getUsageCount(i, comb[i]) == 0)
    {
      isNewSopCovered = true;
    }
    increaseUsageCount(i, comb[i]);
  }

  if (IS_DEBUG_ENABLED(logger))
  {
    std::ostringstream out;
    out << "Updated usages for SOP combination:";
    for (unsigned int i = 0; i < comb.size(); ++i)
    {
      out << " [leg " << i << ", SOP " << comb[i] << ", usages " << getUsageCount(i, comb[i])
          << "]";
    }
    out << " Unused elems " << getNbrOfUnusedSops() << "/" << getNbrOfSops() << ".";
    LOG4CXX_DEBUG(logger, out.str());
  }

  return isNewSopCovered;
}

void SopUsageTracker::combinationRemoved(const utils::SopCombination& comb)
{
  for (unsigned int i = 0; i < comb.size(); ++i)
  {
    // e.g. i = 1, comb[i] = 6  <=>  leg 1, sop 6
    decreaseUsageCount(i, comb[i]);
  }
}

void
SopUsageTracker::increaseUsageCount(unsigned int legId, uint32_t sopId)
{
  LOG4CXX_DEBUG(logger, "Increasing usage count for SOP: leg id " << legId << ", sop id " << sopId);
  _sopUsage.increaseUsageCount(utils::SopEntry(legId, sopId));
}

void
SopUsageTracker::decreaseUsageCount(unsigned int legId, uint32_t sopId)
{
  LOG4CXX_DEBUG(logger, "Decreasing usage count for SOP: leg id " << legId << ", sop id " << sopId);
  _sopUsage.decreaseUsageCount(utils::SopEntry(legId, sopId));
}

void
SopUsageTracker::toStream(std::ostream& out) const
{
  SopUsageTrackerFormatter formatter;
  formatter.format(_sopUsage, out, _trx);
}

void
SopUsageTracker::addSopForTracking(unsigned int legId, uint32_t sopId)
{
  LOG4CXX_DEBUG(logger, "Tracking usage for SOP: leg id " << legId << ", sop id " << sopId);
  _sopUsage.addElement(utils::SopEntry(legId, sopId));
}

// Works in O(n), but simple and not extensively used
unsigned int
SopUsageTracker::getNbrOfUnusedSopsOnLeg(unsigned int legId) const
{
  unsigned int n = 0;
  const SopUsageCounter::ElementSet& unusedElems = _sopUsage.getUnusedElements();
  for (const auto& e : unusedElems)
  {
    if (e.legId == legId)
    {
      ++n;
    }
  }
  return n;
}

std::ostream& operator<<(std::ostream& out, const SopUsageTracker& a)
{
  a.toStream(out);
  return out;
}

void
SopUsageTrackerFormatter::format(const utils::UsageCounter<utils::SopEntry>& counter,
                                    std::ostream& out,
                                    const ShoppingTrx& trx)
{
  using namespace utils;
  out << "IBF SOP usage counter, total elements: " << counter.getNbrOfElements()
      << ", unused elements: " << counter.getNbrOfUnusedElements() << "\n";

  out << std::endl;
  out << "Set of unused elements:\n";
  const UsageCounter<SopEntry>::ElementSet& unused = counter.getUnusedElements();
  for (const auto& elem : unused)
  {
    const ShoppingTrx::SchedulingOption& sOption = utils::findSopInTrx(elem.legId, elem.sopId, trx);
    out << elem << " = " << sOption << std::endl;
  }

  out << std::endl;
  out << "Usage count map:\n";

  const UsageCounter<SopEntry>::UsageCountMap& usages = counter.getUsageCountMap();
  for (const auto& usage : usages)
  {
    const ShoppingTrx::SchedulingOption& sOption =
        findSopInTrx(usage.first.legId, usage.first.sopId, trx);
    out << "element: " << usage.first << " = " << sOption << "\tusages: " << usage.second
        << std::endl;
  }
}


} // namespace tse
