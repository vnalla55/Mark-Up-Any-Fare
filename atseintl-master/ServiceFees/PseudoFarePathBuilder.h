//-------------------------------------------------------------------
//
//  File:        PseudoFarePathBuilder.h
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

#include "Common/ServiceFeeUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AncRequest.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
class FarePath;
class FareUsage;
class PseudoFarePathBuilder
{
  friend class PseudoFarePathBuilderTest;

public:
  PseudoFarePathBuilder(PricingTrx& trx);
  virtual ~PseudoFarePathBuilder() {};

  void build();
  FarePath* build(Itin* itin, Itin* paxItin, PaxType& paxType);

protected:
  PricingTrx& _trx;
  Itin* _currentItin;
  DataHandle& _dataHandle;
  bool _isRTJourney;
  bool _processingBaggage;
  bool _applyFixedTariff;

  void build(Itin* itin, Itin* paxItin);
  PseudoFarePathBuilder(const PseudoFarePathBuilder& rhs);
  PseudoFarePathBuilder& operator=(const PseudoFarePathBuilder& rhs);

  virtual void setJourneyType();
  void buildPU();
  void buildPUWithFareBreakInfo();
  void buildPUWithExistingFareMarket();
  void buildFareUsageIntoSideTripPU(FareUsage* fareUsage, const uint8_t sideTripID);

  bool fareUsageUnknown();
  uint16_t getTotalNumFareComp();
  bool getNextFUTvlSegs(std::vector<TravelSeg*>::const_iterator& tvlSegIBgn,
                        std::vector<TravelSeg*>::const_iterator& tvlSegIEnd);

  FareUsage* buildFareUsage(std::vector<TravelSeg*>::const_iterator& tvlSegIBgn,
                            std::vector<TravelSeg*>::const_iterator& tvlSegIEnd);

  FareUsage* buildFareUsage(const AncRequest::AncFareBreakInfo& fbInfo, uint8_t& sideTripID);

  FareMarket* findFareMarket(const TravelSeg* tvlSegBgn, const TravelSeg* tvlSegEnd);

  void findFareUsageTvlSegs(const AncRequest::AncFareBreakInfo& fbInfo,
                            std::vector<TravelSeg*>& fuTvlSeg,
                            uint8_t& sideTripID);

  PaxTypeFare* buildPaxTypeFare(FareMarket& fm);
  void addPassedSegStatus(PaxTypeFare& paxTypeFare);
  void findCurrentItinAncFareBreakInfo(Itin* itin);

  void insertFareUsageTvlSegToItsPU();
  void setNoFollowByArunkSeg(std::vector<TravelSeg*>::const_iterator& fmTvlSegIBack,
                             const std::vector<TravelSeg*>::const_iterator& tvlSegIEnd);
  void diag881(PaxType& paxType);
  void insertTourCode(Itin* prcItin);
  void sideTripPUType();
  void buildPUWithMixWithFBIAndNoFBI();
  void buildPUWithSpecifiedFareBreakInfo(const AncRequest::AncFareBreakInfo& fbI);

  void buildPUForTravelSegs(std::vector<TravelSeg*>::const_iterator& fmTvlSegIFront,
                            std::vector<TravelSeg*>::const_iterator& fmTvlSegIBack,
                            std::vector<TravelSeg*>::const_iterator& tvlSegIEnd);
  void checkBookingCodeForWPAEMulitItinRequest(Itin* pricingItin);
  void initAccountCodes(Itin* itin);
  bool ignoreFBI();
  bool skipForRW() const;
  bool processingBaggage() const;

private:
  FarePath* _farePath;
  PaxType* _currentPaxType;
  PricingUnit* _pricingUnit;
  uint16_t _nextFareCompNo;
  uint16_t _totalNumFareComp;

  const std::vector<AncRequest::AncFareBreakInfo*>* _currentItinAncFareBreakInfo;
  const std::vector<AncRequest::AncFareBreakAssociation*>* _currentItinAncFareBreakAssoc;
  const AncRequest::AncFareBreakInfo* _currentFareBreakInfo;
  std::vector<PricingUnit*> _sideTripPUs;
};

} // tse
