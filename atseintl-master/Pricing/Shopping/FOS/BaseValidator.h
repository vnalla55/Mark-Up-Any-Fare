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

#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

namespace tse
{

class ShoppingTrx;

namespace fos
{

class FosBaseGenerator;
class FosStatistic;

class BaseValidator
{
public:
  enum ValidationResult
  {
    VALID,
    DEFERRED, // solution might be eventually used but it shouldn't be added at this moment
    INVALID,
    INVALID_SOP_DETAILS // solution is invalid possibly because of sop details filtering
  };

  BaseValidator(ShoppingTrx& trx, FosBaseGenerator& generator, FosStatistic& stats)
    : _trx(trx), _generator(generator), _stats(stats)
  {
  }

  virtual ~BaseValidator() {}

  ShoppingTrx& getTrx() const { return _trx; }
  FosBaseGenerator& getGenerator() const { return _generator; }
  FosStatistic& getStats() const { return _stats; }

  virtual ValidatorType getType() const = 0;
  virtual ValidationResult validate(const SopIdVec& combination) const = 0;
  virtual bool isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const = 0;

private:
  ShoppingTrx& _trx;
  FosBaseGenerator& _generator;
  FosStatistic& _stats;
};

} // fos
} // tse
