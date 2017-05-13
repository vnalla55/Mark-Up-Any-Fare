// vim:ts=2:sts=2:sw=2:cin:et
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include <functional>
#include <memory>
#include <string>

namespace tse
{
class FarePath;
}

namespace tse
{
namespace shpq
{

class SoloPQ;
class SoloTrxData;

class SoloPQItem;
typedef std::shared_ptr<SoloPQItem> SoloPQItemPtr;

class SoloPQItem
{
public:
  SoloPQItem() : _id(++_idGenerator) {}

  virtual ~SoloPQItem() {}

  uint32_t getId() const { return _id; }
  /**
   * @return Score of this SoloPQ item.
   *
   * @warning The score must be const for a whole lifetime of
   * the SoloPQ item. This is necessary for proper priority queue
   * behaviour.
   */
  virtual MoneyAmount getScore() const = 0;

  enum SoloPQItemLevel
  {
    SP_LEVEL,
    CR_LEVEL,
    CRC_LEVEL,
    FMP_LEVEL,
    FPF_LEVEL
  };

  enum StrVerbosityLevel
  {
    SVL_HEADINGONLY,
    SVL_BARE,
    SVL_NORMAL,
    SVL_DETAILS
  };

  /**
   * @return Level of this SoloPQ item.
   *
   * @warning The level must be const for a whole lifetime of
   * the SoloPQ item. This is necessary for proper priority queue
   * behaviour.
   */
  virtual SoloPQItemLevel getLevel() const = 0;

  /**
   * @return SolutionPattern of this SoloPQ item.
   *
   */
  virtual const SolutionPattern* getSolPattern() const = 0;

  /**
   *  Expand item to new ones (the same or next level)
   *
   *  @param pq SoloPQ to which results of expansion should be enqueued
   *
   */
  virtual void expand(SoloTrxData& soloTrxData, SoloPQ& pq) = 0;

  /**
   *  Display the data for diagnostics
   *
   *
   */
  virtual std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const = 0;

  virtual FarePath* getFarePath() const = 0;

  /**
   * Carrier index for Diversity. It can include INTERLINE_CARRIER as well.
   * Is valid starting from CRC_LEVEL.
   */

  virtual const std::vector<CarrierCode>& getApplicableCxrs() const = 0;

  struct FareMarketVisitor
  {
    virtual void visit(const FareMarket*) = 0;
    virtual ~FareMarketVisitor() {};
  };

  /**
   * This actually duplicates
   *    OwrtFMVector ConxRouteCxrPQItem::getFMVector() const
   *  functionality.
   *
   * But indeed, it a light-weight version of aforementioned function,
   * as far this implementation does not require dynamic vector allocation.
   * Further, this one is also implemented in FarePathFactoryPQItem.
   */
  virtual void visitFareMarkets(FareMarketVisitor& visitor) const = 0;

  /**
   *  Comparator type that can be used in sorted collections.
   *  It compares item score, level and solution pattern number.
   */
  struct SoloPQItemComparator
      : public std::binary_function<const SoloPQItemPtr&, const SoloPQItemPtr&, bool>
  {
    bool operator()(const SoloPQItemPtr& a, const SoloPQItemPtr& b) const
    {
      if (a->getScore() - b->getScore() > EPSILON)
        return true;
      if (b->getScore() - a->getScore() > EPSILON)
        return false;

      if (a->getLevel() != b->getLevel())
        return a->getLevel() < b->getLevel();

      if (LIKELY(a->getSolPattern() && b->getSolPattern()))
        return a->getSolPattern()->getSPNumber() > b->getSolPattern()->getSPNumber();

      return false;
    }
  }; // struct SoloPQItemComparator

private:
  uint32_t _id;
  static uint32_t _idGenerator;
}; // class SoloPQItem
}
} // namespace tse::shpq

