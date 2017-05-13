
#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/Diversity.h"

namespace tse
{

enum class SwapperEvaluationResult
{
  SELECTED,
  CARRIERS,
  NON_STOPS,
  LAST_MIN_PRICED,
  TOD_DISTANCE,
  SCORE,
  CUSTOM_SOLUTION,
  ALL_SOPS,
  RC_ONLINES,
  IBF_PREFERENCE,
  NONE,
};

struct NewSolutionAttributes
{
  // Compiler is allowed to change the order of multiple sections
public:
  CarrierCode carrier;

public:
  Diversity::BucketType bucket;

public:
  size_t todBucket;

public:
  bool isNonStop;
};
}

