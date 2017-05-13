//----------------------------------------------------------------------------
//
//  File:        AltPricingUtil.h
//  Created:     2/20/2006
//  Authors:     Andrew Ahmad
//
//  Description: Common functions required for ATSE WPA processing.
//
//  Updates:
//
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <set>

namespace tse
{
class FareCalcConfig;
class FareMarket;
class FarePath;
class Itin;
class Money;
class PaxType;
class PaxTypeFare;
class PricingTrx;

class AltPricingUtil
{
public:
  static bool needToReprocess(PricingTrx& trx);
  static bool prepareToReprocess(PricingTrx& trx);
  static bool prepareToReprocess(FareMarket& fareMarket);
  static bool finalProcessing(PricingTrx& trx);

  static bool isXMrequest(const PricingTrx& trx);
  static void prepareToProcess(PricingTrx& trx);

  static bool validateFarePathBookingCode(PricingTrx& trx, FarePath& farePath);

  static uint32_t getConfigFareOptionMaxDefault();
  static uint32_t getConfigFareOptionMaxLimit();

  static bool ignoreAvail(PricingTrx& trx);

  static bool
  intermediateProcessing(PricingTrx& trx, std::vector<FarePath*>& fPaths, const uint32_t max);

  static void
  getMatchTotals(PricingTrx& trx, PaxType& currPaxType, std::set<MoneyAmount>& matchFarePathTotals);

  static void
  getAmounts(PricingTrx& trx, FarePath* farePath, MoneyAmount& fpTotalAmt, MoneyAmount& diffAmount);

  static FarePath*
  lowestSolution(PricingTrx& trx, const std::vector<FarePath*>& farePath, PaxType* paxType);
  static void keepLowestSolutions(PricingTrx& trx, Itin& itin);

  static bool isCat25SisterFare(const PaxTypeFare& paxTypeFare);

  static bool isRBDasBooked(PricingTrx& trx, FarePath* farePath);

private:
  static Money getFarePathTaxTotal(const FarePath& farePath);
  static const FareCalcConfig* getFareCalcConfig(PricingTrx&);

}; // class AltPricingUtil

}; // namespace tse
