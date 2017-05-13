#pragma once

#include "DBAccess/DBAForwardDecl.h"
#include "DBAccess/CacheManager.h" // needed for below guy
#include "DBAccess/HistoricalDataAccessObject.h"
#include <vector>
#include <functional>

namespace tse
{
class PricingTrx;
class FareMarket;
class FareByRuleCtrlInfo;
class FootNoteCtrlInfo;
class Loc;
class LocKey;
class DateTime;

namespace Rec2Filter
{
class LocFilter
{
public:
  LocFilter(PricingTrx& trx, const FareMarket& fareMarket);
  bool isInLoc(const Loc& loc, const LocKey& r2loc, const FootNoteCtrlInfo& r2) const;
  bool operator()(const FareByRuleCtrlInfo* r2);
  bool operator()(const FootNoteCtrlInfo* r2);
  bool operator()(const GeneralFareRuleInfo* r2);

  const FareMarket& _fareMarket;
  GeoMatchResult _isLocationSwapped{false};

private:
  PricingTrx& _trx;
};

class CxrDependentFilter
{
public:
  CxrDependentFilter(PricingTrx& trx, const FareMarket& fareMarket);
  bool operator()(const FareByRuleCtrlInfo* r2);
  bool operator()(const FootNoteCtrlInfo* r2);
  bool operator()(const GeneralFareRuleInfo* r2);

private:
  PricingTrx& _trx;
  const FareMarket& _fareMarket;
};

template <class Rec2Type>
class CompoundFilter
{
public:
  typedef bool (*InhibitF)(const Rec2Type*);
  typedef typename std::vector<std::pair<const Rec2Type*, GeoMatchResult>> Result;

  CompoundFilter(PricingTrx& trx, const FareMarket& fareMarket, const DateTime& travelDate);
  Result matchR2(const std::vector<Rec2Type*>& r2vec);

  // filters used by diag202 directly, thus public
  IsNotEffectiveG<Rec2Type> _dateFilter;
  InhibitF _inhibitFilter;
  LocFilter _locFilter;
  CxrDependentFilter _cxrFilter;

private:
  // fbr r2 records do not need further matching
  bool stopAtFirstMatch(const FareByRuleCtrlInfo* r2) { return true; }
  // for FootNote / gfr it's just prevalidation
  bool stopAtFirstMatch(const FootNoteCtrlInfo* r2) { return false; }
  bool stopAtFirstMatch(const GeneralFareRuleInfo* r2) { return false; }
};
}
}
