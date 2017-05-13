// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         17-10-2011
//! \file         ConxRouteCxrPQItem.h
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Common/TseObjectPool.h"
#include "DataModel/OwrtFareMarket.h"
#include "Pricing/Shopping/PQ/BaseLegInfo.h"
#include "Pricing/Shopping/PQ/CommonSoloPQItem.h"

#include <vector>

namespace tse
{
class Logger;
}

namespace tse
{
class ItinStatistic;
namespace shpq
{

class ConxRoutePQItem;
class ConxRouteCxrPQItem;
class SoloTrxData;
typedef std::shared_ptr<ConxRouteCxrPQItem> ConxRouteCxrPQItemPtr;

class ConxRouteCxrPQItem : public CommonSoloPQItem
{
  friend class boost::object_pool<ConxRouteCxrPQItem>;
  friend class TseObjectPool<ConxRouteCxrPQItem>;

public:
  typedef std::vector<OwrtFareMarketPtr> OwrtFMVector;

  virtual ~ConxRouteCxrPQItem() {}

  virtual MoneyAmount getScore() const override { return _score; }

  virtual SoloPQItemLevel getLevel() const override { return CRC_LEVEL; }

  virtual void expand(SoloTrxData& soloTrxData, SoloPQ& pq) override;

  OwrtFMVector getFMVector() const;

  const std::vector<CarrierCode>& getApplicableCxrs() const override { return _applCxrs; }

  virtual void visitFareMarkets(FareMarketVisitor& visitor) const override;

  virtual std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const override;

  bool isFailed() const { return _score >= UNKNOWN_MONEY_AMOUNT - EPSILON; }

  bool shouldSkip(SoloTrxData&, const ItinStatistic&, SoloPQ&);

private:
  class FMLegInfo : public details::BaseLegInfo<OwrtFareMarketPtr, OwrtFareMarket::const_iterator>
  {
  public:
    typedef details::BaseLegInfo<OwrtFareMarketPtr, OwrtFareMarket::const_iterator> Base;

    FMLegInfo(const OwrtFareMarketPtr& fmPtr, const OwrtFareMarketPtr& defaultfmPtr);
    FMLegInfo(const FMLegInfo& other, const bool getNext);
  };

private:
  MoneyAmount calculateScore(const MoneyAmount defaultAmount) const;

  const FMLegInfo _outboundFMLeg1; // vector or sth could be used instead of those
  const FMLegInfo _outboundFMLeg2; // four variables, but...
  const FMLegInfo _inboundFMLeg1; // it would be hardly impossible
  const FMLegInfo _inboundFMLeg2; // to mark the vector const
  const MoneyAmount _score;
  std::vector<CarrierCode> _applCxrs;
  static Logger _logger;

private:
  // Use SoloPQItemManager instead
  ConxRouteCxrPQItem(const ConxRoutePQItem& crPQItem,
                     SoloTrxData& soloTrxData);

  template <class P>
  void foreachFareMarket(P pred) const;
  void detectApplicableCxrs(SoloTrxData& soloTrxData);

  static void apply(const OwrtFareMarketPtr& owrtFareMarketPtr, FareMarketVisitor& visitor)
  {
    visitor.visit(owrtFareMarketPtr->getFareMarket());
  }

  bool shouldSkipCheckLeg(SoloTrxData&, const ItinStatistic&, std::vector<const FMLegInfo*>&, SoloPQ&);
  bool shouldSkipCheckThruFM(SoloTrxData&, const ItinStatistic&, std::vector<const FMLegInfo*>&, SoloPQ&);
  bool shouldSkipCheckLocalFM(SoloTrxData&, const ItinStatistic&, std::vector<const FMLegInfo*>&, SoloPQ&);
};

} /* namespace shpq */
} /* namespace tse */
