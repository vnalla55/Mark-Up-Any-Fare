//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Pricing/SavedFPPQItem.h"

#include <list>

namespace tse
{
class DiagCollector;
class FPPQItem;
class FarePath;
class Itin;
class PricingTrx;
class FarePathFactoryStorage;
class FarePathSettings;
class PaxFarePathFactoryBase;

namespace similaritin
{
class FarePathValidator
{
public:
  FarePathValidator(PricingTrx& trx, Itin& motherItin);
  void setConfig(PaxFarePathFactoryBase* paxFPFBase);
  bool isFarePathValid(FPPQItem& fppqItem,
                       FarePathSettings& settings,
                       const SavedFPPQItem::Stage stage,
                       DiagCollector& diag,
                       std::list<FPPQItem*>& clonedFPPQItems);
  bool isFarePathValidMotherSolution(FarePath* farePath);
  bool finalValidation(FarePath&);
  void setLowerBoundFPPQItem(FPPQItem* topFppqItem) { _topFppqItem = topFppqItem; }
  const FPPQItem* getLowerBoundFPPQItem() const { return _topFppqItem; }
  bool pricedThroughFare(const FarePath& farePath) const;
  void processFarePathClones(FPPQItem*& fppqItem, std::list<FPPQItem*>& clonedFPPQItems);

private:
  bool isFarePathValidFPLevel(FPPQItem& fppqItem,
                              FarePathSettings& settings,
                              DiagCollector& diag,
                              std::list<FPPQItem*>& clonedFPPQItems);

  FarePathFactoryStorage* _storage;
  PricingTrx& _trx;
  FPPQItem* _topFppqItem = nullptr;
  PaxFarePathFactoryBase* _paxFPFBase = nullptr;
  std::vector<uint16_t> _categories;
  Itin& _motherItin;
};
}
}
