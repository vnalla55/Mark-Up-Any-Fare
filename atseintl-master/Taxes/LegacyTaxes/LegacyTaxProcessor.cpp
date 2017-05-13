// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the  program(s)
//      have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/BSRCurrencyConverter.h"
#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplaySurcharge.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/TaxReissue.h"
#include "Pricing/PricingUtil.h"
#include "Taxes/LegacyFacades/CopyTaxResponse.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/LegacyTaxProcessor.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxDisplayDriver.h"
#include "Taxes/LegacyTaxes/TaxDriver.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxCalculator.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcDisplayDriver.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(validatingCxrOnBadDataFix);
FALLBACK_DECL(fallbackDuplicateVCxrMultiSP);
FALLBACK_DECL(fallbackLegacyTaxProcessorRefactoring);
FALLBACK_DECL(setHistoricalOTA);
FALLBACK_DECL(taxRefundableIndUseCat33Logic)
FALLBACK_DECL(taxRexPricingRefundableInd);
FALLBACK_DECL(markupAnyFareOptimization);
FALLBACK_DECL(ssdsp1511fix)
FALLBACK_DECL(taxRexPricingRefundableIndAllTypes)
FALLBACK_DECL(taxRefundableIndUseD92Date)
FALLBACK_DECL(fallbackMafTaxFix);
FALLBACK_DECL(AF_CAT33_ATPCO)

namespace
{
void
handleThreadException(PricingTrx& trx, TaxMap::TaxFactoryMap& taxFactoryMap)
{
  trx.taxResponse().clear();

  for (std::vector<Itin*>::iterator itinI = trx.itin().begin(); itinI != trx.itin().end(); itinI++)
  {
    if ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR)
      continue;

    (*itinI)->mutableTaxResponses().clear();

    TaxItinerary taxItinerary;
    taxItinerary.initialize(trx, **itinI, taxFactoryMap);

    taxItinerary.accumulator();
  }
}

void
computeTaxesInSingleThread(PricingTrx& trx, TaxMap::TaxFactoryMap& taxFactoryMap)
{
  if (fallback::fallbackLegacyTaxProcessorRefactoring(&trx))
    return handleThreadException(trx, taxFactoryMap);

  trx.taxResponse().clear();

  for(Itin* itin : ItinSelector(trx).get())
  {
    if (itin->errResponseCode() != ErrorResponseException::NO_ERROR)
      continue;

    itin->mutableTaxResponses().clear();

    TaxItinerary taxItinerary;
    taxItinerary.initialize(trx, *itin, taxFactoryMap);
    taxItinerary.accumulator();
  }
}

} // unnamed namespace

LegacyTaxProcessor::LegacyTaxProcessor(const TseThreadingConst::TaskId taskId, Logger& logger)
  : _taskId(taskId), _logger(logger)
{
}

// ----------------------------------------------------------------------------
//
// bool LegacyTaxProcesor::process
//
// Description:   Calculates and adds taxes to PricingTrx
//
// ----------------------------------------------------------------------------
bool
LegacyTaxProcessor::process(PricingTrx& trx)
{
  if (fallback::fallbackLegacyTaxProcessorRefactoring(&trx))
    return process_OLD(trx);

  if (!TrxUtil::isAutomatedRefundCat33Enabled(trx))
  {
    if (ItinSelector(trx).isExcItin())
      return true;
  }

  LOG4CXX_INFO(_logger, "Started Process()");
  TSELatencyData metrics(trx, "TAX PROCESS");

  if (ItinSelector(trx).isTaxInfoReq())
  {
    addTaxInfoResponsesToTrx(trx);
    return true;
  }

  ItinSelector itinSelector(trx);
  if (itinSelector.isExcItin())
  {
    const RexPricingOptions& ro = static_cast<const RexPricingOptions&>(*trx.getOptions());
    for(Itin* itin : itinSelector.get())
    {
      for (FarePath* fp : itin->farePath())
        fp->baseFareCurrency() = ro.excBaseFareCurrency();
    }
  }

  if (!trx.getOptions()->isMOverride())
  {
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);
    if (!taxTrx)
    {
      BSRCurrencyConverter bsrConverter;
      bsrConverter.hasIndirectEquivAmtOverride(trx);
    }
  }

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(trx.dataHandle(), taxFactoryMap);

  TaxCalculator taxCalculator(_taskId, trx, taxFactoryMap);
  try {
    for(Itin* itin : ItinSelector(trx).get())
      taxCalculator.calculateTaxes(*itin);

    taxCalculator.wait();
  }
  catch (boost::thread_interrupted& e)
  {
    LOG4CXX_ERROR(_logger, "thread_interrupted exception has been thrown");
    computeTaxesInSingleThread(trx, taxFactoryMap);
  }

  //
  // Build PAX Requested Diagnostic Display
  //
  if (fallback::validatingCxrOnBadDataFix(&trx))
  {
    TaxDiagnostic taxDiagnostic;
    findLowestFareSolutionAfterTax(trx, taxFactoryMap);
    taxDiagnostic.collectDiagnostics(trx);
  }
  else
  {
    if (taxCalculator.isAnyItinValid())
      findLowestFareSolutionAfterTax(trx, taxFactoryMap);

    TaxDiagnostic taxDiagnostic;
    taxDiagnostic.collectDiagnostics(trx);
  }

  handleValidatingCarrierError(trx);
  if(fallback::markupAnyFareOptimization(&trx))
  {
    handleAdjustedSellingLevelFarePaths(trx);
  }

  addTaxResponsesToTrx(trx);
  LOG4CXX_INFO(_logger, "Finished Leaving Taxes");

  return true;
}

bool
LegacyTaxProcessor::process_OLD(PricingTrx& trx)
{
  if (ItinSelector(trx).isExcItin())
    return true;

  LOG4CXX_INFO(_logger, "Started Process()");
  TSELatencyData metrics(trx, "TAX PROCESS");

  if (!trx.getOptions()->isMOverride())
  {
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);
    if (!taxTrx)
    {
      BSRCurrencyConverter bsrConverter;
      bsrConverter.hasIndirectEquivAmtOverride(trx);
    }
  }

  TseRunnableExecutor pooledExecutor(_taskId);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(trx.dataHandle(), taxFactoryMap);

  std::vector<Itin*>::iterator itinI = trx.itin().begin();

  bool isAnyItinValid = false;
  try
  {
    uint16_t remainingItin = trx.itin().size();
    for (; itinI != trx.itin().end(); ++itinI, --remainingItin)
    {

      if ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR &&
          (*itinI)->errResponseCode() != ErrorResponseException::UNKNOWN_BAGGAGE_CHARGES)
        continue;

      isAnyItinValid = true;

      TaxItinerary* taxItinerary = nullptr;

      // lint --e{413}
      trx.dataHandle().get(taxItinerary);

      taxItinerary->initialize(trx, **itinI, taxFactoryMap);

      TseRunnableExecutor& taskExecutor =
          (remainingItin > 1) ? pooledExecutor : synchronousExecutor;

      taskExecutor.execute(*taxItinerary);
    }
    pooledExecutor.wait();
  }
  catch (boost::thread_interrupted& e)
  {
    LOG4CXX_ERROR(_logger, "thread_interrupted exception has been thrown");
    computeTaxesInSingleThread(trx, taxFactoryMap);
  }

  //
  // Build PAX Requested Diagnostic Display
  //
  if (fallback::validatingCxrOnBadDataFix(&trx))
  {
    TaxDiagnostic taxDiagnostic;
    findLowestFareSolutionAfterTax(trx, taxFactoryMap);
    taxDiagnostic.collectDiagnostics(trx);
  }
  else
  {
    if (isAnyItinValid)
      findLowestFareSolutionAfterTax(trx, taxFactoryMap);

    TaxDiagnostic taxDiagnostic;
    taxDiagnostic.collectDiagnostics(trx);
  }

  handleValidatingCarrierError(trx);

  if(fallback::markupAnyFareOptimization(&trx))
  {
    handleAdjustedSellingLevelFarePaths(trx);
  }

  addTaxResponsesToTrx(trx);
  LOG4CXX_INFO(_logger, "Finished Leaving Taxes");

  return true;
}

void
LegacyTaxProcessor::findLowestFareSolutionAfterTax(PricingTrx& trx, TaxMap::TaxFactoryMap& taxFactoryMap)
{
  if (fallback::fallbackLegacyTaxProcessorRefactoring(&trx))
    return findLowestFareSolutionAfterTax_OLD(trx, taxFactoryMap);

  if (trx.isValidatingCxrGsaApplicable() &&
      (trx.getTrxType() == PricingTrx::PRICING_TRX ||
       trx.getTrxType() == PricingTrx::IS_TRX ||
       trx.getTrxType() == PricingTrx::MIP_TRX ))
  {
    DiagCollector& diag = *(DCFactory::instance()->create(trx));

    for (Itin* itin : ItinSelector(trx).get())
    {
      TaxItinerary taxItinerary;
      taxItinerary.initialize(trx, *itin, taxFactoryMap);

      if (trx.paxType().size() == 1 ||
          trx.altTrxType() == PricingTrx::WPA ||
          trx.altTrxType() == PricingTrx::WP_NOMATCH ||
          trx.isRfbListOfSolutionEnabled() ||
          trx.noPNRPricing())
      {
        findLowestFareSolutionForPaxType(trx, *itin, taxItinerary, diag);
      }
      else
      {
        findLowestFareSolutionForGroupPaxType(trx, *itin, taxItinerary, diag);
      }
    }
  }
  else
  {
    // Non GSA path
    if (!trx.isValidatingCxrGsaApplicable())
    {
      DiagCollector& diag = *(DCFactory::instance()->create(trx));
      for (Itin* itin : ItinSelector(trx).get())
      {
        TaxItinerary taxItinerary;
        taxItinerary.initialize(trx, *itin, taxFactoryMap);
        for (FarePath* fp : itin->farePath())
          setNetRemitAndNetFarePathForValidatingCxr(trx, *itin, *fp, taxItinerary, diag);
      }
    }
  }
}

void
LegacyTaxProcessor::findLowestFareSolutionAfterTax_OLD(PricingTrx& trx, TaxMap::TaxFactoryMap& taxFactoryMap)
{
  if (trx.isValidatingCxrGsaApplicable() &&
      (trx.getTrxType() == PricingTrx::PRICING_TRX ||
       trx.getTrxType() == PricingTrx::IS_TRX ||
       trx.getTrxType() == PricingTrx::MIP_TRX ))
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(trx));

    for (Itin* itin : trx.itin())
    {
      TaxItinerary taxItinerary;
      taxItinerary.initialize(trx, *itin, taxFactoryMap);

      if (trx.paxType().size() == 1 ||
          trx.altTrxType() == PricingTrx::WPA ||
          trx.altTrxType() == PricingTrx::WP_NOMATCH ||
          trx.isRfbListOfSolutionEnabled() ||
          trx.noPNRPricing())
      {
        findLowestFareSolutionForPaxType(trx, *itin, taxItinerary, diag);
      }
      else
      {
        findLowestFareSolutionForGroupPaxType(trx, *itin, taxItinerary, diag);
      }
    }
  }
  else
  {
    // Non GSA path
    if (!trx.isValidatingCxrGsaApplicable())
    {
      DCFactory* factory = DCFactory::instance();
      DiagCollector& diag = *(factory->create(trx));
      for (Itin* itin : trx.itin())
      {
        TaxItinerary taxItinerary;
        taxItinerary.initialize(trx, *itin, taxFactoryMap);
        for (FarePath* fp : itin->farePath())
          setNetRemitAndNetFarePathForValidatingCxr(trx, *itin, *fp, taxItinerary, diag);
      }
    }
  }
}

//-----------------------------------------------------------------------------------------------------
void
LegacyTaxProcessor::findLowestFareSolutionForPaxType(PricingTrx& trx,
                                                     Itin& itin,
                                                     TaxItinerary& taxItinerary,
                                                     DiagCollector& diag)
{
  std::vector<FarePath*> finalFarePaths;
  bool needFPSwap = false;
  for (FarePath* fp : itin.farePath())
  {
    if (!fp->validatingCarriers().empty())
    {
      std::vector<CarrierCode> valCxrWithLowestTotalAmt;
      SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
      std::set<FarePath*> fpSet;

      FarePath* finalFp = nullptr;
      CarrierCode defVCxr, mktCxr;
      bool retVal = false;
      SettlementPlanType primarySp = "";

      if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      {
        // multiple VC from multiple FP possible
        findValCxrWithLowestTotal(trx, *fp, spValCxrsWithLowestTotalAmt, fpSet);
        if (spValCxrsWithLowestTotalAmt.empty())
        {
          LOG4CXX_ERROR(_logger, "spValCxrsWithLowestTotalAmt is empty - skipping solution!");
          continue;
        }

        primarySp = findPrimarySp(spValCxrsWithLowestTotalAmt);
        DefaultValidatingCarrierFinder defValCxrFinder(trx, itin, primarySp);
        retVal = defValCxrFinder.determineDefaultValidatingCarrier(
            spValCxrsWithLowestTotalAmt[primarySp], defVCxr, mktCxr);
      }
      else
      {
        findValCxrWithLowestTotal(trx, *fp, valCxrWithLowestTotalAmt, fpSet); // multiple VC from multiple FP possible
        if (valCxrWithLowestTotalAmt.empty())
        {
          LOG4CXX_ERROR(_logger, "valCxrWithLowestTotalAmt is empty - skipping solution!");
          continue;
        }

        ValidatingCarrierUpdater validatingCarrier(trx);
        retVal = validatingCarrier.determineDefaultValidatingCarrier(itin, valCxrWithLowestTotalAmt, defVCxr, mktCxr);
      }

      if(retVal)
      {
        finalFp = findFarePathWithDefaultValidatingCarrier(trx, taxItinerary, *fp, defVCxr, needFPSwap, diag);
      }
      else
      {
        if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
        {
          throwIfTicketingEntry(trx, spValCxrsWithLowestTotalAmt);
          finalFp = findFarePathWithNODefaultValidatingCarrier(trx, itin, taxItinerary, *fp,
              spValCxrsWithLowestTotalAmt, needFPSwap, diag, primarySp);
        }
        else
        {
          throwIfTicketingEntry(trx, valCxrWithLowestTotalAmt);
          finalFp = findFarePathWithNODefaultValidatingCarrier(trx, itin, taxItinerary, *fp,
              valCxrWithLowestTotalAmt, needFPSwap, diag);
        }
      }

      if(finalFp)
      {
        processFinalFP(trx, *finalFp, spValCxrsWithLowestTotalAmt, valCxrWithLowestTotalAmt,
                       defVCxr, mktCxr, primarySp);

        if (finalFp->adjustedSellingFarePath())
          processFinalFP(trx, *finalFp->adjustedSellingFarePath(), spValCxrsWithLowestTotalAmt,
                         valCxrWithLowestTotalAmt, defVCxr, mktCxr, primarySp);

        finalFarePaths.push_back(finalFp);
      }
    }
    else
    {
      updateDefaultValidatingCarrier(trx, itin, *fp);
      setNetRemitAndNetFarePathForValidatingCxr(trx, itin, *fp, taxItinerary, diag);
    }
  } //end for-each

  if(needFPSwap)
    itin.farePath().swap(finalFarePaths);
}

void LegacyTaxProcessor::processFinalFP(PricingTrx& trx,
                                        FarePath& fPath,
                                        const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                                        std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
                                        CarrierCode& defVCxr,
                                        CarrierCode& mktCxr,
                                        const SettlementPlanType& primarySp)
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    findSpValCxrsForFinalFp(trx, fPath, spValCxrsWithLowestTotalAmt);
    setDefaultAndAlternateValCarriers(trx, fPath, fPath.settlementPlanValidatingCxrs(),
                                      defVCxr, mktCxr, primarySp);
    modifyItinTaxResponseForFinalFarePath(fPath);
  }
  else
    setDefaultAndAlternateValCarriers(trx, fPath, valCxrWithLowestTotalAmt, defVCxr, mktCxr);
}

void LegacyTaxProcessor::findLowestFareSolutionForGroupPaxType(PricingTrx& trx,
                                                               Itin& itin,
                                                               TaxItinerary& taxItinerary,
                                                               DiagCollector& diag)
{
  bool multiVCtrx = true;
  for (FarePath* fp : itin.farePath())
  {
    if (fp->validatingCarriers().empty())
    {
      multiVCtrx = false;
      updateDefaultValidatingCarrier(trx, itin, *fp);
      setNetRemitAndNetFarePathForValidatingCxr(trx, itin, *fp, taxItinerary, diag);
    }
  }

  if (!multiVCtrx || trx.getTrxType() == PricingTrx::IS_TRX)
    return;
  /*APO43473: the farepath could be empty for single ticket solution
   * but multi-tkt farepath could be present. if farepath is empty return  */
  if ( trx.getRequest()->multiTicketActive()  &&
       !(trx.multiTicketMap().empty())  )
  {
     if (itin.farePath().empty())
       return;
  }

  std::vector<CarrierCode> commonValCxrs;
  findCommonValidatingCxrs(trx, itin.farePath(), commonValCxrs);
  if (commonValCxrs.empty())
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.itin().size() > 1)
    {
      itin.errResponseCode() = ErrorResponseException::VALIDATING_CXR_ERROR;
      itin.farePath().clear();
      return;
    }

    throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
        "NO COMMON VAL CXR FOUND FOR MULTIPLE PASSENGER TYPES");
  }

  std::vector<CarrierCode> valCxrWithLowestTotalAmt;
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;

  bool retVal = false;
  CarrierCode defVCxr, mktCxr;
  std::vector<FarePath*> finalFPVect;
  bool needFPSwap = false;
  SettlementPlanType primarySp = "";

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    findValCxrWithLowestGroupTotal(trx, itin, commonValCxrs, spValCxrsWithLowestTotalAmt);
    TSE_ASSERT(!spValCxrsWithLowestTotalAmt.empty());

    primarySp = findPrimarySp(spValCxrsWithLowestTotalAmt);
    DefaultValidatingCarrierFinder defValCxrFinder(trx, itin, primarySp);
    retVal = defValCxrFinder.determineDefaultValidatingCarrier(
        spValCxrsWithLowestTotalAmt[primarySp], defVCxr, mktCxr);
  }
  else
  {
    findValCxrWithLowestGroupTotal(trx, itin, commonValCxrs, valCxrWithLowestTotalAmt);
    TSE_ASSERT(!valCxrWithLowestTotalAmt.empty());

    ValidatingCarrierUpdater validatingCarrier(trx);
    retVal = validatingCarrier.determineDefaultValidatingCarrier(itin,
        valCxrWithLowestTotalAmt, defVCxr, mktCxr);
  }

  if (retVal)
  {
    findGroupFarePathWithDefaultValidatingCarrier(trx, itin, taxItinerary,
        defVCxr, finalFPVect, needFPSwap, diag);
  }
  else
  {
    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      throwIfTicketingEntry(trx, spValCxrsWithLowestTotalAmt);
      findGroupFarePathWithNODefaultValidatingCarrier(trx, itin, taxItinerary,
          spValCxrsWithLowestTotalAmt, finalFPVect, needFPSwap, diag, primarySp);
    }
    else
    {
      throwIfTicketingEntry(trx, valCxrWithLowestTotalAmt);
      findGroupFarePathWithNODefaultValidatingCarrier(trx, itin, taxItinerary,
          valCxrWithLowestTotalAmt, finalFPVect, needFPSwap, diag);
    }
  }

  if (!finalFPVect.empty())
  {
    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
      setDefaultAndAlternateValCarriersForGroupPaxType(trx, finalFPVect, spValCxrsWithLowestTotalAmt, defVCxr, mktCxr, primarySp);
    else
      setDefaultAndAlternateValCarriersForGroupPaxType(trx, finalFPVect, defVCxr, mktCxr);
  }

  if (needFPSwap)
    itin.farePath().swap(finalFPVect);
}

void
LegacyTaxProcessor::updateDefaultValidatingCarrier(PricingTrx& trx, Itin& itin, FarePath& farePath) const
{
  std::vector<CarrierCode> validatingCxrs;
  validatingCxrs.push_back(farePath.itin()->validatingCarrier());

  bool retVal = false;
  CarrierCode defVCxr, mktCxr;
  SettlementPlanType primarySp = "";
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    findSpValCxrsWithLowestTotal(trx, farePath, spValCxrsWithLowestTotalAmt);
    primarySp = findPrimarySp(spValCxrsWithLowestTotalAmt);
    DefaultValidatingCarrierFinder defValCxrFinder(trx, itin, primarySp);
    retVal = defValCxrFinder.determineDefaultValidatingCarrier(validatingCxrs, defVCxr, mktCxr);
  }
  else
  {
    ValidatingCarrierUpdater validatingCarrier(trx);
    retVal = validatingCarrier.determineDefaultValidatingCarrier(itin, validatingCxrs, defVCxr, mktCxr);
  }

  if (retVal)
  {
    farePath.defaultValidatingCarrier() = defVCxr;
    if (defVCxr != mktCxr && !mktCxr.empty())
      farePath.marketingCxrForDefaultValCxr() = mktCxr;

    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      farePath.defaultValCxrPerSp()[primarySp] = defVCxr;
      if (defVCxr != mktCxr && !mktCxr.empty())
        farePath.marketingCxrForDefaultValCxrPerSp()[primarySp] = mktCxr;

      findSpValCxrsForFinalFp(trx, farePath, spValCxrsWithLowestTotalAmt);
      setDefaultPerSp(trx, farePath, farePath.settlementPlanValidatingCxrs(), primarySp);
    }
  }
}

void
LegacyTaxProcessor::setNetRemitAndNetFarePathForValidatingCxr(PricingTrx& trx,
    Itin& itin, FarePath& fp, TaxItinerary& taxItinerary, DiagCollector& diag) const
{
  if (fp.netRemitFarePath() != nullptr)
    taxItinerary.processNetRemitFarePath(&fp, &diag, true);

  if (TrxUtil::isCat35TFSFEnabled(trx) /* Cat35 TFSF */
      && fp.netFarePath() != nullptr /* NetFarePath */)
  {
    std::unique_ptr<ExchangeUtil::RaiiProcessingDate> ptrDateSetter;
    if (trx.isExchangeTrx() &&
        trx.excTrxType() != PricingTrx::PORT_EXC_TRX)
    {
      ptrDateSetter.reset( new ExchangeUtil::RaiiProcessingDate (
        static_cast<RexBaseTrx&>(trx), *fp.netFarePath(), false));
    }

    taxItinerary.processFarePathPerCxr(fp.netFarePath(), &diag);
  }

  if (fallback::markupAnyFareOptimization(&trx))
  {
    if (fp.adjustedSellingFarePath())
      taxItinerary.processFarePathPerCxr(fp.adjustedSellingFarePath(), &diag);
  }
}

void
LegacyTaxProcessor::handleAdjustedSellingLevelFarePaths(PricingTrx& trx)
{
  if (fallback::fallbackLegacyTaxProcessorRefactoring(&trx))
    return handleAdjustedSellingLevelFarePaths_OLD(trx);

  for (Itin* itin : ItinSelector(trx).get())
   for (FarePath* fp : itin->farePath())
     if (fp && fp->adjustedSellingFarePath())
     {
       processAdjustedSellingLevelFarePath(trx, *fp);
     }
}

void
LegacyTaxProcessor::handleAdjustedSellingLevelFarePaths_OLD(PricingTrx& trx)
{
  for (Itin* itin : trx.itin())
   for (FarePath* fp : itin->farePath())
     if (fp && fp->adjustedSellingFarePath())
       processAdjustedSellingLevelFarePath(trx, *fp);
}

void
LegacyTaxProcessor::processAdjustedSellingLevelFarePath(PricingTrx& trx, FarePath& fp) const
{
  const TaxResponse* origTaxResponse = TaxResponse::findFor(&fp);
  if (!origTaxResponse)
  {
    LOG4CXX_ERROR(_logger, "Missing Tax Response");
    return;
  }

  TaxResponse* adjTaxResponse =
    const_cast<TaxResponse*>(TaxResponse::findFor(fp.adjustedSellingFarePath()));
  if (!adjTaxResponse)
    throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION, "ASL TAX ERROR");

  std::vector<TaxRecord*> gstTaxRecords;
  for (TaxRecord* taxRecord : adjTaxResponse->taxRecordVector())
    if (taxRecord->gstTaxInd())
      gstTaxRecords.push_back(taxRecord);

  int gstSize = gstTaxRecords.size();
  int gstIndex = 0;

  adjTaxResponse->taxRecordVector().clear();
  for (TaxRecord* taxRecord : origTaxResponse->taxRecordVector())
  {
    if (!taxRecord->gstTaxInd())
      adjTaxResponse->taxRecordVector().push_back(taxRecord);
    else if (gstIndex < gstSize)
      adjTaxResponse->taxRecordVector().push_back(gstTaxRecords[gstIndex++]);
    else if (fallback::fallbackMafTaxFix(&trx))
      throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION, "ASL TAX ERROR");
  }

  if (!fallback::fallbackMafTaxFix(&trx) && (gstIndex < gstSize)) // in case adj has more GSTs
    for (int i = gstIndex; i < gstSize; ++i)
      adjTaxResponse->taxRecordVector().push_back(gstTaxRecords[i]);

  std::vector<TaxItem*> gstTaxItems;
  for (TaxItem* taxItem : adjTaxResponse->taxItemVector())
    if (taxItem->gstTax())
      gstTaxItems.push_back(taxItem);

  gstSize = gstTaxItems.size();
  gstIndex = 0;

  adjTaxResponse->taxItemVector().clear();
  for (TaxItem* taxItem : origTaxResponse->taxItemVector())
  {
    if (!taxItem->gstTax())
      adjTaxResponse->taxItemVector().push_back(taxItem);
    else if (gstIndex < gstSize)
      adjTaxResponse->taxItemVector().push_back(gstTaxItems[gstIndex++]);
    else if (fallback::fallbackMafTaxFix(&trx))
      throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION, "ASL TAX ERROR");
  }

  if (!fallback::fallbackMafTaxFix(&trx) && (gstIndex < gstSize)) // in case adj has more GSTs
    for (int i = gstIndex; i < gstSize; ++i)
      adjTaxResponse->taxItemVector().push_back(gstTaxItems[i]);
}

//-----------------------------------------------------------------------------------------------------
void
LegacyTaxProcessor::findCommonValidatingCxrs(PricingTrx& trx,
                                             std::vector<FarePath*>& farePathVect,
                                             std::vector<CarrierCode>& res) const
{
  if (!farePathVect.empty())
  {
    std::vector<FarePath*>::iterator itFp = farePathVect.begin();
    std::vector<FarePath*>::iterator itFpEnd = farePathVect.end();
    getValidValCxr(trx, **itFp, res);
    while(++ itFp != itFpEnd)
    {
      std::vector<CarrierCode> tmpVect;
      getValidValCxr(trx, **itFp, tmpVect);
      PricingUtil::intersectCarrierList(res, tmpVect);
    }
  }
}

// Get Val-Cxr for which TaxResponse is available.
// Shopping MIP, may not have TaxResponse for all the VC
// We need to ignore them
void
LegacyTaxProcessor::getValidValCxr(PricingTrx& trx, FarePath& fp, std::vector<CarrierCode>& vcVect) const
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    for (const auto& it : fp.valCxrTaxResponses())
      vcVect.push_back(it.first);
  }
  else
  {
    for (const auto& it : fp.valCxrTaxResponseMap())
      vcVect.push_back(it.first);
  }

  for (FarePath* f : fp.gsaClonedFarePaths())
    if (f)
      getValidValCxr(trx, *f, vcVect);
}

//-----------------------------------------------------------------------------------------------------
void
LegacyTaxProcessor::findValCxrWithLowestTotal(PricingTrx& trx,
    FarePath& farePath,
    std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
    std::set<FarePath*>& fpSet)
{
  MoneyAmount currentLowTaxTotal = std::numeric_limits<MoneyAmount>::max();

  findValCxrWithLowestTotal(trx, farePath, valCxrWithLowestTotalAmt, fpSet, currentLowTaxTotal);

  for (FarePath* fp : farePath.gsaClonedFarePaths())
     findValCxrWithLowestTotal(trx, *fp, valCxrWithLowestTotalAmt, fpSet, currentLowTaxTotal);
}

void
LegacyTaxProcessor::findValCxrWithLowestTotal(PricingTrx& trx,
                                              FarePath& farePath,
                                              SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                                              std::set<FarePath*>& fpSet)
{
  MoneyAmount currentLowTaxTotal = std::numeric_limits<MoneyAmount>::max();
  findValCxrWithLowestTotal(trx, farePath, spValCxrsWithLowestTotalAmt, fpSet, currentLowTaxTotal);
  for (FarePath* fp : farePath.gsaClonedFarePaths())
     findValCxrWithLowestTotal(trx, *fp, spValCxrsWithLowestTotalAmt, fpSet, currentLowTaxTotal);
}

void LegacyTaxProcessor::findValCxrWithLowestTotal(PricingTrx& trx,
                                                   FarePath& farePath,
                                                   std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
                                                   std::set<FarePath*>& fpSet,
                                                   MoneyAmount& currentLowTotal)
{
  if (farePath.valCxrTaxResponseMap().empty())
    return;

  std::map<const CarrierCode, TaxResponse*>::iterator it =
      farePath.valCxrTaxResponseMap().begin();
  std::map<const CarrierCode, TaxResponse*>::iterator itEnd =
      farePath.valCxrTaxResponseMap().end();
  for (; it != itEnd; ++it)
  {
    if (it->second)
    {
      Money trt(NUC);
      it->second->getTaxRecordTotal(trt);
      MoneyAmount nextTotalAmt = trt.value() + farePath.getTotalNUCAmount();

      const double diff = currentLowTotal - nextTotalAmt;

      if (diff > EPSILON)
      {
        //found new low
        currentLowTotal = nextTotalAmt;
        valCxrWithLowestTotalAmt.clear();
        valCxrWithLowestTotalAmt.push_back(it->first);
        fpSet.clear();
        fpSet.insert(&farePath);
      }
      else if (fabs(diff) < EPSILON)
      {
        //equal amount, this cxr is also a candidate
        valCxrWithLowestTotalAmt.push_back(it->first);
        fpSet.insert(&farePath);
      }
    }
  }
}

void LegacyTaxProcessor::findValCxrWithLowestTotal(
    PricingTrx& trx,
    FarePath& farePath,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
    std::set<FarePath*>& fpSet,
    MoneyAmount& currentLowTotal)
{
  for (auto& it : farePath.valCxrTaxResponses())
  {
    for (TaxResponse* taxResp : it.second)
    {
      Money trt(NUC);
      taxResp->getTaxRecordTotal(trt);
      MoneyAmount nextTotalAmt = trt.value() + farePath.getTotalNUCAmount();

      const double diff = currentLowTotal - nextTotalAmt;

      if (diff > EPSILON)
      {
        //found new low
        currentLowTotal = nextTotalAmt;

        spValCxrsWithLowestTotalAmt.clear();
        for (const SettlementPlanType& sp : taxResp->settlementPlans())
          spValCxrsWithLowestTotalAmt[sp].push_back(it.first);

        fpSet.clear();
        fpSet.insert(&farePath);
      }
      else if (fabs(diff) < EPSILON)
      {
        //equal amount, this cxr is also a candidate
        for (const SettlementPlanType& sp : taxResp->settlementPlans())
          spValCxrsWithLowestTotalAmt[sp].push_back(it.first);
        fpSet.insert(&farePath);
      }
    }
  }
}

void
LegacyTaxProcessor::findValCxrWithLowestGroupTotal(PricingTrx& trx,
                                                   Itin& itin,
                                                   const std::vector<CarrierCode>& commonValCxrs,
                                                   std::vector<CarrierCode>& valCxrWithLowestTotalAmt)
{
  std::map< MoneyAmount, std::vector<CarrierCode>, compareMoneyAmount> priceToCxrs;
  for (const CarrierCode& cxr : commonValCxrs)
  {
    MoneyAmount taxAmt = 0.0;
    for(FarePath* fp : itin.farePath())
    {
      FarePath* fpath = findFarePathWithValidatingCarrier(trx, *fp, cxr);
      if(fpath)
        taxAmt += (fpath->getTotalNUCAmount() + fpath->getTaxMoneyAmount(cxr)) *
                  fpath->paxType()->number();
    }

    priceToCxrs[taxAmt].push_back(cxr);
  }

  if(!priceToCxrs.empty())
    valCxrWithLowestTotalAmt = priceToCxrs.begin()->second;
}

void
LegacyTaxProcessor::findValCxrWithLowestGroupTotal(
    PricingTrx& trx,
    Itin& itin,
    const std::vector<CarrierCode>& commonValCxrs,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt)
{
  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;
  for (const CarrierCode& cxr : commonValCxrs)
  {
    // Find total amount for a given validating carrier across FPs
    SettlementPlanTaxAmountGroupMap spTaxAmtGroup;
    FarePath* finalFp = nullptr;
    for (FarePath* fp : itin.farePath())
    {
      if ((finalFp = findFarePathWithValidatingCarrier(trx, *fp, cxr)) != nullptr)
      {
        auto it = finalFp->valCxrTaxResponses().find(cxr);
        if (it != finalFp->valCxrTaxResponses().end())
          findSpTotalAmount(*finalFp, it->second, spTaxAmtGroup);
      }
    }

    ValidatingCxrTotalAmount valCxrTotalAmt;
    processSettlementPlanTaxData(valCxrTotalAmt.totalAmount,
        valCxrTotalAmt.spTaxAmountCol,
        spTaxAmtGroup);
    valCxrTotalAmtPerCxr[cxr] = valCxrTotalAmt;
  }

  if (!valCxrTotalAmtPerCxr.empty())
    processValidatingCxrTotalAmount(valCxrTotalAmtPerCxr, spValCxrsWithLowestTotalAmt);
}

FarePath*
LegacyTaxProcessor::findFarePathWithValidatingCarrier(PricingTrx& trx, FarePath& farePath, const CarrierCode& cxr)
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (farePath.valCxrTaxResponses().find(cxr) != farePath.valCxrTaxResponses().end())
      return &farePath;

    for (FarePath* fp : farePath.gsaClonedFarePaths())
      if (fp && fp->valCxrTaxResponses().find(cxr) != fp->valCxrTaxResponses().end())
        return fp;
  }
  else
  {
    if (farePath.valCxrTaxResponseMap().find(cxr) != farePath.valCxrTaxResponseMap().end())
      return &farePath;

    for (FarePath* fp : farePath.gsaClonedFarePaths())
      if (fp->valCxrTaxResponseMap().find(cxr) != fp->valCxrTaxResponseMap().end())
        return fp;
  }

  return nullptr;
}

FarePath*
LegacyTaxProcessor::findFarePathWithDefaultValidatingCarrier(
    PricingTrx& trx,
    TaxItinerary& taxItinerary,
    FarePath& fp,
    CarrierCode& defVCxr,
    bool& needFPSwap,
    DiagCollector& diag)
{
  FarePath* finalFp = findFarePathWithValidatingCarrier(trx, fp, defVCxr);
  if(!finalFp)
    return nullptr;

  if(finalFp != &fp)
  {
    needFPSwap = true;
    finalFp->copyBaggageDataFrom(fp);
  }
  else
    finalFp->gsaClonedFarePaths().clear();

  if(finalFp->netRemitFarePath() != 0)
  {
    finalFp->defaultValidatingCarrier() = defVCxr;
    taxItinerary.processNetRemitFarePath(finalFp, &diag, true);
  }

  if (TrxUtil::isCat35TFSFEnabled(trx) &&     // Cat35 TFSF
      finalFp->netFarePath() != 0)            // NetFarePath
  {
    std::unique_ptr<ExchangeUtil::RaiiProcessingDate> ptrDateSetter;
    if (trx.isExchangeTrx() &&
        trx.excTrxType() != PricingTrx::PORT_EXC_TRX)
    {
      ptrDateSetter.reset( new ExchangeUtil::RaiiProcessingDate (
        static_cast<RexBaseTrx&>(trx), *finalFp->netFarePath(), false));
    }

    finalFp->itin()->validatingCarrier() = defVCxr;
    taxItinerary.processFarePathPerCxr(finalFp->netFarePath(), &diag);
  }

  if (fallback::markupAnyFareOptimization(&trx))
  {
    if (finalFp->adjustedSellingFarePath())
    {
      finalFp->defaultValidatingCarrier() = defVCxr;
      taxItinerary.processFarePathPerCxr(finalFp->adjustedSellingFarePath(), &diag);
    }
  }
  return finalFp;
}

void
LegacyTaxProcessor::findGroupFarePathWithDefaultValidatingCarrier(PricingTrx& trx,
                                          Itin& itin,
                                          TaxItinerary& taxItinerary,
                                          CarrierCode& defVCxr,
                                          std::vector<FarePath*>& finalFPVect,
                                          bool& needFPSwap,
                                          DiagCollector& diag)
{
  for (FarePath* fp : itin.farePath())
  {
    FarePath* finalFp = findFarePathWithDefaultValidatingCarrier(
        trx, taxItinerary, *fp, defVCxr, needFPSwap, diag);
    if(finalFp)
      finalFPVect.push_back(finalFp);
  }
}

FarePath*
LegacyTaxProcessor::findFarePathWithNODefaultValidatingCarrier(
    PricingTrx& trx,
    Itin& itin,
    TaxItinerary& taxItinerary,
    FarePath& fp,
    std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
    bool& needFPSwap,
    DiagCollector& diag)
{
  // Since no default VC, picking up solution for the first val-cxr
  // TaxResponse is also picked up for the first val-cxr
  // but adding all the val carriers as optional
  //
  CarrierCode cxr = valCxrWithLowestTotalAmt[0];
  FarePath* finalFp = findFarePathWithValidatingCarrier(trx, fp, cxr);
  if(finalFp)
  {
    finalFp->validatingCarriers() = valCxrWithLowestTotalAmt;
    TaxResponse* tr = finalFp->valCxrTaxResponseMap()[cxr];
    collectTaxAndBaggageForAdditionalFPs(trx, itin, taxItinerary, diag, finalFp, fp, tr, cxr, needFPSwap);
  }

  itin.validatingCarrier() = CarrierCode();
  return finalFp;
}

FarePath*
LegacyTaxProcessor::findFarePathWithNODefaultValidatingCarrier(
    PricingTrx& trx,
    Itin& itin,
    TaxItinerary& taxItin,
    FarePath& fp,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
    bool& needFPSwap,
    DiagCollector& diag,
    const SettlementPlanType& primarySp)
{
  // Since no default VC, picking up solution for the first val-cxr
  // TaxResponse is also picked up for the first val-cxr
  // but adding all the val carriers as optional

  const std::vector<CarrierCode>& valCxrWithLowestTotalAmt = spValCxrsWithLowestTotalAmt[primarySp];
  const CarrierCode& cxr = valCxrWithLowestTotalAmt.front();
  FarePath* finalFp = findFarePathWithValidatingCarrier(trx, fp, cxr);
  if (finalFp)
  {
    copyAllValCxrsWithNoDefaultValCxr(*finalFp, spValCxrsWithLowestTotalAmt);
    TaxResponse* tr = getTaxResponseForSp(finalFp->valCxrTaxResponses()[cxr], primarySp);
    collectTaxAndBaggageForAdditionalFPs(trx, itin, taxItin, diag, finalFp, fp, tr, cxr, needFPSwap);
  }

  itin.validatingCarrier() = CarrierCode();
  return finalFp;
}

void
LegacyTaxProcessor::collectTaxAndBaggageForAdditionalFPs(
    PricingTrx& trx,
    Itin& itin,
    TaxItinerary& taxItinerary,
    DiagCollector& diag,
    FarePath* finalFp,
    FarePath& fp,
    TaxResponse* taxResponse,
    const CarrierCode& cxr,
    bool& needFPSwap) const
{
  if(finalFp != &fp)
  {
    needFPSwap = true;
    finalFp->copyBaggageDataFrom(fp);
  }
  else
  {
    finalFp->gsaClonedFarePaths().clear();
  }

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    setTaxResponseInItin(trx, *finalFp, taxResponse);
  }
  else
  {
    if (taxResponse && !taxResponse->taxRecordVector().empty())
      itin.mutableTaxResponses().push_back(taxResponse);
  }

  if(finalFp->netRemitFarePath() != 0 )
    taxItinerary.processNetRemitFarePath(finalFp, &diag, true);

  if (TrxUtil::isCat35TFSFEnabled(trx) &&     // Cat35 TFSF
      finalFp->netFarePath() != 0)            // NetFarePath
  {
    std::unique_ptr<ExchangeUtil::RaiiProcessingDate> ptrDateSetter;
    if (trx.isExchangeTrx() &&
        trx.excTrxType() != PricingTrx::PORT_EXC_TRX)
    {
      ptrDateSetter.reset( new ExchangeUtil::RaiiProcessingDate (
        static_cast<RexBaseTrx&>(trx), *finalFp->netFarePath(), false));
    }

    finalFp->itin()->validatingCarrier() = cxr;
    taxItinerary.processFarePathPerCxr(finalFp->netFarePath(), &diag);
  }

  if (fallback::markupAnyFareOptimization(&trx))
  {
    if (finalFp->adjustedSellingFarePath())
    {
      finalFp->itin()->validatingCarrier() = cxr;
      taxItinerary.processFarePathPerCxr(finalFp->adjustedSellingFarePath(), &diag);
    }
  }
}

void
LegacyTaxProcessor::findGroupFarePathWithNODefaultValidatingCarrier(
    PricingTrx& trx,
    Itin& itin,
    TaxItinerary& taxItinerary,
    std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
    std::vector<FarePath*>& finalFPVect,
    bool& needFPSwap,
    DiagCollector& diag)
{
  // Since no default VC, picking up solution for the first val-cxr
  // TaxResponse is also picked up for the first val-cxr
  // but adding all the val carriers as optional
  //
  for (FarePath* fp : itin.farePath())
  {
    FarePath* finalFp = findFarePathWithNODefaultValidatingCarrier(trx, itin, taxItinerary,
        *fp, valCxrWithLowestTotalAmt,
        needFPSwap, diag);
    if(finalFp)
      finalFPVect.push_back(finalFp);
  }
}

void
LegacyTaxProcessor::findGroupFarePathWithNODefaultValidatingCarrier(
    PricingTrx& trx,
    Itin& itin,
    TaxItinerary& taxItinerary,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
    std::vector<FarePath*>& finalFPVect,
    bool& needFPSwap,
    DiagCollector& diag,
    const SettlementPlanType& primarySp)
{
  // Since no default VC, picking up solution for the first val-cxr
  // TaxResponse is also picked up for the first val-cxr
  // but adding all the val carriers as optional
  //
  for (FarePath* fp : itin.farePath())
  {
    FarePath* finalFp = findFarePathWithNODefaultValidatingCarrier(
        trx,
        itin,
        taxItinerary,
        *fp,
        spValCxrsWithLowestTotalAmt,
        needFPSwap,
        diag,
        primarySp);
    if(finalFp)
      finalFPVect.push_back(finalFp);
  }
}

//-----------------------------------------------------------------------------------------------------

void
LegacyTaxProcessor::setDefaultAndAlternateValCarriersForGroupPaxType(
    PricingTrx& trx,
    std::vector<FarePath*>& finalFPVect,
    const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
    CarrierCode& defVCxr,
    CarrierCode& mktCxr,
    const SettlementPlanType& primarySp)
{
  // After selecting the farePath, we need to find the common VC again across
  // multiple PaxType's FarePath
  std::vector<CarrierCode> commonValCxrs;
  findCommonValidatingCxrs(trx, finalFPVect, commonValCxrs);

  TSE_ASSERT(!commonValCxrs.empty()); // should never be empty

  for (FarePath* fp : finalFPVect)
  {
    //@todo Do we need fallback here?
    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      findSpValCxrsForFinalFp(trx, *fp, spValCxrsWithLowestTotalAmt, commonValCxrs);
      setDefaultAndAlternateValCarriers(trx, *fp, fp->settlementPlanValidatingCxrs(), defVCxr, mktCxr, primarySp);
      if (fp->adjustedSellingFarePath())
        setDefaultAndAlternateValCarriers(trx, *fp->adjustedSellingFarePath(), fp->settlementPlanValidatingCxrs(), defVCxr, mktCxr, primarySp);
    }
    else
    {
      setDefaultAndAlternateValCarriers(trx, *fp, commonValCxrs, defVCxr, mktCxr);
      if (fp->adjustedSellingFarePath())
        setDefaultAndAlternateValCarriers(trx, *fp->adjustedSellingFarePath(), commonValCxrs, defVCxr, mktCxr);
    }
  }
}

void
LegacyTaxProcessor::setDefaultAndAlternateValCarriersForGroupPaxType(
    PricingTrx& trx,
    std::vector<FarePath*>& finalFPVect,
    CarrierCode& defVCxr,
    CarrierCode& mktCxr)
{
  // After selecting the farePath, we need to find the common VC again across
  // multiple PaxType's FarePath
  std::vector<CarrierCode> commonValCxrs;
  findCommonValidatingCxrs(trx, finalFPVect, commonValCxrs);

  TSE_ASSERT(!commonValCxrs.empty()); // should never be empty

  for (FarePath* fp : finalFPVect)
  {
    setDefaultAndAlternateValCarriers(trx, *fp, commonValCxrs, defVCxr, mktCxr);
    if (fp->adjustedSellingFarePath())
      setDefaultAndAlternateValCarriers(trx, *fp->adjustedSellingFarePath(), commonValCxrs, defVCxr, mktCxr);
  }
}

void
LegacyTaxProcessor::setDefaultAndAlternateValCarriers(
    PricingTrx& trx,
    FarePath& farePath,
    std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
    CarrierCode& defVCxr,
    CarrierCode& mktCxr)
{
  // Set Default Cxr
  farePath.defaultValidatingCarrier() = defVCxr;
  farePath.itin()->validatingCarrier() = defVCxr;

  if((defVCxr != mktCxr) && !mktCxr.empty())
    farePath.marketingCxrForDefaultValCxr() = mktCxr;

  // Set Alternate Cxr
  //
  farePath.validatingCarriers().clear();
  if( farePath.valCxrTaxResponseMap().size() == 1)
  {
    TaxResponse* taxResponse = farePath.valCxrTaxResponseMap().begin()->second;
    if(!taxResponse->taxRecordVector().empty())
      farePath.itin()->mutableTaxResponses().push_back(taxResponse);
  }
  else if(! defVCxr.empty())
  {
    FarePath::ValCxrTaxResponseMap::iterator it = farePath.valCxrTaxResponseMap().find(defVCxr);
    if(it != farePath.valCxrTaxResponseMap().end())
    {
      TaxResponse* finalTaxResponse = it->second;
      if(!finalTaxResponse->taxRecordVector().empty())
      {
        farePath.itin()->mutableTaxResponses().push_back(finalTaxResponse);
      }
      farePath.valCxrTaxResponseMap().erase(it);

      for (CarrierCode& vc : valCxrWithLowestTotalAmt)
      {
        if(vc == defVCxr)
          continue;

        FarePath::ValCxrTaxResponseMap::iterator it = farePath.valCxrTaxResponseMap().find(vc);
        if(it != farePath.valCxrTaxResponseMap().end())
        {
          TaxResponse* taxResponse = it->second;
          if (sameZeroTaxes(*finalTaxResponse, *taxResponse) ||
              sameTaxes(*finalTaxResponse, *taxResponse))
          {
            // these are possible alternate VC IF other PaxType also have it
            if (!farePath.netRemitFarePath())
              farePath.validatingCarriers().push_back(vc);
          }
        }
      }
      farePath.valCxrTaxResponseMap().clear();
    }
  }
  else
  {
    farePath.validatingCarriers() = valCxrWithLowestTotalAmt;
  }
}

void
LegacyTaxProcessor::setDefaultAndAlternateValCarriers(
    PricingTrx& trx,
    FarePath& farePath,
    SettlementPlanValCxrsMap& finalSpValCxrsWithLowestTotalAmt,
    const CarrierCode& defVCxr,
    const CarrierCode& mktCxr,
    const SettlementPlanType& primarySp)
{
  // Set Default Cxr
  farePath.defaultValidatingCarrier() = defVCxr;
  farePath.itin()->validatingCarrier() = defVCxr;

  // Set Marketing Cxr
  if((defVCxr != mktCxr) && !mktCxr.empty())
  {
    farePath.marketingCxrForDefaultValCxrPerSp()[primarySp] = mktCxr;
    // farePath.marketingCxrForDefaultValCxr is used in single settlement plan path
    farePath.marketingCxrForDefaultValCxr() = mktCxr;
  }

  // Set Default per Sp
  farePath.defaultValCxrPerSp()[primarySp] = defVCxr;
  setDefaultPerSp(trx, farePath, finalSpValCxrsWithLowestTotalAmt, primarySp);

  // Set Alternate Cxr
  farePath.validatingCarriers().clear();
  if (farePath.valCxrTaxResponses().size() == 1) //no alt valcxrs
  {
    TaxResponse* taxResponse = getTaxResponseForSp(farePath.valCxrTaxResponses().begin()->second, primarySp);
    setTaxResponseInItin(trx, farePath, taxResponse);
  }
  else if (!defVCxr.empty())
  {
    auto it = farePath.valCxrTaxResponses().find(defVCxr);
    if (it != farePath.valCxrTaxResponses().end())
    {
      TaxResponse* finalTaxResponse = getTaxResponseForSp(it->second, primarySp);
      setTaxResponseInItin(trx, farePath, finalTaxResponse);

      farePath.valCxrTaxResponses().erase(it);
      setAlternatePerSp(trx, farePath, finalTaxResponse, finalSpValCxrsWithLowestTotalAmt);
      farePath.valCxrTaxResponses().clear();
    }
  }
  else
  {
    copyAllValCxrsWithNoDefaultValCxr(farePath, finalSpValCxrsWithLowestTotalAmt);
  }
}

//-----------------------------------------------------------------------------------------------------
bool
LegacyTaxProcessor::sameZeroTaxes(const TaxResponse& taxRspCurrent, const TaxResponse& taxRspInMap) const
{
   // special code when all taxes are zero
   Money currentTaxTotal(NUC);
   taxRspCurrent.getTaxRecordTotal(currentTaxTotal);

   Money mapTaxTotal(NUC);
   taxRspInMap.getTaxRecordTotal(mapTaxTotal);

   if(currentTaxTotal.value() < EPSILON  && mapTaxTotal.value() < EPSILON )
     return true;

   return false;
}

bool
LegacyTaxProcessor::sameTaxes(const TaxResponse& taxRspCurrent, const TaxResponse& taxRspInMap) const
{
  // test for identical Tax Records in both TaxResponses
  if(taxRspCurrent.taxRecordVector().size() != taxRspInMap.taxRecordVector().size())
  {
    int current = 0;
    int stored = 0;
    for (const TaxRecord* taxRecordC : taxRspCurrent.taxRecordVector())
    {
      if(fabs(taxRecordC->getTaxAmount()) > EPSILON)
        ++ current;
    }

    for (const TaxRecord* taxRecordC : taxRspInMap.taxRecordVector())
    {
      if(fabs(taxRecordC->getTaxAmount()) > EPSILON)
        ++ stored;
    }

    if (current != stored)
      return false;
  }

  bool found = false;
  for (const TaxRecord* taxRecordC : taxRspCurrent.taxRecordVector())
  {
    if(fabs(taxRecordC->getTaxAmount()) < EPSILON)
      continue;

    found = false;
    for (const TaxRecord* taxRecordM : taxRspInMap.taxRecordVector())
    {
      if(fabs(taxRecordM->getTaxAmount()) < EPSILON)
        continue;

      if (0 == strncmp(taxRecordC->taxCode().c_str(), taxRecordM->taxCode().c_str(), 2) &&
          fabs(taxRecordC->getTaxAmount() - taxRecordM->getTaxAmount()) < EPSILON)
      {
        found = true;
        break;
      }
    }

    if(!found)
      break;
  }
  return found;
}

//-----------------------------------------------------------------------------------------------------
void
LegacyTaxProcessor::throwIfTicketingEntry(PricingTrx& trx, std::vector<CarrierCode>& valCxrWithLowestTotalAmt)
{

  if (trx.getRequest()->isTicketEntry())
  {
    // Ticketing Entry means there is only one Itin and one solution to return
    // therefore, we can throw without worrying about other solutions.
    std::string valCxr;
    for (const CarrierCode& vcr : valCxrWithLowestTotalAmt)
      valCxr = valCxr + vcr.c_str() + " ";

    std::string message = "MULTIPLE VALIDATING CARRIER OPTIONS - " + valCxr;
    throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, message.c_str());
  }
}

void
LegacyTaxProcessor::throwIfTicketingEntry(
    PricingTrx& trx,
    const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt)
{
  if (trx.getRequest()->isTicketEntry())
  {
    // Ticketing Entry means there is only one Itin and one solution to return
    // therefore, we can throw without worrying about other solutions.
    std::string valCxr;
    for (const auto& it : spValCxrsWithLowestTotalAmt)
      for (const CarrierCode& vcr : it.second)
        valCxr = valCxr + vcr.c_str() + " ";

    std::string message = "MULTIPLE VALIDATING CARRIER OPTIONS - " + valCxr;
    throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, message.c_str());
  }
}

void
LegacyTaxProcessor::handleValidatingCarrierError(PricingTrx& trx)
{
  if (trx.isValidatingCxrGsaApplicable() &&
      !(trx.getRequest()->isMultiTicketRequest()) &&
      !trx.itin().empty())
  {
    if(trx.isExchangeTrx())
      return;

    if (trx.getTrxType() == PricingTrx::PRICING_TRX &&
        oneFpPerPaxType(trx, *trx.itin()[0]))
    {
      if(!trx.itin()[0]->farePath().empty())
      {
        FarePath* fp = trx.itin()[0]->farePath().front();
        if(fp && isNetRemitFPNotValid(fp))
        {
          // throw an error
          std::string valCxr;
          for (const CarrierCode& vcr : fp->validatingCarriers())
            valCxr = valCxr + vcr.c_str() + " ";

          std::string message;
          if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
            message = "MULTIPLE VALIDATING CARRIER OPTIONS - SPECIFY #A OR #VM";
          else
            message = "MULTIPLE VALIDATING CARRIER OPTIONS - " + valCxr;

          throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, message.c_str());
        }
      }
    }
    else
      if (trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        std::vector<Itin*>::iterator itinI = trx.itin().begin();
        std::vector<Itin*>::iterator itinE = trx.itin().end();
        for (; itinI != itinE; ++itinI )
        {
          Itin& itin = **itinI;
          if (itin.errResponseCode() != ErrorResponseException::NO_ERROR ||
              itin.farePath().empty())
            continue;

          std::vector<FarePath*>::const_iterator fpI = itin.farePath().begin();
          std::vector<FarePath*>::const_iterator fpE = itin.farePath().end();
          uint32_t fpNotActive = 0;
          for(; fpI != fpE; ++ fpI)
          {
            if(isNetRemitFPNotValid(*fpI))
            {
              ++fpNotActive;
            }
          }
          if(fpNotActive == itin.farePath().size())
          {
            itin.errResponseCode() = ErrorResponseException::VALIDATING_CXR_ERROR;
            itin.farePath().clear();
          }
        }
      }
  }
}

bool
LegacyTaxProcessor::oneFpPerPaxType(PricingTrx& trx, Itin& itin) const
{
  if (itin.farePath().size() == 1)
    return true;

  int farePaths = 0;
  for (PaxType* pT : trx.paxType())
  {
    farePaths = 0;
    for (FarePath* fp : itin.farePath())
    {
      if(fp->processed() && fp->paxType() == pT )
        ++farePaths;
    }

    if (farePaths > 1)
      return false;
  }
  return true;
}

void
LegacyTaxProcessor::handleTaxResponsesAllItins(PricingTrx& trx)
{
  if (fallback::fallbackLegacyTaxProcessorRefactoring(&trx))
    return handleTaxResponsesAllItins_OLD(trx);

  if( trx.isValidatingCxrGsaApplicable() &&
      (trx.getTrxType() == PricingTrx::PRICING_TRX ||
       trx.getTrxType() == PricingTrx::MIP_TRX ))
  {
    trx.taxResponse().clear();

    for (Itin* itin : ItinSelector(trx).get())
    {

      if (itin->errResponseCode() != ErrorResponseException::NO_ERROR ||
          itin->getTaxResponses().empty())
        continue;

      trx.taxResponse().insert(trx.taxResponse().end(),
                               itin->getTaxResponses().begin(),
                               itin->getTaxResponses().end());
    }
  }
}

void
LegacyTaxProcessor::handleTaxResponsesAllItins_OLD(PricingTrx& trx)
{
  if( trx.isValidatingCxrGsaApplicable() &&
      (trx.getTrxType() == PricingTrx::PRICING_TRX ||
       trx.getTrxType() == PricingTrx::MIP_TRX ))
  {
    trx.taxResponse().clear();

    std::vector<Itin*>::const_iterator itinI = trx.itin().begin();
    std::vector<Itin*>::const_iterator itinE = trx.itin().end();
    for (; itinI != itinE; ++itinI )
    {
      Itin& itin = **itinI;

      if (itin.errResponseCode() != ErrorResponseException::NO_ERROR ||
          itin.getTaxResponses().empty())
        continue;

      trx.taxResponse().insert(trx.taxResponse().end(),
                               itin.getTaxResponses().begin(),
                               itin.getTaxResponses().end());
    }
  }
}

void
LegacyTaxProcessor::addTaxInfoResponsesToTrx(PricingTrx& trx) const
{
  if (fallback::taxRexPricingRefundableInd(&trx))
    return;

  PricingRequest* req = trx.getRequest();

  ItinSelector itinSelector(trx);
  bool isRefundabeRequired = itinSelector.isExcItin() || itinSelector.isTaxInfoReq();
  if (!isRefundabeRequired && !fallback::taxRexPricingRefundableIndAllTypes(&trx) &&
      !itinSelector.isRefundTrx() && !itinSelector.isExchangeTrx() &&
      trx.getTaxInfoResponse().empty() && req && !req->getTaxRequestedInfo().empty())
    isRefundabeRequired = true;

  if (isRefundabeRequired)
  {
    if (!fallback::ssdsp1511fix(&trx))
    {
      if (!req || req->getTaxRequestedInfo().empty())
        return;
    }
    else
    {
      if (!req)
        return;
    }

    TaxResponse* taxResponse = nullptr;
    trx.dataHandle().get(taxResponse);
    if (!taxResponse)
      return;


    ExchangeUtil::RaiiProcessingDate raiiDate(trx, fallback::taxRefundableIndUseD92Date(&trx));
    raiiDate.useOriginalTktIssueDT();
    DateTime originalTicketDate = trx.dataHandle().ticketDate();
    bool skip33 = !TrxUtil::isAutomatedRefundCat33Enabled(trx) || !itinSelector.isRefundTrx();
    for(const TaxCode& taxCode: req->getTaxRequestedInfo())
    {
      TaxItem *taxItem = nullptr;
      trx.dataHandle().get(taxItem);
      if (!taxItem)
      {
        taxResponse->taxItemVector().clear();
        return;
      }

      taxItem->taxCode() = taxCode;

      if (!fallback::taxRefundableIndUseCat33Logic(&trx))
      {
        TaxReissueSelector reissues(
          trx.dataHandle().getTaxReissue(taxCode, originalTicketDate));

        Cat33TaxReissue cat33TaxReissue(
            reissues.getTaxReissue(TaxReissueSelector::LEGACY_TAXES_TAX_TYPE, "", skip33));

        taxItem->setRefundableTaxTag(cat33TaxReissue.getRefundableTaxTag());
      }
      else
      {
        const std::vector<TaxReissue*>& reissues =
          trx.dataHandle().getTaxReissue(taxCode, originalTicketDate);
        auto reissue = std::find_if(reissues.begin(), reissues.end(),
              [](const TaxReissue* reissue) { return reissue->cat33onlyInd() != 'Y'; });

        if (reissue == reissues.end() || (*reissue)->refundInd() != 'N')
          taxItem->setRefundableTaxTag('Y');
        else
          taxItem->setRefundableTaxTag('N');
      }
      taxResponse->taxItemVector().push_back(taxItem);
    }

    if (fallback::taxRexPricingRefundableIndAllTypes(&trx) || !taxResponse->taxItemVector().empty())
      trx.addToTaxInfoResponse(taxResponse);
  }
}

void
LegacyTaxProcessor::addTaxResponsesToTrx(PricingTrx& trx)
{
  if (fallback::fallbackLegacyTaxProcessorRefactoring(&trx))
    return handleTaxResponsesAllItins(trx);

  addTaxInfoResponsesToTrx(trx);

  if (!fallback::AF_CAT33_ATPCO(&trx))
  {
    ItinSelector itinSelector(trx);
    CopyTaxResponse copyTaxResponse(trx, itinSelector);
    if (copyTaxResponse.isCopyToNewItin())
      copyTaxResponse.copyToNewItin();
  }
  else
  {
    ItinSelector itinSelector(trx);

    if (itinSelector.isRefundTrx() && (itinSelector.isExcItin() || itinSelector.isCat33FullRefund()))
    {
      for(Itin* itin : itinSelector.get())
      {
        if (itin->errResponseCode() != ErrorResponseException::NO_ERROR ||
            itin->getTaxResponses().empty())
          continue;

        if (TrxUtil::isAutomatedRefundCat33Enabled(trx) &&
            itinSelector.isRefundTrx() && itinSelector.isExcItin())
          trx.addToExcItinTaxResponse(itin->getTaxResponses().begin(), itin->getTaxResponses().end());
      }
    }
    else if (itinSelector.isNewItin() &&
        trx.isValidatingCxrGsaApplicable() &&
        (trx.getTrxType() == PricingTrx::PRICING_TRX ||
         trx.getTrxType() == PricingTrx::MIP_TRX ))
    {
      trx.taxResponse().clear();
      for(Itin* itin : itinSelector.get())
      {
        if (itin->errResponseCode() != ErrorResponseException::NO_ERROR ||
            itin->getTaxResponses().empty())
          continue;

        trx.taxResponse().insert(trx.taxResponse().end(),
                                 itin->getTaxResponses().begin(),
                                 itin->getTaxResponses().end());
      }
    }
  }
}

// ----------------------------------------------------------------------------
//
// bool TaxOrchestrator::process
//
// Description:   Processing Tax only Services..
//
// ----------------------------------------------------------------------------

bool
LegacyTaxProcessor::process(FareDisplayTrx& fareDisplayTrx)
{
  LOG4CXX_INFO(_logger, "Started Process()");

  TSELatencyData metrics(fareDisplayTrx, "TAX PROCESS");

  if (fareDisplayTrx.itin().empty())
  {
    LOG4CXX_ERROR(_logger, "No Fare Quote Itinerary");
    return false;
  }
  /*
      if (!FareDisplayTax::shouldCalculateTax(fareDisplayTrx))
      {
          LOG4CXX_DEBUG(_logger, "No Tax Calculation required");
          return true;
      }
  */
  if (fareDisplayTrx.getOptions() != nullptr && fareDisplayTrx.getOptions()->currencyOverride().empty())
  {
    Itin* itin = fareDisplayTrx.itin().front();
    fareDisplayTrx.getOptions()->currencyOverride() = itin->calculationCurrency();
  }

  /*Get one farepath which will be reused for all paxtypefares by repopulating with each
   * paxtypefares
   * info*/
  FarePath* farePath = nullptr;
  fareDisplayTrx.dataHandle().get(farePath);
  if (farePath == nullptr)
  {
    LOG4CXX_ERROR(_logger, "TaxOrchestrator::process() - UNABLE TO ALLOCATE MEMORY FOR FAREPATH");
    LOG4CXX_INFO(_logger, "Leaving TaxOrchestrator:process");
    return false;
  }
  /****
  InitializeFarePath initializes farepath with an itin, which is a clone of trx itin.
  This itin will be updated with a return travel segment, if one doesn't already exist.
  The farepath will also be initialized with two pricing units one for each direction.
  Each pricing unit will have its own fare usage.
  ****/
  if (!FareDisplayTax::initializeFarePath(fareDisplayTrx, farePath))
  {
    LOG4CXX_ERROR(_logger, "TaxOrchestrator::process() - UNABLE TO INITIALIZE FAREPATH");
    LOG4CXX_INFO(_logger, "Leaving TaxOrchestrator:process");
  }

  // Iterate through each FareMarket
  std::vector<Itin*>::iterator itinI = fareDisplayTrx.itin().begin();
  std::vector<FareMarket*>::const_iterator fmItr = (*itinI)->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = (*itinI)->fareMarket().end();

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket& fareMarket = **fmItr;

    if (fareMarket.allPaxTypeFare().empty())
    {
      LOG4CXX_DEBUG(_logger, "No PaxTypeFares");
      continue;
    }

    if (!FareDisplayTax::shouldCalculateTax(fareDisplayTrx, &fareMarket))
    {
      if (TrxUtil::isSupplementChargeEnabled(fareDisplayTrx))
      {
        std::string carrierList = TrxUtil::supplementChargeCarrierListData(fareDisplayTrx);
        std::vector<CarrierCode> supplementChargeCarriersVec;
        if (carrierList.empty())
        {
          LOG4CXX_DEBUG(_logger, "No Tax Calculation required - " << fareMarket.governingCarrier());
          continue;
        }

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> separator("|");
        tokenizer tokens(carrierList, separator);
        tokenizer::iterator tokenI = tokens.begin();

        for (; tokenI != tokens.end(); ++tokenI)
          supplementChargeCarriersVec.push_back(tokenI->data());

        const std::vector<CarrierCode>::const_iterator itor =
            std::find(supplementChargeCarriersVec.begin(),
                      supplementChargeCarriersVec.end(),
                      fareMarket.governingCarrier());
        if (itor == supplementChargeCarriersVec.end())
        {
          LOG4CXX_DEBUG(_logger, "No Tax Calculation required - " << fareMarket.governingCarrier());
          continue;
        }
      }
      else
      {
        LOG4CXX_DEBUG(_logger, "No Tax Calculation required - " << fareMarket.governingCarrier());
        continue;
      }
    }

    std::vector<PaxTypeFare*>::iterator paxTypeFareIter = fareMarket.allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator paxTypeFareEnd = fareMarket.allPaxTypeFare().end();

    for (; paxTypeFareIter != paxTypeFareEnd; paxTypeFareIter++)
    {
      PaxTypeFare* paxTypeFare = *paxTypeFareIter;

      if (!paxTypeFare->isValid())
        continue;

      FareDisplayInfo* fareDisplayInfo = paxTypeFare->fareDisplayInfo();
      if (!fareDisplayInfo)
      {
        LOG4CXX_ERROR(_logger, "No FareDisplayInfo for paxTypeFare");
        continue;
      }
      /****
      populateFarePath populates the farepath with paxtypefare specific information
      ****/
      if (!FareDisplayTax::populateFarePath(fareDisplayTrx, paxTypeFare, farePath))
      {
        LOG4CXX_ERROR(_logger, "Error populating FarePath");
        continue;
      }

      if (farePath->paxType() == nullptr)
      {
        LOG4CXX_ERROR(_logger, "No Fare Quote Pax Type");
        continue;
      }

      TaxResponse taxResponse;

      MoneyAmount basefare = farePath->getTotalNUCAmount();

      MoneyAmount rtSurcharge = 0;
      MoneyAmount owSurcharge = 0;
      if (fareDisplayTrx.getOptions()->applySurcharges() == YES)
      {
        FareDisplaySurcharge::getTotalRTSurcharge(fareDisplayTrx, *paxTypeFare, rtSurcharge);
        FareDisplaySurcharge::getTotalOWSurcharge(fareDisplayTrx, *paxTypeFare, owSurcharge);
      }

      taxResponse.paxTypeCode() = farePath->paxType()->paxType();
      taxResponse.farePath() = farePath;

      taxResponse.farePath()->setTotalNUCAmount(basefare + rtSurcharge);

      DCFactory* factory = DCFactory::instance();
      DiagCollector& diag = *(factory->create(fareDisplayTrx));

      taxResponse.diagCollector() = &diag;

      // ROUND TRIP TAX COLLECTION FOR ROUNDTRIP AND ONEWAY_MAY_BE_DOUBLED FARES
      // AND ONEWAY TAX COLLECTION FOR ONEWAY_MAYNOT_BE_DOUBLED FARES

      LOG4CXX_INFO(_logger, "Entering TaxDriver::ProcessTaxesAndFees");
      TaxDriver taxDriver;
      TaxRecord taxRecord;
      TravelSeg* travelSeg(nullptr);
      Itin* itin = taxResponse.farePath()->itin();
      if (paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
      {
        std::vector<TravelSeg*>::iterator travelSegIter = itin->travelSeg().begin();
        std::vector<TravelSeg*>::iterator travelSegEndIter = itin->travelSeg().end();
        travelSegIter++;

        if (travelSegIter != travelSegEndIter)
        {
          travelSeg = *travelSegIter;
          // Remove the return travel segment for oneway may not be doubled fares
          itin->travelSeg().erase(travelSegIter);
        }
      }
      taxDriver.ProcessTaxesAndFees(fareDisplayTrx, taxResponse);

      LOG4CXX_INFO(_logger, "Entering TaxRecord::buildTicketLine");

      taxRecord.buildTicketLine(fareDisplayTrx, taxResponse, true);

      std::vector<TaxRecord*>::const_iterator taxRecordIter = taxResponse.taxRecordVector().begin();
      std::vector<TaxRecord*>::const_iterator taxRecordEnd = taxResponse.taxRecordVector().end();

      for (; taxRecordIter != taxRecordEnd; taxRecordIter++)
      {
        TaxRecord* taxRecord = *taxRecordIter;
        fareDisplayInfo->taxRecordVector().push_back(taxRecord);
      }

      std::vector<TaxItem*>::const_iterator taxItemIter = taxResponse.taxItemVector().begin();
      std::vector<TaxItem*>::const_iterator taxItemEndIter = taxResponse.taxItemVector().end();

      for (; taxItemIter != taxItemEndIter; taxItemIter++)
      {
        TaxItem* taxItem = *taxItemIter;
        fareDisplayInfo->taxItemVector().push_back(taxItem);
      }

      // ONE WAY TAX COLLECTION FOR FOR ROUND TRIP AND ONEWAY_MAYBE_DOUBLED FARES
      // THIS IS DONE FOR FT AND TWO COLUMN FAREAMOUNT TEMPLATE DISPLAYS

      std::vector<TravelSeg*>::iterator travelSegIter = itin->travelSeg().begin();
      std::vector<TravelSeg*>::iterator travelSegEndIter = itin->travelSeg().end();
      travelSegIter++;

      if (travelSegIter == travelSegEndIter)
      {
        // Restore information and continue to next fare
        // as oneway_maynot_be_doubled fare taxes are already collected
        itin->travelSeg().push_back(travelSeg);
        continue;
      }

      travelSeg = *travelSegIter;
      // Remove the return travel segment for oneway tax collection
      itin->travelSeg().erase(travelSegIter);

      taxResponse.taxItemVector().clear();
      taxResponse.taxRecordVector().clear();

      taxResponse.farePath()->setTotalNUCAmount(basefare / 2 + owSurcharge);
      if (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED)
        taxResponse.farePath()->pricingUnit()[0]->fareUsage()[0]->isRoundTrip() = false;

      taxDriver.ProcessTaxesAndFees(fareDisplayTrx, taxResponse);
      taxRecord.buildTicketLine(fareDisplayTrx, taxResponse, true);

      taxRecordIter = taxResponse.taxRecordVector().begin();
      taxRecordEnd = taxResponse.taxRecordVector().end();

      for (; taxRecordIter != taxRecordEnd; taxRecordIter++)
      {
        TaxRecord* taxRecord = *taxRecordIter;
        fareDisplayInfo->taxRecordOWRTFareTaxVector().push_back(taxRecord);
      }

      taxItemIter = taxResponse.taxItemVector().begin();
      taxItemEndIter = taxResponse.taxItemVector().end();

      for (; taxItemIter != taxItemEndIter; taxItemIter++)
      {
        TaxItem* taxItem = *taxItemIter;
        fareDisplayInfo->taxItemOWRTFareTaxVector().push_back(taxItem);
      }
      // restore return travel segment
      itin->travelSeg().push_back(travelSeg);
    } // For Loop PaxType
  } // For Loop Fare Market                                                   i
  return true;
}

// ----------------------------------------------------------------------------
//
// bool LegacyTaxProcessor::process
//
// Description:   Processing Tax only Services..
//
// ----------------------------------------------------------------------------

bool
LegacyTaxProcessor::process(TaxTrx& taxTrx)
{
  PricingTrx& trx = taxTrx;

  if (taxTrx.requestType() == DISPLAY_REQUEST)
  {
    TaxDisplayDriver taxDisplayDriver;
    return taxDisplayDriver.buildTaxDisplay(taxTrx);
  }

  if (taxTrx.requestType() == PFC_DISPLAY_REQUEST)
  {
    PfcDisplayDriver driver(&taxTrx);
    driver.buildPfcDisplayResponse();
    return true;
  }

  assert (taxTrx.requestType() != TAX_INFO_REQUEST);

  if (taxTrx.requestType() == OTA_REQUEST)
  {
    return processOTA(trx);
  }

  return process(trx);
}

bool
LegacyTaxProcessor::processOTA(PricingTrx& trx)
{
  LOG4CXX_INFO(_logger, "Started Process()");
  TSELatencyData metrics(trx, "TAX OTA PROCESS");

  TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);
  if (taxTrx)
  {
    if (!fallback::setHistoricalOTA(&trx))
    {
      if (!(trx.getRequest() && trx.getRequest()->ticketingDT().isValid()))
      {
        taxTrx->errorMsg() = "TICKETING DATE IS NOT VALID";
      }
      else if (trx.getRequest()->ticketingDT().date() < trx.dataHandle().getToday().date())
      {
        taxTrx->errorMsg() = "TICKETING DATE IS FROM THE PAST";
        trx.dataHandle().setTicketDate(trx.getRequest()->ticketingDT());
      }
      else if (trx.getRequest()->ticketingDT().date() > trx.dataHandle().getToday().date())
      {
        taxTrx->errorMsg() = "TICKETING DATE IS FROM THE FUTURE";
        trx.dataHandle().setTicketDate(trx.getRequest()->ticketingDT());
      }
    }
  }

  if (!trx.getOptions()->isMOverride())
  {
    if (!taxTrx)
    {
      BSRCurrencyConverter bsrConverter;
      bsrConverter.hasIndirectEquivAmtOverride(trx);
    }
  }

  TseRunnableExecutor pooledExecutor(_taskId);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(trx.dataHandle(), taxFactoryMap);

  std::vector<Itin*>::iterator itinI = trx.itin().begin();
  try
  {
    uint16_t remainingItin = trx.itin().size();
    // save original agent from transaction
    Agent* trxAgent = trx.getRequest()->ticketingAgent();

    LocCode prevAgentCity;
    //
    for (; itinI != trx.itin().end(); ++itinI, --remainingItin)
    {
      if ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR)
        continue;

      // Tax Service can override Agent in itinerary-------------
      // override of AgentPCC can be done only in transaction
      // normally thread per itinerary is spawned but if AgentPCC is overrided
      // we have to wait for different PCC-itinerary process to finis
      if (!prevAgentCity.empty() && prevAgentCity != (*itinI)->agentPCCOverride()->agentCity())
      {
        pooledExecutor.wait();
      }

      //-----------------------------------------------------------------------
      if ((*itinI)->agentPCCOverride() != nullptr)
      {
        trx.getRequest()->ticketingAgent() = (*itinI)->agentPCCOverride();
      }
      else
        trx.getRequest()->ticketingAgent() = trxAgent;
      //---------------------------------------------------------

      TaxItinerary* taxItinerary = nullptr;
      trx.dataHandle().get(taxItinerary);
      taxItinerary->initialize(trx, **itinI, taxFactoryMap);

      TseRunnableExecutor& taskExecutor =
          (remainingItin > 1) ? pooledExecutor : synchronousExecutor;

      taskExecutor.execute(*taxItinerary);
    }
    pooledExecutor.wait();
  }
  catch (boost::thread_interrupted& e)
  {
    LOG4CXX_ERROR(_logger, "thread_interrupted exception has been thrown");
    computeTaxesInSingleThread(trx, taxFactoryMap);
  }

  TaxDiagnostic taxDiagnostic;
  taxDiagnostic.collectDiagnostics(trx);

  LOG4CXX_INFO(_logger, "Finished Leaving Tax_ota");

  return true;
}

TaxResponse*
LegacyTaxProcessor::getTaxResponseForSp(std::vector<TaxResponse*>& taxResponses,
                                        const SettlementPlanType& sp) const
{
  for (TaxResponse* taxResp : taxResponses)
  {
    if (std::find(
          taxResp->settlementPlans().begin(),
          taxResp->settlementPlans().end(),
          sp) != taxResp->settlementPlans().end())
      return taxResp;
  }
  return nullptr;
}

SettlementPlanType
LegacyTaxProcessor::findPrimarySp(const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  if (spValCxrsWithLowestTotalAmt.empty())
    return "";

  if (spValCxrsWithLowestTotalAmt.size() == 1)
    return spValCxrsWithLowestTotalAmt.begin()->first;
  else
  {
    std::vector<SettlementPlanType> spCol;
    for (const auto& it : spValCxrsWithLowestTotalAmt)
      spCol.push_back(it.first);
    return ValidatingCxrUtil::determinePlanFromHierarchy(spCol);
  }
  return "";
}

void
LegacyTaxProcessor::setDefaultPerSp(PricingTrx& trx,
                                    FarePath& farePath,
                                    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                                    const SettlementPlanType& primarySp) const
{
  for (const auto& it : spValCxrsWithLowestTotalAmt)
  {
    if (it.first == primarySp)
      continue;

    CarrierCode dvcxr, mcxr;
    DefaultValidatingCarrierFinder defValCxrFinder(trx, *farePath.itin(), it.first);
    bool retVal = defValCxrFinder.determineDefaultValidatingCarrier(
        spValCxrsWithLowestTotalAmt[it.first], dvcxr, mcxr);

    if (retVal)
    {
      farePath.defaultValCxrPerSp()[it.first] = dvcxr;
      if(dvcxr != mcxr && !mcxr.empty())
        farePath.marketingCxrForDefaultValCxrPerSp()[it.first] = mcxr;
    }
  }
}

void
LegacyTaxProcessor::setAlternatePerSp(PricingTrx& trx, FarePath& farePath,
                                      TaxResponse* primaryTaxResponse,
                                      const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  for (const auto& item : spValCxrsWithLowestTotalAmt)
  {
    const SettlementPlanType& sp = item.first;
    for (const CarrierCode& vc : item.second)
    {
      if (vc == farePath.defaultValCxrPerSp()[sp])
        continue;

      auto trIt = farePath.valCxrTaxResponses().find(vc);
      if (trIt != farePath.valCxrTaxResponses().end())
      {
        TaxResponse* taxResponse = getTaxResponseForSp(trIt->second, sp);
        if (sameZeroTaxes(*primaryTaxResponse, *taxResponse) ||
            sameTaxes(*primaryTaxResponse, *taxResponse))
        {
          // these are possible alternate VC IF other PaxType also have it
          if (!farePath.netRemitFarePath())
          {
            if(!fallback::fallbackDuplicateVCxrMultiSP(&trx))
            {
               if(std::find(farePath.validatingCarriers().begin(), farePath.validatingCarriers().end(), vc)
                   == farePath.validatingCarriers().end())
                  farePath.validatingCarriers().push_back(vc);
            }
            else
              farePath.validatingCarriers().push_back(vc); //Do we still need it to populate?
          }

        }
      }
    }
  }
}

void
LegacyTaxProcessor::copyAllValCxrsWithNoDefaultValCxr(FarePath& fp,
                                          const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  for (const auto& it : spValCxrsWithLowestTotalAmt)
  {
    const SettlementPlanType& sp = it.first;
    for (const CarrierCode& cxr : it.second)
    {
      if (std::find(fp.validatingCarriers().begin(),
            fp.validatingCarriers().end(),
            cxr) == fp.validatingCarriers().end())
        fp.validatingCarriers().push_back(cxr);
    }

    if (!fp.validatingCarriers().empty())
      fp.defaultValCxrPerSp()[sp] = ""; //BLANK DEF VAL CXR
  }
}

void
LegacyTaxProcessor::findSpValCxrsWithLowestTotal(PricingTrx& trx,
                                     FarePath& farePath,
                                     SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  MoneyAmount currentLowTotal = std::numeric_limits<MoneyAmount>::max();
  for (auto& it : farePath.valCxrTaxResponses())
  {
    for (TaxResponse* taxResp : it.second)
    {
      Money trt(NUC);
      taxResp->getTaxRecordTotal(trt);
      MoneyAmount nextTotalAmt = trt.value() + farePath.getTotalNUCAmount();

      const double diff = currentLowTotal - nextTotalAmt;

      if (diff > EPSILON)
      {
        //found new low
        currentLowTotal = nextTotalAmt;

        spValCxrsWithLowestTotalAmt.clear();
        for (const SettlementPlanType& sp : taxResp->settlementPlans())
          spValCxrsWithLowestTotalAmt[sp].push_back(it.first);
      }
      else if (fabs(diff) < EPSILON)
      {
        //equal amount, this cxr is also a candidate
        for (const SettlementPlanType& sp : taxResp->settlementPlans())
          spValCxrsWithLowestTotalAmt[sp].push_back(it.first);
      }
    }
  }
}

void
LegacyTaxProcessor::findSpValCxrsForFinalFp(
    PricingTrx& trx,
    FarePath& finalFp,
    const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  bool suppressNonBspForNetRemit =
         handleNetRemitForMultiSp(finalFp, spValCxrsWithLowestTotalAmt);

  for (const auto& spValCxr : spValCxrsWithLowestTotalAmt)
  {
    if (suppressSettlementPlanGTC(trx, spValCxr.first, spValCxrsWithLowestTotalAmt.size()))
        continue;

    // Net Remit is a BSP only solution, do not include other SP
    if (suppressNonBspForNetRemit && "BSP" != spValCxr.first)
      continue;

    const SettlementPlanType& sp = spValCxr.first;
    for (const CarrierCode& cxr : spValCxr.second)
    {
      auto it = finalFp.valCxrTaxResponses().find(cxr);
      if (it == finalFp.valCxrTaxResponses().end())
        continue;

      // Check whether FarePath's validatingCxr participates in SP
      if (isTaxCollectedForSp(it->second, sp, cxr))
        finalFp.settlementPlanValidatingCxrs()[sp].push_back(cxr);
    }
  }
}

void
LegacyTaxProcessor::findSpValCxrsForFinalFp(
    PricingTrx& trx,
    FarePath& finalFp,
    const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
    const std::vector<CarrierCode>& commonValCxrs) const
{
  for (const auto& spValCxr : spValCxrsWithLowestTotalAmt)
  {
    if (suppressSettlementPlanGTC(trx, spValCxr.first, spValCxrsWithLowestTotalAmt.size()))
        continue;

    const SettlementPlanType& sp = spValCxr.first;
    for (const CarrierCode& cxr : spValCxr.second)
    {
      if (std::find(commonValCxrs.begin(), commonValCxrs.end(), cxr) == commonValCxrs.end())
        continue;

      auto valCxrTaxRespIt = finalFp.valCxrTaxResponses().find(cxr);
      if (valCxrTaxRespIt == finalFp.valCxrTaxResponses().end())
        continue;

      for (TaxResponse* taxResp : valCxrTaxRespIt->second)
      {
        if (std::find(
              taxResp->settlementPlans().begin(),
              taxResp->settlementPlans().end(), sp) != taxResp->settlementPlans().end())
        {
          finalFp.settlementPlanValidatingCxrs()[sp].push_back(cxr);
          break;
        }
      }
    }
  }
}

bool
LegacyTaxProcessor::isTaxCollectedForSp(const std::vector<TaxResponse*>& taxResponses,
                                        const SettlementPlanType& sp,
                                        const CarrierCode& valCxr) const
{
  for (TaxResponse* taxResp : taxResponses)
    if (std::find(taxResp->settlementPlans().begin(),
                  taxResp->settlementPlans().end(),
                  sp) != taxResp->settlementPlans().end())
      return true;
  return false;
}

void
LegacyTaxProcessor::modifyItinTaxResponseForFinalFarePath(FarePath& finalFp) const
{
  if (!finalFp.itin() || finalFp.settlementPlanValidatingCxrs().empty())
    return;

  auto it = finalFp.itin()->mutableTaxResponses().begin();
  while (it != finalFp.itin()->mutableTaxResponses().end())
  {
    if (*it && !isFinalSettlementPlanInTaxResponse(**it, finalFp.settlementPlanValidatingCxrs()))
      it = finalFp.itin()->mutableTaxResponses().erase(it);
    else
      ++it;
  }
}

bool
LegacyTaxProcessor::isFinalSettlementPlanInTaxResponse(TaxResponse& taxResp,
                                                       const SettlementPlanValCxrsMap& spValCxrs) const
{
  for (const SettlementPlanType& sp : taxResp.settlementPlans())
    if (spValCxrs.find(sp) != spValCxrs.end())
      return true;
  return false;
}

// Return true when collection has settlement plans beside GTC and
// customer has multiple-sp OFF
bool
LegacyTaxProcessor::suppressSettlementPlanGTC(
    PricingTrx& trx,
    const SettlementPlanType& sp,
    size_t numOfSpInFinalSolution) const
{
  return ("GTC" == sp &&
      numOfSpInFinalSolution > 1 &&
      (trx.getRequest() &&
       trx.getRequest()->ticketingAgent() &&
       !trx.getRequest()->ticketingAgent()->isMultiSettlementPlanUser()));
}

// Find total amount for a given FP for a given SP Group
void
LegacyTaxProcessor::findSpTotalAmount(
    const FarePath& farePath,
    std::vector<TaxResponse*>& taxResponses,
    SettlementPlanTaxAmountGroupMap& spTaxAmtGroup) const
{

  for (TaxResponse* taxResp : taxResponses)
  {
    Money taxTotal(NUC);
    taxResp->getTaxRecordTotal(taxTotal);
    MoneyAmount totalAmt =
      (farePath.getTotalNUCAmount() + taxTotal.value()) * farePath.paxType()->number();

    SettlementPlanTaxAmount spTaxAmount(taxResp, totalAmt);
    SettlementPlanGroup spType =
      (std::find(taxResp->settlementPlans().begin(),
                 taxResp->settlementPlans().end(),
                 "TCH") != taxResp->settlementPlans().end()) ?
      SettlementPlanGroup::TCH_SP:
      SettlementPlanGroup::REG_SP;
    spTaxAmtGroup[spType].push_back(spTaxAmount);
  }
}

// Compare totalAmount between sp groups (REG, TCH) for a given val-cxr across farePaths
// If total and tax-components are same then merge otherwise apply hierarchy
void
LegacyTaxProcessor::processSettlementPlanTaxData(
    MoneyAmount& currentLowestTotal,
    std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const SettlementPlanTaxAmountGroupMap& spTaxAmtGroup) const
{
  auto it = spTaxAmtGroup.begin();
  if (it == spTaxAmtGroup.end())
    return;

  currentLowestTotal = getSpTotalAmount(it->second);
  currentSpTaxAmountCol = it->second;
  while (++it != spTaxAmtGroup.end())
  {
    MoneyAmount totalAmt = getSpTotalAmount(it->second);
    const double diff = currentLowestTotal - totalAmt;
    if (diff > EPSILON)
    {
      currentLowestTotal = totalAmt;
      currentSpTaxAmountCol = it->second;
    }
    else if (fabs(diff) < EPSILON) // equal amount, check for equality of tax components
      checkTaxComponentsAndMerge(currentSpTaxAmountCol, it->second);
  }
}

// Find total journey amount using stored amount accross FPs
MoneyAmount
LegacyTaxProcessor::getSpTotalAmount(const std::vector<SettlementPlanTaxAmount>& spTaxAmtCol) const
{
  MoneyAmount currentTotal(0.0);
  for (const SettlementPlanTaxAmount& spTaxAmt : spTaxAmtCol)
    currentTotal += spTaxAmt.totalAmount;
  return currentTotal;
}

// Return true if tax components are same
bool
LegacyTaxProcessor::canMergeTaxResponses(
    const std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol) const
{
  if (currentSpTaxAmountCol.empty() || nextSpTaxAmountCol.empty())
    return false;

  const TaxResponse* currentTaxResponse = currentSpTaxAmountCol.front().taxResponse;
  const TaxResponse* nextTaxResponse = nextSpTaxAmountCol.front().taxResponse;
  if (currentTaxResponse &&
      nextTaxResponse &&
      (sameZeroTaxes(*currentTaxResponse, *nextTaxResponse) ||
       sameTaxes(*currentTaxResponse, *nextTaxResponse)))
    return true;
  return false;
}

// Return true if currentSpTaxAmountCol has item that is higher in hierarchy
// compared to nextSpTaxAmountCol. It also return sp higher in the hierachy
bool
LegacyTaxProcessor::checkSettlementPlanHierarchy(
    const std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol,
    SettlementPlanType& higherSp) const
{
  for (const SettlementPlanType& sp : vcx::SP_HIERARCHY)
  {
    bool isCurrSpFound=false, isNextSpFound=false;
    size_t currInd = 0, nextInd = 0;

    if (!currentSpTaxAmountCol.empty() && currentSpTaxAmountCol.front().taxResponse)
      isCurrSpFound = checkSpIndex(*currentSpTaxAmountCol.front().taxResponse, sp, currInd);

    if (!nextSpTaxAmountCol.empty() && nextSpTaxAmountCol.front().taxResponse)
      isNextSpFound = checkSpIndex(*nextSpTaxAmountCol.front().taxResponse, sp, nextInd);

    if (!isCurrSpFound && !isNextSpFound)
      continue;

    higherSp = sp;
    if (isCurrSpFound && isNextSpFound)
      return currInd <= nextInd;
    else if (isCurrSpFound && !isNextSpFound)
      return true;
    else
      return false;
  }
  return false;
}

// Return true if hierarchy-sp found in TaxResponse and also set ind
bool
LegacyTaxProcessor::checkSpIndex(
    const TaxResponse& taxResp,
    const SettlementPlanType& sp,
    size_t& ind) const
{
  ind = std::find(taxResp.settlementPlans().begin(),
      taxResp.settlementPlans().end(),
      sp) - taxResp.settlementPlans().begin();

  return (ind < taxResp.settlementPlans().size());
}

// Find the SP group that given lowest total amount and store that information
// in SettlementPlanValCxrMap
void
LegacyTaxProcessor::processValidatingCxrTotalAmount(
    const std::map<CarrierCode, ValidatingCxrTotalAmount>& valCxrTotalAmtPerCxr,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  auto it = valCxrTotalAmtPerCxr.begin();
  if (it == valCxrTotalAmtPerCxr.end())
    return;

  MoneyAmount currentLowestTotal = it->second.totalAmount;
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol = it->second.spTaxAmountCol;
  setSpValCxrsMap(it->first, it->second.spTaxAmountCol, spValCxrsWithLowestTotalAmt);

  while (++it != valCxrTotalAmtPerCxr.end())
  {
    MoneyAmount totalAmt = it->second.totalAmount;
    const double diff = currentLowestTotal - totalAmt;
    if (diff > EPSILON)
    {
      spValCxrsWithLowestTotalAmt.clear();
      setSpValCxrsMap(it->first, it->second.spTaxAmountCol, spValCxrsWithLowestTotalAmt);
      currentLowestTotal = totalAmt;
      currentSpTaxAmountCol = it->second.spTaxAmountCol;
    }
    else if (fabs(diff) <= EPSILON) // equal amount, check for equality of tax components
    {
      const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol = it->second.spTaxAmountCol;
      checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
      setSpValCxrsMap(it->first, currentSpTaxAmountCol, spValCxrsWithLowestTotalAmt);
    }
  }
}

void
LegacyTaxProcessor::checkTaxComponentsAndMerge(
  std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
  const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol) const
{
  bool mergeTaxResponses = canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol);
  if (mergeTaxResponses)
  {
    currentSpTaxAmountCol.insert(currentSpTaxAmountCol.end(),
        nextSpTaxAmountCol.begin(),
        nextSpTaxAmountCol.end());
  }
  else
  {
    SettlementPlanType higherSp;
    bool isCurrentHigher = checkSettlementPlanHierarchy(
        currentSpTaxAmountCol,
        nextSpTaxAmountCol,
        higherSp);

    if (!isCurrentHigher)
      currentSpTaxAmountCol = nextSpTaxAmountCol;

    if (!higherSp.empty())
      clearSettlementPlanData(currentSpTaxAmountCol, higherSp);
  }
}

// Remove SettlementPlanTaxAmount if sp is not in its TaxResponse
// Remove all settlement plans except sp
void
LegacyTaxProcessor::clearSettlementPlanData(
    std::vector<SettlementPlanTaxAmount>& v,
    const SettlementPlanType& sp) const
{
  v.erase(std::remove_if(
        v.begin(),
        v.end(),
        [&sp] (const SettlementPlanTaxAmount& obj)->bool
        {
        return obj.taxResponse &&
        (std::find(obj.taxResponse->settlementPlans().begin(),
                   obj.taxResponse->settlementPlans().end(),
                   sp) == obj.taxResponse->settlementPlans().end());
        }
        ), v.end());
}

// Populate map and avoid duplication of crriers in sp=>{carriers}
void
LegacyTaxProcessor::setSpValCxrsMap(
    const CarrierCode& cxr,
    const std::vector<SettlementPlanTaxAmount>& spTaxAmountCol,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  for (const SettlementPlanTaxAmount& spTaxAmt : spTaxAmountCol)
  {
    if (!spTaxAmt.taxResponse)
      continue;

    for (const SettlementPlanType& sp : spTaxAmt.taxResponse->settlementPlans())
    {
      auto it = spValCxrsWithLowestTotalAmt.find(sp);
      if (it != spValCxrsWithLowestTotalAmt.end() &&
          std::find(it->second.begin(), it->second.end(), cxr) != it->second.end())
        continue;
      spValCxrsWithLowestTotalAmt[sp].push_back(cxr);
    }
  }
}

bool
LegacyTaxProcessor::handleNetRemitForMultiSp(
    FarePath& finalFp,
    const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
{
  if (finalFp.collectedNegFareData() &&
      finalFp.collectedNegFareData()->bspMethod() != BLANK)
  {
    auto it = spValCxrsWithLowestTotalAmt.find("BSP");
    if (it != spValCxrsWithLowestTotalAmt.end())
      return true;

    // If there is no BSP, then it is not a net remit solution
    finalFp.collectedNegFareData()->bspMethod() = BLANK;
    finalFp.collectedNegFareData()->trailerMsg() =
      "NET REMIT FARE - PHASE 4 AND USE NET/ FOR TKT ISSUANCE";
    finalFp.netRemitFarePath() = nullptr;
  }
  return false;
}

// In MultiSp Path only
void
LegacyTaxProcessor::setTaxResponseInItin(const PricingTrx& trx,
                                         FarePath& farePath,
                                         TaxResponse* taxResponse) const
{
  if (taxResponse &&
      (!taxResponse->taxRecordVector().empty() || !taxResponse->taxItemVector().empty()))
  {
    farePath.itin()->mutableTaxResponses().push_back(taxResponse); // used in FareCalc
  }
}

} //tse
