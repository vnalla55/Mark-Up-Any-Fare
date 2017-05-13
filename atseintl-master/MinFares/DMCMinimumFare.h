//----------------------------------------------------------------------------
//
//
//  File:     DMCMinimumFare.h
//  Created:  3/03/2004
//  Authors:
//
//  Description:
//          This is the class for Directional Minimum Check for Minimum Fares.
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
#include "DataModel/FarePath.h"
#include "Diagnostic/Diagnostic.h"

namespace tse
{


class FareUsage;
class PaxTypeFare;
class PricingUnit;
class PricingTrx;

/**
 * @class DMCMinimumFare - Directional Minimum Fare Check
 * This class will be called from MinFareChecker to process DMC if condition
 * is met using pricing unit type.
 *
 **/

class DMCMinimumFare : public MinimumFare
{
  friend class DMCMinimumFareTest;

public:
  DMCMinimumFare() = default;

  DMCMinimumFare(const DMCMinimumFare&) = delete;
  DMCMinimumFare& operator=(const DMCMinimumFare&) = delete;

  /**
   * This function is used to apply DMC check on the FarePath.  The function
   * will perform the DMC check on each contained Pricing Unit.  It also
   * do the necessary check for consecutive oneway PUs if applicable.
   *
   * @param trx - the current pricing transaction.
   * @param fare path - the fare path to perform the DMC check on.
   * @return DMC plus up amount for the whole fare path
   *
   */
  MoneyAmount process(PricingTrx& trx, FarePath& farePath);

  /**
   * This function is used to apply DMC check for each Pricing Unit.
   *
   * @param trx - the current pricing transaction.
   * @param fare path - the containing fare path.
   * @param pu - the pricing unit to perform the DMC check on.
   *
   * @return DMC plus up amount for the specified pricing unit.
   *
   */
  MoneyAmount process(PricingTrx& trx, FarePath& farePath, const PricingUnit& pu);

protected:
  MoneyAmount process(PricingTrx& trx,
                      FarePath& farePath,
                      const std::vector<PricingUnit*>& multiPu,
                      const FarePath::OscPlusUp& oscPlusUp,
                      DiagCollector& diag);

  MoneyAmount
  process(PricingTrx& trx, FarePath& farePath, const PricingUnit& pu, DiagCollector& diag);

  bool qualify(const PricingUnit& pu) const;

  /**
   * Return the DMC diagnostic
   *
   * @return the diagnostic type designated for DMC
   */
  DiagnosticTypes getDiagnostic() { return Diagnostic762; }

  /**
   * Calculate the accummulated base fare
   *
   * @param fare path
   * @param fare usage
   * @return the base fare with accummulated plus ups (from HIP, EMS, BHC,
   *         OSC, COM)
   **/
  MoneyAmount calcBaseFare(const FarePath& farePath, const FareUsage& fareUsage);

  /**
   * This function is called to Match the MinimumFare Rule Level Exclusion
   * Table
   *
   * @param pu (pricing unit) A reference to the pricing unit.
   * @return true if the through market match an item in the "Minimum Fare
   *         Rule Level Exclusion Table", false otherwise.
   **/
  bool checkThroughMarketExclusion(DiagCollector& diag,
                                   PricingTrx& trx,
                                   const FarePath& farePath,
                                   const PaxTypeFare& paxTypeFare);

  /**
   * This function is called to Match the MinimumFare Application Table
   *
   * @param pu (pricing unit) A reference to the pricing unit.
   * @return true if the through market match an item in the "Minimum Fare
   *         Application Table", false otherwise.
   **/
  bool checkExceptionTable(DiagCollector& diag,
                           PricingTrx& trx,
                           const FarePath& farePath,
                           const PaxTypeFare& paxTypeFare);

  /**
   * Process "Intermediate Location Minimum Fare Exclusion"
   *
   * @param pricing trx
   * @param fare path
   * @param pricing unit
   * @param fare usage
   */
  void processIntermediate(DiagCollector& diag,
                           PricingTrx& trx,
                           const FarePath& farePath,
                           const PricingUnit& pu,
                           FareUsage& fu);

  /**
   * Override MinimumFare::compareAndSaveFare - compare, calculate, and
   * save the plus-up amount
   *
   * @param the through pax type fare
   * @param the intermediate pax type fare
   **/
  bool compareAndSaveFare(const PaxTypeFare& paxTypeFareThru,
                          const PaxTypeFare& paxTypeFareIntermediate,
                          MinFarePlusUpItem& curPlusUp,
                          bool useInternationalRounding,
                          bool outbound = true) override;

  bool compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                          const PtfPair& ptfPair,
                          MinFarePlusUpItem& curPlusUp,
                          bool useInternationalRounding,
                          bool outbound = true) override;

  /**
   * This function is called to qualify the MH for DMC
   *
   * @param DMC diagnostic
   * @param PricingTrx A reference to the Trx.
   * @param farePath A reference to the Fare Path.
   * @return true if through fare qualifies for MH exception, false otherwise.
   **/
  bool
  qualifyMHException(DiagCollector& diag, const PricingTrx& trx, const FarePath& farePath) const;

  PtfPair selectInboundThruFare(PricingTrx& trx,
                                const FarePath& farePath,
                                const PricingUnit& pu,
                                const FareUsage& fu) const;

private:
  MinFarePlusUpItem* _plusUp = nullptr;
};
} // namespace tse
