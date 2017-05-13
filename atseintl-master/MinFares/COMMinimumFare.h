//----------------------------------------------------------------------------
//
//  File:     COMMinimumFare.h
//  Created:  5/03/2004
//  Authors:
//
//  Description	:This is the class for Country Minimum Fare Check.
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
#include "Common/TSEException.h"
#include "DataModel/FarePath.h"

namespace tse
{
class PricingTrx;
class PaxTypeFare;
class PricingUnit;

/**
 * @class COMMinimumFare
 *
 * This class is to do (COM) Country of Origin Minimum check
 *
 */
class COMMinimumFare : public MinimumFare
{
  friend class COMMinimumFareTest;

private:
  struct ComProcInfo;

public:
  COMMinimumFare(PricingTrx& trx,
                 FarePath& farePath,
                 std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                 std::multimap<uint16_t, const MinFareAppl*>& applMap,
                 std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap);

  /**
   * This function is used to do COM check across pricing units in a fare path.
   *
   * @return total plus up amount for a fare path
   */
  MoneyAmount process();

  MoneyAmount processPricingUnit(ComProcInfo& comProcInfo, DiagCollector& diag);

private:
  bool qualifyComCheck(std::vector<ComProcInfo>& comProcInfoList);

  bool checkMinFareTables(const PaxTypeFare& paxTypeFare, DiagCollector& diag);

  /**
   * This function is to process specific intermediate point if it is a
   * country of origin.
   *
   * @param paxTypeFare - through fare component is being validated.
   * @param countryOfPaymentCity - an iterator which point to the
   *        intermediate point of the same country of origin
   * @param diag - reference to diagnostic collector
   *
   * @return true - success process, false - data error
   */
  void processIntermediateCityPair(const PricingUnit& pu,
                                   const FarePath::OscPlusUp& oscPlusUp,
                                   std::vector<TravelSeg*>::const_iterator& countryOfOriginCity,
                                   MinFarePlusUpItem*& comPlusUp,
                                   bool isCrossPu,
                                   DiagCollector& diag);

  void processIntermediateCityPair(const PricingUnit& pu,
                                   FareUsage& fareUsage,
                                   std::vector<TravelSeg*>::const_iterator& countryOfOriginCity,
                                   MinFarePlusUpItem*& comPlusUp,
                                   bool isCrossPu,
                                   DiagCollector& diag);

  void processIntermediateCityPair(ComProcInfo& comProcInfo,
                                   FareUsage& fareUsage,
                                   MinFarePlusUpItem*& comPlusUp,
                                   bool isCrossPu,
                                   DiagCollector& diag);

  void processCityPair(const PricingUnit& pu,
                       FareUsage& fareUsage,
                       std::vector<TravelSeg*>::const_iterator& aIter,
                       std::vector<TravelSeg*>::const_iterator& zIter,
                       MinFareFareSelection::FareDirectionChoice fareDirection,
                       MinFarePlusUpItem*& comPlusUp,
                       DiagCollector& diag);

  /**
   * Calculate the base fare for the component accumulating any HIP, BHC, OSC, and
   * mileage surchage.
   *
   * @param paxTypeFare/fareUsage - through fare component is being validated.
   * @return the accumulated fare amount.
   */
  MoneyAmount accumulateBaseFare(const FareUsage& fareUsage);
  MoneyAmount accumulateBaseFare(const PaxTypeFare& paxTypeFare, const PricingUnit& pricingUnit);

  /**
   * This function is to calculate the plus up fare amount when the
   * intermediate fare amount is higher than a new base fare amount.
   * Then temporary save for the next comparison
   *
   * @param paxTypeFare - through fare component is being validated.
   * @param paxTypeFareIntermediate - intermediate fare component is being
   *        validated.
   * @return none
   */
  MoneyAmount calculatePlusUp(const MoneyAmount& baseFare,
                              const PaxTypeFare& paxTypeFare,
                              const PaxTypeFare& paxTypeFareIntermediate,
                              const PricingUnit& pricingUnit,
                              MinFarePlusUpItem*& comPlusUp);

  /**
   * This function is called to compare the latest higher fare amount against
   *  the current unpublish fare amount.
   *
   * @param paxTypeFare A reference to current thru fare component.
   * @param unPubAmount A total amount of unpublished construction fare.
   */
  MoneyAmount calculatePlusUp(const MoneyAmount& baseFare,
                              const PaxTypeFare& paxTypeFare,
                              const MoneyAmount& unPubAmount,
                              const LocCode& constructPoint,
                              const LocCode& boardPoint,
                              const LocCode& offPoint,
                              const PricingUnit& pricingUnit,
                              MinFarePlusUpItem*& comPlusUp);

  /**
   * This function is to print plus up information if exists into diagnostic
   * message.
   *
   * @param paxTypeFare - through fare component is being validated.
   * @param diag - diagnostic collector.
   * @return plus up amount
   */
  void printPlusUpInfo(const FareUsage& fareUsage,
                       const MinFarePlusUpItem* comPlusUp,
                       DiagCollector& diag);
  void printPlusUpInfo(const PaxTypeFare& paxTypeFare,
                       const PricingUnit& pricingUnit,
                       const MinFarePlusUpItem* comPlusUp,
                       DiagCollector& diag);
  void printPlusUpInfo(const MoneyAmount& baseFare,
                       const MinFarePlusUpItem* comPlusUp,
                       DiagCollector& diag);

  PricingTrx& _trx;
  FarePath& _farePath;

  std::multimap<uint16_t, const MinFareRuleLevelExcl*>& _ruleLevelMap;
  std::multimap<uint16_t, const MinFareAppl*>& _applMap;
  std::map<uint16_t, const MinFareDefaultLogic*>& _defaultLogicMap;

  struct ComProcInfo
  {
    ComProcInfo(PricingUnit* p, TravelSeg* t) : pu(p), comTvlSeg(t) {}
    PricingUnit* pu;
    const TravelSeg* comTvlSeg;

    struct PuEqual : std::binary_function<ComProcInfo, PricingUnit*, bool>
    {
      bool operator()(const ComProcInfo& comProcInfo, const PricingUnit* pu) const
      {
        return comProcInfo.pu == pu;
      }
    };
  };
};
}
