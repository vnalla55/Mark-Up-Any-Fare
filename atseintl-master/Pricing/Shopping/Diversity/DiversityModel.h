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

#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"
#include "Pricing/Shopping/PQ/SOPCombinationList.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"
#include "Pricing/Shopping/PQ/SOPInfo.h"

#include <memory>

namespace tse
{

namespace shpq
{
class FarePathFactoryPQItem;
}

struct GroupFarePath;

class DiversityModel
{
public:
  enum PQItemAction
  {
    STOP,
    SKIP,
    USE
  };

  typedef tse::shpq::SOPCombination SOPCombination;
  typedef tse::shpq::SOPCombinationList SOPCombinationList;
  typedef std::vector<std::vector<SOPInfo> > SOPInfos;

  virtual ~DiversityModel() {};

  virtual PQItemAction getPQItemAction(const shpq::SoloPQItem* pqItem) = 0;

  virtual SOPCombinationList::iterator
  getDesiredSOPCombination(SOPCombinationList& combinations, MoneyAmount score, size_t fpKey) = 0;

  /**
   * This call allow us to drop combinations on build SOPCombinationList stage,
   * which can address performance significantly
   */
  virtual bool getIsNewCombinationDesired(const SOPCombination& combination, MoneyAmount score) = 0;

  virtual bool isNonStopNeededOnly() = 0;
  /**
   * This is called prior FPFPQItem.expand(), so DiversityModel can advice SoloPQ
   * to validate only non-stops from this pqItm
   */
  virtual bool isNonStopNeededOnlyFrom(const shpq::SoloPQItem* pqItem)
  {
    // default stub
    return isNonStopNeededOnly();
  }

  virtual bool shouldPerformNSPreValidation(const shpq::SoloPQItem* pqItem)
  {
    // default stub
    return false;
  }

  /**
   * Returns if solution was accepted and added to flight matrix
   *
   * Here we can doublecheck if solution is needed, because between
   * getDesiredSOPCombination() and addSolution() solution price can be adjusted
   * by PricingUnitRuleController in SoloItinGenerator
   */
  virtual bool addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                           ShoppingTrx::FlightMatrix& flightMatrix,
                           size_t farePathKey,
                           const DatePair* datePair) = 0;

  virtual int getBucketStatus(const Diversity::BucketType bucket) const = 0;
  virtual bool isNonStopOptionNeeded() const = 0;
  virtual bool isAdditionalNonStopEnabled() const = 0;
  virtual bool isAdditionalNonStopOptionNeeded() const = 0;
  virtual void printSolutions() const {};
  virtual void printSummary() const {};
  virtual void handlePQStop() {}
  virtual void handlePQHurryOutCondition() {}
  virtual void removeUnwantedSolutions(ShoppingTrx::FlightMatrix& flightMatrix) {}

  virtual std::unique_ptr<SOPInfos>
  filterSolutionsPerLeg(MoneyAmount score, const SOPInfos& sopInfos, SOPInfos* thrownSopInfos)
  {
    return std::unique_ptr<SOPInfos>();
  }
};
}

