//-------------------------------------------------------------------
////
////  Copyright Sabre 2004
////
////          The copyright to the computer program(s) herein
////          is the property of Sabre.
////          The program(s) may be used and/or copied only with
////          the written permission of Sabre or in accordance
////          with the terms and conditions stipulated in the
////          agreement/contract under which the program(s)
////          have been supplied.
////
////-------------------------------------------------------------------

#pragma once

#include "Rules/RuleApplicationBase.h"
#include "Rules/SubjectObserved.h"

#include "Rules/IObserver.h"

namespace tse
{
class PaxTypeFare;
class FareMarket;
class PricingTrx;
class Itin;
class DiagCollector;
class DiagManager;
class TravelRestriction;

class TravelRestrictions
    : public RuleApplicationBase,
      public SubjectObserved<
          IObserver<NotificationType, uint16_t, const DateTime&, const LocCode, const int>>
{
public:
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* ruleInfo,
                              const FareMarket& fareMarket) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* ruleInfo,
                              const Itin& itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* ruleInfo,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override;

  static constexpr Indicator NOTAPPLY = ' ';
  static constexpr Indicator TVLMUSTBECOMMENCEDBY = 'C';
  static constexpr Indicator TVLMUSTBECOMPLETEDBY = 'P';

protected:
  Record3ReturnTypes validateTvlStartDate(const DateTime& refDT,
                                          const TravelRestriction& tvlRstr,
                                          DiagManager& diag) const;

  Record3ReturnTypes validateTvlStartDate(const DateTime& refJourneyDT,
                                          const DateTime& refFareCompDT,
                                          const TravelRestriction& tvlRstr,
                                          DiagManager& diag) const;

  Record3ReturnTypes validateTvlStopTime(const DateTime& refDT,
                                         const TravelRestriction& tvlRstr,
                                         DiagManager& diag) const;

  void displayRuleDataToDiag(const TravelRestriction& tvlRestrictionInfo, DiagCollector& diag);

  bool isTravelIndNotApplicable(Indicator tvlInd) const;

  bool isGeoScopeSubJourney(RuleConst::TSIScopeType geoScope) const;

private:
  static Logger _logger;
};

class TravelRestrictionsObserverWrapper
{
public:
  virtual ~TravelRestrictionsObserverWrapper() = default;
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      PaxTypeFare& fare,
                                      const RuleItemInfo* ruleInfo,
                                      const FareMarket& fareMarket);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* ruleInfo,
                              const Itin& itin,
                              const PricingUnit& pricingUnit,
                              FareUsage& fareUsage);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* ruleInfo,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              FareUsage& fareUsage);

  void setRuleDataAccess(RuleControllerDataAccess* ruleDataAccess);

private:
  TravelRestrictions _travelRestriction;
};

} // namespace tse

