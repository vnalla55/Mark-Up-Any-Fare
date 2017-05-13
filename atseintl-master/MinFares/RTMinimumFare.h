//----------------------------------------------------------------------------
//  File:     RTMinimumFare.h
//  Created:  2/27/2011
//  Authors:
//
//  Description	:This is the class for Round Trip Minimum Check for Minimum Fares.
//
//  Copyright Sabre 2011
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

#include "MinFares/MinimumFare.h"

namespace tse
{
class RTMinimumFareTest;
class DiagCollector;
class FarePath;
class FareUsage;
class PricingUnit;
class PricingTrx;

/**
 * @class RTMinimumFare - Highest Round Trip Minimum Fare Check
 * This class will be called from MinFareChecker to process Round Trip if condition
 * is met using pricing unit type.
 *
*/

class RTMinimumFare : public MinimumFare
{
  friend class OJMAndRTMinimumFareTest;

public:
  RTMinimumFare(PricingTrx& trx, FarePath& farePath);

  RTMinimumFare(const RTMinimumFare&) = delete;
  RTMinimumFare& operator=(const RTMinimumFare&) = delete;

  /**
   * This function is used to apply RT check for each Pricing Unit.
   * @param pu (pricing unit) A reference to the pricing unit.
   * @return bool true if the pricing unit passed RT check.
   */
  MoneyAmount process(PricingUnit& pu);
  void processNetFares(PricingUnit& pu);

protected:
  bool qualifyCat10Subcat102Byte13(const PricingUnit& pu) const;
  void printDiagHeader();
  void printPlusUps(const MinFarePlusUpItem* plusUp) const;
  using MinimumFare::printFareInfo;
  void printFareInfo(const PricingUnit& pu, bool isRt = false) const;
  MinFarePlusUpItem* getHrtPlusUp(const MoneyAmount& OJ_FareAmt,
                                  const MoneyAmount& RT_FareAmt,
                                  const FareUsage* highestRtFu) const;

private:
  PricingTrx& _trx;
  FarePath& _farePath;
  bool _isNetRemitFp;
  bool _isWpNet = false;

  DiagCollector* _diag = nullptr;
  bool _diagEnabled = false;

  static const MoneyAmount ONE_HUNDREDTHS;
};
} // namespace tse
