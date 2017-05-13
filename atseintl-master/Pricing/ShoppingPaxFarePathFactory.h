//-------------------------------------------------------------------
//  Created: Feb 2006
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/MatrixUtils.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/FareUsageMatrixMap.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/SOPCombinationIterator.h"

#include <map>
#include <vector>

namespace tse
{

class Logger;
class ShoppingPQ;
class ShoppingTrx;
class BitmapOpOrderer;
class ShoppingPaxFarePathFactory;

class ConnectionPointsCounter
{
public:
  ConnectionPointsCounter(const ShoppingTrx& trx, const FareUsageMatrixMap& fumMap)
    : _trx(&trx), _map(fumMap)
  {
  }

  int operator()(const std::vector<int>& sops) const;

private:
  const ShoppingTrx* _trx;
  const FareUsageMatrixMap _map;
};

//------------------------------------------------------------------------------
class FarePathFlightsInfo
{
public:
  FarePathFlightsInfo(ShoppingTrx& trx,
                      ShoppingPQ& pq,
                      FPPQItem& fppqItem,
                      ShoppingPaxFarePathFactory& spfpf,
                      BitmapOpOrderer& op,
                      int maxCells,
                      const DatePair* dates);
  const std::set<std::vector<int> >& getPassedSops() const;
  bool findNewPassedSops(const ShoppingTrx* trx, std::vector<int>* sops = nullptr);
  bool sopsPass(const std::vector<int>& sops);
  void saveFailedFare(const uint puIdx, PaxTypeFare* ptf, const DatePair* datePair);

private:
  ShoppingTrx& _trx;
  ShoppingPQ& _pq;
  FPPQItem& _fppqItem;
  ShoppingPaxFarePathFactory& _spfpf;
  const FareUsageMatrixMap _mapping;
  std::vector<bool> _validated, _passed;
  std::set<std::vector<int> > _passedSet;
  MatrixRatingIterator<ConnectionPointsCounter> _itor;
  SOPCombinationIterator iter_;
  int _maxCells;
};

//------------------------------------------------------------------------------
class ShoppingPaxFarePathFactory : public PaxFarePathFactory
{
public:
  ShoppingPaxFarePathFactory(const FactoriesConfig& factoriesConfig);
  void setShoppingTrx(ShoppingTrx* trx);
  void setShoppingPQ(ShoppingPQ* pq);
  void setBitmapOpOrderer(BitmapOpOrderer* op);
  void setminMaxCellAltDate(ShoppingTrx* trx);

  FarePathFlightsInfo& getFarePathFlightsInfo(FPPQItem& fppqItem, const DatePair* dates);
  virtual bool initPaxFarePathFactory(DiagCollector& diag) override;

  virtual bool isFarePathValidAfterPlusUps(FPPQItem& fppqItem, DiagCollector& diag) override;

private:
  bool checkFarePathHasValidCell(FPPQItem& fppqItem);

  ShoppingTrx* _trx;
  ShoppingPQ* _pq;
  BitmapOpOrderer* _op;

  typedef std::pair<std::vector<PaxTypeFare*>, const DatePair*> FarePathFlightsInfoMapKey;
  typedef std::map<FarePathFlightsInfoMapKey, FarePathFlightsInfo> FarePathFlightsInfoMap;
  FarePathFlightsInfoMap _info;

  int _minCellsToValidate, _maxCellsToValidate, _curCellsToValidate;

  static Logger _logger;
};

//------------------------------------------------------------------------------
class ShoppingPaxFarePathFactoryCreator : public PaxFarePathFactoryCreator
{
public:
  ShoppingPaxFarePathFactoryCreator(ShoppingTrx& trx, ShoppingPQ& pq, BitmapOpOrderer& op);

  virtual PaxFarePathFactory*
  create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx) const override;

private:
  ShoppingTrx& _trx;
  ShoppingPQ& _pq;
  BitmapOpOrderer& _op;
};
}

