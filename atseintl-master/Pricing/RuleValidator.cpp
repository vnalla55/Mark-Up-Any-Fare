/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#include "Pricing/RuleValidator.h"

#include "BookingCode/MixedClassController.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/FarePathFactoryFailedPricingUnits.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/NegotiatedFareCombinationValidator.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PUPath.h"
#include "Rules/RuleUtil.h"

namespace tse
{

bool
RuleValidator::validate(FPPQItem& fppqItem, const bool validatePULevel)
{
  bool valid = checkPULevelRuleValidation(fppqItem, validatePULevel);

  if (valid)
  {
    valid = checkFPLevelRuleValidation(fppqItem);
  }

  if (UNLIKELY(valid && TrxUtil::reuseFarePUResult()))
  {
    revalidateResultReusedFU(fppqItem);
  }

  return valid;
}

void
RuleValidator::showPricingUnitDiag(const tse::PricingUnit& prU) const
{
  if (LIKELY(_trx.diagnostic().diagnosticType() != Diagnostic555) ||
      _trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL,
                                       Diagnostic::MAX_PEN))
  {
    return;
  }

  DiagCollector& dc = *(DCFactory::instance()->create(_trx));
  dc.enable(Diagnostic555);
  dc.printLine();
  if (!DiagnosticUtil::filter(_trx, prU))
  {
    dc << "           PRICING UNIT/FARE USAGE RULE VALIDATION DIAGNOSTICS" << std::endl;
    dc << " validation in Rule Validator  " << std::endl;
    dc << " " << std::endl;
    dc << "PRICING UNIT " << prU;
    dc << " " << std::endl;
  }
  dc.flushMsg();
}

bool
RuleValidator::checkPULevelRuleValidation(FPPQItem& fppqItem, const bool validatePULevel)
{
  if (UNLIKELY(!validatePULevel))
    return true;

  _ruleController.setCategoryPhase(PURuleValidation);


  const bool isFareX = (_trx.getOptions() && _trx.getOptions()->fareX());

  TSE_ASSERT(fppqItem.farePath());
  FarePath& fpath = *fppqItem.farePath();
  std::vector<PricingUnit*>::iterator puIt = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = fpath.pricingUnit().end();
  for (uint32_t puFactIdx = 0; puIt != puItEnd; ++puIt, ++puFactIdx)
  {
    PricingUnit& prU = **puIt;
    const auto status = getPUScopeRuleReValStatus(fppqItem, puFactIdx, prU);
    if (UNLIKELY(status == PUPQItem::PUValidationStatus::FAILED))
      return false;
    else if (status == PUPQItem::PUValidationStatus::PASSED)
      continue;

    showPricingUnitDiag(prU);

    FareUsage* failedFareUsage(nullptr);

    bool valid = _ruleController.validate(_trx, prU, failedFareUsage, *fpath.itin());

    if (valid || ignoreRuleForKeepFare(prU)) // Even though rule validation fails,
    // if there is keep fare,
    // Neg fare combination still needs to be checked to see if
    // it can be hard failed.
    {
      if (!isFareX && !prU.isCmdPricing())
      {
        NegotiatedFareCombinationValidator combinationValidator(_trx);
        if (!combinationValidator.validate(prU))
        {
          if (!valid && failedFareUsage != nullptr)
          {
            saveFailedFare(puFactIdx, failedFareUsage, nullptr); // Remember failed FU if PU
            // fails rule validation for performance.
          }

          setPUScopeRuleReValStatus(
              fppqItem, puFactIdx, PUPQItem::PUValidationStatus::FAILED); // Hard fail PU.
          return false;
        }
      }
    }

    if (valid)
    {
      setPUScopeRuleReValStatus(fppqItem, puFactIdx, PUPQItem::PUValidationStatus::PASSED);
    }
    else
    {
      if (prU.ruleFailedButSoftPassForKeepFare()) // PU fails only on keep fares
      {
        setPUScopeRuleReValStatus(fppqItem, puFactIdx, PUPQItem::PUValidationStatus::PASSED);
        continue;
      }
      if (failedFareUsage != nullptr)
        saveFailedFare(puFactIdx, failedFareUsage, nullptr);
      setPUScopeRuleReValStatus(fppqItem, puFactIdx, PUPQItem::PUValidationStatus::FAILED);
      return false;
    }
  }

  return true;
}

const bool
RuleValidator::ignoreRuleForKeepFare(const PricingUnit& prU) const
{
  return (_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
          (static_cast<RexPricingTrx*>(&_trx))->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE &&
          prU.hasKeepFare());
}

void
RuleValidator::setPUScopeRuleReValStatus(const FPPQItem& fppqItem,
                                         const uint16_t puFactIdx,
                                         PUPQItem::PUValidationStatus status)
{
  const PUPath& puPath = *fppqItem.puPath();
  if (puPath.totalPU() == 1)
  {
    // no saved status, validation needed
    return;
  }
  PricingUnitFactory* puf = _allPUF[puFactIdx];

  uint32_t puIdx = fppqItem.puIndices()[puFactIdx];
  puf->setPUScopeRuleReValStatus(puIdx, status);
}

PUPQItem::PUValidationStatus
RuleValidator::getPUScopeRuleReValStatus(const FPPQItem& fppqItem,
                                         const uint16_t puFactIdx,
                                         PricingUnit& pu)
{
  PricingUnitFactory* puf = _allPUF[puFactIdx];
  uint32_t puIdx = fppqItem.puIndices()[puFactIdx];
  return puf->getPUScopeRuleReValStatus(puIdx, pu);
}

void
RuleValidator::saveFailedFare(const uint16_t puFactIdx,
                              FareUsage* failedFareUsage1,
                              FareUsage* failedFareUsage2)
{
  PricingUnitFactory* puf = _allPUF[puFactIdx];
  puf->saveCat10FailedFare(failedFareUsage1, failedFareUsage2);
}

void
RuleValidator::reuseSurchargeData(FPPQItem& fppqItem) const
{
  if (LIKELY(!TrxUtil::isFVOSurchargesEnabled()))
    return;

  fppqItem.reuseSurchargeData();
}

bool
RuleValidator::allFaresProcessedSurcharges(FPPQItem& fppqItem) const
{
  if (LIKELY(!TrxUtil::isFVOSurchargesEnabled()))
    return false;

  return !fppqItem.needRecalculateCat12();
}

void
RuleValidator::showFarePathDiag(FPPQItem& fppqItem)
{
  _diag.enable(Diagnostic302, Diagnostic303, Diagnostic306, Diagnostic307);

  if (LIKELY(_diag.diagnosticType() != Diagnostic555) ||
      _trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN))
  {
    return;
  }

  DiagCollector& dc = *(DCFactory::instance()->create(_trx));
  dc.enable(Diagnostic555);
  dc.printLine();
  if (!DiagnosticUtil::filter(_trx, *fppqItem.farePath()))
  {
    dc << "           PRICING UNIT/FARE USAGE RULE VALIDATION DIAGNOSTICS" << std::endl;
    dc << " " << std::endl;
    dc << "FARE PATH " << *fppqItem.farePath();
    dc << " " << std::endl;
  }
  dc.flushMsg();
}
bool
RuleValidator::checkFPLevelRuleValidation(FPPQItem& fppqItem)
{
  FarePath& fpath = *fppqItem.farePath();

  reuseSurchargeData(fppqItem);

  TSELatencyData metrics(_trx, "PO RULE VALIDATION ");
  _ruleController.setCategoryPhase(FPRuleValidation);

  if (((_trx.getTrxType() == PricingTrx::MIP_TRX) && _trx.isAltDates()) ||
      allFaresProcessedSurcharges(fppqItem))
  {
    _ruleController.removeCat(RuleConst::SURCHARGE_RULE);
  }

  showFarePathDiag(fppqItem);

  // Do currency Adjusment processing (for Nigeria)
  RuleUtil::processCurrencyAdjustment(_trx, fpath);

  std::vector<PricingUnit*>::iterator puIt = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = fpath.pricingUnit().end();

  for (uint32_t puFactIdx = 0; puIt != puItEnd; ++puIt, ++puFactIdx)
  {
    PricingUnit& pu = **puIt;
    if (!_ruleController.validate(_trx, fpath, pu))
    {
      if (UNLIKELY(pu.ruleFailedButSoftPassForKeepFare()))
        continue;
      else
      {
        farepathutils::setFailedPricingUnits(puFactIdx, fppqItem, _failedPricingUnits);
        return false;
      }
    }
  }
  return true;
}
void
RuleValidator::revalidateResultReusedFU(FPPQItem& fppqItem)
{
  _ruleController.setCategoryPhase(RevalidatePassResultReusedFU);

  FarePath& fpath = *fppqItem.farePath();
  bool diagHeaderDisplayed = false;

  for (PricingUnit* pu : fppqItem.farePath()->pricingUnit())
  {
    for (FareUsage* fu : pu->reusedResultFUs())
    {
      std::vector<FareUsage*>& fuV = pu->fareUsage();
      if (std::find(fuV.begin(), fuV.end(), fu) != fuV.end())
      {
        _ruleController.validate(_trx, *pu, *fu, *fpath.itin());
        if (!diagHeaderDisplayed && _diag.diagnosticType() == Diagnostic555 &&
            !_trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN))
        {
          _diag.enable(Diagnostic555);
          _diag << "REVALIDATE PASSED FU TO COLLECT RESULT INTO PU\n";
          _diag << *fu << "\n";
          diagHeaderDisplayed = true;
        }
        else if (diagHeaderDisplayed)
        {
          _diag << *fu << "\n";
        }
      }
    }
  }
}
}
