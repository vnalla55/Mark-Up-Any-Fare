//----------------------------------------------------------------------------
//
//
//  File:     OJMMinimumFare.h
//  Created:  3/03/2004
//  Authors:
//
//  Description	:This is the class for Open-Jaw Minimum Check for Minimum Fares.
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

#include "MinFares/MinimumFare.h"

namespace tse
{

class DiagCollector;
class FarePath;
class FareUsage;
class PricingUnit;
class PricingTrx;

/**
 * @class OJMMinimumFare - Highest Round Trip Open Jaw Minimum Fare Check
 * This class will be called from MinFareChecker to process OJM if condition
 * is met using pricing unit type.
 *
*/

class OJMMinimumFare : public MinimumFare
{
  friend class OJMAndRTMinimumFareTest;

public:
  OJMMinimumFare(PricingTrx& trx, FarePath& farePath);

  OJMMinimumFare(const OJMMinimumFare&) = delete;
  OJMMinimumFare& operator=(const OJMMinimumFare&) = delete;

  /**
   * This function is used to apply OJM check for each Pricing Unit.
   *
   * @param PricingTrx A reference to the Trx.
   * @param farePath A reference to the Fare Path.
   * @param pu (pricing unit) A reference to the pricing unit.
   * @param logger A reference to the Logger.
   *
   * @return bool true if the pricing unit passed OJM check.
   *
   * @todo OJM check will process in the following order
   * - Qualify Open Jaw Normal/Special Fare Combinatons
   * - Qualify Category 10, Subcategory 101, byte 13
   * - Perform Open Jaw Minimum(OJM) Check
   * - Accumulate through Fare Components
   * - Select Round Trip OJM Fares
   * - Fare Comparison
   * - Compare and Save OJM plus up amount, into pricing unit and return.
   */
  MoneyAmount process(PricingUnit& pu);
  void processNetFares(PricingUnit& pu);

protected:
  /**
   * This function is called to qualify Cat10 Subcat101 Byte13
   *
   * @param the current pricing unit
   */
  bool qualifyCat10Subcat101Byte13(const PricingUnit& pu) const;

  void printDiagHeader();
  void printPlusUps(const MinFarePlusUpItem* ojmPlusUp) const;

  using MinimumFare::printFareInfo;
  void printFareInfo(const PricingUnit& pu, bool isRt = false) const;
  MinFarePlusUpItem* getHrtojPlusUp(const MoneyAmount& OJ_FareAmt,
                                    const MoneyAmount& RT_FareAmt,
                                    const FareUsage* highestRtFu) const;

private:
  PricingTrx& _trx;
  FarePath& _farePath;
  bool _isNetRemitFp = false;
  bool _isWpNet = false;

  DiagCollector* _diag = nullptr;
  bool _diagEnabled = false;
};
} // namespace tse
