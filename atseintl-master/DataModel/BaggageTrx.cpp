#include "DataModel/BaggageTrx.h"

#include "DataModel/BaggagePolicy.h"

namespace tse
{
BaggageTrx::BaggageTrx()
{
  _baggagePolicy->setupPolicy(BaggagePolicy::SELECTED_TRAVELS, false);
}
}
