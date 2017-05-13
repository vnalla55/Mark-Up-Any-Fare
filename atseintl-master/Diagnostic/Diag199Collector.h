//----------------------------------------------------------------------------
//  File:        Diag199Collector.h
//  Authors:     Mike Carroll
//  Created:     Feb 2004
//
//  Description: Diagnostic 199 formatter
//
//  Updates:
//          date - initials - description.
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

#include "DataModel/PaxType.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Itin;
class FareMarket;
class TravelSeg;
class PaxTypeFare;
class PaxType;
class Billing;
class FareClassAppInfo;
class FareClassAppSegInfo;
class RexBaseTrx;

class Diag199Collector : public DiagCollector
{
public:
  friend class Diag199CollectorTest;

  Diag199Collector& operator<<(const TravelSeg& tvlSeg) override;
  Diag199Collector& operator<<(const Itin& itin) override;
  Diag199Collector& operator<<(const PricingTrx& trx) override;

private:
  void addDiscountInfo(const PricingTrx& trx, const FareMarket& fareMarket);
  void addFareMarket(const PricingTrx& trx, const FareMarket& fareMarket);
  void addSideTripTvlSegs(const FareMarket& fareMarket);
  void addTravelSegment(const TravelSeg* tvlSeg);
  void addCOS(const std::vector<ClassOfService*>* cosVec);
  std::string addGlobalDirection(const FareMarket& fareMarket);
  std::string oneFM(const PricingTrx& trx);
  void addCustomerCxrData(const PricingTrx& trx, const FareMarket& fatreMarket);
  void printBrandingInfo(const PricingTrx& trx, const FareMarket& fm);
  void printBrandingInfo(const PricingTrx& trx, const FareMarket& fm, const int& marketId);
  void addBilling(const Billing& billing);
  void addPricingDates(const PricingTrx& trx);
  void addRefundDates(const RexBaseTrx& trx);
  std::string getFMValidatingCarriers(const PricingTrx& trx, const FareMarket& fareMarket);
  void printItinList(const PricingTrx& trx, const FareMarket& fm);
  void displayInterlineIntralineTables(const PricingTrx& trx);
  void displayContextShoppingInfo(const PricingTrx& trx);
};

} // namespace tse

