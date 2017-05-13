//-------------------------------------------------------------------
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

#include "RexPricing/ReissuePenaltyCalculator.h"

#include "Common/CurrencyUtil.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TSEException.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/Diag689Collector.h"
#include "Diagnostic/DiagManager.h"
#include "RexPricing/FarePathChangeDetermination.h"
#include "Rules/OriginallyScheduledFlightValidator.h"
#include "Rules/RuleConst.h"
#include "Rules/VoluntaryChanges.h"

namespace tse
{
FALLBACK_DECL(ssdsp1720Cat31CurrencyFix)

namespace
{
inline bool
isNoShow(const PricingTrx* trx, const VoluntaryChangesInfoW* r3)
{
  const std::vector<ReissueSequence*>& t988Seqs =
      trx->dataHandle().getReissue(r3->vendor(),
                                   r3->reissueTblItemNo(),
                                   trx->dataHandle().ticketDate(),
                                   trx->dataHandle().ticketDate());

  auto isUnusedFlight = [](const ReissueSequence* seq)
  {
    return seq->unusedFlightInd() == OriginallyScheduledFlightValidator::ANYTIME_AFTER ||
           seq->unusedFlightInd() == OriginallyScheduledFlightValidator::DAY_AFTER;
  };

  return !t988Seqs.empty() && std::all_of(t988Seqs.cbegin(), t988Seqs.cend(), isUnusedFlight);
}

class CurrentTickeetingDateSetter
{
public:
  CurrentTickeetingDateSetter(PricingTrx& trx) : _trx(trx)
  {
    if (trx.isRexBaseTrx())
    {
      RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(_trx);
      _savedDate = rexTrx.fareApplicationDT();
      rexTrx.setFareApplicationDT(rexTrx.currentTicketingDT());
    }
  }

  ~CurrentTickeetingDateSetter()
  {
    if (_trx.isRexBaseTrx())
    {
      static_cast<RexPricingTrx&>(_trx).setFareApplicationDT(_savedDate);
    }
  }
private:
  PricingTrx& _trx;
  DateTime _savedDate;
};

} // namespace

Logger
ReissuePenaltyCalculator::_logger("atseintl.RexPricing.ReissuePenaltyCalculator");

const std::vector<PaxTypeCode> ReissuePenaltyCalculator::YOUTH_CODES = {
    "GIY", "GYT", "ITY", "YSB", "YTH", "YTR", "ZNN", "ZYC", "ZZS"};
const std::vector<PaxTypeCode> ReissuePenaltyCalculator::SENIOR_CODES = {
    "JRC", "SCC", "SCF", "SCM", "SNN", "SRC", "SRR", "TS1", "YCB", "ZSF", "ZSM"};

const Indicator ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC;
const Indicator ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC;
const Indicator ReissuePenaltyCalculator::EACH_OF_CHANGED_FC;
const Indicator ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU;
const Indicator ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU_ADDS;

void
ReissuePenaltyCalculator::initialize(PricingTrx& trx,
                                     const CurrencyCode& calculationCurrency,
                                     const FarePathChangeDetermination& farePathChangeDetermination,
                                     const CarrierCode& validatingCarrier,
                                     const PaxTypeInfo& paxTypeInfo,
                                     ProcessTagPermutation* permutation,
                                     Diag689Collector* dc)
{
  _trx = &trx;
  _calculationCurrency = &calculationCurrency;
  _farePathChangeDetermination = &farePathChangeDetermination;
  _validatingCarrier = &validatingCarrier;
  _paxTypeInfo = &paxTypeInfo;
  _permutation = permutation;
  _dc = dc;

  if (_trx->getOptions()->currencyOverride().empty())
    _equivalentCurrency = _trx->getRequest()->ticketingAgent()->currencyCodeAgent();
  else
    _equivalentCurrency = _trx->getOptions()->currencyOverride();

  _trx->dataHandle().get(_reissueCharges);
}

ReissueCharges*
ReissuePenaltyCalculator::process()
{
  CurrentTickeetingDateSetter dateSetter(*_trx);
  determineApplicationScenario();
  setUpInfantPassangerTypes();
  calculatePenaltyFees();
  insertCurrenciesForOldProcess();

  accumulateCharges();
  adjustChangeFeeByMinAmount();

  print();
  return _reissueCharges;
}

ReissuePenaltyCalculator::FcFees
ReissuePenaltyCalculator::getPenalties(
    const std::unordered_set<const VoluntaryChangesInfoW*>& records,
    const PaxTypeFare& ptf,
    smp::RecordApplication departureInd,
    DiagManager& diag)
{
  FcFees calcs;
  _conversionType = ConversionType::NO_ROUNDING;

  bool diagEnabled = _trx->diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "MAXPEN");

  if (diagEnabled)
  {
    diag << "   " << ptf.createFareBasis(_trx) << " - CAT31\n";
  }

  for (const VoluntaryChangesInfoW* r3 : records)
  {
    if (diagEnabled)
      smp::printRecord3(*_trx, *r3, departureInd, diag);

    if (!isMatchingRecord(r3, departureInd) || !smp::isPsgMatch(*ptf.paxType(), *r3) || isNoShow(_trx, r3))
    {
      diag << "    RECORD " << r3->itemNo() << " FAILED\n";
      continue;
    }

    diag << "    RECORD " << r3->itemNo() << " PASSED\n";

    ProcessTagPermutation perm;

    ProcessTagInfo pti;
    pti.record3()->orig() = r3->orig();
    pti.record3()->overriding() = r3->overriding();
    pti.record3()->setConditionallyOverridenBytes(VoluntaryChangesInfoW::coByte47_75,
                                                  r3->isAmountOverridden());
    pti.paxTypeFare() = &ptf;
    perm.processTags().push_back(&pti);

    _permutation = &perm;

    ReissueCharges* charges = process();

    calcs.emplace_back(charges->changeFeeInCalculationCurrency, _applScenario, r3);

    clearReissueCharges();
  }

  return calcs;
}

bool
ReissuePenaltyCalculator::isMatchingRecord(const VoluntaryChangesInfoW* record3,
                                           smp::RecordApplication departureInd)
{
  smp::RecordApplication application = smp::getRecordApplication(*record3, departureInd, false, false);
  return smp::isDepartureMatching(application, departureInd);
}

void
ReissuePenaltyCalculator::clearReissueCharges()
{
  _reissueCharges->changeFee = 0;
  _reissueCharges->changeFeeCurrency = "";
  _reissueCharges->changeFeeInCalculationCurrency = 0;
  _reissueCharges->changeFeeInEquivCurrency = 0;
  _reissueCharges->minAmtApplied = false;
  _reissueCharges->penaltyFees.clear();
  if (!fallback::ssdsp1720Cat31CurrencyFix(_trx))
    _chargeCurrency.clear();
}

void
ReissuePenaltyCalculator::setUpInfantPassangerTypes()
{
  if (_paxTypeInfo)
  {
    if (_paxTypeInfo->isInfant() && _paxTypeInfo->numberSeatsReq() > 0)
      _infantWithSeat = true;

    if (_paxTypeInfo->infantInd() == YES && !_paxTypeInfo->numberSeatsReq())
      _infantWithoutSeat = true;
  }
}

void
ReissuePenaltyCalculator::calculatePenaltyFees()
{
  for (auto processTag : _permutation->processTags())
  {
    if (considerFareComponent(*processTag->paxTypeFare()))
    {
      if (!isInTrxBasedPenaltyFeeCache(*processTag))
        calculateDiscountedPenaltyFee(*processTag);
    }

    else
      insertIntoPrintingStructure(*processTag, nullptr);
  }
}

bool
ReissuePenaltyCalculator::considerFareComponent(const PaxTypeFare& fareComponent) const
{
  switch (_applScenario)
  {
  case HIGHEST_OF_CHANGED_FC:
    fareComponent.track("RPC_HFC");
    return _farePathChangeDetermination->changedFC(fareComponent);
  case HIGHEST_OF_ALL_FC:
    fareComponent.track("RPC_HFCA");
    return true;
  case EACH_OF_CHANGED_FC:
    fareComponent.track("RPC_CFC");
    return _farePathChangeDetermination->changedFC(fareComponent);
  case HIGHEST_FROM_CHANGED_PU:
    fareComponent.track("RPC_HPU");
    return _farePathChangeDetermination->insideChangedPU(fareComponent);
  case HIGHEST_FROM_CHANGED_PU_ADDS:
    fareComponent.track("RPC_HPUA");
    return _farePathChangeDetermination->insideChangedPU(fareComponent) ||
           _farePathChangeDetermination->sameInsideExtendedPU(fareComponent);
  }

  return false;
}

void
ReissuePenaltyCalculator::insertFee(const ProcessTagInfo& pti, const PenaltyFee& fee)
{
  if (fee.penaltyAmount > EPSILON)
  {
    PenaltyFee& ncFee = const_cast<PenaltyFee&>(fee);

    if (_applScenario == EACH_OF_CHANGED_FC)
      _reissueCharges->penaltyFees.insert(std::make_pair(pti.paxTypeFare(), &ncFee));

    else if (_reissueCharges->penaltyFees.empty() ||
             isPenalty1HigherThanPenalty2(ncFee, *_reissueCharges->penaltyFees.begin()->second))
    {
      // this one from lowest permutation will be stored in fare path and sent in xml
      _reissueCharges->penaltyFees.clear();
      _reissueCharges->penaltyFees.insert(std::make_pair(pti.paxTypeFare(), &ncFee));
    }
  }

  insertIntoPrintingStructure(pti, &fee);

  // needed for min amt calculation in old process
  _fareVoluntaryChangeMap.insert(std::make_pair(pti.paxTypeFare(), pti.record3()));
}

void
ReissuePenaltyCalculator::calculateDiscountedPenaltyFee(const ProcessTagInfo& pti)
{
  PenaltyFee* fee = _trx->dataHandle().create<PenaltyFee>();

  fee->applicableDiscount = calculateDiscount(pti);
  calculatePenaltyFee(*pti.paxTypeFare(), *pti.record3(), *fee);

  if (_trx->excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    // this is only a cache on trx scope for calculator class
    static_cast<RexPricingTrx*>(_trx)->penaltyFees().insert(std::make_pair(
        std::make_pair(pti.paxTypeFare(), pti.record3()->overridingWhenExists()), fee));
  }

  insertFee(pti, *fee);
}

void
ReissuePenaltyCalculator::insertCurrenciesForOldProcess()
{
  for (auto penaltyFee : _reissueCharges->penaltyFees)
  {
    if (!fallback::ssdsp1720Cat31CurrencyFix(_trx) ? penaltyFee.second->penaltyCurrency != ""
                                                   : penaltyFee.second->penaltyAmount > EPSILON)
    {
      _chargeCurrency.insert(penaltyFee.second->penaltyCurrency);
    }
  }

  if (_chargeCurrency.empty())
  {
    _chargeCurrency.insert(_equivalentCurrency);
  }
}

bool
ReissuePenaltyCalculator::isInTrxBasedPenaltyFeeCache(const ProcessTagInfo& pti)
{
  if (_trx->excTrxType() != PricingTrx::AR_EXC_TRX)
    return false;

  const PenaltyFee* fee = static_cast<RexPricingTrx*>(_trx)->getPenaltyFee(
      pti.paxTypeFare(), pti.record3()->overridingWhenExists());

  if (!fee)
    return false;

  insertFee(pti, *fee);
  return true;
}

void
ReissuePenaltyCalculator::insertIntoPrintingStructure(const ProcessTagInfo& pti,
                                                      const PenaltyFee* fee)
{
  if (_dc && _dc->filterPassed())
    _printInfos.push_back(
        PrintInfo{&pti,
                  fee,
                  getApplicationScenarioForComponent(*pti.record3()->overridingWhenExists()),
                  false});
}

namespace
{
class RuntimeApplScenarioComp
{
  const char* _tab;
  static const char* _normal;
  static const char* _carrier;

public:
  explicit RuntimeApplScenarioComp(bool carrier) : _tab(carrier ? _carrier : _normal) {}

  bool operator()(const Indicator a, const Indicator b) { return _tab[a - 49] > _tab[b - 49]; }
};

const char* RuntimeApplScenarioComp::_normal = "14023";
const char* RuntimeApplScenarioComp::_carrier = "03412";
}

using ApplicationScenarioSet = std::set<Indicator, RuntimeApplScenarioComp>;

void
ReissuePenaltyCalculator::determineApplicationScenario()
{
  bool carrierOwnsFC = validatingCarrierOwnFareComponent(*_validatingCarrier);
  RuntimeApplScenarioComp valueCompare(carrierOwnsFC);
  ApplicationScenarioSet applicationScenarios(valueCompare);
  auto cxrCompare = [&](const ProcessTagInfo* pti)
  {
      return (pti->paxTypeFare()->carrier() == INDUSTRY_CARRIER)
                 ? pti->fareMarket()->governingCarrier() == *_validatingCarrier
                 : pti->paxTypeFare()->carrier() == *_validatingCarrier;
  };

  for (auto processTag : _permutation->processTags())
  {
    if (!carrierOwnsFC || cxrCompare(processTag))
      applicationScenarios.insert(
          getApplicationScenarioForComponent(*processTag->record3()->overridingWhenExists()));
  }

  _applScenario = *applicationScenarios.begin();
}

Indicator
ReissuePenaltyCalculator::getApplicationScenarioForComponent(const VoluntaryChangesInfo& rec3)
{
  if (rec3.journeyInd() != BLANK || rec3.feeAppl() == BLANK)
    return adaptOldApplicationScenario(rec3);

  return rec3.feeAppl();
}

Indicator
ReissuePenaltyCalculator::adaptOldApplicationScenario(const VoluntaryChangesInfo& rec3)
{
  switch (rec3.journeyInd())
  {
  case PU_APPL:
    return HIGHEST_FROM_CHANGED_PU;

  case JOURNEY_APPL:
    return rec3.feeAppl() == HIGHEST_OF_CHANGED_FC ? HIGHEST_OF_CHANGED_FC : HIGHEST_OF_ALL_FC;
  }

  return EACH_OF_CHANGED_FC;
}

bool
ReissuePenaltyCalculator::validatingCarrierOwnFareComponent(const CarrierCode& validatingCxr)
{
  auto FCCarrier = [&](const ProcessTagInfo* pti)
  {
      return (pti->paxTypeFare()->carrier() == INDUSTRY_CARRIER)
                 ? pti->fareMarket()->governingCarrier() == validatingCxr
                 : pti->paxTypeFare()->carrier() == validatingCxr;
  };

  return std::find_if(_permutation->processTags().begin(),
                      _permutation->processTags().end(),
                      FCCarrier) != _permutation->processTags().end();
}

void
ReissuePenaltyCalculator::print()
{
  if (_dc && _dc->filterPassed())
  {
    markChargedPenalties();

    for (auto& processInfo : _printInfos)
    {
      _dc->print(*processInfo.pti,
                 processInfo.applicationScenario,
                 _farePathChangeDetermination->applicationStatus(*processInfo.pti->paxTypeFare()));
      _dc->print(processInfo.fee, processInfo.charged);
    }

    _dc->printTotalChangeFee(
        Money(_reissueCharges->changeFeeInCalculationCurrency, *_calculationCurrency),
        _applScenario);
  }
}

void
ReissuePenaltyCalculator::markChargedPenalties()
{
  for (auto& processInfo : _printInfos)
  {
    if (_reissueCharges->penaltyFees.find(processInfo.pti->paxTypeFare()) !=
        _reissueCharges->penaltyFees.end())
      processInfo.charged = true;
  }
}

Percent
ReissuePenaltyCalculator::getChildrenDiscount(Indicator type,
                                              const PaxTypeInfo& pti,
                                              const PaxTypeCode& paxCode,
                                              const DiscountInfo& di) const
{
  switch (type)
  {
  case INFANT_WITH_SEAT_CAT19:
  {
    if (_infantWithSeat)
      return 100.0 - di.discPercent();
    if (_infantWithoutSeat)
      return 100.0;
    break;
  }

  case INFANT:
    if (pti.isInfant())
      return 100.0 - di.discPercent();
    break;

  case CHILD:
    if (pti.isChild())
      return 100.0 - di.discPercent();
    break;

  case OTHER_19:
    if (di.paxType() == paxCode)
      return 100.0 - di.discPercent();
    break;

  case INFANT_WITH_SEAT:
    if (_infantWithSeat)
      return 100.0 - di.discPercent();
    break;

  case INFANT_WITHOUT_SEAT:
    if (_infantWithoutSeat)
      return 100.0 - di.discPercent();
    break;

  case INFANT_WITHOUT_SEAT_NOFEE:
    if (_infantWithoutSeat)
      return 100.0;
  }

  return 0.0;
}

Percent
ReissuePenaltyCalculator::calculateDiscount(const ProcessTagInfo& tag)
{
  if (!_paxTypeInfo)
    return 0.0;

  std::vector<Indicator> discountTags = {tag.record3()->discountTag1(),
                                         tag.record3()->discountTag2(),
                                         tag.record3()->discountTag3(),
                                         tag.record3()->discountTag4()};
  std::set<Percent> discounts;
  discounts.insert(0.0);
  for (Indicator discountTag : discountTags)
    if (discountTag != NO_APPLY)
      discounts.insert(getDiscountApplicable(discountTag, *tag.paxTypeFare()));

  return *discounts.rbegin();
}

//--- moved from old ReissueChargeCalculator when removed RexMandatesActivated ---

MoneyAmount
ReissuePenaltyCalculator::getDiscountApplicable(Indicator type, const PaxTypeFare& paxTypeFare)
{
  if (!paxTypeFare.isDiscounted())
    return 0.0;

  const PaxTypeCode& paxCode = _paxTypeInfo->paxType();
  const DiscountInfo* di;

  try
  {
    di = &paxTypeFare.discountInfo();
  }
  catch (TSEException& exc)
  {
    LOG4CXX_ERROR(_logger, "DISCOUNT RULE INFO: " << exc.what());
    return 0.0;
  }

  // if amount exists, than percentage isn't specify
  if (di->fareAmt1() > EPSILON || di->fareAmt2() > EPSILON)
    return 0;

  bool valid = false;

  if (di->category() == RuleConst::CHILDREN_DISCOUNT_RULE)
    return getChildrenDiscount(type, *_paxTypeInfo, paxCode, *di);

  else if (di->category() == RuleConst::OTHER_DISCOUNT_RULE)
  {
    auto paxCodeIsEqual = [&paxCode](const PaxTypeCode& suppliedPTC) -> bool
    { return paxCode == suppliedPTC; };

    if (type == YOUTH && std::any_of(YOUTH_CODES.begin(), YOUTH_CODES.end(), paxCodeIsEqual))
    {
      valid = true;
    }
    else if (type == SENIOR &&
             std::any_of(SENIOR_CODES.begin(), SENIOR_CODES.end(), paxCodeIsEqual))
    {
      valid = true;
    }
    else if (type == OTHER_22 && di->paxType() == paxCode)
    {
      valid = true;
    }
  }

  return valid ? 100 - di->discPercent() : 0;
}

void
ReissuePenaltyCalculator::calculatePenaltyFee(const PaxTypeFare& paxTypeFare,
                                              const VoluntaryChangesInfoW& volutaryChangesInfo,
                                              PenaltyFee& penaltyFee)
{
  bool selectPenaltyAmt1 = false;
  bool selectPenaltyAmt2 = false;
  bool selectPenaltyPercent = false;
  bool comparedInEquivCurrency = false;

  if ((volutaryChangesInfo.penaltyAmt1() != 0) &&
      (volutaryChangesInfo.cur1() == paxTypeFare.currency()))
  {
    selectPenaltyAmt1 = true;
  }
  else if ((volutaryChangesInfo.penaltyAmt2() != 0) &&
           (volutaryChangesInfo.cur2() == paxTypeFare.currency()))
  {
    selectPenaltyAmt2 = true;
  }
  else if (volutaryChangesInfo.penaltyAmt1() != 0)
  {
    selectPenaltyAmt1 = true;
  }

  if (volutaryChangesInfo.percent() != 0)
  {
    penaltyFee.penaltyFromPercent = paxTypeFare.fareAmount() * volutaryChangesInfo.percent() / 100;
    penaltyFee.penaltyFromPercentCurrency = paxTypeFare.currency();

    selectPenaltyPercent = true;

    if (selectPenaltyAmt1 || selectPenaltyAmt2)
    {
      // Compare amounts in equivalent currency
      if ((selectPenaltyAmt1 &&
           (volutaryChangesInfo.cur1() != penaltyFee.penaltyFromPercentCurrency)) ||
          (selectPenaltyAmt2 &&
           (volutaryChangesInfo.cur2() != penaltyFee.penaltyFromPercentCurrency)))
      {
        comparedInEquivCurrency = true;

        penaltyFee.penaltyFromPercentInEquivCurrency =
                            CurrencyUtil::convertMoneyAmount(penaltyFee.penaltyFromPercent,
                                                             penaltyFee.penaltyFromPercentCurrency,
                                                             _equivalentCurrency,
                                                             *_trx,
                                                             _conversionType);

        if (selectPenaltyAmt1)
        {
          penaltyFee.penaltyAmt1InEquivCurrency =
                                CurrencyUtil::convertMoneyAmount(volutaryChangesInfo.penaltyAmt1(),
                                                                 volutaryChangesInfo.cur1(),
                                                                 _equivalentCurrency,
                                                                 *_trx,
                                                                 _conversionType);
        }
        else if (selectPenaltyAmt2)
        {
          penaltyFee.penaltyAmt2InEquivCurrency =
                                CurrencyUtil::convertMoneyAmount(volutaryChangesInfo.penaltyAmt2(),
                                                                 volutaryChangesInfo.cur2(),
                                                                 _equivalentCurrency,
                                                                 *_trx,
                                                                 _conversionType);
        }
      }

      if (selectPenaltyAmt1)
      {
        bool isAmt1LessThanPercent =
            ((comparedInEquivCurrency ? penaltyFee.penaltyAmt1InEquivCurrency
                                      : volutaryChangesInfo.penaltyAmt1()) <
             (comparedInEquivCurrency ? penaltyFee.penaltyFromPercentInEquivCurrency
                                      : penaltyFee.penaltyFromPercent));

        if (volutaryChangesInfo.highLowInd() == HIGH_PENALTY)
        {
          selectPenaltyAmt1 = !isAmt1LessThanPercent;
          selectPenaltyPercent = isAmt1LessThanPercent;
        }
        else
        {
          selectPenaltyAmt1 = isAmt1LessThanPercent;
          selectPenaltyPercent = !isAmt1LessThanPercent;
        }
      }
      else if (selectPenaltyAmt2)
      {
        bool isAmt2LessThanPercent =
            ((comparedInEquivCurrency ? penaltyFee.penaltyAmt2InEquivCurrency
                                      : volutaryChangesInfo.penaltyAmt2()) <
             (comparedInEquivCurrency ? penaltyFee.penaltyFromPercentInEquivCurrency
                                      : penaltyFee.penaltyFromPercent));
        if (volutaryChangesInfo.highLowInd() == HIGH_PENALTY)
        {
          selectPenaltyAmt2 = !isAmt2LessThanPercent;
          selectPenaltyPercent = isAmt2LessThanPercent;
        }
        else
        {
          selectPenaltyAmt2 = isAmt2LessThanPercent;
          selectPenaltyPercent = !isAmt2LessThanPercent;
        }
      }
    }
  }

  if (selectPenaltyAmt1)
  {
    penaltyFee.penaltyAmount = volutaryChangesInfo.penaltyAmt1();
    penaltyFee.penaltyCurrency = volutaryChangesInfo.cur1();
    if (comparedInEquivCurrency)
      penaltyFee.penaltyAmountInEquivCurrency = penaltyFee.penaltyAmt1InEquivCurrency;
  }
  else if (selectPenaltyAmt2)
  {
    penaltyFee.penaltyAmount = volutaryChangesInfo.penaltyAmt2();
    penaltyFee.penaltyCurrency = volutaryChangesInfo.cur2();
    if (comparedInEquivCurrency)
      penaltyFee.penaltyAmountInEquivCurrency = penaltyFee.penaltyAmt2InEquivCurrency;
  }
  else if (selectPenaltyPercent)
  {
    penaltyFee.penaltyAmount = penaltyFee.penaltyFromPercent;
    penaltyFee.penaltyCurrency = penaltyFee.penaltyFromPercentCurrency;
    if (comparedInEquivCurrency)
      penaltyFee.penaltyAmountInEquivCurrency = penaltyFee.penaltyFromPercentInEquivCurrency;
  }

  if (penaltyFee.applicableDiscount != 0)
  {
    double discountFactor = 1.0 - penaltyFee.applicableDiscount / 100.0;

    penaltyFee.penaltyAmount *= discountFactor;

    if (_conversionType != ConversionType::NO_ROUNDING)
    {
      Money target(penaltyFee.penaltyAmount, penaltyFee.penaltyCurrency);
      CurrencyRoundingUtil crUtil;
      if (crUtil.round(target, *_trx))
        penaltyFee.penaltyAmount = target.value();
    }

    penaltyFee.penaltyFromPercent *= discountFactor;
    penaltyFee.penaltyAmt1InEquivCurrency *= discountFactor;
    penaltyFee.penaltyAmt2InEquivCurrency *= discountFactor;
    penaltyFee.penaltyFromPercentInEquivCurrency *= discountFactor;
    penaltyFee.penaltyAmountInEquivCurrency *= discountFactor;
  }
}

bool
ReissuePenaltyCalculator::isPenalty1HigherThanPenalty2(PenaltyFee& penalty1,
                                                       PenaltyFee& penalty2,
                                                       const CurrencyCode& equivalentCurrency,
                                                       PricingTrx& trx)
{
  if ((penalty1.penaltyAmount == 0) || (penalty2.penaltyAmount == 0) ||
      (penalty1.penaltyCurrency == penalty2.penaltyCurrency))
  {
    return penalty1.penaltyAmount - penalty2.penaltyAmount > EPSILON;
  }
  else
  {
    if (penalty1.penaltyAmountInEquivCurrency == 0)
      penalty1.penaltyAmountInEquivCurrency =
                                         CurrencyUtil::convertMoneyAmount(penalty1.penaltyAmount,
                                                                          penalty1.penaltyCurrency,
                                                                          equivalentCurrency, trx);

    if (penalty2.penaltyAmountInEquivCurrency == 0)
      penalty2.penaltyAmountInEquivCurrency =
                                         CurrencyUtil::convertMoneyAmount(penalty2.penaltyAmount,
                                                                          penalty2.penaltyCurrency,
                                                                          equivalentCurrency, trx);

    return penalty1.penaltyAmountInEquivCurrency - penalty2.penaltyAmountInEquivCurrency > EPSILON;
  }
}

bool
ReissuePenaltyCalculator::isPenalty1HigherThanPenalty2(PenaltyFee& penalty1, PenaltyFee& penalty2)
{
  return isPenalty1HigherThanPenalty2(penalty1, penalty2, _equivalentCurrency, *_trx);
}

void
ReissuePenaltyCalculator::accumulateCharges()
{
  if (_reissueCharges->penaltyFees.empty())
    return;

  bool convertAmountToEquivCurrency = (_chargeCurrency.size() > 1);
  _reissueCharges->changeFeeCurrency =
      (convertAmountToEquivCurrency ? _equivalentCurrency : *_chargeCurrency.begin());

  for (const auto& penaltyFeeItem : _reissueCharges->penaltyFees)
  {
    PenaltyFee* penaltyFee = penaltyFeeItem.second;
    if (convertAmountToEquivCurrency)
    {
      if (penaltyFee->penaltyAmountInEquivCurrency < EPSILON)
      {
        penaltyFee->penaltyAmountInEquivCurrency =
                                      CurrencyUtil::convertMoneyAmount(penaltyFee->penaltyAmount,
                                                                       penaltyFee->penaltyCurrency,
                                                                       _equivalentCurrency,
                                                                       *_trx,
                                                                       _conversionType);
      }

      _reissueCharges->changeFee += penaltyFee->penaltyAmountInEquivCurrency;
    }
    else
    {
      _reissueCharges->changeFee += penaltyFee->penaltyAmount;
    }
  }

  populateChargesInEquivAndCalcCurr();
}

void
ReissuePenaltyCalculator::populateChargesInEquivAndCalcCurr()
{
  _reissueCharges->changeFeeInEquivCurrency =
      (_equivalentCurrency == _reissueCharges->changeFeeCurrency)
          ? _reissueCharges->changeFee
          : CurrencyUtil::convertMoneyAmount(_reissueCharges->changeFee,
                                             _reissueCharges->changeFeeCurrency,
                                             _equivalentCurrency,
                                             *_trx,
                                             _conversionType);

  _reissueCharges->changeFeeInCalculationCurrency =
      (*_calculationCurrency == _reissueCharges->changeFeeCurrency)
          ? _reissueCharges->changeFee
          : CurrencyUtil::convertMoneyAmount(_reissueCharges->changeFee,
                                             _reissueCharges->changeFeeCurrency,
                                             *_calculationCurrency,
                                             *_trx,
                                             _conversionType);
}

void
ReissuePenaltyCalculator::adjustChangeFeeByMinAmount()
{
  if (_reissueCharges->penaltyFees.empty())
    return;

  for (const auto& elem : _fareVoluntaryChangeMap)
  {
    const VoluntaryChangesInfoW& voluntaryChange = *elem.second;
    if (voluntaryChange.minAmt() > 0)
    {
      if (voluntaryChange.minCur() == _reissueCharges->changeFeeCurrency)
      {
        if (voluntaryChange.minAmt() > _reissueCharges->changeFee)
        {
          _reissueCharges->changeFee = voluntaryChange.minAmt();
          _reissueCharges->minAmtApplied = true;
          populateChargesInEquivAndCalcCurr();
        }
      }
      else
      {
        MoneyAmount minAmtInEquivCurrency =
            (_equivalentCurrency == voluntaryChange.minCur())
                ? voluntaryChange.minAmt()
                : CurrencyUtil::convertMoneyAmount(voluntaryChange.minAmt(),
                                                   voluntaryChange.minCur(),
                                                   _equivalentCurrency,
                                                   *_trx,
                                                   _conversionType);

        MoneyAmount minAmtInCalcCurrency =
            (*_calculationCurrency == voluntaryChange.minCur())
                ? voluntaryChange.minAmt()
                : CurrencyUtil::convertMoneyAmount(voluntaryChange.minAmt(),
                                                   voluntaryChange.minCur(),
                                                   *_calculationCurrency,
                                                   *_trx,
                                                   _conversionType);

        if (_reissueCharges->changeFeeInEquivCurrency < EPSILON)
        {
          populateChargesInEquivAndCalcCurr();
        }

        if (minAmtInEquivCurrency > _reissueCharges->changeFeeInEquivCurrency)
        {
          _reissueCharges->changeFee = voluntaryChange.minAmt();
          _reissueCharges->changeFeeCurrency = voluntaryChange.minCur();
          _reissueCharges->changeFeeInEquivCurrency = minAmtInEquivCurrency;
          _reissueCharges->changeFeeInCalculationCurrency = minAmtInCalcCurrency;
          _reissueCharges->minAmtApplied = true;
        }
      }
    }
  }
}
}
