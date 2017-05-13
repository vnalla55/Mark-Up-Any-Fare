//---------------------------------------------------------------------
//  File:        Diag204Collector.h
//  Authors:
//  Created:     11/23/2005
//
//  Description: Diagnostic 204 formatter
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

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareMarket;
class PricingTrx;

class Diag204Collector : public DiagCollector
{
public:
  static Diag204Collector* getDiag204(PricingTrx& trx);

  bool parseQualifiers(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeFare(const Fare& fare);

  void writeBadGlobalDirection(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeNoRecord1(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeNoFCA(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeNoMatchFareClass(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeNoMatchLocation(PricingTrx& trx,
                            const Fare& fare,
                            const FareMarket& fareMarket,
                            const FareClassAppInfo& fcaInfo);

  void writeFareCanNotDouble(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeFareCanNotBeHalved(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);

  void writeNoMatchRoutingNumber(PricingTrx& trx,
                                 const Fare& fare,
                                 const FareMarket& fareMarket,
                                 const FareClassAppInfo& fcaInfo);

  void writeNoFootnoteMatch(PricingTrx& trx,
                            const Fare& fare,
                            const FareMarket& fareMarket,
                            const FareClassAppInfo& fcaInfo);

  void writeFareTypeNotRetrieved(PricingTrx& trx,
                                 const Fare& fare,
                                 const FareMarket& fareMarket,
                                 const FareClassAppInfo& fcaInfo);

  void writeNoMatchOnPaxTypes(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket);
};

}; // namespace tse

