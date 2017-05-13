// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         22-11-2011
//! \file         FarePathFactoryPQItem.h
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
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SoloPUFactoryWrapper.h"

namespace tse
{
class PricingUnitFactoryBucket;
class ShoppingTrx;
class Logger;
}

namespace tse
{
namespace shpq
{

class FarePathFactoryPQItem;
typedef std::shared_ptr<FarePathFactoryPQItem> FarePathFactoryPQItemPtr;
class SoloFarePathFactory;

class FarePathFactoryPQItem : public SoloPQItem
{
  friend class boost::object_pool<FarePathFactoryPQItem>;
  friend class TseObjectPool<FarePathFactoryPQItem>;

public:
  typedef SoloPUFactoryWrapper::PUStruct PUStruct;
  typedef std::vector<PricingUnitFactoryBucket*> PUFBucketVector;

  virtual MoneyAmount getScore() const override { return _score; }

  virtual const SolutionPattern* getSolPattern() const override { return &_solutionPattern; }

  virtual SoloPQItemLevel getLevel() const override { return FPF_LEVEL; }

  virtual void expand(SoloTrxData& soloTrxData, SoloPQ& pq) override;

  virtual std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const override;

  virtual FarePath* getFarePath() const override { return _farePath; }

  virtual const std::vector<CarrierCode>& getApplicableCxrs() const override { return _applCxrs; }

  virtual void visitFareMarkets(FareMarketVisitor& visitor) const override;

  virtual ~FarePathFactoryPQItem() {}

  bool isFailed() const { return (_score >= UNKNOWN_MONEY_AMOUNT - EPSILON); }

  void setPrevalidateDirectFlights(bool validate) { _prevalidateDirectFlights = validate; }

  void setPerformNSPreValidation(bool perform) { _performNSPreValidation = perform; }

  bool canProduceNonStops(ShoppingTrx& trx);

private:
  class CheckNSVisitor;

  const SolutionPattern& _solutionPattern; // for diag purposes only

  SoloFarePathFactory* _farePathFactory; // once set in constructors, shouldn't be
  // changed to anything else except for NULL.

  FarePath* _farePath; // set in expand(), but str() displays the
  // value even if expand() has not been called yet

  MoneyAmount _score; // set in constructors, shouldn't be changed
  // by any other function (including expand())

  std::vector<CarrierCode> _applCxrs;
  bool _prevalidateDirectFlights;
  bool _performNSPreValidation;

  CheckNSVisitor* _nsVisitor;

  static Logger _logger;

private:
  // Use SoloPQItemManager instead
  FarePathFactoryPQItem(const ConxRouteCxrPQItem& crcPQItem,
                        SoloTrxData& soloTrxData,
                        PUStruct& puStruct);
  FarePathFactoryPQItem(const FarePathFactoryPQItem& other, DiagCollector& diag);
};

} /* namespace shpq */
} /* namespace tse */
