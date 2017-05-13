#include "Common/Rec2Filter.h"
#include "Common/LocUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleUtil.h"

namespace tse
{
namespace Rec2Filter
{
LocFilter::LocFilter(PricingTrx& trx, const FareMarket& fareMarket)
  : _fareMarket(fareMarket), _trx(trx)
{
}

bool
LocFilter::
operator()(const FareByRuleCtrlInfo* r2)
{
  return !RuleUtil::matchLocation(_trx,
                                  r2->loc1(),
                                  r2->loc1zoneTblItemNo(),
                                  r2->loc2(),
                                  r2->loc2zoneTblItemNo(),
                                  r2->vendorCode(),
                                  _fareMarket,
                                  _isLocationSwapped,
                                  r2->carrierCode());
}

bool
LocFilter::isInLoc(const Loc& loc, const LocKey& r2loc, const FootNoteCtrlInfo& r2) const
{
  return r2loc.locType() == RuleConst::ANY_LOCATION_TYPE ||
         LocUtil::isInLoc(loc,
                          r2loc.locType(),
                          r2loc.loc(),
                          r2.vendorCode(),
                          RESERVED,
                          LocUtil::OTHER,
                          _fareMarket.geoTravelType(),
                          r2.carrierCode(),
                          _trx.getRequest()->ticketingDT());
}

bool
LocFilter::
operator()(const FootNoteCtrlInfo* r2)
{
  if (isInLoc(*(_fareMarket.origin()), r2->loc1(), *r2) &&
      isInLoc(*(_fareMarket.destination()), r2->loc2(), *r2))
  {
    _isLocationSwapped = false;
    return false;
  }
  if (isInLoc(*(_fareMarket.origin()), r2->loc2(), *r2) &&
      isInLoc(*(_fareMarket.destination()), r2->loc1(), *r2))
  {
    _isLocationSwapped = true;
    return false;
  }

  // true means "do filter", false means "matched"
  return true;
}

bool
LocFilter::
operator()(const GeneralFareRuleInfo* r2)
{
  // true means "do filter", false means "matched"
  // try to fail geo, TBD
  return false;
}

CxrDependentFilter::CxrDependentFilter(PricingTrx& trx, const FareMarket& fareMarket)
  : _trx(trx), _fareMarket(fareMarket)
{
}

bool
CxrDependentFilter::
operator()(const FareByRuleCtrlInfo* r2)
{
  return false;
}

bool
CxrDependentFilter::
operator()(const FootNoteCtrlInfo* r2)
{
  return !RuleUtil::matchJointCarrier(
             _trx, _fareMarket, r2->vendorCode(), r2->carrierCode(), r2->jointCarrierTblItemNo());
}

bool
CxrDependentFilter::
operator()(const GeneralFareRuleInfo* r2)
{
  return !RuleUtil::matchJointCarrier(
             _trx, _fareMarket, r2->vendorCode(), r2->carrierCode(), r2->jointCarrierTblItemNo());
}

template <class Rec2Type>
CompoundFilter<Rec2Type>::CompoundFilter(PricingTrx& trx,
                                         const FareMarket& fareMarket,
                                         const DateTime& travelDate)
  : _dateFilter(travelDate, trx.dataHandle().ticketDate()),
    _inhibitFilter(trx.dataHandle().isFareDisplay() ? InhibitForFD<Rec2Type> : Inhibit<Rec2Type>),
    _locFilter(trx, fareMarket),
    _cxrFilter(trx, fareMarket)
{
}

template <class Rec2Type>
typename CompoundFilter<Rec2Type>::Result
CompoundFilter<Rec2Type>::matchR2(const std::vector<Rec2Type*>& r2vec)
{
  Result result;

  for (Rec2Type* r2 : r2vec)
  {
    _locFilter._isLocationSwapped = false;
    if (!_dateFilter(r2) && !_inhibitFilter(r2) && !_cxrFilter(r2) && !_locFilter(r2))
    {
      result.push_back(std::make_pair(r2, _locFilter._isLocationSwapped));
      if (stopAtFirstMatch(r2))
        break;
    }
  }

  return result;
}

template class CompoundFilter<FareByRuleCtrlInfo>;
template class CompoundFilter<FootNoteCtrlInfo>;
template class CompoundFilter<GeneralFareRuleInfo>;
}
}
