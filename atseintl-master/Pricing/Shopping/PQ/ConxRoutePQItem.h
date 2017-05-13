// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         29-09-2011
//! \file         ConxRoutePQItem.h
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
#include "DataModel/CxrFareMarkets.h"
#include "DataModel/DirFMPath.h"
#include "DataModel/OwrtFareMarket.h"
#include "Pricing/Shopping/PQ/BaseLegInfo.h"
#include "Pricing/Shopping/PQ/CommonSoloPQItem.h"

namespace tse
{
class Logger;
class Trx;
}

namespace tse
{
namespace shpq
{

class SolutionPattern;

class ConxRoutePQItem;

typedef std::shared_ptr<ConxRoutePQItem> ConxRoutePQItemPtr;

class ConxRoutePQItem : public CommonSoloPQItem
{
  friend class boost::object_pool<ConxRoutePQItem>;
  friend class TseObjectPool<ConxRoutePQItem>;

  friend class ConxRoutePQItemTest;

public:
  typedef std::vector<CxrFareMarketsPtr> CxrFMVector;

  virtual ~ConxRoutePQItem() {}

  virtual MoneyAmount getScore() const override { return _score; }

  virtual SoloPQItemLevel getLevel() const override { return CR_LEVEL; }

  virtual void expand(SoloTrxData& soloTrxData, SoloPQ& pq) override;

  virtual std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const override;

  CxrFMVector getFMVector() const;

  const OwrtFareMarketPtr getOutboundCxrLeg1() const
  {
    return (_outboundCxrLeg1.hasCurrent() ? *_outboundCxrLeg1 : OwrtFareMarketPtr());
  }
  const OwrtFareMarketPtr getOutboundCxrLeg2() const
  {
    return (_outboundCxrLeg2.hasCurrent() ? *_outboundCxrLeg2 : OwrtFareMarketPtr());
  }
  const OwrtFareMarketPtr getInboundCxrLeg1() const
  {
    return (_inboundCxrLeg1.hasCurrent() ? *_inboundCxrLeg1 : OwrtFareMarketPtr());
  }
  const OwrtFareMarketPtr getInboundCxrLeg2() const
  {
    return (_inboundCxrLeg2.hasCurrent() ? *_inboundCxrLeg2 : OwrtFareMarketPtr());
  }

private:
  class CxrLegInfo : public details::BaseLegInfo<CxrFareMarketsPtr, CxrFareMarkets::const_iterator>
  {
  public:
    typedef details::BaseLegInfo<CxrFareMarketsPtr, CxrFareMarkets::const_iterator> Base;
    CxrLegInfo(const DirFMPathPtr& dfM, const int idx, const DirFMPathPtr& defaultDfm);
    CxrLegInfo(const CxrLegInfo& other, const bool getNext);

  private:
    static CxrFareMarketsPtr
    getFareMarketsPtr(const DirFMPathPtr& dfM, const int idx, const CxrFareMarketsPtr& defaultDfm);
  };

  class CRLevelTuning
  {
  public:
    CRLevelTuning(const ConxRoutePQItem&);
    bool validate(const SoloTrxData& soloTrxData, LegPosition legPosition = LEG_NONE) const;
    bool validateDomesticReturning(const SoloTrxData& soloTrxData,
                                   LegPosition legPosition = LEG_NONE) const;

  private:
    /*
     * First group : apply only T1&T3&N&S for solultion patterns: 20
     * Second group: disallow only N combination for patterns: 30, 31, 32, 33, 34, 35
     *                                                         40, 44, 45, 46, 47, 48, 49
     * Third group : allow only N&S combination for patterns: 42, 43
     */
    bool validateFirstGroup(LegPosition legPosition) const;
    bool validateSecondGroup(LegPosition legPosition) const;
    bool validateThirdGroup(LegPosition legPosition) const;

    bool validateTag2() const;

    bool isFirstGroup() const;
    bool isSecondGroup() const;
    bool isThirdGroup() const;

    bool isFirstGroupFailAllowed(LegPosition legPosition) const;
    bool isSecondGroupFailAllowed(LegPosition legPosition) const;
    bool isThirdGroupFailAllowed(LegPosition legPosition) const;

    void collectFareMarkets(std::vector<OwrtFareMarketPtr>& container) const;

    const ConxRoutePQItem& _crItem;
  };

  MoneyAmount calculateScore(const MoneyAmount defaultAmount) const;

  bool isOnlineSolution() const;
  bool detectCarriers(const ShoppingTrx& trx) const;
  bool checkCarriers(const CxrLegInfo&, const CxrLegInfo&, const ShoppingTrx& trx) const;
  bool validateCandidate(const SoloTrxData& soloTrxData, LegPosition legPosition = LEG_NONE) const;

  const CxrLegInfo* getCarrierLegInfo(LegPosition legPosition) const
  {
    if (legPosition == LEG_2ND)
      return &_outboundCxrLeg2;
    else if (legPosition == LEG_3RD)
      return &_inboundCxrLeg1;
    else if (legPosition == LEG_4TH)
      return &_inboundCxrLeg2;
    return &_outboundCxrLeg1;
  }

  const CxrLegInfo _outboundCxrLeg1; // vector or sth could be used instead of those
  const CxrLegInfo _outboundCxrLeg2; // four variables, but...
  const CxrLegInfo _inboundCxrLeg1; // it would be hardly impossible
  const CxrLegInfo _inboundCxrLeg2; // to mark the vector const
  const MoneyAmount _score;

  const CRLevelTuning _validator;

  static Logger _logger;

private:
  // Use SoloPQItemManager instead
  ConxRoutePQItem(const SolutionPattern& solutionPattern,
                  const DirFMPathPtr& outboundDFm,
                  const DirFMPathPtr& inboundDFm);
  ConxRoutePQItem(const ConxRoutePQItem& other, const LegPosition expandedLegPos);
};
} /* namespace shpq */
} /* namespace tse */

