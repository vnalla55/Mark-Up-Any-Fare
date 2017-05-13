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

#include "Pricing/Shopping/FOS/FosFilter.h"

#include <boost/unordered_set.hpp>

#include <string>
#include <vector>

namespace tse
{
class ShoppingTrx;
class FareMarket;

namespace fos
{
class FosTaskScope;

class RestrictionFilter : public FosFilter
{
public:
  RestrictionFilter(const ShoppingTrx& trx, const FosTaskScope& task);

  void addExistingFareMarkets(const std::vector<FareMarket*>& fareMarkets);

  virtual FilterType getType() const override { return FILTER_RESTRICTION; }

  virtual ValidatorBitMask getAffectedValidators() const override
  {
    return ~validatorBitMask(VALIDATOR_NONSTOP);
  }

  virtual bool isApplicableSolution(const SopCombination& sopCombination) const override;
  virtual const SopDetailsPtrVec& getFilteredSopDetails(const DetailedSop& orginal) override;

  virtual bool isCombinationForReuse(const SopCombination& sopCombination) const override { return true; }

private:
  bool isApplicableSop(uint32_t legId, int sopId) const;

  const ShoppingTrx& _trx;
  SopsWithDetailsSet _sopsWithDetailsFilteredSet;

  size_t _numTSPerLeg;
  bool _checkConnectingCities;
  boost::unordered_set<std::string> _existingFM;
};

} // fos
} // tse
