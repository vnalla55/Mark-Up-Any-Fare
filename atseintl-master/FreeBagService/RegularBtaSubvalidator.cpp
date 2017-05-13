//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "FreeBagService/RegularBtaSubvalidator.h"

#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/IS7RecordFieldsValidator.h"
#include "Util/Algorithm/Bitset.h"
#include "Util/IteratorRange.h"

#include <boost/container/flat_set.hpp>

namespace tse
{
namespace
{
using Validator = RegularBtaSubvalidator;
using CheckMask = RegularBtaSubvalidator::CheckMask;

inline CheckMask
createMask(const size_t from, const size_t to)
{
  return alg::create_bitset<Validator::NUM_CHECKS>(from, to);
}

inline CheckMask
createMask(const size_t bit)
{
  return CheckMask(1u << bit);
}

inline const PaxTypeFare*
mapSegToFare(const Ts2ss& ts2ss, TravelSeg* ts)
{
  const auto tsI = ts2ss.find(ts);
  return (tsI != ts2ss.end()) ? tsI->second.second : nullptr;
}

inline RegularBtaSubvalidator::SegAndFare
createSegAndFare(const Ts2ss& ts2ss, TravelSeg* ts)
{
  return {ts, mapSegToFare(ts2ss, ts)};
}

inline bool
isCarryOn(const OCFees& oc)
{
  return oc.subCodeInfo()->fltTktMerchInd() == CARRY_ON_ALLOWANCE &&
         oc.subCodeInfo()->serviceSubTypeCode().equalToConst("0LN");
}

inline bool
isEmbargo(const OCFees& oc)
{
  return oc.subCodeInfo()->fltTktMerchInd() == BAGGAGE_EMBARGO && oc.subCodeInfo()->concur() == 'X';
}

inline bool
isCarryOn(const OptionalServicesInfo& s7)
{
  return s7.fltTktMerchInd() == CARRY_ON_ALLOWANCE && s7.serviceSubTypeCode().equalToConst("0LN");
}

inline bool
isEmbargo(const OptionalServicesInfo& s7)
{
  return s7.fltTktMerchInd() == BAGGAGE_EMBARGO;
}

inline StatusS7Validation
conditionalResult(bool passCondition, bool failCondition, StatusS7Validation result)
{
  if (!passCondition && result == PASS_S7)
    return SOFT_PASS_S7;
  if (!failCondition && result != PASS_S7)
    return SOFT_PASS_S7;
  return result;
}

inline StatusS7Validation
conditionalPass(bool condition, StatusS7Validation result)
{
  return conditionalResult(condition, true, result);
}

} // anonymous ns

const CheckMask Validator::ALL_CHECKS = createMask(0, Validator::NUM_CHECKS);
const CheckMask Validator::NON_FARE_CHECKS = createMask(0, Validator::FARE_CLASS);
const CheckMask Validator::FARE_CHECKS = createMask(Validator::FARE_CLASS, Validator::NUM_CHECKS);
// What restrictions should be softpassed in prevalidation steps and need to be revalidated
const CheckMask Validator::REVALIDATION_CHECKS =
    Validator::ALL_CHECKS & ~(createMask(Validator::T186));
// What restrictions can't be validated in fare component-level revalidation step
const CheckMask Validator::FC_SKIPPED_CHECKS =
    createMask(Validator::CABIN) | createMask(Validator::RBD);

void
RegularBtaSubvalidator::onFaresUpdated()
{
  _isFareInfoInitialized = false;
  _mostSignificantSeg = {nullptr, nullptr};
  _specialSegForT186 = {nullptr, nullptr};
  _bagTravelSegs.clear();
  _bagTravelFares.clear();
  _journeySegs.clear();
}

StatusS7Validation
RegularBtaSubvalidator::validate(const OptionalServicesInfo& s7, OCFees& ocFees)
{
  Context ctx(s7, ocFees, _bt._trx->dataHandle());
  return validateImpl(ALL_CHECKS, ctx);
}

// Revalidate items that had been conditionally passed by SoftPassBtaSubvalidator.
// Note that it should work correctly even if not all required fares are known
// and return SOFT_PASS_S7 in such a case.

StatusS7Validation
RegularBtaSubvalidator::revalidate(const OptionalServicesInfo& s7,
                                   OCFees& ocFees,
                                   const bool fcLevel)
{
  Context ctx(s7, ocFees, _bt._trx->dataHandle());

  if (!_isFareInfoInitialized)
    prepareFareInfoForRevalidation();

  const Indicator bta = ctx.s7().baggageTravelApplication();
  CheckMask mask = adjustMask(ALL_CHECKS, ctx);
  bool mssValidationOk = true;
  bool skippedChecks = false;

  if (fcLevel && (mask & FC_SKIPPED_CHECKS).any())
  {
    skippedChecks = true;
    mask &= ~FC_SKIPPED_CHECKS;
  }

  switch (bta)
  {
  case BTA_A:
    return conditionalPass(_allFaresOnBtKnown && !skippedChecks,
                           validateA(mask & REVALIDATION_CHECKS, ctx));
  case BTA_S:
    // BTA=S <=> "At least one segment on the baggage travel meets the restrictions"
    // It doesn't have to be the same that was passed at prevalidation level so we can't
    // take advantage of that and have to relivate all the restrictions.
    return conditionalResult(!skippedChecks, _allFaresOnBtKnown, validateS(mask, ctx));
  case BTA_J:
    return conditionalPass(_allFaresOnJnyKnown && !skippedChecks,
                           validateJ(mask & REVALIDATION_CHECKS, ctx));
  case BTA_M:
    if (!_mostSignificantSeg.second)
      return SOFT_PASS_S7;
    return conditionalPass(!skippedChecks, validateM(mask & REVALIDATION_CHECKS, ctx));
  default:
    if (mask[CABIN] && !_mostSignificantSeg.second)
    {
      mask.reset(CABIN);
      mssValidationOk = false;
    }
    return conditionalPass(_allFaresOnBtKnown && mssValidationOk && !skippedChecks,
                           validateEmpty(mask & REVALIDATION_CHECKS, ctx));
  }
}

void
RegularBtaSubvalidator::prepareFareInfo()
{
  // TODO: I promise I'll remove the const_cast below...
  _mostSignificantSeg = createSegAndFare(_ts2ss, *_bt._MSS);
  _specialSegForT186 = createSegAndFare(
      _ts2ss, const_cast<TravelSeg*>(_s7Validator.determineSpecialSegmentForT186(_bt)));

  for (TravelSeg* ts : makeIteratorRange(_bt.getTravelSegBegin(), _bt.getTravelSegEnd()))
    if (ts->isAir())
      _bagTravelSegs.emplace_back(createSegAndFare(_ts2ss, ts));

  prepareBagTravelFares();

  for (TravelSeg* ts : _bt.itin()->travelSeg())
    if (ts->isAir())
      _journeySegs.emplace_back(createSegAndFare(_ts2ss, ts));

  _isFareInfoInitialized = true;
}

void
RegularBtaSubvalidator::prepareFareInfoForRevalidation()
{
  _mostSignificantSeg = createSegAndFare(_ts2ss, *_bt._MSS);
  _allFaresOnBtKnown = true;
  _allFaresOnJnyKnown = true;

  for (TravelSeg* ts : makeIteratorRange(_bt.getTravelSegBegin(), _bt.getTravelSegEnd()))
  {
    if (!ts->isAir())
      continue;
    const PaxTypeFare* const ptf = mapSegToFare(_ts2ss, ts);
    if (ptf)
      _bagTravelSegs.emplace_back(ts, ptf);
    else
      _allFaresOnBtKnown = false;
  }

  prepareBagTravelFares();

  for (TravelSeg* ts : _bt.itin()->travelSeg())
  {
    if (!ts->isAir())
      continue;
    const PaxTypeFare* const ptf = mapSegToFare(_ts2ss, ts);
    if (ptf)
      _journeySegs.emplace_back(ts, ptf);
    else
      _allFaresOnJnyKnown = false;
  }

  _isFareInfoInitialized = true;
}

void
RegularBtaSubvalidator::prepareBagTravelFares()
{
  boost::container::flat_set<const PaxTypeFare*> processedFares;
  processedFares.reserve(_bagTravelSegs.size());

  for (const SegAndFare& segAndFare : _bagTravelSegs)
  {
    if (!processedFares.insert(segAndFare.second).second)
      continue;
    _bagTravelFares.emplace_back(segAndFare);
  }
}

CheckMask
RegularBtaSubvalidator::adjustMask(CheckMask mask, const Context& ctx) const
{
  if (ctx.s7().cabin() == ' ')
    mask.reset(CABIN);

  if (!ctx.s7().serviceFeesResBkgDesigTblItemNo())
    mask.reset(RBD);

  if (!ctx.s7().carrierFltTblItemNo())
    mask.reset(T186);

  if (!ctx.s7().serviceFeesCxrResultingFclTblItemNo())
    mask.reset(FARE_CLASS);

  if (!ctx.s7().resultServiceFeesTktDesigTblItemNo())
    mask.reset(TKT_DESIG);

  if (ctx.s7().ruleTariff() == uint16_t(-1))
    mask.reset(TARIFF);

  if (ctx.s7().rule().empty())
    mask.reset(RULE);

  if (ctx.s7().fareInd() == ' ')
    mask.reset(FARE_IND);

  return mask;
}

StatusS7Validation
RegularBtaSubvalidator::validateImpl(CheckMask mask, Context& ctx)
{
  mask = adjustMask(mask, ctx);

  if (mask.none())
    return PASS_S7;

  if (!_isFareInfoInitialized)
    prepareFareInfo();

  const bool isCarryOnOrEmbargo = isCarryOn(ctx.s7()) || isEmbargo(ctx.s7());
  const Indicator bta = (isCarryOnOrEmbargo) ? BTA_EMPTY : ctx.s7().baggageTravelApplication();

  switch (bta)
  {
  case BTA_A:
    return validateA(mask, ctx);
  case BTA_S:
    return validateS(mask, ctx);
  case BTA_J:
    return validateJ(mask, ctx);
  case BTA_M:
    return validateM(mask, ctx);
  default:
    return validateEmpty(mask, ctx);
  }
}

StatusS7Validation
RegularBtaSubvalidator::validateA(CheckMask mask, Context& ctx) const
{
  const CheckMask fareMask = mask & FARE_CHECKS;
  const CheckMask nonFareMask = mask & NON_FARE_CHECKS;

  // It doesn't make sense to validate fare-related fields against all segments
  // on the fare market so we split the validation accordingly.
  if (fareMask.any())
  {
    for (const SegAndFare& seg : _bagTravelFares)
    {
      const StatusS7Validation res = validateSegment(seg, fareMask, ctx);
      if (res != PASS_S7)
        return res;
    }
  }

  if (nonFareMask.any())
  {
    for (const SegAndFare& seg : _bagTravelSegs)
    {
      const StatusS7Validation res = validateSegment(seg, nonFareMask, ctx);
      if (res != PASS_S7)
        return res;
    }
  }

  return PASS_S7;
}

StatusS7Validation
RegularBtaSubvalidator::validateS(CheckMask mask, Context& ctx) const
{
  StatusS7Validation res = PASS_S7;

  for (const SegAndFare& seg : _bagTravelSegs)
  {
    res = validateSegment(seg, mask, ctx);
    if (res == PASS_S7)
      return PASS_S7;
  }

  return res;
}

StatusS7Validation
RegularBtaSubvalidator::validateJ(CheckMask mask, Context& ctx) const
{
  StatusS7Validation res = PASS_S7;

  for (const SegAndFare& seg : _journeySegs)
  {
    res = validateSegment(seg, mask, ctx);
    if (res == PASS_S7)
      return PASS_S7;
  }

  return res;
}

StatusS7Validation
RegularBtaSubvalidator::validateM(CheckMask mask, Context& ctx) const
{
  return validateSegment(_mostSignificantSeg, mask, ctx);
}

StatusS7Validation
RegularBtaSubvalidator::validateEmpty(CheckMask mask, Context& ctx) const
{
  if (mask[CABIN])
  {
    const StatusS7Validation res = validateSegment(_mostSignificantSeg, createMask(CABIN), ctx);
    if (res != PASS_S7)
      return res;

    mask.reset(CABIN);
  }

  if (AllowanceUtil::isDefer(ctx.s7()) && mask[T186])
  {
    if (!_specialSegForT186.first)
      return FAIL_S7_CXR_FLT_T186;
    const StatusS7Validation res = validateSegment(_specialSegForT186, createMask(T186), ctx);
    if (res != PASS_S7)
      return res;

    mask.reset(T186);
  }

  return validateA(mask, ctx);
}

StatusS7Validation
RegularBtaSubvalidator::validateSegment(const SegAndFare& seg, const CheckMask mask, Context& ctx)
    const
{
  _validationObserver.notifySegmentValidationStarted(*seg.first);

  const auto res = validateSegmentImpl(seg, mask, ctx);

  return _validationObserver.notifySegmentValidationFinished(res, *seg.first);
}

StatusS7Validation
RegularBtaSubvalidator::validateSegmentImpl(const SegAndFare& seg,
                                            const CheckMask mask,
                                            Context& ctx) const
{
  if (mask[CABIN] && !checkCabin(seg, ctx.s7().cabin()))
    return FAIL_S7_CABIN;

  if (mask[RBD] && !checkRbd(seg, ctx))
    return FAIL_S7_RBD_T198;

  if (mask[T186] && !checkT186(seg, ctx))
    return FAIL_S7_CXR_FLT_T186;

  if (mask[FARE_CLASS] && !checkFareClass(seg, ctx))
    return FAIL_S7_RESULT_FC_T171;

  if (mask[TKT_DESIG] && !checkTktDesig(seg, ctx))
    return FAIL_S7_OUTPUT_TKT_DESIGNATOR;

  if (mask[TARIFF] && !checkRuleTariff(seg, ctx))
    return FAIL_S7_RULE_TARIFF;

  if (mask[RULE] && !checkRule(seg, ctx))
    return FAIL_S7_RULE;

  if (mask[FARE_IND] && !checkFareInd(seg, ctx.s7().fareInd()))
    return FAIL_S7_FARE_IND;

  return PASS_S7;
}

bool
RegularBtaSubvalidator::checkCabin(const SegAndFare& seg, const Indicator s7Cabin) const
{
  const bool res = _s7Validator.checkCabinInSegment(seg.first, s7Cabin);
  return _validationObserver.notifyCabinValidationFinished(res);
}

bool
RegularBtaSubvalidator::checkRbd(const SegAndFare& seg, Context& ctx) const
{
  const bool res = _s7Validator.checkRBDInSegment(
      seg.first, ctx.ocFees(), ctx.s7().serviceFeesResBkgDesigTblItemNo(), ctx.getRbdInfos());
  return _validationObserver.notifyRBDValidationFinished(res);
}

bool
RegularBtaSubvalidator::checkT186(const SegAndFare& seg, Context& ctx) const
{
  const bool res = _s7Validator.checkCarrierFlightApplT186InSegment(
      seg.first, ctx.s7().vendor(), ctx.s7().carrierFltTblItemNo());
  return _validationObserver.notifyCarrierFlightApplT186Finished(res);
}

bool
RegularBtaSubvalidator::checkFareClass(const SegAndFare& seg, Context& ctx) const
{
  const bool res = _s7Validator.checkResultingFareClassInSegment(
      seg.second, ctx.s7().serviceFeesCxrResultingFclTblItemNo(), ctx.ocFees(), ctx.getFareClassInfos());
  return _validationObserver.notifyTable171ValidationFinished(res);
}

bool
RegularBtaSubvalidator::checkTktDesig(const SegAndFare& seg, Context& ctx) const
{
  const bool res = _s7Validator.checkOutputTicketDesignatorInSegment(seg.first, seg.second, ctx.s7());
  return _validationObserver.notifyOutputTicketDesignatorValidationFinished(res);
}

bool
RegularBtaSubvalidator::checkRuleTariff(const SegAndFare& seg, Context& ctx) const
{
  const bool res =
      _s7Validator.checkRuleTariffInSegment(seg.second, ctx.s7().ruleTariff(), ctx.ocFees());
  return _validationObserver.notifyRuleTariffValidationFinished(res);
}

bool
RegularBtaSubvalidator::checkRule(const SegAndFare& seg, Context& ctx) const
{
  const bool res = _s7Validator.checkRuleInSegment(seg.second, ctx.s7().rule(), ctx.ocFees());
  return _validationObserver.notifyRuleValidationFinished(res);
}

bool
RegularBtaSubvalidator::checkFareInd(const SegAndFare& seg, const Indicator fareInd) const
{
  const bool res = _s7Validator.checkFareIndInSegment(seg.second, fareInd);
  return _validationObserver.notifyFareIndValidationFinished(res);
}

const std::vector<SvcFeesResBkgDesigInfo*>&
RegularBtaSubvalidator::Context::getRbdInfos()
{
  if (!_rbdInfos)
    _rbdInfos = &_dh.getSvcFeesResBkgDesig(_s7.vendor(),
                                           _s7.serviceFeesResBkgDesigTblItemNo());
  return *_rbdInfos;
}

const std::vector<SvcFeesCxrResultingFCLInfo*>&
RegularBtaSubvalidator::Context::getFareClassInfos()
{
  if (!_fareClassInfos)
    _fareClassInfos = &_dh.getSvcFeesCxrResultingFCL(_s7.vendor(),
                                                     _s7.serviceFeesCxrResultingFclTblItemNo());
  return *_fareClassInfos;
}

}
