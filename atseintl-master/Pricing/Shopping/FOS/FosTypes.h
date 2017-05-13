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

#include "Common/TseCodeTypes.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <vector>

#include <stdint.h>

namespace tse
{
namespace fos
{

typedef std::set<CarrierCode> CarrierSet;
typedef utils::SopCombination SopCombination;

struct SopDetails
{
  CarrierCode cxrCode[2];
  LocCode destAirport;
  std::string fareMarketOD[2];
};

typedef std::vector<SopDetails*> SopDetailsPtrVec;

enum ValidatorType
{
  VALIDATOR_DIAMOND = 0,
  VALIDATOR_SNOWMAN,
  VALIDATOR_TRIANGLE,
  VALIDATOR_ONLINE,
  VALIDATOR_CUSTOM,
  VALIDATOR_LONGCONX,
  VALIDATOR_NONSTOP,
  VALIDATOR_LAST = VALIDATOR_NONSTOP
};

typedef uint32_t ValidatorBitMask;

inline ValidatorBitMask
validatorBitMask(ValidatorType vt)
{
  return (1u << vt);
}

enum FilterType
{
  FILTER_RESTRICTION,
  FILTER_NONRESTRICTION,
  FILTER_OWTHREESEGS,
  FILTER_CUSTOM,
  FILTER_NONSTOP,
  FILTER_COMPOSITE,
  FILTER_LAST = FILTER_COMPOSITE
};

struct FosGeneratorStats
{
  uint32_t totalProcessedCombinations;
  uint32_t uniqueProcessedCombinations;
  uint32_t validatedCombinations;
  uint32_t currentFilterCutoff;

  FosGeneratorStats()
    : totalProcessedCombinations(0),
      uniqueProcessedCombinations(0),
      validatedCombinations(0),
      currentFilterCutoff(0)
  {
  }
};

} // fos
} // tse
