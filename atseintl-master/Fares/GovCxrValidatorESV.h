//----------------------------------------------------------------------------
//  File:        GovCxrValidator.h
//  Created:     2008-04-16
//
//  Description: Class used to validate fares for governing carrier
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Common/Thread/TseCallableTrxTask.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/FareValidatorESV.h"
#include "Server/TseServer.h"

namespace tse
{
class FareMarket;

class GovCxrValidatorESV : public TseCallableTrxTask
{
public:
  GovCxrValidatorESV(ShoppingTrx& shoppingTrx,
                     TseServer& server,
                     ShoppingTrx::Leg& leg,
                     ItinIndex::ItinRow& itinRow,
                     Itin* journeyItin,
                     RuleControllers& ruleControllersESV,
                     std::vector<uint32_t>& rule_validation_order)
    : _trx(&shoppingTrx),
      _tseServer(&server),
      _leg(&leg),
      _itinRow(&itinRow),
      _journeyItin(journeyItin),
      _ruleControllersESV(&ruleControllersESV),
      _rule_validation_order(&rule_validation_order),
      _checkMarriedBookingCodes(true),
      _flightId(0),
      _itin(nullptr)
  {
    trx(&shoppingTrx);
  }

  void performTask() override;

private:
  void processGovCarrier();

  void processFlight(ItinIndex::ItinCellInfo& itinCellInfo);

  void processFareMarketPath(ItinIndex::ItinCellInfo& itinCellInfo,
                             const std::vector<FareMarket*>& fareMarketPath);

  void processFareMarketPathForPaxType(ItinIndex::ItinCellInfo& itinCellInfo,
                                       const std::vector<FareMarket*>& fareMarketPath,
                                       PaxType* paxType);

  bool resolveValidatedLocalJourneyFares(
      std::vector<PaxTypeFare*>& resultFaresVec,
      std::vector<FareValidatorESV::VFHolder>& vfHolder,
      const std::vector<FareValidatorESV::ValidatedFareType>& fareTypes);

  bool
  resolveValidatedLocalJourneyFares(std::vector<PaxTypeFare*>& paxTypeFares,
                                    std::vector<FareValidatorESV::ValidatedPaxTypeFares*>& vptfs);

  bool resolveValidatedLocalJourneyFaresOld(
      std::vector<PaxTypeFare*>& paxTypeFares,
      std::vector<FareValidatorESV::ValidatedPaxTypeFares*>& vptfs);

  void calculateTotalAmount(const std::vector<FareMarket*>& fareMarketPath,
                            FareValidatorESV::ValidatedFareType fareType,
                            std::vector<FareValidatorESV::VFHolder>& vfHoldersVec,
                            double& totalAmount,
                            std::vector<PaxTypeFare*>& paxTypeFareVec);

  void calculateTotalAmount(const std::vector<FareMarket*>& fareMarketPath,
                            FareValidatorESV::ValidatedFareType firstFareType,
                            FareValidatorESV::ValidatedFareType secondFareType,
                            std::vector<FareValidatorESV::VFHolder>& vfHoldersVec,
                            double& totalAmount,
                            std::vector<PaxTypeFare*>& paxTypeFareVec);

  void calculateTotalAmount(const std::vector<FareMarket*>& fareMarketPath,
                            const std::vector<FareValidatorESV::ValidatedFareType>& fareTypes,
                            std::vector<FareValidatorESV::VFHolder>& vfHoldersVec,
                            double& totalAmount,
                            std::vector<PaxTypeFare*>& paxTypeFareVec);

  void buildFareMarketPaths(uint32_t sizeOfPath,
                            std::vector<FareMarket*>& fareMarketPath,
                            std::vector<std::vector<FareMarket*> >& fareMarketPathMatrix);

  void buildFareMarketPaths(std::vector<std::vector<FareMarket*> >& fareMarketPathMatrix);

  void processItinJCBFlag(std::vector<std::vector<FareMarket*> >& fareMarketPathMatrix);

  void sortSopFareLists();

  void removeExpensiveOwFarePaths(std::vector<SOPFarePath*>& owSopFarePaths);

  bool setupJourneyItinTravelSeg(std::vector<TravelSeg*>& tempTravelSeg);

  void addSOPFarePath(MoneyAmount& totalAmount,
                      std::vector<PaxTypeFare*>& paxTypeFareVec,
                      std::vector<PaxTypeFare*>& paxTypeFareVecNoCat10,
                      const std::vector<FareMarket*>& fareMarketPath,
                      std::vector<SOPFarePath*>& sopFarePaths,
                      SOPFarePath::CombinationType combinationType = SOPFarePath::NOT_INITIALIZED);

  void addSOPFarePath(MoneyAmount& totalAmount,
                      std::vector<PaxTypeFare*>& paxTypeFareVec,
                      const std::vector<FareMarket*>& fareMarketPath,
                      std::vector<SOPFarePath*>& sopFarePaths,
                      bool cat10Restrictions,
                      SOPFarePath::CombinationType combinationType = SOPFarePath::NOT_INITIALIZED);

  void updateFarePath(PaxTypeFare* paxTypeFare,
                      MoneyAmount& amt,
                      std::vector<PaxTypeFare*>& paxTypeFareVec);

  static bool checkIfSameFares(const std::vector<PaxTypeFare*>& fareVector1,
                               const std::vector<PaxTypeFare*>& fareVector2);

  ShoppingTrx* _trx;
  TseServer* _tseServer;
  ShoppingTrx::Leg* _leg;
  ItinIndex::ItinRow* _itinRow;
  Itin* _journeyItin;
  RuleControllers* _ruleControllersESV;
  std::vector<uint32_t>* _rule_validation_order;
  bool _checkMarriedBookingCodes;
  uint32_t _flightId;
  Itin* _itin;
};
}

