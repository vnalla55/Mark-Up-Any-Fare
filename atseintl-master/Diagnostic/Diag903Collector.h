//----------------------------------------------------------------------------
//  File:        Diag903Collector.h
//  Created:     2004-08-20
//
//  Description: Diagnostic 903 formatter
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Diagnostic/Diag902Collector.h"

namespace tse
{
class Diag903Collector : public Diag902Collector
{
public:
  explicit Diag903Collector(Diagnostic& root)
    : Diag902Collector(root),
      _filter(nullptr),
      _paxNum(0),
      _doLocalMarkets(false),
      _isSpanishDiscountTrx(false)
  {
  }
  Diag903Collector()
    : Diag902Collector(),
      _filter(nullptr),
      _paxNum(0),
      _doLocalMarkets(false),
      _isSpanishDiscountTrx(false)
  {
  }

  virtual Diag903Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

  ShoppingTrx& getShoppingTrx();
  const ShoppingTrx& getShoppingTrx() const;

private:
  virtual Diag903Collector& operator<<(const ItinIndex& itinGroup) override;
  virtual Diag903Collector& operator<<(const FareMarket& fareMarket) override;
  virtual Diag903Collector& operator<<(const PaxTypeFare& paxFare) override;
  virtual Diag903Collector& operator<<(const PaxTypeFareRuleData& ptfRuleData) override;

  typedef std::pair<std::string /* carrier */, uint16_t /* market idx*/> DisplayMktInfo;
  typedef std::map<const FareMarket*, DisplayMktInfo> DisplayedMkts;
  class Diag903Filter;

  const Itin* getCurrentItin(const ItinIndex& itinIndex, const ItinIndex::Key& cxrKey) const;

  bool marketAlreadyDisplayed(const FareMarket* const fM,
                              const FareMarket* const govFM,
                              const size_t fmIdx);

  bool isThruFm(const FareMarket* const fM, const FareMarket* const govFM) const;

  DisplayedMkts _displayedMkts;
  const Diag903Filter* _filter;
  uint16_t _paxNum;
  bool _doLocalMarkets;
  bool _isSpanishDiscountTrx;
};

} // namespace tse

