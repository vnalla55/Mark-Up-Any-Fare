//----------------------------------------------------------------------------
//
//  File:     HIPMinimumFare.h
//  Created:  3/03/2004
//  Authors:
//
//  Description	:This is the class for Higher Intermediate Point Check for Minimum Fares.
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "MinFares/MinimumFare.h"

#include <vector>

namespace tse
{
class DiagCollector;
class DiscountInfo;
class FareByRuleItemInfo;
class FarePath;
class FareUsage;
class MinFarePlusUp;
class MinFarePlusUpItem;
class MinFareRuleLevelExcl;
class PaxType;
class PaxTypeFare;
class PricingUnit;
class PricingTrx;

/**
 * @class HIPMinimumFare - Higher Intermediate Point Minimum Fare Check
 * This class will be called using Fare Path or PricingUnit or PaxType Fare.
 * HIP process should be take placed before
 *  any others minimum fare modules process(BHC, CTM,,,).
 * Also HIP can be called later when needed for pricing lowest class of service.
*/
class HIPMinimumFare : public MinimumFare
{
  friend class HIPMinimumFareTest;
  friend class BHCMinimumFareTest;

public:
  HIPMinimumFare(PricingTrx& trx,
                 MinimumFare::HipProcess hipProcess = MinimumFare::DEFAULT_HIP_PROCESSING);

  virtual ~HIPMinimumFare();

  /**
    * This function is used to apply HIP check for a fare path. It returns HIP and BHC total
    * plus up amount the fare path.
    */
  MoneyAmount process(const FarePath& farePath,
                      std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                      std::multimap<uint16_t, const MinFareAppl*>& applMap,
                      std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap);

  /**
    * This function is the overload function which receive pricing unit as an input. It returns
    * HIP and BHC total plus up amount for the pricing unit.
    */
  MoneyAmount process(const PricingUnit& pu,
                      const FarePath& farePath,
                      std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                      std::multimap<uint16_t, const MinFareAppl*>& applMap,
                      std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap);

  /**
    * This function is the overload function which receive fare usage as an input. It returns
    * only HIP plus up amount for the fare usage.
    */
  MoneyAmount process(FareUsage& fu, const PricingUnit& pu, const FarePath& farePath);

  MoneyAmount process(FareUsage& fu,
                      const PricingUnit& pu,
                      const FarePath& farePath,
                      std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                      std::multimap<uint16_t, const MinFareAppl*>& applMap,
                      std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap);

  bool passedLimitation() { return _passLit; }

  const std::set<uint32_t> normalExemptSet() const { return _normalExemptSet; }
  bool isNormalExempted(const std::vector<TravelSeg*>& tvl) const;
  bool isExemptedByApplication(const PaxTypeFare& paxTypeFare);

protected:
  /**
   * This function is to check if thru cat25 fare also has cat19-22 apply
   *
   * @param the through pax type fare
   * @return PaxTypeCode - from record 8 primaly passenger type
   **/
  PaxTypeCode checkDoubleDiscount(const PaxTypeFare& paxTypeFare);

  /**
   * This function is to check if thru cat25 fare can be HIP exempted
   *
   * @param the through pax type fare
   * @return bool - true (if no hip apply)
   **/
  bool cat25FareExempt(const PaxTypeFare& paxTypeFare);

  /**
   * Override MinimumFare::compareAndSaveFare - compare, calculate, and
   * save the plus-up amount
   *
   * @param the through pax type fare
   * @param the intermediate pax type fare
   * @param the plusup class for fare comparison
   **/
  bool compareAndSaveFare(const PaxTypeFare& paxTypeFareThru,
                          const PaxTypeFare& paxTypeFareIntermediate,
                          MinFarePlusUpItem& curPlusUp,
                          bool useInternationalRounding = false,
                          bool outbound = true) override;

  /**
   * Override MinimumFare::compareAndSaveFare - compare, calculate, and
   * save the plus-up amount
   *
   * @param the through pax type fare
   * @param the unpublished construction fare amount
   * @param the travelSeg board point iterator
   * @param the travelSeg off point iterator
   * @param the travelSeg construction point iterator
   * @param the plusup class for fare comparison
   **/
  bool compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                          const PtfPair& ptfPair,
                          MinFarePlusUpItem& curPlusUp,
                          bool useInternationalRounding = false,
                          bool outbound = true) override;

  /**
   * This function is to check cat25 thru fare and then calculate the discount amount
   *   otherwise return thru fare amount with EMS (extra mileage surcharge)
   *
   * @param the through pax type fare
   * @return fare amount for intermediate fare
   **/
  MoneyAmount getIntermediateFareAmount(const PaxTypeFare& paxTypeFare,
                                        MoneyAmount originalIntermediateAmt,
                                        bool useInternationalRounding = false);

  /**
   * This function is to convert an extra specified amount to paxTypeFare currency
   *    in order to add or subtract to discounted amount.
   *
   * @param the through pax type fare
   * @param the reference to FareByRuleItemInfo
   * @return NUC fare amount
   **/
  MoneyAmount convertCat25Currency(const PaxTypeFare& paxTypeFare,
                                   const FareByRuleItemInfo& fbrItemInfo,
                                   bool useInternationalRounding = false);

  /**
   * This function is to convert an extra specified amount to paxTypeFare currency
   *    in order to add or subtract to discounted amount.
   *
   * @param the through pax type fare
   * @param the reference to DiscountInfo
   * @return NUC fare amount
   **/
  MoneyAmount
  convertCat19Currency(const PaxTypeFare& paxTypeFare, const DiscountInfo& discountInfo);

  /**
   * This function is to print plus up informatiom
   *
   * @param the reference to paxTypeFare
   * @param the reference to requestedPaxType
   * @param the reference to diag
   * @return none
   **/
  void printPlusUpInfo(const PaxTypeFare& paxTypeFare,
                       const MinFarePlusUp& minFarePlusUp,
                       DiagCollector& diag);

  bool skipProcessForSpecialOnly();

private:
  PricingTrx& _trx;
  DiagCollector* _diag = nullptr;
  bool _throw = false;
  MoneyAmount _bhcPlusUp = 0.0;
  bool _passLit = true;
  std::set<uint32_t> _normalExemptSet;
};
} // namespace tse
