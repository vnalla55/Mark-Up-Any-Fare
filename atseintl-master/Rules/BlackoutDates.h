#pragma once

#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/DataHandle.h"
#include "Rules/RuleApplicationBase.h"

#include <vector>

namespace tse
{
class Predicate;
class IsDateBetween;

class BlackoutDates : public RuleApplicationBase
{
  friend class BlackoutDatesTest;

public:
  BlackoutDates();

  void initialize(const PricingTrx& trx, const BlackoutInfo* blackout);

  virtual Record3ReturnTypes process(PaxTypeFare& paxTypeFare, PricingTrx& trx);

  virtual Record3ReturnTypes process(PaxTypeFare& paxTypeFare, PricingTrx& trx, bool isInbound);

  virtual Record3ReturnTypes
  process(PricingTrx&, const PricingUnit&, const FareUsage&, const Itin&);

  const BlackoutInfo& info() const { return *_blackoutInfo; }

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override
  {
    return FAIL;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override
  {
    return FAIL;
  }

  static constexpr char RANGE = 'R';
  static constexpr char DATES = 'D';
  static constexpr char INT_DONT_APPLY = 'X';
  static constexpr char BLANK = ' ';
  static constexpr char UNAVAILABLE = 'X';
  static constexpr char TEXT_ONLY = 'Y';

protected:
  virtual IsDateBetween* createDateBetween();

  Record3ReturnTypes processCommon(PricingTrx& trx,
                                   const PaxTypeFare& paxTypeFare,
                                   const BlackoutInfo& blackout,
                                   RuleConst::TSIScopeParamType scope,
                                   bool isIbound);

  Predicate* getPredicate(const BlackoutInfo& rule);
  Predicate* getTextOnlyPredicate(const BlackoutInfo& rule);
  Predicate* getSameSegmentsPredicate(const BlackoutInfo& rule);
  Predicate* getContainsInternationalPredicate(const BlackoutInfo& rule);
  virtual Predicate* getDateBetweenPredicate(const BlackoutInfo& rule);

  template <typename T>
  Predicate* getDatePredicate(const BlackoutInfo& rule)
  {
    T* p = _dataHandle.create<T>();
    p->initialize(rule.tvlStartYear(),
                  rule.tvlStartMonth(),
                  rule.tvlStartDay(),
                  rule.tvlStopYear(),
                  rule.tvlStopMonth(),
                  rule.tvlStopDay(),
                  RuleConst::BLACKOUTS_RULE);
    return p;
  }

  struct GeoBools;

  bool callGeo(int geoTable,
               RuleUtil::TravelSegWrapperVector& appSegVec,
               GeoBools& geoBools,
               TSICode& tsi) const;

  bool hasTsiPuScope(RuleConst::TSIScopeType& geoScope,
                     PricingTrx& trx,
                     const VendorCode& vendor,
                     int itemNo) const;

  const BlackoutInfo* _blackoutInfo;
  DataHandle _dataHandle;
  Predicate* _root;

  const PaxTypeFare* _paxTypeFare;
  const PricingUnit* _pricingUnit;
  const FareUsage* _fareUsage;
  PricingTrx* _trx;
  RuleConst::TSIScopeParamType _defaultScope;
  const std::vector<TravelSeg*>* _travelSegs;
  const Itin* _itin;

  static Logger _logger;
};

} // namespace tse

