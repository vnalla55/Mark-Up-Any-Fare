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
#ifndef DIVERSITY_TEST_UTIL_H
#define DIVERSITY_TEST_UTIL_H

#include "DataModel/Diversity.h"

namespace tse
{

namespace shpq
{

class DiversityTestUtil
{
public:
  DiversityTestUtil(Diversity& diversity) : _diversity(diversity) {}
  void resetBucketDistribution()
  {
    std::fill(_diversity._bucketDistribution,
              _diversity._bucketDistribution + Diversity::BUCKET_COUNT,
              0);
  }
  void setHighDensityMarket(bool hdm) { _diversity._highDensityMarket = hdm; }
  void setMaxOnlineNonStopCount(size_t count) { _diversity._maxOnlineNonStopCount = count; }
  void setMaxInterlineNonStopCount(size_t count) { _diversity._maxInterlineNonStopCount = count; }
  void setNonStopOptionsCount(size_t count) { _diversity._nonStopOptionsCount = count; }
  void setNonStopOptionsPerCarrierEnabled(bool enabled)
  {
    _diversity._isNonStopOptionPerCarrierEnabled = enabled;
  }
  void setMaxNonStopCountForCarrier(CarrierCode cxr, size_t count)
  {
    _diversity._maxNonStopCountPerCarrier[cxr] = count;
  }
  void setNumberOfOptionsToGenerate(size_t count) { _diversity._numberOfOptionsToGenerate = count; }
  void addDirectCarrier(CarrierCode cxr) { _diversity._directOptionsCarriers.insert(cxr); }

  Diversity& getDiversity() const { return _diversity; }

private:
  Diversity& _diversity;
};

} // shpq

} // tse

#endif
