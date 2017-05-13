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

#include "Pricing/Shopping/FOS/BaseValidator.h"

#include <boost/unordered_set.hpp>

namespace tse
{
class PaxTypeFare;
class FareMarket;

namespace fos
{

class TriangleValidator : public BaseValidator
{
  friend class TriangleValidatorMock;

public:
  TriangleValidator(ShoppingTrx& trx, FosBaseGenerator& generator, FosStatistic& stats);

  virtual ValidatorType getType() const override { return VALIDATOR_TRIANGLE; }
  virtual ValidationResult validate(const SopIdVec& combination) const override;
  virtual bool isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const override;

private:
  void prepareCandidates();
  PaxTypeFare* getCheapestPaxTypeFare(FareMarket* fareMarket) const;

  FareMarket* _cheapestThruFM;
  boost::unordered_set<int> _triangleCandidates;
};

} // fos
} // tse

