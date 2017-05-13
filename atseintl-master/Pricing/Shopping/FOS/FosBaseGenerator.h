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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/DetailedSop.h"
#include "Pricing/Shopping/FOS/FosTypes.h"
#include "Pricing/Shopping/Utils/FosGenerator.h"

#include <list>

namespace tse
{
class Diag910Collector;

namespace fos
{
class FosFilterComposite;

class FosBaseGenerator
{
public:
  FosBaseGenerator(ShoppingTrx& trx,
                   FosFilterComposite& fosFilterComposite,
                   Diag910Collector* dc910);
  virtual ~FosBaseGenerator() {}

  const FosGeneratorStats& getStats() const { return _genStats; }
  Diag910Collector* getDiagCollector910() const { return _dc910; }

  virtual void initGenerators() = 0;
  virtual void addPredicates() = 0;

  bool getNextCombination(ValidatorBitMask lackingValidators, SopCombination& outCombination);
  SopDetailsPtrVec getSopDetails(uint32_t legId, uint32_t sopId);
  void addCombinationForReuse(SopCombination sopComb);

protected:
  typedef std::vector<utils::FosGenerator*> GeneratorList;

  utils::FosGenerator* createFosGenerator() const;
  bool updateFilters(ValidatorBitMask lackingValidators);
  void popFilter();
  DetailedSop* generateSopDetails(uint32_t legId, uint32_t sopId);

  ShoppingTrx& _trx;
  FosFilterComposite& _fosFilterComposite;
  Diag910Collector* _dc910;

  // two queues, one keeps combinations which are not yet checked by filter
  // the other one is for combinations that were checked, but not applicable for given filter
  // also in case that because of any condition we want to use again combination
  //(which was not put into flight matrix) we put in to _checkedCombinations
  std::list<SopCombination> _toCheckCombinations;
  std::list<SopCombination> _checkedCombinations;

  GeneratorList _fosGenerators;
  size_t _currentGenerator;

  uint32_t _maxNumFilterCombinations;
  FosGeneratorStats _genStats;

  SopsWithDetailsSet _sopsWithDetailsSet;
};

} // namespace fos
} // namespace tse
