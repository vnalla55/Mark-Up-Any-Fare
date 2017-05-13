//-------------------------------------------------------------------
// File:    ShoppingFarePathFactory.h
// Created: August 2005
// Authors: David White
//
//  Copyright Sabre 2005
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

#include "Pricing/FarePathFactory.h"
#include "Pricing/FareUsageMatrixMap.h"

#include <map>
#include <vector>

namespace tse
{
class ShoppingPQ;
class ShoppingTrx;
class BitmapOpOrderer;

class ShoppingFarePathFactory : public FarePathFactory
{
public:
  ShoppingFarePathFactory(const FactoriesConfig& factoriesConfig) : FarePathFactory(factoriesConfig)
  {
  }

  void setShoppingTrx(ShoppingTrx* trx) { _trx = trx; }
  void setShoppingPQ(ShoppingPQ* pq) { _pq = pq; }
  void setBitmapOpOrderer(BitmapOpOrderer* op) { _op = op; }

private:
  bool isFarePathValid(FPPQItem& fppqItem,
                       DiagCollector& diag,
                       std::list<FPPQItem*>& fppqItems) override;
  bool callbackFVO(FarePath& fpath, DiagCollector& diag, const char*& resultFVO, int& fareIndex);
  bool checkAltDates(FarePath& fpath, DiagCollector& diag);

  ShoppingTrx* _trx = nullptr;
  ShoppingPQ* _pq = nullptr;
  BitmapOpOrderer* _op = nullptr;
};

class ShoppingFarePathFactoryCreator : public FarePathFactoryCreator
{
public:
  ShoppingFarePathFactoryCreator(ShoppingTrx& trx, ShoppingPQ& pq, BitmapOpOrderer& op)
    : _trx(trx), _pq(pq), _op(op)
  {
  }

  FarePathFactory* create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx) const override;

private:
  ShoppingTrx& _trx;
  ShoppingPQ& _pq;
  BitmapOpOrderer& _op;
};
}

