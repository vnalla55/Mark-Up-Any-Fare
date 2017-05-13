//---------------------------------------------------------------------
//  File:        Diag258Collector.h
//  Authors:
//  Created:     4/17/2006
//
//  Description: Diagnostic 258 formatter
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
//---------------------------------------------------------------------

#pragma once

#include "Diagnostic/ACDiagCollector.h"

namespace tse
{
class FareMarket;

class Diag258Collector : public ACDiagCollector
{
public:
  enum DupRemoveReason
  {
    DRR_PUBLISHED_OVER_CONSTRUCTED,
    DRR_PUBLISHED_LESS_OR_EQ_FARE_AMOUNT,
    DRR_CONSTRUCTED_LESS_FARE_AMOUNT
  };

  explicit Diag258Collector(Diagnostic& root) : ACDiagCollector(root), _numOfDups(0), _fareMarket(nullptr)
  {
  }
  Diag258Collector() : _numOfDups(0), _fareMarket(nullptr) {}

  static Diag258Collector* getDiag258(PricingTrx& trx, FareMarket* fm);

  static void reclaim(Diag258Collector* diag258);

  FareMarket*& fareMarket() { return _fareMarket; };

  void writeDupRemovalHeader(bool pubVsConstructed);
  void writeDupRemovalFooter();
  void
  writeDupDetail(const Fare& fare1, const ConstructedFareInfo& cfi2, const DupRemoveReason reason);
  void writeConstructedFare(const ConstructedFareInfo& cfi) override;
  void writeSpecifiedFare(const FareInfo& fareInfo) override;

private:
  unsigned int _numOfDups;
  FareMarket* _fareMarket;

  void writePublishedOrConstructed(bool isPublished,
                                   const Indicator inhibit,
                                   const LocCode& city1,
                                   const LocCode& city2,
                                   const DateTime& effectiveDate,
                                   const DateTime& expireDate,
                                   const Indicator owrt,
                                   const Directionality& directionality,
                                   const CurrencyCode& curency,
                                   const MoneyAmount amount,
                                   const FareClassCode& fareClass,
                                   const TariffNumber tariff,
                                   const bool showFareAmount);

  void formatDateTime(const DateTime& dt);
};

}; // namespace tse

