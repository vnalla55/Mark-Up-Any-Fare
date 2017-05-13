#pragma once

#include "DBAccess/DayTimeAppInfo.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <log4cxx/helpers/objectptr.h>

namespace tse
{

class Fare;
class FareMarket;
class FarePath;
class PricingTrx;
class DateTime;
class PricingUnit;
class Itin;
class Logger;

class DayTimeApplication : public RuleApplicationBase
{
public:
  DayTimeApplication() : _itemInfo(nullptr),_applyCat2SystemDefaults(false){}

  virtual ~DayTimeApplication() {}
  bool applyCat2SystemDefaults() const { return _applyCat2SystemDefaults; }
  void setApplyCat2SystemDefaults(bool applyDefaults) {_applyCat2SystemDefaults = applyDefaults;} 

  void initialize(const DayTimeAppInfo* ruleItemInfo);

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& paxTypeFare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* ruleInfo,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage ) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath* farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) const;

  static constexpr char subJourneyBased = 'X';
  static constexpr char sameDay = 'X';
  static constexpr char dataUnavailable = 'X';
  static constexpr char textOnly = 'Y';
  static constexpr char range = 'R';
  static constexpr char negative = 'X';
  static constexpr char nonNegative = ' ';
  static constexpr int dayStart = 1; // first minut of the day
  static constexpr int dayEnd = 1440; // last minute of the day

protected:
  Record3ReturnTypes callGeoSpecTable(const Fare& fare,
                                      const FareMarket* fareMarket,
                                      const PricingUnit* pricingUnit,
                                      const FarePath* farePath,
                                      PricingTrx& trx,
                                      RuleUtil::TravelSegWrapperVector& appSegVec,
                                      RuleConst::TSIScopeParamType scope) const;

  Record3ReturnTypes validateDayOfWeek(const DateTime& travelDate,
                                       uint32_t travelDow,
                                       bool& skipTODCheck,
                                       bool& displayWqWarning) const;

  Record3ReturnTypes checkUnavailableAndText() const;

  Record3ReturnTypes checkDOWSame(const PricingUnit& pricingUnit,
                                  const FarePath* farePath,
                                  const Fare& fare,
                                  PricingTrx& trx) const;

  Record3ReturnTypes checkOccurrence(const PricingUnit& pricingUnit,
                                     const FarePath* farePath,
                                     const Fare& fare,
                                     PricingTrx& trx) const;

  Record3ReturnTypes checkTOD(const DateTime& travelDateTime, bool& displayWqWarning) const;

  const DayTimeAppInfo* _itemInfo;
  bool  _applyCat2SystemDefaults;
private:
  static Logger _logger;
};

} // namespace tse

