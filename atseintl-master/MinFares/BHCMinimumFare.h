//----------------------------------------------------------------------------
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

#include "Common/TsePrimitiveTypes.h"
#include "MinFares/MinimumFare.h"

#include <set>

namespace tse
{
class PricingTrx;
class PaxType;
class PaxTypeFare;
class FarePath;
class PricingUnit;
class FareUsage;

/**
 * @class BHCMinimumFare
 * This class will process the One-Way Backhaul(BHC) after HIPS for One-Way
 * pricing unit.
*/

class BHCMinimumFare : public MinimumFare
{
public:
  BHCMinimumFare(std::set<uint32_t>& normalExemptSet) : _normalExemptSet(normalExemptSet) {}

  MoneyAmount process(PricingTrx& trx,
                      const FarePath& farePath,
                      const PricingUnit& pu,
                      FareUsage& fu,
                      DiagCollector* diag);

private:
  bool checkTableItem(PricingTrx& trx,
                      const FarePath& farePath,
                      const Itin& itin,
                      const PaxTypeFare& paxTypeFare,
                      DiagCollector* diag);

  /**
   * Override MinimumFare::compareAndSaveFare - compare, calculate, and
   * save the plus-up amount for normal condition (found intermediate fare without construction)
   *
   * @param the through pax type fare
   * @param the intermediate pax type fare
   **/
  bool compareAndSaveFare(const PaxTypeFare& paxTypeFareThru,
                          const PaxTypeFare& paxTypeFareIntermediate,
                          MinFarePlusUpItem& curPlusUp,
                          bool useInternationalRounding = false,
                          bool outbound = true) override;

  /**
   * Override MinimumFare::compareAndSaveFare - compare, calculate, and
   * save the plus-up amount for construction of unpublished fare condition
   *
   * @param the through pax type fare
   * @param the intermediate pax type fare
   **/
  bool compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                          const PtfPair& ptfPair,
                          MinFarePlusUpItem& curPlusUp,
                          bool useInternationalRounding = false,
                          bool outbound = true) override;

  std::set<uint32_t>& _normalExemptSet;
};
} // namespace tse
