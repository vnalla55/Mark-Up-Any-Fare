//-------------------------------------------------------------------
//
//  File:        ESVPQDiversifier.h
//  Created:     Jan 27, 2008
//  Authors:
//
//  Description: Class managing ESV Diversity logic
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/PaxType.h"
#include "DataModel/ShoppingTrx.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{

class ESVPQItem;

class ESVPQDiversifier
{
  friend class ESVPQDiversifierStub;
  friend class ESVPQDiversifierTest;

public:
  using FlightMap = std::map<ShoppingTrx::SchedulingOption*, int>;
  using CarrierMap = std::map<CarrierCode, int>;

private:
  void insertToFlightMap(ShoppingTrx::SchedulingOption* sop, int count, FlightMap& flightMap);
  bool checkFlightLimit(ShoppingTrx::SchedulingOption* sop, FlightMap& flightMap);

  void insertToCarrierMap(CarrierCode carrier, int count, CarrierMap& carrierMap);
  bool checkCarrierLimit(CarrierCode carrier, CarrierMap& carrierMap);
  bool
  checkCarrierLimit(CarrierCode carrier, CarrierMap& carrierCountMap, CarrierMap& carrierLimitMap);

  void initFlightMustPriceAndSopsPerCxrMaps();
  bool checkItinFlightsMustPriceLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);
  void decreaseItinFlightsMustPriceCd(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);

  void initCarrierMustPriceMaps();
  bool checkItinCarriersMustPriceLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                        bool onlineFlag);
  void decreaseItinCarriersMustPriceCd(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                       bool onlineFlag);

  void initFlightLFSMaps();
  bool checkItinFlightsLFSLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);
  void decreaseItinFlightsLFSCd(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);

  void initCarrierLFSMaps();
  bool checkItinCarriersLFSLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                  CarrierCode& carrier);
  void decreaseItinCarriersLFSCd(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);

  bool checkItinCarriersOnlineLFSLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);
  void increaseItinOnlineCarriersLFSC(std::vector<ShoppingTrx::SchedulingOption*>& sopVec);
  void initInterlineRestrictedCxr();
  void addCarriers(std::set<CarrierCode>& cxrList,
                   const std::vector<PaxTypeFare*>* const paxTypeFareVec) const;
  bool cxrRestrictionCorrect(CarrierCode mainCxr, CarrierCode cxr) const;

public:
  virtual ~ESVPQDiversifier() = default;

  void init(ShoppingTrx* trx);

  bool
  processItinMustPriceFlightsAndCarriersLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                               int onlineInterlineInd);

  MoneyAmount calcMustPriceUpperBound(MoneyAmount minFareValue, bool isNonstopFlag);
  MoneyAmount calcLFSUpperBound(MoneyAmount minFareValue);
  int calcMustPriceLimitForCurrPass(int numOfAlreadyPickedItins, int maxMustPricePassSize);
  bool processItinLFSFlightsAndCarriersLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                              CarrierCode& carrier);
  bool
  processItinLFSFlightsAndCarriersOnlineLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                               CarrierCode& carrier);
  void initAVSCarriers();
  bool isAVSCarrier(CarrierCode);
  int calcLFSLimitForPassOne(int);
  int calcLFSLimitForPassTwo(int);
  int calcLFSLimitForPassThree(int);
  void recalcCarrierOnlineLimitLFSMap();
  bool checkRestrictedCarriers(ESVPQItem* pqItem, std::string& addInfo) const;
  bool isOnlineOnlyRestricted(CarrierCode cxr) const;
  // accessors
  const ESVOptions* esvOptions() const { return _esvOptions; }

private:
  ShoppingTrx* _shoppingTrx = nullptr;
  ESVOptions* _esvOptions = nullptr;
  PaxType* _paxType = nullptr;
  int _totalOutbound = 0;
  int _totalInbound = 0;
  std::vector<CarrierCode> _avsCarriersVec;
  std::vector<CarrierCode> _onlineOnlyCxr;
  std::vector<std::pair<CarrierCode, std::vector<CarrierCode> > > _restrictedCxr;

  // auxiliary data structures
  CarrierMap _outSopsPerCxrMap;
  CarrierMap _inSopsPerCxrMap;

  // Must Price diversity data structures
  FlightMap _inFlightCountdownMPMap;
  FlightMap _outFlightCountdownMPMap;

  CarrierMap _inOnlineCxrCountdownMPMap;
  CarrierMap _outOnlineCxrCountdownMPMap;
  CarrierMap _inInterlineCxrCountdownMPMap;
  CarrierMap _outInterlineCxrCountdownMPMap;

  // LFS diversity data structures
  CarrierMap _inCxrCountdownLFSMap; // request.inBndCarrierCntMap
  CarrierMap _outCxrCountdownLFSMap; // request.outBndCarrierCntMap

  CarrierMap _onlineCxrLimitLFSMap; // onlineCarrierCntMap
  CarrierMap _onlineCxrCountMapLFS; // onlineCarrierRunningCntMap

  FlightMap _flightCountdownLFSMap; // itin.maxOptions

}; // class

} // tse namespace

