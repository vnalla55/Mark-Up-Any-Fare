//----------------------------------------------------------------------------
//
//  File:  NetRemitPricing.cpp
//  Created:  Feb 17, 2006
//  Authors:  Andrea Yang
//
//  Description: Process Fare Path for Net Remit
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Pricing/NetRemitPricing.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "MinFares/MinFareChecker.h"
#include "Pricing/Combinations.h"
#include "Pricing/PricingUtil.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <iostream>
#include <map>

namespace tse
{
static Logger
logger("atseintl.Pricing.NetRemitPricing");

NetRemitPricing::NetRemitPricing(PricingTrx& trx, FarePath& farePath)
  : _trx(trx), _farePath(farePath)
{
  init();
}

NetRemitPricing::~NetRemitPricing() {}

void
NetRemitPricing::init()
{
}

bool
NetRemitPricing::process()
{
  if (_trx.getRequest()->ticketingAgent()->axessUser() && _trx.getRequest()->isWpNettRequested())
  {
    processAxessPricing();
    if (_farePath.netRemitFarePath() == nullptr && _farePath.collectedNegFareData() != nullptr)
    {
      _farePath.collectedNegFareData()->trailerMsg() =
          "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
    }
    return true;
  }

  if (LIKELY(!isNetRemit()))
    return true;

  // Create NetRemitFarePath
  NetRemitFarePath* netRemitFp = _trx.dataHandle().create<NetRemitFarePath>();

  if (netRemitFp == nullptr)
    return false;

  FareCombRuleInfoMap cat10ForNetFaresMap;

  getCat10ForNetFares(cat10ForNetFaresMap);

  netRemitFp->initialize(&_trx, &_farePath, &cat10ForNetFaresMap);

  if (!netRemitFp->usedFareBox())
  {
    // Call Surcharge rules
    calcSurcharges(*netRemitFp);

    // Call Minimum Fares
    MinFareChecker minFareChecker;
    minFareChecker.process(_trx, *netRemitFp);
  }

  // Set currency codes
  PricingUtil::determineBaseFare(netRemitFp, _trx, netRemitFp->itin());

  netRemitFp->processed() = true;

  // Save new NetRemitFarePath
  _farePath.netRemitFarePath() = netRemitFp;

  displayNetRemitFarePath(*netRemitFp);

  compareNetRemitFareAmount();

  return true;
}

void
NetRemitPricing::processAxessPricing()
{

  if (!_farePath.selectedNetRemitFareCombo())
    return;

  // Create AxessFarePath
  NetRemitFarePath* axessFp = _trx.dataHandle().create<NetRemitFarePath>();

  if (axessFp == nullptr)
    return;

  axessFp->initialize(&_trx, &_farePath);

  // Call RuleController for the RuleRevalidation process (PU/FP scope).
  if (!processRuleRevalidationForAxess(*axessFp))
    return;

  // Call Combinability.
  if (!processCombinabilityForAxess(*axessFp))
    return;

  axessFp->processed() = true;

  // Save new AxessFarePath (with selected published fares)
  _farePath.netRemitFarePath() = axessFp;

  displayNetRemitFarePath(*axessFp);

  return;
}

bool
NetRemitPricing::processCombinabilityForAxess(FarePath& axessFp)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(_trx));

  diag.enable(&axessFp, Diagnostic605, Diagnostic610, Diagnostic620, Diagnostic601);
  diag.printHeader();

  CombinabilityScoreboard comboScoreboard;
  comboScoreboard.trx() = &_trx;

  Combinations combinations;
  combinations.trx() = &_trx;
  combinations.comboScoreboard() = &comboScoreboard;

  FareUsage* failedFareUsage = nullptr; // dummy
  FareUsage* failedTargetFareUsage = nullptr; // dummy

  bool ret = true;

  std::vector<PricingUnit*>::iterator puIt = axessFp.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = axessFp.pricingUnit().end();

  for (; puIt != puItEnd; ++puIt)
  {
    // Do not check PU type for:
    //---------- OPENJAW PU Validation
    //---------- ROUNDTRIP PU Validation
    //---------- CIRCLETRIP PU Validation
    // ----------Check if Tag-2 Fare Needed
    PricingUnit& prU = *(*puIt);

    diag.enable(&prU, Diagnostic605);

    if (diag.isActive())
    {
      diag << prU << std::endl;
    }

    if (combinations.process(prU, failedFareUsage, failedTargetFareUsage, diag, axessFp.itin()) !=
        CVR_PASSED)
    {
      ret = false;
    }
  }

  // check EOE
  if (ret)
  {

    diag.enable(&axessFp, Diagnostic610, Diagnostic620);
    if (diag.isActive())
    {
      diag << axessFp;
    }

    if (combinations.process(axessFp, 1, failedFareUsage, failedTargetFareUsage, diag) !=
        CVR_PASSED)
    {
      ret = false;
      diag.enable(&axessFp, Diagnostic610, Diagnostic620);
      if (diag.isActive())
        diag << "FAILED FARE PATH" << std::endl;
    }
    else
    {
      diag.enable(&axessFp, Diagnostic610, Diagnostic620);
      if (diag.isActive())
        diag << "PASSED FARE PATH" << std::endl;
    }
  }

  // Add diag601 for the Jal/Axess user
  diag.enable(&axessFp, Diagnostic601);

  if (diag.isActive())
  {
    std::vector<PricingUnit*>::iterator puItD = axessFp.pricingUnit().begin();
    const std::vector<PricingUnit*>::iterator puItEndD = axessFp.pricingUnit().end();

    for (; puItD != puItEndD; ++puItD)
    {
      PricingUnit& prU = *(*puItD);

      std::vector<FareUsage*>::iterator fareUsageI = prU.fareUsage().begin();
      std::vector<FareUsage*>::iterator fareUsageEnd = prU.fareUsage().end();

      for (; fareUsageI != fareUsageEnd; ++fareUsageI)
      {
        bool isLocationSwapped = false;
        CombinabilityRuleInfo* pCat10 = nullptr;

        if ((*fareUsageI)->rec2Cat10() == nullptr)
        {
          pCat10 = RuleUtil::getCombinabilityRuleInfo(
              _trx, *((*fareUsageI)->paxTypeFare()), isLocationSwapped);

          (*fareUsageI)->rec2Cat10() = pCat10;

          PaxTypeFare* ptf = (*fareUsageI)->paxTypeFare();

          if (!DiagnosticUtil::filter(_trx, *ptf))
          {
            diag.printLine();
            diag << *(ptf->fareMarket()) << std::endl;
            diag << "REQUESTED PAXTYPE: " << prU.paxType()->paxType() << "     "
                 << (prU.puType() == PricingUnit::Type::OPENJAW
                         ? "101/OJ"
                         : (prU.puType() == PricingUnit::Type::ROUNDTRIP
                                ? "102/RT"
                                : (prU.puType() == PricingUnit::Type::CIRCLETRIP
                                       ? "103/CT"
                                       : (prU.puType() == PricingUnit::Type::ONEWAY ? "104/OW" : " "))))
                 << std::endl;

            diag.displayPtfItins(*ptf);

            diag << (**fareUsageI) << std::endl;

            diag << " FARE CURR:" << ptf->currency() << std::endl;

            diag << "FARE OWRT VALUE:" << ptf->owrt() << std::endl;

            diag << "FARE CLASS:" << ptf->fareClass()
                 << " MARKET FARE BASIS:" << ptf->fareMarket()->fareBasisCode() << std::endl;
          }
        }
      }
    }
  }

  diag.flushMsg();

  return ret;
}

bool
NetRemitPricing::processRuleRevalidationForAxess(FarePath& axessFp)
{
  std::vector<uint16_t> puCategories;

  // Note that Cat1 is out of scope for AXESS.
  // PU scope. ReValidate Cat2/3/4/11.

  puCategories.push_back(RuleConst::DAY_TIME_RULE);
  puCategories.push_back(RuleConst::SEASONAL_RULE);
  puCategories.push_back(RuleConst::FLIGHT_APPLICATION_RULE);
  puCategories.push_back(RuleConst::BLACKOUTS_RULE);

  // FP scope. ReValidate Cat23|5|14|15|6|7|8|9|12|19|20|21|22
  puCategories.push_back(RuleConst::MISC_FARE_TAG);
  puCategories.push_back(RuleConst::ADVANCE_RESERVATION_RULE);
  puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);
  puCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);
  puCategories.push_back(RuleConst::MINIMUM_STAY_RULE);
  puCategories.push_back(RuleConst::MAXIMUM_STAY_RULE);
  puCategories.push_back(RuleConst::STOPOVER_RULE);
  puCategories.push_back(RuleConst::TRANSFER_RULE);
  puCategories.push_back(RuleConst::SURCHARGE_RULE);

  puCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);
  puCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);
  puCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);
  puCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);

  // Set currency codes
  PricingUtil::determineBaseFare(&axessFp, _trx, axessFp.itin());

  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(
      FPRuleValidation, puCategories, &_trx);

  std::vector<PricingUnit*>::iterator puIt = axessFp.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = axessFp.pricingUnit().end();
  bool passRules = true;

  for (; puIt != puItEnd; ++puIt)
  {
    PricingUnit& prU = **puIt;
    if (_trx.diagnostic().diagnosticType() == Diagnostic555 &&
        !_trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN))
    {
      DiagCollector& dc = *(DCFactory::instance()->create(_trx));
      dc.enable(Diagnostic555);
      dc.printLine();
      if (!DiagnosticUtil::filter(_trx, prU))
      {
        dc << "           PRICING UNIT/FARE USAGE RULE VALIDATION DIAGNOSTICS" << std::endl;
        dc << " " << std::endl;
        dc << "PRICING UNIT " << prU;
        dc << " " << std::endl;
      }
      dc.flushMsg();
    }

    passRules = ruleController.validate(_trx, axessFp, **puIt);
    if (!passRules)
    {
      // diag ?
      return false;
    }
  }

  RuleUtil::getSurcharges(_trx, axessFp);
  return true;
}

void
NetRemitPricing::displayFarePathAxess(const FarePath& fp)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic691)
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(_trx);

    diag->enable(Diagnostic691);

    *diag << " \n*************** JAL AXESS FARE PATH - ANALYSIS ***************\n";
    *diag << "PRICING FARE PATH ------------------------------------------\n";
    *diag << *(fp.originalFarePathAxess());

    *diag << "JAL AXESS FARE PATH ----------------------------------------\n";
    *diag << fp;

    diag->flushMsg();

    diag->disable(Diagnostic691);
  }
}

void
NetRemitPricing::displayNetRemitFarePath(const NetRemitFarePath& netRemitFp)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic691)
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(_trx);

    diag->enable(Diagnostic691);

    if (_trx.getRequest()->ticketingAgent()->axessUser())
    {
      *diag << " \n*************** JAL AXESS FARE PATH - ANALYSIS ***************\n";
      *diag << "PRICING FARE PATH ------------------------------------------\n";
      *diag << *(netRemitFp.originalFarePath());

      *diag << "JAL AXESS FARE PATH ----------------------------------------\n";
    }

    else
    {
      *diag << " \n*************** NET REMIT FARE PATH - ANALYSIS ***************\n";
      *diag << "PRICING FARE PATH ------------------------------------------\n";
      *diag << *(netRemitFp.originalFarePath());

      *diag << "NET REMIT FARE PATH ----------------------------------------\n";
    }
    *diag << netRemitFp;

    diag->flushMsg();

    diag->disable(Diagnostic691);
  }
}

void
NetRemitPricing::calcSurcharges(FarePath& netRemitFp)
{
  std::vector<uint16_t> puCategories;
  puCategories.push_back(RuleConst::SURCHARGE_RULE);

  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(
      FPRuleValidation, puCategories, &_trx);

  std::vector<PricingUnit*>::iterator puIt = netRemitFp.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = netRemitFp.pricingUnit().end();
  for (; puIt != puItEnd; ++puIt)
    ruleController.validate(_trx, netRemitFp, **puIt);

  RuleUtil::getSurcharges(_trx, netRemitFp);
}

bool
NetRemitPricing::isNetRemit()
{
  if (_farePath.selectedNetRemitFareCombo() && !_farePath.tktRestricted() &&
      !_farePath.tfrRestricted())
  {
    const FareUsage* firstFu = _farePath.pricingUnit().front()->fareUsage().front();
    const NegFareRest& negFareRest = firstFu->paxTypeFare()->negotiatedInfo();
    Indicator tktFareDataInd = negFareRest.tktFareDataInd1();
    if (tktFareDataInd == RuleConst::BLANK && TrxUtil::optimusNetRemitEnabled(_trx) &&
        _trx.dataHandle().getVendorType(firstFu->paxTypeFare()->vendor()) == RuleConst::SMF_VENDOR)
    {
      const NegPaxTypeFareRuleData* ruleData = firstFu->paxTypeFare()
                                                   ->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE)
                                                   ->toNegPaxTypeFareRuleData();
      if (ruleData)
      {
        const NegFareRestExt* negFareRestExt = ruleData->negFareRestExt();
        if (negFareRestExt)
          tktFareDataInd = negFareRestExt->fareBasisAmtInd();
      }
    }
    if (tktFareDataInd == RuleConst::NR_VALUE_F) // Fare Basis only, no need to build new fare path
    {
      return (NegotiatedFareRuleUtil::hasFareBoxAmount(&negFareRest) &&
              NegotiatedFareRuleUtil::hasFareBoxNetRemitMethod(&negFareRest));
    }
    return true;
  }

  return false;
}

void
NetRemitPricing::compareNetRemitFareAmount()
{
  if (_farePath.collectedNegFareData() != nullptr && !_trx.getRequest()->isTicketEntry())
  {
    const CollectedNegFareData& negFareData = *(_farePath.collectedNegFareData());

    if (negFareData.bspMethod() == RuleConst::NRR_METHOD_3)
    {
      MoneyAmount cat35NetTotalAmt = negFareData.netTotalAmtCharges();
      cat35NetTotalAmt += negFareData.totalMileageCharges();
      cat35NetTotalAmt += negFareData.otherSurchargeTotalAmt();
      cat35NetTotalAmt += negFareData.cat12SurchargeTotalAmt(); // 12 surcharges
      if (cat35NetTotalAmt - _farePath.netRemitFarePath()->getTotalNUCAmount() > EPSILON)
      {
        _farePath.collectedNegFareData()->trailerMsg() =
            "NET AMOUNT EXCEEDS FARE - VERIFY NET AMOUNT";
      }
    }
  }
}

void
NetRemitPricing::setCat10ForNetFaresMap(FareCombRuleInfoMap& cat10ForNetFaresMap,
                                        PaxTypeFare& paxTypeFare)
{
  const CombinabilityRuleInfo* pCat10 = paxTypeFare.rec2Cat10();
  if (pCat10 == nullptr)
  {
    cat10ForNetFaresMap[&paxTypeFare] = RuleUtil::getCombinabilityRuleInfo(_trx, paxTypeFare);
  }
}

void
NetRemitPricing::getCat10ForNetFares(FareCombRuleInfoMap& cat10ForNetFaresMap)
{
  std::vector<PricingUnit*>::iterator puIt = _farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::iterator puItEnd = _farePath.pricingUnit().end();

  for (; puIt != puItEnd; ++puIt)
  {
    PricingUnit* pu = *puIt;

    // Process only OJ and RT puType
    // process only allowed vendors for HRTC
    if ((PricingUnit::Type::OPENJAW != pu->puType() && PricingUnit::Type::ROUNDTRIP != pu->puType()) ||
        (PricingUnit::Type::ROUNDTRIP == pu->puType() && !(PricingUtil::allowHRTCForVendor(_trx, *pu) ||
                                                     TrxUtil::isAtpcoTTProjectHighRTEnabled(_trx))))
    {
      continue;
    }

    std::vector<FareUsage*>::iterator fuIt = pu->fareUsage().begin();
    std::vector<FareUsage*>::iterator end = pu->fareUsage().end();
    for (; fuIt != end; ++fuIt)
    {
      if (TrxUtil::optimusNetRemitEnabled(_trx) && NetRemitFarePath::isTFDPSCFareAmtFare(*fuIt))
      {
        for (FareUsage::TktNetRemitPscResult& pscResult : (*fuIt)->netRemitPscResults())
          setCat10ForNetFaresMap(cat10ForNetFaresMap,
                                 const_cast<PaxTypeFare&>(*pscResult._resultFare));
      }
      else
      {
        const PaxTypeFare* tktNetRemitFare = (*fuIt)->tktNetRemitFare();
        if (tktNetRemitFare)
          setCat10ForNetFaresMap(cat10ForNetFaresMap, const_cast<PaxTypeFare&>(*tktNetRemitFare));
      }
    }
  }
}
}
