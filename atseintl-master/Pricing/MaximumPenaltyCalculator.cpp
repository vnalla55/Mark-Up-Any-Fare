
#include "Pricing/MaximumPenaltyCalculator.h"
#include "Common/Money.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ProcessTagInfo.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/Cat31PenaltyEstimator.h"
#include "RexPricing/EnhancedRefundDiscountApplier.h"
#include "RexPricing/FarePathChangeDetermination.h"
#include "RexPricing/PenaltyCalculator.h"
#include "RexPricing/RefundDiscountApplier.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Rules/VoluntaryChanges.h"
#include "Rules/VoluntaryRefunds.h"

#include <boost/optional/optional_io.hpp>

#include <memory>

namespace tse
{
FIXEDFALLBACK_DECL(enhancedRefundDiscountApplierRefactor)

namespace
{
MoneyAmount
getAmountFromVoluntaryChangesRecord(const VoluntaryChangesInfo& r3,
                                    const PaxTypeFare& ptf,
                                    PricingTrx& trx)
{
  CurrencyCode fareCurrency = ptf.currency();

  if (r3.percent() > EPSILON)
  {
    return ptf.totalFareAmount() * r3.percent() / 100.0;
  }
  else if (r3.cur2() == fareCurrency)
  {
    return r3.penaltyAmt2();
  }
  else if (r3.cur1() == fareCurrency)
  {
    return r3.penaltyAmt1();
  }
  else if (r3.penaltyAmt1() > EPSILON)
  {
    return CurrencyUtil::convertMoneyAmount(
        r3.penaltyAmt1(), r3.cur1(), fareCurrency, trx, ConversionType::NO_ROUNDING);
  }
  else
  {
    return 0.0;
  }
}
} // namespace

MaximumPenaltyCalculator::MaximumPenaltyCalculator(PricingTrx& trx, FarePath& farePath)
  : _trx(trx),
    _farePath(farePath),
    _diagEnabled(_trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL,
                                                  Diagnostic::MAX_PEN))
{
  const std::vector<uint16_t> categories = {RuleConst::PENALTIES_RULE,
                                            RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                            RuleConst::VOLUNTARY_REFUNDS_RULE};

  RuleControllerWithChancelor<PricingUnitRuleController> puRuleController(
      LoadRecords, categories, &_trx);
  for (PricingUnit* pu : _farePath.pricingUnit())
  {
    FareUsage* failedFareUsage = nullptr;
    puRuleController.validate(_trx, *pu, failedFareUsage, *_farePath.itin());

    for (FareUsage* fu : pu->fareUsage())
    {
      PaxTypeFare* ptf = fu->paxTypeFare();

      ptf->setPenaltyLoadedFlag();
    }
  }
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::changePenalty(smp::RecordApplication departureInd,
                                        const CurrencyCode& currencyCode) const
{
  MaxPenaltyResponse::Fees cat31Fees = changePenaltyCat31(departureInd, currencyCode);

  if (cat31Fees._cat16) // fallback to cat 16
  {
    MaxPenaltyResponse::Fees cat16Fees = changePenaltyCat16(cat31Fees._cat16, currencyCode);

    if (cat31Fees._cat16 & smp::BEFORE)
    {
      cat31Fees._before = cat16Fees._before;
    }
    if (cat31Fees._cat16 & smp::AFTER)
    {
      cat31Fees._after = cat16Fees._after;
    }
  }

  cat31Fees.finalRounding(_trx);

  return cat31Fees;
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::refundPenalty(smp::RecordApplication departureInd,
                                        CurrencyCode currencyCode) const
{
  MaxPenaltyResponse::Fees cat33Fees = refundPenaltyCat33(departureInd, currencyCode);

  if (cat33Fees._cat16) // fallback to cat 16
  {
    MaxPenaltyResponse::Fees cat16Fees = refundPenaltyCat16(cat33Fees._cat16, currencyCode);

    if (cat33Fees._cat16 & smp::BEFORE)
    {
      cat33Fees._before = cat16Fees._before;
    }
    if (cat33Fees._cat16 & smp::AFTER)
    {
      cat33Fees._after = cat16Fees._after;
    }
  }

  cat33Fees.finalRounding(_trx);

  return cat33Fees;
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::changePenaltyCat31(smp::RecordApplication departureInd,
                                             const CurrencyCode& currencyCode) const
{
  DiagManager diag(_trx);
  if (_diagEnabled)
  {
    diag.activate(Diagnostic555);
  }

  diag << "CHANGE PENALTY CAT31\n";

  std::shared_ptr<FarePathChangeDetermination> fpcd =
      std::make_shared<FarePathChangeDeterminationMock>();

  ReissuePenaltyCalculator calc;
  calc.initialize(_trx,
                  currencyCode,
                  *fpcd.get(),
                  _trx.getRequest()->validatingCarrier(),
                  *_farePath.paxType()->paxTypeInfo(),
                  nullptr,
                  nullptr);

  std::vector<const PaxTypeFare*> fares;
  RuleUtil::getAllPTFs(fares, _farePath);

  FcFeesVec r3Calculations;

  if (isOverridePossible())
  {
    overrideDomesticRecords();
  }

  for (const PaxTypeFare* ptf : fares)
  {
    if (!smp::ptfHasCat31Rec2(*ptf))
      return {{}, {}, departureInd}; // go to c16, no general record c31 found for this fare

    const ReissuePenaltyCalculator::FcFees penalties =
        calc.getPenalties(ptf->getVoluntaryChangesInfo(), *ptf, departureInd, diag);

    if (_diagEnabled)
    {
      diag << "  " << DiagnosticUtil::printPaxTypeFare(*ptf) << '\n';

      for (const auto& pen : penalties)
      {
        diag << "    REC " << std::get<2>(pen)->itemNo() << ' ' << std::get<1>(pen) << std::setw(8)
             << Money(std::get<0>(pen), currencyCode) << '\n';
      }
    }

    r3Calculations.push_back(std::move(penalties));
  }

  return Cat31PenaltyEstimator(currencyCode, _farePath, _trx, std::move(diag))
      .estimate(r3Calculations, departureInd);
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::changePenaltyCat16(smp::RecordApplication departureInd,
                                             CurrencyCode currencyCode) const
{
  DiagManager diag(_trx);
  if (_diagEnabled)
  {
    diag.activate(Diagnostic555);
  }

  diag << "CHANGE PENALTY CAT16\n";

  MaxPenaltyResponse::Fees sumFees =
      penaltyCat16(Cat16MaxPenaltyCalculator::CHANGE_PEN, departureInd, currencyCode, diag);

  smp::resultDiagnostic(diag, sumFees, departureInd, "CHANGE", "CAT16");

  return sumFees;
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::refundPenaltyCat33(smp::RecordApplication departureInd,
                                             CurrencyCode currencyCode) const
{
  if (_diagEnabled)
  {
    DiagManager diag(_trx, Diagnostic555);
    diag << "REFUND PENALTY CAT33\n";
  }

  RefundPermutationGenerator::Permutations permutations;
  RefundPermutationGenerator::FCtoSequence seqsByFc;
  RefundPermutationGenerator(_trx).processForSMP(permutations, _farePath, seqsByFc, departureInd);

  MaxPenaltyResponse::Fees fees;
  fees._before._non = fees._after._non = false;

  if (!permutations.empty())
  {
    const RefundDiscountApplier& discountApplier =
        (fallback::fixed::enhancedRefundDiscountApplierRefactor()
             ? *EnhancedRefundDiscountApplier::create(_trx.dataHandle(), *_farePath.paxType())
             : *RefundDiscountApplier::create(_trx.dataHandle(), *_farePath.paxType()));

    PenaltyCalculator penaltyCalc(_trx, _farePath, currencyCode, discountApplier);
    MoneyAmount maximumBefore = -1.0;
    MoneyAmount maximumAfter = -1.0;
    int minimumAmountNonRefBefore = INT_MAX;
    int minimumAmountNonRefAfter = INT_MAX;

    auto countNonRef = [](const RefundPermutation& perm) -> bool
    {
      int amount = 0;
      for (auto processInfo : perm.processInfos())
      {
        if (processInfo->record3().cancellationInd() == 'X')
        {
          amount++;
        }
      }
      return amount;
    };

    auto ptfPuMap = mapPaxTypeFaresToPricingUnit();

    // partially flown processing to determine non refundability
    bool hasPartiallyFlownFC = isAnyFCPartiallyFlownAfter(seqsByFc, ptfPuMap);

    for (RefundPermutation* perm : permutations)
    {
      smp::RecordApplication application = refundPermutationAppl(*perm, ptfPuMap);

      if (application == smp::INVALID)
      {
        continue;
      }

      if (hasPartiallyFlownFC && (application & departureInd & smp::AFTER))
        fees._after._non = true;

      penaltyCalc.calculate(*perm);

      const bool refundable = perm->refundable(_farePath.pricingUnit());

      if (refundable)
      {
        int currentNonRef = countNonRef(*perm);

        if (application & departureInd & smp::BEFORE)
        {
          if (currentNonRef == minimumAmountNonRefBefore)
          {
            maximumBefore = std::max(maximumBefore, perm->totalPenalty().value());
            minimumAmountNonRefBefore = currentNonRef;
          }
          else if (currentNonRef < minimumAmountNonRefBefore)
          {
            maximumBefore = perm->totalPenalty().value();
            minimumAmountNonRefBefore = currentNonRef;
          }
        }

        if (application & departureInd & smp::AFTER)
        {
          if (currentNonRef == minimumAmountNonRefAfter)
          {
            maximumAfter = std::max(maximumAfter, perm->totalPenalty().value());
            minimumAmountNonRefAfter = currentNonRef;
          }
          else if (currentNonRef < minimumAmountNonRefAfter)
          {
            maximumAfter = perm->totalPenalty().value();
            minimumAmountNonRefAfter = currentNonRef;
          }
        }
      }
      else
      {
        if (application & departureInd & smp::BEFORE)
        {
          fees._before._non = true;
        }

        if (application & departureInd & smp::AFTER)
        {
          fees._after._non = true;
        }
      }

      if (_diagEnabled)
      {
        DiagManager diag(_trx, Diagnostic555);
        printDiagnostic555RefundCat33(diag, *perm, refundable, ptfPuMap);
      }
    }

    if (maximumBefore > -EPSILON)
    {
      fees._before._fee = Money(maximumBefore, currencyCode);
    }

    if (maximumAfter > -EPSILON)
    {
      fees._after._fee = Money(maximumAfter, currencyCode);
    }
    if (_diagEnabled)
    {
      DiagManager diag(_trx, Diagnostic555);
      smp::resultDiagnostic(diag, fees, departureInd, "REFUND", "CAT33");
    }
  }

  if (fees.calculateCat16Application(departureInd))
  {
    if (_diagEnabled)
    {
      DiagManager diag(_trx, Diagnostic555);
      diag << "  NO PERMUTATIONS " << smp::printRecordApplication(fees._cat16)
           << " - FALLBACK TO CAT16\n";
    }
  }

  return fees;
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::refundPenaltyCat16(smp::RecordApplication departureInd,
                                             CurrencyCode currencyCode) const
{
  DiagManager diag(_trx);
  if (_diagEnabled)
  {
    diag.activate(Diagnostic555);
  }

  diag << "REFUND PENALTY CAT16\n";

  MaxPenaltyResponse::Fees sumFees =
      penaltyCat16(Cat16MaxPenaltyCalculator::REFUND_PEN, departureInd, currencyCode, diag);

  smp::resultDiagnostic(diag, sumFees, departureInd, "REFUND", "CAT16");

  return sumFees;
}

MaxPenaltyResponse::Fees
MaximumPenaltyCalculator::penaltyCat16(Cat16MaxPenaltyCalculator::PenaltyType penaltyType,
                                       smp::RecordApplication departureInd,
                                       CurrencyCode currencyCode,
                                       DiagManager& diag) const
{
  MaxPenaltyResponse::Fees sumFees;
  sumFees._cat16 = departureInd;
  sumFees._before._non = sumFees._after._non = false;
  sumFees._before.setDefaultFee(currencyCode);
  sumFees._after.setDefaultFee(currencyCode);

  Cat16MaxPenaltyCalculator calc(_trx, false);

  for (const PricingUnit* pu : _farePath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pu->fareUsage())
    {
      const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();

      MaxPenaltyResponse::Fees fees = calc.calculateMaxPenalty(paxTypeFare->getPenaltyInfo(),
                                                               currencyCode,
                                                               *paxTypeFare,
                                                               fareUsage,
                                                               departureInd,
                                                               penaltyType);

      smp::printDiagnosticFareFees(diag, *paxTypeFare, fees);

      const smp::RecordApplication missingDataApp = fees.missingDataInsideCalc();
      if (missingDataApp & smp::BEFORE)
      {
        sumFees._before._missingDataVec.push_back(paxTypeFare->fare()->fareInfo());
      }
      if (missingDataApp & smp::AFTER)
      {
        sumFees._after._missingDataVec.push_back(paxTypeFare->fare()->fareInfo());
      }

      if ((fees._before.isFullyNon() || !(departureInd & smp::BEFORE)) &&
          (fees._after.isFullyNon() || !(departureInd & smp::AFTER)))
      {
        sumFees.setFullyNon();
      }

      sumFees += fees;
    }
  }

  return sumFees;
}

smp::RecordApplication
MaximumPenaltyCalculator::refundPermutationAppl(const RefundPermutation& permutation,
                                                const PtfPuMap& ptfPuMap) const
{
  const std::vector<RefundProcessInfo*>& processInfos = permutation.processInfos();

  smp::RecordApplication status =
      smp::getRecordApplication(processInfos.front()->record3(),
                                true, // because it's on first FC
                                (ptfPuMap.at(&processInfos.front()->paxTypeFare()) == 0));

  for (auto iter = ++processInfos.begin(); iter != processInfos.end(); ++iter)
  {
    auto& record3 = (*iter)->record3();
    smp::RecordApplication nextStatus =
        smp::getRecordApplication(record3, false, (ptfPuMap.at(&(*iter)->paxTypeFare()) == 0));

    if (status == smp::BOTH)
    {
      status = nextStatus;
    }
    else if (nextStatus != smp::BOTH && nextStatus != status)
    {
      return smp::INVALID;
    }
  }

  return status & validateFullyFlown(permutation);
}

smp::RecordApplication
MaximumPenaltyCalculator::validateFullyFlown(const RefundPermutation& permutation) const
{
  smp::RecordApplication status = smp::BOTH;

  bool fullyFlownBFound = false;
  for(const RefundProcessInfo* refundProcessInfo : permutation.processInfos())
  {
    switch(refundProcessInfo->record3().fullyFlown())
    {
    case VoluntaryRefunds::PARTIALLY_FLOWN:
      if (fullyFlownBFound)
        return smp::INVALID;

      fullyFlownBFound = true;
      status = smp::AFTER;

      if (refundProcessInfo->paxTypeFare().fareMarket()->travelSeg().size() == 1)
        return smp::INVALID;

      break;
    }
  }
  return status;
}

void MaximumPenaltyCalculator::printDiagnostic555RefundCat33(DiagManager& diag,
                                                             const RefundPermutation& perm,
                                                             const bool& refundable,
                                                             const PtfPuMap& ptfPuMap) const
{
  diag << "  PERM" << perm.number() << ' '
       << (refundable ? perm.totalPenalty().toString() : "NONREFUNDABLE") << ' '
       << smp::printRecordApplication(refundPermutationAppl(perm, ptfPuMap)) << '\n';
  for (const RefundProcessInfo* rpi : perm.processInfos())
  {
    diag << "    REC " << rpi->record3().itemNo() << ' '
         << DiagnosticUtil::printPaxTypeFare(rpi->paxTypeFare())
         << ' ' << Money(rpi->record3().penalty1Amt(), rpi->record3().penalty1Cur()) << ' '
         << Money(rpi->record3().penalty2Amt(), rpi->record3().penalty2Cur()) << '\n';
  }
}

std::vector<const PaxTypeFare*>
MaximumPenaltyCalculator::findInternationalFCs(std::vector<const PaxTypeFare*> fares,
                                               uint32_t domesticFCNumber) const
{
  int16_t leftIndex = domesticFCNumber - 1, rightIndex = domesticFCNumber + 1,
          faresCount = static_cast<int16_t>(fares.size());

  std::vector<const PaxTypeFare*> intlFCs;

  while (leftIndex >= 0 || rightIndex < faresCount)
  {
    if (leftIndex >= 0)
    {
      if (fares[leftIndex]->fareMarket()->geoTravelType() == GeoTravelType::International)
      {
        intlFCs.push_back(fares[leftIndex]);
      }
      --leftIndex;
    }

    if (rightIndex < faresCount)
    {
      if (fares[rightIndex]->fareMarket()->geoTravelType() == GeoTravelType::International)
      {
        intlFCs.push_back(fares[rightIndex]);
      }
      ++rightIndex;
    }
  }

  return intlFCs;
}

const VoluntaryChangesInfoW*
MaximumPenaltyCalculator::getOverridingRecord(const std::vector<CarrierCode>& carriers,
                                              const std::vector<const PaxTypeFare*>& fareComponents)
    const
{
  for (const PaxTypeFare* ptf : fareComponents)
  {
    bool isValid = std::any_of(
        carriers.begin(),
        carriers.end(),
        [&](const CarrierCode& cxr) -> bool
        { return cxr == ptf->fareMarket()->governingCarrier() || cxr == DOLLAR_CARRIER; });
    if ((isValid || carriers.empty()) && !ptf->getVoluntaryChangesInfo().empty())
    {
      const std::unordered_set<const VoluntaryChangesInfoW*>& records =
          ptf->getVoluntaryChangesInfo();
      return *std::max_element(
                 records.begin(),
                 records.end(),
                 [&](const VoluntaryChangesInfoW* lhs, const VoluntaryChangesInfoW* rhs)
                 {
                   return getAmountFromVoluntaryChangesRecord(*lhs->orig(), *ptf, _trx) <
                          getAmountFromVoluntaryChangesRecord(*rhs->orig(), *ptf, _trx);
                 });
    }
  }
  return nullptr;
}

bool
MaximumPenaltyCalculator::isOverridePossible() const
{
  bool isDomestic = false;
  bool isInternational = false;

  for (PricingUnit* pu : _farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      if (fu->paxTypeFare()->fareMarket()->geoTravelType() == GeoTravelType::International)
      {
        isInternational = true;
      }

      if (fu->paxTypeFare()->fareMarket()->geoTravelType() == GeoTravelType::Domestic ||
          fu->paxTypeFare()->fareMarket()->geoTravelType() == GeoTravelType::ForeignDomestic)
      {
        isDomestic = true;
      }

      if (isInternational && isDomestic)
        return true;
    }
  }
  return false;
}

void
MaximumPenaltyCalculator::overrideDomesticRecords() const
{
  std::vector<const PaxTypeFare*> fares = smp::getAllPaxTypeFares(_farePath);

  uint16_t index = 0;
  for (const PaxTypeFare* ptf : fares)
  {
    const FareMarket* fm = ptf->fareMarket();
    if (fm->isDomestic() || fm->isForeignDomestic() || fm->isWithinRussianGroup())
    {
      std::vector<const PaxTypeFare*> intlFCs = findInternationalFCs(fares, index);

      if (intlFCs.empty())
        continue;

      for (const VoluntaryChangesInfoW* r3 : ptf->getVoluntaryChangesInfo())
      {
        if (r3->domesticIntlComb() == VoluntaryChanges::DOMINTL_COMB_NOT_APPLY)
          continue;

        const std::vector<CarrierCode> carriers = getCarrierList(*r3, *fm);
        const VoluntaryChangesInfoW* overridingRecord = getOverridingRecord(carriers, intlFCs);

        if (overridingRecord)
        {
          bool origRecChangeRestr = smp::voluntaryChangesNotPermitted(*r3);
          bool overrideRecChangeRestr = smp::voluntaryChangesNotPermitted(*overridingRecord);

          if (origRecChangeRestr != overrideRecChangeRestr || (!origRecChangeRestr && !overrideRecChangeRestr))
          {
            const_cast<VoluntaryChangesInfoW*>(r3)
                ->setConditionallyOverridenBytes(VoluntaryChangesInfoW::coByte47_75, true);
          }
          const_cast<VoluntaryChangesInfoW*>(r3)->overriding() = overridingRecord->orig();
        }
      }
    }
    ++index;
  }
}

const std::vector<CarrierCode>
MaximumPenaltyCalculator::getCarrierList(const VoluntaryChangesInfoW& record, const FareMarket& fm)
    const
{
  std::vector<CarrierCode> carriers;
  switch (record.domesticIntlComb())
  {
  case VoluntaryChanges::DOMINTL_COMB_BLANK:
    carriers.push_back(fm.governingCarrier());
    break;

  case VoluntaryChanges::DOMINTL_COMB_APPLY:
  {
    const std::vector<CarrierApplicationInfo*>& carrierApplInfo =
        _trx.dataHandle().getCarrierApplication(
            record.vendor(), record.itemNo(), fm.ruleApplicationDate());
    for (const auto& carrierAppl : carrierApplInfo)
    {
      carriers.push_back(carrierAppl->carrier());
    }
    break;
  }
  }
  return carriers;
}

MaximumPenaltyCalculator::PtfPuMap MaximumPenaltyCalculator::mapPaxTypeFaresToPricingUnit() const
{
  PtfPuMap result;
  auto& pu = _farePath.pricingUnit();
  for (std::vector<PricingUnit*>::size_type i = 0; i < pu.size(); ++i)
    for (auto fu : pu[i]->fareUsage())
      result[fu->paxTypeFare()] = i;
  return result;
}

bool MaximumPenaltyCalculator::isAnyFCPartiallyFlownAfter(
    const RefundPermutationGenerator::FCtoSequence& seqsByFc, const PtfPuMap& ptfPuMap) const
{
  for (const auto& seq : seqsByFc)
  {
    bool partiallyFlownFound = false;

    auto isPartiallyFlown = [&](const RefundProcessInfo* refundProcessInfo)
    {
      const auto& r3 = refundProcessInfo->record3();
      bool isFirstFc = (seq.first == 0);
      bool isFirstPu = (ptfPuMap.at(&refundProcessInfo->paxTypeFare()) == 0);

      if (smp::getRecordApplication(r3, isFirstFc, isFirstPu) & smp::AFTER)
      {
        if (r3.fullyFlown() != VoluntaryRefunds::PARTIALLY_FLOWN)
          return false;
        else
          partiallyFlownFound = true;
      }
      return true;
    };

    const auto& records = seq.second;
    if (std::all_of(records.begin(), records.end(), isPartiallyFlown) && partiallyFlownFound)
      return true;
  }
  return false;
}

} /* namespace tse */
