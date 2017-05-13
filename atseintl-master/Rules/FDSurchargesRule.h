//----------------------------------------------------------------
//
//  File:    FDSurchargesRule.h
//  Authors:
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
//-----------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"
#include "Rules/RuleItem.h"
#include "Rules/RuleUtil.h"
#include "Rules/SurchargesRule.h"


namespace tse
{
class FareDisplayInfo;
class PricingTrx;
class FarePath;
class PricingUnit;
class FareUsage;
class RuleItemInfo;
class SurchargesInfo;
class SurchargeData;
class RuleItemInfo;

class FDSurchargesRule : public SurchargesRule
{
  friend class FDSurchargesRuleTest;

public:
  FDSurchargesRule();
  virtual ~FDSurchargesRule();

  Record3ReturnTypes process(PricingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             const CategoryRuleItemInfo* rule,
                             const SurchargesInfo* surchInfo);

  Record3ReturnTypes processSectorSurcharge(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

private:
  void updateFareInfo(const FareDisplayTrx& fdTrx,
                      std::vector<SurchargeData*>& fdiSurchargeData,
                      const std::vector<SurchargeData*>& surchargeData);

  bool checkValidData(FareDisplayTrx& fdTrx,
                      const FarePath* farePath,
                      const PricingUnit* pricingUnit,
                      FareUsage* fareUsage);
  bool isDirectionPass(const CategoryRuleItemInfo* cfrItem, bool isLocationSwapped, bool isInbound);

  void buildInboundFareMarket(const FareMarket* fareMarket, FareMarket* inbFareMarket);

  AirSeg* buildReturnSegment(const FareDisplayTrx& trx, const TravelSeg& tvlSeg);

  void addSectorSurcharges(const FareDisplayTrx& trx,
                           std::vector<SurchargeData*>& sectorSurcharges,
                           const AirSeg* airSeg,
                           std::vector<SurchargeData*>& cat12Surcharges);
  void updateSectorSurchargeText(FareDisplayInfo* fareDisplayInfo,
                                 const std::vector<SurchargeData*>& sectorSurcharges,
                                 const AirSeg* airSeg);

};

} // namespace tse

