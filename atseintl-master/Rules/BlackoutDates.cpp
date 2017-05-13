#include "Rules/BlackoutDates.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag311Collector.h"
#include "Rules/CommonPredicates.h"
#include "Rules/ConstPredicates.h"
#include "Rules/DatePredicates.h"
#include "Rules/RuleItem.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
Logger
BlackoutDates::_logger("atseintl.Rules.BlackoutDates");

struct BlackoutDates::GeoBools
{
  GeoBools() : orig(true), dest(true), fltStop(false) {}

  bool orig;
  bool dest;
  bool fltStop;
};

BlackoutDates::BlackoutDates()
  : _blackoutInfo(nullptr),
    _root(nullptr),
    _paxTypeFare(nullptr),
    _pricingUnit(nullptr),
    _fareUsage(nullptr),
    _trx(nullptr),
    _defaultScope(RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT),
    _travelSegs(nullptr),
    _itin(nullptr)
{
}

void
BlackoutDates::initialize(const PricingTrx& trx, const BlackoutInfo* blackout)
{
  _blackoutInfo = blackout;

  if (UNLIKELY(blackout->unavailtag() == UNAVAILABLE))
  {
    TextOnly* predicate;
    _dataHandle.get(predicate);
    predicate->initialize("UNAVAILABLE TAG - UNAVAILABLE", Diagnostic311, true);
    _root = predicate;
    return;
  }
  else if (UNLIKELY(blackout->unavailtag() == TEXT_ONLY))
  {
    TextOnly* predicate;
    // lint -e{530}
    _dataHandle.get(predicate);
    predicate->initialize("UNAVAILABLE TAG - TEXT ONLY", Diagnostic311, false);
    _root = predicate;
    return;
  }

  And* andP;
  _dataHandle.get(andP); // lint !e530
  Not* notP;
  _dataHandle.get(notP); // lint !e530

  if (blackout->blackoutAppl() == RANGE)
  {
    IsDateBetween* dateBetween = createDateBetween();
    dateBetween->initialize(blackout->tvlStartYear(),
                            blackout->tvlStartMonth(),
                            blackout->tvlStartDay(),
                            blackout->tvlStopYear(),
                            blackout->tvlStopMonth(),
                            blackout->tvlStopDay(),
                            RuleConst::BLACKOUTS_RULE);
    notP->initialize(dateBetween);
  }
  else
  {
    IsDate* date;
    _dataHandle.get(date); // lint !e530
    date->initialize(blackout->tvlStartYear(),
                     blackout->tvlStartMonth(),
                     blackout->tvlStartDay(),
                     blackout->tvlStopYear(),
                     blackout->tvlStopMonth(),
                     blackout->tvlStopDay(),
                     RuleConst::BLACKOUTS_RULE);
    notP->initialize(date);
  }

  andP->addComponent(notP);
  SameSegments* same;
  _dataHandle.get(same); // lint !e530
  same->initialize(andP);

  if (UNLIKELY(blackout->intlRest() == INT_DONT_APPLY))
  {
    // lint --e{530}
    ContainsInternational* intl;
    _dataHandle.get(intl);
    IfThenElse* ite;
    _dataHandle.get(ite);
    PassPredicate* pass;
    _dataHandle.get(pass);
    ite->initialize(intl, pass);
    ite->addElse(same);
    _root = ite;
  }
  else
    _root = same;
}

Record3ReturnTypes
BlackoutDates::process(PaxTypeFare& paxTypeFare, PricingTrx& trx)
{
  // LOG4CXX_INFO(_logger, " Entered BlackoutDates::validate(PaxTypeFare&)");
  _paxTypeFare = &paxTypeFare;
  _pricingUnit = nullptr;
  _fareUsage = nullptr;
  _trx = &trx;
  _travelSegs = &paxTypeFare.fareMarket()->travelSeg();
  _defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  Record3ReturnTypes retval = processCommon(trx, paxTypeFare, *_blackoutInfo, _defaultScope, false);
  // Invalidate the pointers
  _paxTypeFare = nullptr;
  _trx = nullptr;
  // LOG4CXX_INFO(_logger, " Leaving BlackoutDates::validate(PaxTypeFare&)");
  return retval;
}

Record3ReturnTypes
BlackoutDates::process(PaxTypeFare& paxTypeFare, PricingTrx& trx, bool isInbound)
{
  // LOG4CXX_INFO(_logger, " Entered BlackoutDates::validate(PaxTypeFare&)");
  _paxTypeFare = &paxTypeFare;
  _pricingUnit = nullptr;
  _fareUsage = nullptr;
  _trx = &trx;
  _travelSegs = &paxTypeFare.fareMarket()->travelSeg();
  _defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  Record3ReturnTypes retval = processCommon(trx, paxTypeFare, *_blackoutInfo, _defaultScope, false);
  // Invalidate the pointers
  _paxTypeFare = nullptr;
  _trx = nullptr;
  // LOG4CXX_INFO(_logger, " Leaving BlackoutDates::validate(PaxTypeFare&)");
  return retval;
}

Record3ReturnTypes
BlackoutDates::process(PricingTrx& trx,
                       const PricingUnit& pricingUnit,
                       const FareUsage& fareUsage,
                       const Itin& itin)
{
  // LOG4CXX_INFO(_logger, " Entered BlackoutDates::validate(PricingUnit&, ...)");
  _paxTypeFare = fareUsage.paxTypeFare();
  _pricingUnit = &pricingUnit;
  _fareUsage = &fareUsage;
  _itin = &itin;
  _trx = &trx;
  _travelSegs = &pricingUnit.travelSeg();
  _defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  Record3ReturnTypes retval =
      processCommon(trx, *(fareUsage.paxTypeFare()), *_blackoutInfo, _defaultScope, false);
  // Invalidate the pointers
  _paxTypeFare = nullptr;
  _pricingUnit = nullptr;
  _trx = nullptr;
  _fareUsage = nullptr;
  // LOG4CXX_INFO(_logger, " Leaving BlackoutDates::validate(PricingUnit&, ...)");
  return retval;
}

Record3ReturnTypes
BlackoutDates::processCommon(PricingTrx& trx,
                             const PaxTypeFare& paxTypeFare,
                             const BlackoutInfo& blackout,
                             RuleConst::TSIScopeParamType defaultScope,
                             bool isInbound)
{
  // lint !e578
  _trx = &trx;
  _paxTypeFare = &paxTypeFare;
  _defaultScope = defaultScope;
  _travelSegs = &paxTypeFare.fareMarket()->travelSeg();
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;
  // for NoPNR transaction, log cat11 warning message if fare PASSed
  bool isNoPNRTrx = (nullptr != dynamic_cast<NoPNRPricingTrx*>(&trx));

  if (UNLIKELY(isNoPNRTrx))
  {
    paxTypeFare.fare()->warningMap().set(WarningMap::cat11_warning); // add cat11 warning
  }

  if (UNLIKELY(isInbound && fdUtil.getFareDisplayTrx(&trx, fdTrx)))
  {
    _travelSegs = &fdTrx->inboundFareMarket()->travelSeg();
  }

  if (UNLIKELY(!_root))
  {
    /// @todo Probably not initialized. What is the assumtion?
    return PASS;
  }

  uint32_t geoBtw = _blackoutInfo->geoTblItemNoBetween();
  uint32_t geoAnd = _blackoutInfo->geoTblItemNoAnd();
  const bool any = (geoBtw != 0) || (geoAnd != 0);

  if (!any)
  {
    PredicateReturnType retc = _root->test(*_travelSegs, *_trx);

    if (UNLIKELY(isNoPNRTrx && !_pricingUnit && trx.itin().front()->dateType() == Itin::PartialDate))
    {
      // for NoPNR trx - if there is some segment without date in this fareMarket
      // (but there are some dates entered) - skip validation now,
      // we will have to revalidate at Pricing Unit level anyway
      for (const auto ts : *_travelSegs)
        if (ts->hasEmptyDate())
          return SOFTPASS;
    }

    // in this case cat11 returns FAIL or PASS (not SOFTPASS)
    return retc.valid;
  }

  // Geo TSI can override defaultScope, so we do not want to match and
  // validate on FU if it is suppose for PU scope. For example, origin of PU
  // is not same as origin of FU, we do not want to fail incorrectly
  RuleConst::TSIScopeType tsiScope;

  if (((geoBtw != 0) && hasTsiPuScope(tsiScope, trx, paxTypeFare.vendor(), geoBtw)) ||
      ((geoAnd != 0) && hasTsiPuScope(tsiScope, trx, paxTypeFare.vendor(), geoAnd)))
  {
    // as set in tseserver.cfg, we do not do cat11 Journey
    // scope validation now, so _farePath is always 0.
    // Thus we need to force Journey scope to PU scope
    _defaultScope = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;
  }

  if (_defaultScope == RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY && !_pricingUnit)
    return SOFTPASS;

  const bool both = (geoBtw != 0) && (geoAnd != 0);
  std::vector<TravelSeg*> tsVec;

  if (both && (geoBtw != geoAnd) &&
      (!RuleUtil::table995WhollyWithin(*_trx,
                                       _blackoutInfo->geoTblItemNoBetween(),
                                       _blackoutInfo->geoTblItemNoAnd(),
                                       _paxTypeFare->vendor())))
  {
    bool fltStopCheck = false;
    Diag311Collector* diag = nullptr;
    DCFactory* factory = DCFactory::instance();

    if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic311))
    {
      diag = dynamic_cast<Diag311Collector*>(factory->create(*_trx));
    }

    RuleUtil::getTvlSegBtwAndGeo(trx,
                                 _blackoutInfo->geoTblItemNoBetween(),
                                 _blackoutInfo->geoTblItemNoAnd(),
                                 paxTypeFare.vendor(),
                                 tsVec,
                                 nullptr,
                                 _defaultScope,
                                 nullptr,
                                 _pricingUnit,
                                 _paxTypeFare->fareMarket(),
                                 false,
                                 trx.ticketingDate(),
                                 fltStopCheck,
                                 diag);

    if (UNLIKELY(diag))
    {
      diag->flushMsg();
    }

    if (tsVec.empty())
      return SKIP;
  }
  else
  {
    RuleUtil::TravelSegWrapperVector appSegVec;
    GeoBools boolsBetween;
    TSICode tsiBtw = 0;
    bool retval = callGeo(_blackoutInfo->geoTblItemNoBetween(), appSegVec, boolsBetween, tsiBtw);

    if (!retval)
      return SKIP;

    // AA (9-27-2004): Get the origin and destination check flags from
    //                  the first item in the return vector.
    //                 Not sure if this is the right thing to do.
    if (LIKELY(!appSegVec.empty()))
    {
      const RuleUtil::TravelSegWrapper* tsw = appSegVec.front();
      boolsBetween.orig = tsw->origMatch();
      boolsBetween.dest = tsw->destMatch();
    }

    if (both)
    {
      // all points were not matched 995
      return SKIP;
    }

    // AA (9-27-2004): Copy the travel segs from the TravelSegWrapperVector
    //                  to a std::vector<TravelSeg*>.
    //
    tsVec.reserve(appSegVec.size());
    RuleUtil::TravelSegWrapperVectorI iter = appSegVec.begin();
    RuleUtil::TravelSegWrapperVectorI iterEnd = appSegVec.end();

    for (; iter != iterEnd; ++iter)
    {
      tsVec.push_back((*iter)->travelSeg());
    }
  }

  if (UNLIKELY(isNoPNRTrx && !_pricingUnit && trx.itin().front()->dateType() == Itin::PartialDate))
  {
    // for NoPNR trx - if there is some segment without date in this fareMarket
    // (but there are some dates entered) - skip validation now,
    // we will have to revalidate at Pricing Unit level anyway
    for (const auto ts : *_travelSegs)
      if (ts->hasEmptyDate())
        return SOFTPASS;
  }

  PredicateReturnType retc = _root->test(tsVec, *_trx);
  return retc.valid;
}

bool
BlackoutDates::callGeo(int geoTable,
                       RuleUtil::TravelSegWrapperVector& appSegVec,
                       GeoBools& geoBools,
                       TSICode& tsi) const
{
  // TSICode tsi;
  LocKey locKey1;
  LocKey locKey2;
  bool ret = RuleUtil::validateGeoRuleItem(geoTable,
                                           _paxTypeFare->vendor(),
                                           _defaultScope,
                                           // RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                           true,
                                           true,
                                           false,
                                           *_trx,
                                           nullptr,
                                           _itin,
                                           _pricingUnit,
                                           _paxTypeFare->fareMarket(),
                                           _trx->getRequest()->ticketingDT(),
                                           appSegVec,
                                           geoBools.orig,
                                           geoBools.dest,
                                           geoBools.fltStop,
                                           tsi,
                                           locKey1,
                                           locKey2,
                                           Diagnostic311);
  return ret;
}

IsDateBetween*
BlackoutDates::createDateBetween()
{
  IsDateBetween* dateBetween;
  _dataHandle.get(dateBetween);
  return dateBetween;
}

bool
BlackoutDates::hasTsiPuScope(RuleConst::TSIScopeType& geoScope,
                             PricingTrx& trx,
                             const VendorCode& vendor,
                             int itemNo) const
{
  return RuleUtil::getTSIScopeFromGeoRuleItem(itemNo, vendor, trx, geoScope) &&
         geoScope != RuleConst::TSI_SCOPE_SJ_AND_FC &&
         geoScope != RuleConst::TSI_SCOPE_FARE_COMPONENT;
}

Predicate*
BlackoutDates::getPredicate(const BlackoutInfo& rule)
{
  if (Predicate* p = getTextOnlyPredicate(rule))
    return p;

  if (Predicate* p = getContainsInternationalPredicate(rule))
    return p;

  return getSameSegmentsPredicate(rule);
}

Predicate*
BlackoutDates::getTextOnlyPredicate(const BlackoutInfo& rule)
{
  if ((rule.unavailtag() != UNAVAILABLE) && (rule.unavailtag() != TEXT_ONLY))
    return nullptr;

  TextOnly* p = _dataHandle.create<TextOnly>();
  std::string msg = rule.unavailtag() == UNAVAILABLE ? "UNAVAILABLE" : "TEXT ONLY";
  p->initialize("UNAVAILABLE TAG - " + msg, Diagnostic311, rule.unavailtag() == UNAVAILABLE);
  return p;
}

Predicate*
BlackoutDates::getDateBetweenPredicate(const BlackoutInfo& rule)
{
  return getDatePredicate<IsDateBetween>(rule);
}

Predicate*
BlackoutDates::getSameSegmentsPredicate(const BlackoutInfo& rule)
{
  Not* notP = _dataHandle.create<Not>();
  notP->initialize(rule.blackoutAppl() == RANGE ? getDateBetweenPredicate(rule)
                                                : getDatePredicate<IsDate>(rule));
  And* andP = _dataHandle.create<And>();
  andP->addComponent(notP);
  SameSegments* p = _dataHandle.create<SameSegments>();
  p->initialize(andP);
  return p;
}

Predicate*
BlackoutDates::getContainsInternationalPredicate(const BlackoutInfo& rule)
{
  if (rule.intlRest() != INT_DONT_APPLY)
    return nullptr;

  IfThenElse* p = _dataHandle.create<IfThenElse>();
  p->initialize(_dataHandle.create<ContainsInternational>(), _dataHandle.create<PassPredicate>());
  p->addElse(getSameSegmentsPredicate(rule));
  return p;
}

} // tse
