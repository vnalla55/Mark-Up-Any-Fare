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

#include "Pricing/PU.h"
#include "Pricing/Shopping/PQ/SoloPUPathCollector.h"

#include <set>
#include <vector>

namespace tse
{
class Logger;
class PricingOrchestrator;
class PricingUnitFactoryBucket;
class PUPath;
class ShoppingTrx;
}

namespace tse
{
namespace shpq
{

class ConxRouteCxrPQItem;
class DiagSoloPQCollector;

class SoloPUFactoryWrapper
{
public:
  typedef std::set<PU*, PU::PUPtrCmp> PUSet;
  typedef SoloPUPathCollector::PUStruct PUStruct;
  typedef std::vector<PricingUnitFactoryBucket*> PUFactoryBucketVec;

  SoloPUFactoryWrapper(ShoppingTrx&, PricingOrchestrator&);

  PUStruct getPUStruct(const ConxRouteCxrPQItem* const, DiagSoloPQCollector&);

private:
  void initPUFactory(const PUPath* const);

private:
  ShoppingTrx& _trx;
  PricingOrchestrator& _pricingOrchestrator;

  PUSet _updatedPUFactorySet;
  PUFactoryBucketVec _puFactoryBucketVec;
  SoloPUPathCollector _puPathCollector;

  static Logger _logger;
};
}
} // namespace tse::shpq

