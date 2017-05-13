//-------------------------------------------------------------------
//
//  File:        PseudoObjectBuilder.h
//
//  Copyright Sabre 2010
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

#include "DataModel/TktFeesRequest.h"

namespace tse
{
class DataHandle;
class DiagCollector;
class DifferentialData;
class FareMarket;
class FarePath;
class FareUsage;
class PaxTypeFare;
class PricingUnit;
class TktFeesPricingTrx;

class PseudoObjectsBuilder
{
  friend class PseudoObjectsBuilderTest;

public:
  PseudoObjectsBuilder(TktFeesPricingTrx& trx);
  virtual ~PseudoObjectsBuilder() {}

  void build();
  FarePath* build(Itin* paxItin, PaxType& paxType);

protected:
  void build(Itin* itin);
  PseudoObjectsBuilder(const PseudoObjectsBuilder& rhs);
  PseudoObjectsBuilder& operator=(const PseudoObjectsBuilder& rhs);

  void buildPU();
  void buildPUWithFareBreakInfo();
  void buildFareUsageIntoSideTripPU(FareUsage* fareUsage, const uint8_t sideTripID);

  uint16_t getTotalNumFareComp();

  FareUsage* buildFareUsage(std::vector<TravelSeg*>::const_iterator& tvlSegIBgn,
                            std::vector<TravelSeg*>::const_iterator& tvlSegIEnd);

  FareUsage*
  buildFareUsage(const TktFeesRequest::TktFeesFareBreakInfo& fbInfo, uint8_t& sideTripID);

  FareMarket* findFareMarket(const TravelSeg* tvlSegBgn, const TravelSeg* tvlSegEnd);

  void findFareUsageTvlSegs(const TktFeesRequest::TktFeesFareBreakInfo& fbInfo,
                            std::vector<TravelSeg*>& fuTvlSeg,
                            uint8_t& sideTripID);

  PaxTypeFare* buildPaxTypeFare(FareMarket& fm);

  void findCurrentItinTktFeesFareBreakInfo(Itin* itin);

  void insertFareUsageTvlSegToItsPU();
  void diag881(PaxType& paxType);
  void printFareUsage(PricingUnit& pu, DiagCollector& diag);

  void buildPUWithSpecifiedFareBreakInfo(const TktFeesRequest::TktFeesFareBreakInfo& fbI);

  void initAccountCodes(Itin* itin);
  void buildDifferentialData(FareUsage* fareUsage);
  DifferentialData*
  buildDifferentialItem(FareUsage& fareUsage, const TktFeesRequest::TktFeesDifferentialData& tfdd);
  PaxTypeFare*
  buildPaxTypeFareDiff(FareMarket& fm, const TktFeesRequest::TktFeesDifferentialData& tfdd);
  void initFOPBinNumber(Itin* itin);
  void printAccountCode(DiagCollector& diag);
  void printCorpId(DiagCollector& diag);
  void printTktDesignator(DiagCollector& diag);
  void printFopBin(DiagCollector& diag);

  TktFeesPricingTrx& _trx;
  Itin* _currentItin;
  DataHandle& _dataHandle;
  bool _isRTJourney;

private:

  FarePath* _farePath;
  PaxType* _currentPaxType;
  PricingUnit* _pricingUnit;
  uint16_t _nextFareCompNo;
  uint16_t _totalNumFareComp;

  const std::vector<TktFeesRequest::TktFeesFareBreakInfo*>* _currentItinTktFeesFareBreakInfo;
  const std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>*
  _currentItinTktFeesFareBreakAssoc;
  const TktFeesRequest::TktFeesFareBreakInfo* _currentFareBreakInfo;
  std::vector<PricingUnit*> _sideTripPUs;
};

} // tse

