//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Pricing/Shopping/FiltersAndPipes/GeneratingFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/Shopping/Utils/UsageProgressGenerator.h"

#include <boost/utility.hpp>

namespace tse
{


class SopUsageProgressGenerator : public utils::IGenerator<utils::SopCombination>,
                                  boost::noncopyable
{
public:
  // srl zero for no srl
  SopUsageProgressGenerator(unsigned int legCount, unsigned int srl = 0,
      size_t maxFilterRetries = 0, utils::ILogger* logger = nullptr)
    : _generator(legCount),
      _sopCombinationsfilter(_generator, maxFilterRetries, logger),
      _srl(srl)
  {
  }

  void setSrl(unsigned int srl) { _srl = srl; }

  void addPredicate(utils::INamedPredicate<utils::SopCombination>* pred)
  {
    _sopCombinationsfilter.addPredicate(pred);
  }

  void addSop(const utils::SopEntry& sopEntry, unsigned int usages)
  {
    if (usages == 0)
    {
      _unusedSops[sopEntry.legId].insert(sopEntry.sopId);
      return;
    }

    if (usageObeysSrl(usages))
    {
      _clearUsedSops[sopEntry.legId].insert(sopEntry.sopId);
      return;
    }

    _dirtyUsedSops[sopEntry.legId].insert(sopEntry.sopId);
  }

  void loadGenerator()
  {
    const unsigned int legCount = _generator.getDimensionsCount();
    for (unsigned int legId = 0; legId < legCount; ++legId)
    {
      unsigned int totalPerLeg = 0;

      // 1. transfer all unused SOPs automatically
      totalPerLeg += transferMapToGenerator(_unusedSops, legId, false);

      // 2. also transfer SOPs that do not break SRL
      totalPerLeg += transferMapToGenerator(_clearUsedSops, legId, true);

      // 3. only transfer the dirty SOPs if no SOPs gathered
      // to this point on the current leg (this prevents from scenario
      // where a dimension is completely empty). We do not expect
      // that this will happen often although we want to do it
      // since we have no choice.
      if (0 == totalPerLeg)
      {
        totalPerLeg += transferMapToGenerator(_dirtyUsedSops, legId, true);
      }
      TSE_ASSERT(totalPerLeg > 0);
    }
  }

  utils::SopCombination next() override
  {
    return _sopCombinationsfilter.next();
  }

  void sopUsed(unsigned int legId, unsigned int sopId, unsigned int usages)
  {
    _generator.removeElement(sopId, legId);
    if (usageObeysSrl(usages))
    {
      // If SRL is not violated by any of SOPs
      // used just a moment ago, we can reuse it
      // in further combinations
      _generator.addElement(sopId, legId, true);
    }
    else
    {
      // If this SOP breaks SLR, we add it back only
      // if this was the last SOP remaining on this leg (dimension)
      if (_generator.getDimensionSize(legId) == 0)
      {
        _generator.addElement(sopId, legId, true);
      }
    }
  }

  // Adds observer being notified about SOP combinations
  // discarded because of one of the added predicates.
  void addObserver(utils::IFilterObserver<utils::SopCombination>* observer)
  {
    _sopCombinationsfilter.addObserver(observer);
  }

private:
  typedef utils::UsageProgressGenerator<int> ImplGenerator;
  typedef std::set<unsigned int> SopSet;
  // legId -> sopIds
  typedef std::map<unsigned int, SopSet> SopsPerLeg;

  bool usageObeysSrl(unsigned int usage) const
  {
    if (_srl == 0)
    {
      return true;
    }
    return usage < _srl;
  }

  // returns the number of transferred SOPs
  unsigned int transferMapToGenerator(const SopsPerLeg& m, unsigned int legId, bool usedMark)
  {
    SopsPerLeg::const_iterator found = m.find(legId);
    if (found == m.end())
    {
      return 0;
    }
    const SopSet& sopsOnLeg = found->second;
    for (const auto& elem : sopsOnLeg)
    {
      _generator.addElement(elem, legId, usedMark);
    }
    return static_cast<unsigned int>(sopsOnLeg.size());
  }

  ImplGenerator _generator;
  utils::GeneratingFilter<utils::SopCombination> _sopCombinationsfilter;
  unsigned int _srl;
  SopsPerLeg _unusedSops;
  SopsPerLeg _clearUsedSops;
  SopsPerLeg _dirtyUsedSops;
};

} // namespace tse

