// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include "Pricing/PUPathMatrix.h"
#include "Pricing/Shopping/PQ/SoloFMPathCollector.h"
#include "Pricing/Shopping/PQ/SoloFMPathKey.h"

#include <map>

namespace tse
{
class MergedFareMarket;
class PU;
class PUPath;
class ShoppingTrx;
}

namespace tse
{
namespace shpq
{

class ConxRouteCxrPQItem;
class SolutionPattern;

class SoloPUPathCollector : protected tse::PUPathMatrix
{
  typedef std::map<MergedFMKey, PU*, MFMKeyLess> OWPUMap;
  typedef std::map<SoloFMPathKey, PU*, SoloFMPathKeyLess> PUMap;
  typedef std::pair<OWPUMap::iterator, bool> OWMapPair;
  typedef std::pair<PUMap::iterator, bool> PUMapPair;

public:
  typedef std::vector<PricingUnitFactoryBucket*> PUFactoryBucketVec;
  typedef std::set<PU*, PU::PUPtrCmp> PUSet;
  struct PUStruct
  {
    PUStruct() : _puPath(nullptr), _itin(nullptr), _fmPath(nullptr), _puFactoryBucketVec(nullptr) {}

    bool isSinglePax() { return (_puFactoryBucketVec && _puFactoryBucketVec->size() == 1); }

    PUPath* _puPath;
    Itin* _itin;
    FareMarketPath* _fmPath;
    std::vector<PricingUnitFactoryBucket*>* _puFactoryBucketVec;
  };

  SoloPUPathCollector(ShoppingTrx&);
  ~SoloPUPathCollector() {}

  PUStruct buildPUStruct(const ConxRouteCxrPQItem* const, PUFactoryBucketVec& puFactoryBucketVec);

  PU* getOrCreateOWPU(MergedFareMarket*);
  PU* getOrCreateRTPU(MergedFareMarket*, MergedFareMarket*);
  PU* getOrCreateOJPU(MergedFareMarket* outboundMfm, MergedFareMarket* inboudMfm);
  bool createCTPUPath(PUPath& puPath, MergedFMVector& fmPath);

private:
  void updatePUFactoryBucket(PUStruct&, PUFactoryBucketVec& puFactoryBucketVec);
  void setPUPath(PUPath*);
  PUPath* buildPuPath(const SolutionPattern&, FareMarketPath*, Itin*);
  bool isPUPathComplete(PUPath&, const FareMarketPath&, size_t totalMktCnt);
  void collectDiagnostic(PUStruct&);

private:
  ShoppingTrx& _trx;
  SoloFMPathCollector _fmPathCollector;
  PUSet _updatedPUSet;

  OWPUMap _owPUMap;
  PUMap _rtPUMap;
  PUMap _ojPUMap;
};
}
} // namespace tse::shpq

