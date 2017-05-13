//----------------------------------------------------------------------------
//
//  File:     CTMMinimumFare.h
//  Created:  4/19/2004
//  Authors:
//
//  Description	:This is the class for Circle Trip Minimum Check for Minimum Fares.
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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/ErrorResponseException.h"

namespace tse
{
class DiagCollector;
class Fare;
class FareMarket;
class FarePath;
class FareUsage;
class PaxTypeFare;
class PricingUnit;
class PricingTrx;
class RoundTripFareSelection;

/**
 * @class CTMMinimumFare - Circle Trip Minimum Fare Check
 * This class will be called from MinFareChecker to process CTM if condition is met using
 *  pricing unit type.
 *
*/
class CTMMinimumFare : public MinimumFare
{

  friend class CTMMinimumFareTest;

public:
  CTMMinimumFare(PricingTrx& trx,
                 FarePath& farePath,
                 PricingUnit& pu,
                 std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                 std::multimap<uint16_t, const MinFareAppl*>& applMap,
                 std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap);

  virtual ~CTMMinimumFare();

  /**
    * This function is used to apply CTM check for each Pricing Unit.
    *
    * @param PricingTrx A reference to the Trx.
    * @param farePath A reference to the Fare Path.
    * @param pu (pricing unit) A reference to the pricing unit.
    *
    * @return plus up amount for pricing unit after CTM check.
    *
    \todo CTM check will process in the following order
     - Calculate base fare.
     - Check lowest cabin.
     - Get lowest cabin fare or get next higher cabin fare.
     - Call HIP for base fare.
     - Total base fare.
     - Check thru market exclusion for outbound and inbound fare.
     -   using Minimum Fare Rule Level Exclusion Data if CTM process can be excluded.
     - Determine check point using Minimum Fare Application Data.
     - Determine domestic point if can be excluded.
     - Determine exception point.
     - Print exception information for diagnostic 719
     - Build reprice Itin.
     - Select fare.
     - Calculate circle trip fare.
     - Print fare information for diagnostic 719
     - Compare and Save CTM plus up amount, cities into pricing unit and return.
  */
  MoneyAmount process();

  /**
   * This version is used by CPM
   */
  MoneyAmount process(DiagCollector& diag, bool cpmCheck = false);

private:
  struct ConstructInfo // Store construct info.
  {
    LocCode _constructedPoint;
    LocCode _boardConstructed;
    LocCode _offConstructed;
    MoneyAmount _highestUnPubAmount = 0.0;
  };

  struct CtmFareCheck
  {
    FareUsage* fareUsage = nullptr;
    const PaxTypeFare* thruMarketFare = nullptr;
    std::vector<TravelSeg*>::iterator tvlOff;

    const MinFareRuleLevelExcl* ruleLevelExcl = nullptr;
    bool excludedByRuleLevelExcl = false;

    const MinFareAppl* appl = nullptr;
    const MinFareDefaultLogic* defaultLogic = nullptr;

    const PaxTypeFare* selectedFare = nullptr;
    ConstructInfo constructInfo;
    FMDirection selectFareForDirection = FMDirection::UNKNOWN;
  };

  bool qualifyCtmCheck() const;

  void printThruFare(DiagCollector& diag) const;
  bool isPuSameCarrier() const;
  int checkRuleLevelExcl(bool sameCarrier, DiagCollector& diag);
  void checkApplication();

  bool getThruMarketFare();

  /**
   * This function is called to select the lower of outbound or inbound
   * circle trip fare
   *
   * @param paxTypeFareOutBound - pointer to outbound fare
   * @param paxTypeFareInBound - pointer to inbound fare
   */
  void compareAndSave(const PaxTypeFare* paxTypeFareOutBound,
                      const PaxTypeFare* paxTypeFareInBound,
                      CtmFareCheck& ctmFareCheck);

  void compareAndSave(const RoundTripFareSelection& rtFareSel, CtmFareCheck& ctmFareCheck);

  /**
   * This function is called to calculate base fare for CTM fare comparison
   * later.
   *
   */
  void calculateBaseFare();

  /**
   * This function is called to save the latest circle trip plus up amount
   *
   */
  void savePlusUp(const CtmFareCheck& ctmFareCheck);

  /**
   * This function is to print plus up information if exists into diagnostic message.
   *
   * @param diag - diagnostic collector.
   * @return plus up amount
   */
  MoneyAmount printPlusUpInfo(DiagCollector& diag);
  MoneyAmount getCtmFareAmt(const CtmFareCheck* ctmFareCheck) const;
  void updateCtmFareMap(CtmFareCheck& ctmFareCheck);
  const CtmFareCheck* getHighestCtmFare() const;
  MoneyAmount getFareAmount(const CtmFareCheck& ctmFareCheck) const;

  /**
   * Check if travel is around the world travel originating in Australia, or
   * New Zealand, then CTM will not apply
   */
  bool checkRtwException();

  /**
   * Check if PU contain infant domestic 0 amount fare
   *  then CTM will not apply
   */
  bool checkInfantException();

  PricingTrx& _trx;
  FarePath& _farePath;
  PricingUnit& _pu;

  MoneyAmount _baseFare = 0;

  std::multimap<uint16_t, const MinFareRuleLevelExcl*>& _ruleLevelMap;
  std::multimap<uint16_t, const MinFareAppl*>& _applMap;
  std::map<uint16_t, const MinFareDefaultLogic*>& _defaultLogicMap;
  std::vector<CtmFareCheck> _ctmFareList;
  std::map<std::string, CtmFareCheck*> _ctmFareMap;
};
} // namespace tse
