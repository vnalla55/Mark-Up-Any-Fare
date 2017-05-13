//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Rules/RuleApplicationBase.h"

namespace tse
{

class SeasonalAppl;

class SeasonalApplication : public RuleApplicationBase
{
public:
  SeasonalApplication() {}

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& fare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const Itin* itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage)
  {
    return validate(trx, rule, nullptr, itin, pricingUnit, fareUsage);
  }

  static constexpr Indicator HIGH_SEASON = 'H';

protected:
  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath* farePath,
                              const Itin* itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage);

  Record3ReturnTypes checkUnavailableAndText(const SeasonalAppl* seasonRule) const;

  Record3ReturnTypes checkHighSeasonOpenSeg(const SeasonalAppl* seasonRule,
                                            const PaxTypeFare& paxTypeFare,
                                            const std::vector<TravelSeg*>& travelSeg) const;

  Record3ReturnTypes
  checkAssumptionOverride(const SeasonalAppl* seasonRule, Itin& itin, PricingTrx* ptrx = nullptr) const;

  Record3ReturnTypes
  checkOverrideForDateRange(const SeasonalAppl* seasonRule, const FareMarket& fareMarket) const;

  Record3ReturnTypes checkGeoTblItemNo(const SeasonalAppl* seasonRule,
                                       const PaxTypeFare& paxTypeFare,
                                       const FareMarket& fareMarket,
                                       Itin& itin,
                                       PricingTrx& trx) const;

  Record3ReturnTypes checkGeoForDateRange(const SeasonalAppl* seasonRule,
                                          const PaxTypeFare& paxTypeFare,
                                          const FareMarket& fareMarket,
                                          Itin& itin,
                                          PricingTrx& trx) const;

  void diagRestriction(const SeasonalAppl& seasonRule, DiagManager& diag) const;

  static Logger _logger;
};

} // tse

