// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxItinerary.h"

#include "Common/ItinUtil.h"
#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagVisitor.h"
#include "Taxes/Common/ReissueExchangeDateSetter.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxDriver.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fixCoreDumpTaxItinerary)
FALLBACK_DECL(markupAnyFareOptimization)
FALLBACK_DECL(Cat33_Diag)
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(fallbackTaxTrxTimeout);

namespace
{
Logger
logger("atseintl.Taxes.TaxItinerary");

bool skipGSA(PricingTrx& trx)
{
  return trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax();
  //return false;
}
}

class TaxItinerary::Impl
{
public:
  static void
  processAllFarePathsSimple(TaxItinerary& _this, DiagCollector& diag)
  {
    if (!fallback::markupAnyFareOptimization(_this._trx))
    {
      ItinUtil::collectMarkupsForFarePaths(*_this._itin);
    }

    std::vector<FarePath*>::iterator farePathI = _this._itin->farePath().begin();
    for (; farePathI != _this._itin->farePath().end(); farePathI++)
    {
      FarePath* fp = *farePathI;
      if (fp->paxType() == nullptr)
      {
        LOG4CXX_WARN(logger, "No Pax Type");
        continue;
      }

      {
        ReissueExchangeDateSetter dateSetter(*_this._trx, *fp);
        _this.processFarePath(fp, &diag);
      }
    }
  }

};

void
TaxItinerary::initialize(PricingTrx& trx, Itin& itin, TaxMap::TaxFactoryMap& taxFactoryMap)
{
  // Base class initialization - needs Trx pointer & static string for metrics
  // measurement
  TseCallableTrxTask::trx(&trx);
  desc("TAX ITINERARY TASK");
  _trx = &trx;
  _itin = &itin;
  _taxFactoryMap = &taxFactoryMap;
}

// ----------------------------------------------------------------------------
//
// bool TaxItinerary::accumulater
//
// Description:  Main controller for all forms of Tax services Shopping / Pricing
//
// ----------------------------------------------------------------------------

void
TaxItinerary::accumulator()
{
  if (!fallback::fallbackTaxTrxTimeout(_trx))
  {
    if (fallback::reworkTrxAborter(_trx))
      checkTrxAborted(*_trx);
    else
      _trx->checkTrxAborted();
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(*_trx));

  if (TrxUtil::isAutomatedRefundCat33Enabled(*_trx))
  {
    if (!fallback::Cat33_Diag(_trx))
    {
      PrintHeader printHeader(*_trx);
      diag.accept(printHeader);
      diag.flushMsg();
    }
  }

  Impl::processAllFarePathsSimple(*this, diag);
}

//*********************************************************************************************
// VALIDATING CARRIER PROJECT notes
// 1. Calculate TAXes for each Validating carrier in the loop
// 2. Remove TaxResponse* from corresponding vectors in the Itin and Trx
// 3. Collect result(s) in the FarePath valCxrTaxResponseMap map,
//*********************************************************************************************
FarePath*
TaxItinerary::processFarePath(FarePath* farePath, DiagCollector* diag)
{
  if (skipGSA(*_trx))
  {
    processFarePathPerCxr(farePath, diag);
    return nullptr;
    // all validating carrier logic will be performed in ATPCO Taxes
  }

  if(_trx->isValidatingCxrGsaApplicable() && !farePath->validatingCarriers().empty())
  {
    for (const CarrierCode cxr : farePath->validatingCarriers())
    {
      if (!farePath->gsaClonedFarePaths().empty())
      {
        FarePath* fp = farePath->findTaggedFarePath(cxr);
        if (!fp)
          fp = farePath;

        fp->itin()->validatingCarrier() = cxr;
        processFarePathPerCxr(fp, diag);
      }
      else
      {
        farePath->itin()->validatingCarrier() = cxr;
        processFarePathPerCxr(farePath, diag);
      }
    }
  }
  else
  {
    //Non GSA or single val-cxr
    processFarePathPerCxr(farePath, diag);
  }

  return nullptr;
}

void
TaxItinerary::processNetRemitFarePath(FarePath* farePath, DiagCollector* diag, bool analyze)
{
   NetRemitFarePath* netRemitFp = farePath->netRemitFarePath();
   if (netRemitFp->paxType() != nullptr)
   {
      std::vector<TravelSeg*> netRemitTravelSegs;

      const bool swap = netRemitFp->itin() != _itin;

      if (swap)
      {
        // Replace travel segments in itin as original just for Tax
        netRemitTravelSegs.insert(netRemitTravelSegs.end(),
                                  netRemitFp->itin()->travelSeg().begin(),
                                  netRemitFp->itin()->travelSeg().end());

        netRemitFp->itin()->travelSeg().clear();
        netRemitFp->itin()->travelSeg().insert(netRemitFp->itin()->travelSeg().end(),
                                               _itin->travelSeg().begin(),
                                               _itin->travelSeg().end());
      }

      if(analyze)
        analyzeValCxrNetRemitFP(farePath, netRemitFp);

      processFarePathPerCxr(netRemitFp, diag);

      if (swap)
      {
        // Put NetRemit travel segments back
        netRemitFp->itin()->travelSeg().clear();
        netRemitFp->itin()->travelSeg().insert(netRemitFp->itin()->travelSeg().end(),
                                               netRemitTravelSegs.begin(),
                                               netRemitTravelSegs.end());
        if (_trx->diagnostic().diagnosticType() != DiagnosticNone)
          farePath->itin()->mutableTaxResponses().insert(farePath->itin()->mutableTaxResponses().end(),
                                                   netRemitFp->itin()->getTaxResponses().begin(),
                                                   netRemitFp->itin()->getTaxResponses().end());
       }
   }
}

void
TaxItinerary::analyzeValCxrNetRemitFP(FarePath* farePath, NetRemitFarePath* netRemitFp)
{
   if(_trx->isValidatingCxrGsaApplicable())
   {
      if(!farePath->defaultValidatingCarrier().empty())
      {
         netRemitFp->itin()->validatingCarrier() = farePath->defaultValidatingCarrier();
         farePath->validatingCarriers().clear();
      }
      else
      {
         farePath->processed() = false;
      }
   }
}

void
TaxItinerary::processFarePathPerCxr(FarePath* farePath, DiagCollector* diag)
{
  if ((!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
        && (_trx->getTrxType() != PricingTrx::IS_TRX))
    return processFarePathPerCxr_new(*farePath, diag);

  TaxResponse* taxResponse = nullptr;

  // lint --e413
  _trx->dataHandle().get(taxResponse);

  if (taxResponse == nullptr)
  {
    LOG4CXX_WARN(logger, "No MEMORY Available");
    return;
  }
  taxResponse->validatingCarrier() = farePath->itin()->validatingCarrier();

  taxResponse->paxTypeCode() = farePath->paxType()->paxType();
  taxResponse->farePath() = farePath;
  taxResponse->diagCollector() = diag;

  //
  // Check to collect Taxes and Fees
  //
  LOG4CXX_INFO(logger, "Entering TaxDriver::ProcessTaxesAndFees");
  TaxDriver taxDriver;
  taxDriver.ProcessTaxesAndFees(
      *_trx, *taxResponse, *_taxFactoryMap, _trx->countrySettlementPlanInfo());

  //
  // Check to collect PFCs
  //

  if (LIKELY(_trx->getOptions()->getCalcPfc()))
  {
    LOG4CXX_INFO(logger, "Entering PfcItem::build");

    PfcItem pfcItem;
    pfcItem.build(*_trx, *taxResponse);
  }

  //
  // Check to Build Tax Records in TaxResponse for Ticketing
  //

  LOG4CXX_INFO(logger, "Entering TaxRecord::buildTicketLine");

  TaxRecord taxRecord;
  taxRecord.buildTicketLine(*_trx, *taxResponse);

  // Update VC and TaxResponse map in the FarePath
  //
  if (_trx->isValidatingCxrGsaApplicable() &&
      (_trx->diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
       _trx->diagnostic().diagnosticType() == TaxRecSummaryDiagnostic ||
       _trx->diagnostic().diagnosticType() == LegacyTaxDiagnostic24 ||
       _trx->diagnostic().diagnosticType() == PFCRecSummaryDiagnostic ||
       _trx->diagnostic().diagnosticType() == Diagnostic24  ||
       _trx->diagnostic().diagnosticType() == Diagnostic817 ||
       _trx->diagnostic().diagnosticType() == Diagnostic827))
  {
    // Please DO NOT remove it, this vector is called by getAllValCxrTaxResponses(),
    // in TaxDiagnostic object
    farePath->itin()->valCxrTaxResponses().push_back(taxResponse);
  }

  if(farePath->valCxrTaxResponseMap().size() > 0 ||
      (_trx->isValidatingCxrGsaApplicable() && !farePath->validatingCarriers().empty()))
  {
    farePath->setValCxrTaxResponse(farePath->itin()->validatingCarrier(), taxResponse);
    if(!taxResponse->taxRecordVector().empty())
      removeCurrentTaxResponse(*farePath); // remove taxResponse from itin
  }
}

void
TaxItinerary::processFarePathPerCxr_new(FarePath& farePath, DiagCollector* diag, const FarePath* originalFarePath)
{
  std::vector<CountrySettlementPlanInfo*> cspiCol;
  if (farePath.itin()->spValidatingCxrGsaDataMap())
    getCountrySettlementPlanInfoForValidatingCxr(farePath, cspiCol);

  std::vector<TaxResponse*> taxResponses;
  bool hasTCHForMultipleSp = collectTaxResponses(farePath, diag, cspiCol, taxResponses, originalFarePath);
  storeTaxResponses(farePath, taxResponses, hasTCHForMultipleSp);

  if (!fallback::markupAnyFareOptimization(_trx))
  {
    if ((originalFarePath == nullptr) &&
        !farePath.isAdjustedSellingFarePath() &&
        farePath.adjustedSellingFarePath())
    {
      processFarePathPerCxr_new(*farePath.adjustedSellingFarePath(), diag, &farePath);
    }
  }
}

void
TaxItinerary::getCountrySettlementPlanInfoForValidatingCxr(
    FarePath& farePath,
    std::vector<CountrySettlementPlanInfo*>& cspiCol) const
{
  if (!farePath.itin()->spValidatingCxrGsaDataMap())
    return;

  for (const auto& spValCxrGsaData : *farePath.itin()->spValidatingCxrGsaDataMap())
  {
    const ValidatingCxrGSAData* valCxrGsaData = spValCxrGsaData.second;
    if (!valCxrGsaData ||
        valCxrGsaData->validatingCarriersData().find(farePath.itin()->validatingCarrier()) ==
        valCxrGsaData->validatingCarriersData().end())
      continue;

    CountrySettlementPlanInfo *cspi = ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(
        _trx->countrySettlementPlanInfos(),
        spValCxrGsaData.first);

    if (cspi)
      cspiCol.push_back(cspi);
  }
}

// This method is called in both GSA and non GSA path
bool
TaxItinerary::collectTaxResponses(FarePath& farePath,
                                  DiagCollector* diag,
                                  const std::vector<CountrySettlementPlanInfo*>& cspiCol,
                                  std::vector<TaxResponse*>& taxResponses,
                                  const FarePath* originalFarePath)
{
  bool hasTCHForMultipleSp = false;
  CountrySettlementPlanInfo* cspiTCH =
    ValidatingCxrUtil::getCountrySettlementPlanInfoForSp(cspiCol, "TCH");

  if (!_trx->isValidatingCxrGsaApplicable() || cspiCol.size()==1 || !cspiTCH)
  {
    CountrySettlementPlanInfo* cspi = cspiCol.empty() ? nullptr : cspiCol.front();
    TaxResponse* taxResponse = collectTaxResponse(farePath, diag, cspi, originalFarePath);

    if (_trx->isValidatingCxrGsaApplicable() && taxResponse)
    {
      taxResponses.push_back(taxResponse);
      for (CountrySettlementPlanInfo* it : cspiCol)
        taxResponse->settlementPlans().push_back(it->getSettlementPlanTypeCode());
    }
  }
  else // GSA path with multiple sp and having TCH
    hasTCHForMultipleSp = collectTaxResponsesForMultipleSpWithTCH(
        farePath, diag, cspiTCH, cspiCol, taxResponses, originalFarePath);

  return hasTCHForMultipleSp;
}

bool
TaxItinerary::collectTaxResponsesForMultipleSpWithTCH(FarePath& farePath,
                                                      DiagCollector* diag,
                                                      CountrySettlementPlanInfo* cspiTCH,
                                                      const std::vector<CountrySettlementPlanInfo*>& cspiCol,
                                                      std::vector<TaxResponse*>& taxResponses,
                                                      const FarePath* originalFarePath)
{
  bool hasTCHForMultipleSp = false;
  TaxResponse* tchTaxResponse = collectTaxResponse(farePath, diag, cspiTCH, originalFarePath);
  if (tchTaxResponse)
  {
    hasTCHForMultipleSp = true;
    tchTaxResponse->settlementPlans().push_back(cspiTCH->getSettlementPlanTypeCode());
    taxResponses.push_back(tchTaxResponse);
  }

  CountrySettlementPlanInfo* cspi = getNonTCHCountrySettlementPlanInfo(cspiCol);
  if (cspi)
  {
    TaxResponse* taxResponse = collectTaxResponse(farePath, diag, cspi, originalFarePath);
    if (taxResponse)
    {
      taxResponses.push_back(taxResponse);
      for (CountrySettlementPlanInfo* cspi : cspiCol)
        if (cspiTCH != cspi)
          taxResponse->settlementPlans().push_back(cspi->getSettlementPlanTypeCode());
    }
  }
  return hasTCHForMultipleSp;
}

TaxResponse*
TaxItinerary::collectTaxResponse(FarePath& farePath,
                                 DiagCollector* diag,
                                 CountrySettlementPlanInfo* cspi,
                                 const FarePath* originalFarePath)
{
  TaxResponse* taxResponse = nullptr;
  _trx->dataHandle().get(taxResponse);

  if (!taxResponse)
    return nullptr;

  CountrySettlementPlanInfo* origCspi = _trx->countrySettlementPlanInfo();
  if (fallback::fixCoreDumpTaxItinerary(_trx))
    _trx->countrySettlementPlanInfo() = cspi;

  taxResponse->validatingCarrier() = farePath.itin()->validatingCarrier();
  taxResponse->paxTypeCode() = farePath.paxType()->paxType();
  taxResponse->farePath() = &farePath;
  taxResponse->diagCollector() = diag;

  if (fallback::markupAnyFareOptimization(_trx) ||
      !farePath.isAdjustedSellingFarePath())
  {
    // Check to collect Taxes and Fees
    LOG4CXX_INFO(logger, "Entering TaxDriver::ProcessTaxesAndFees");
    TaxDriver taxDriver;
    taxDriver.ProcessTaxesAndFees(*_trx, *taxResponse, *_taxFactoryMap, cspi);

    // Check to collect PFCs
    if (_trx->getOptions()->getCalcPfc())
    {
      LOG4CXX_INFO(logger, "Entering PfcItem::build");
      PfcItem pfcItem;
      pfcItem.build(*_trx, *taxResponse);
    }
  }
  else
  {
    if(originalFarePath == nullptr)
      return nullptr;

    const TaxResponse* origTaxResponse = TaxResponse::findFor(originalFarePath);

    if(origTaxResponse == nullptr)
      return nullptr;

    // Copy already collected tax items
    for(TaxItem* taxItem : origTaxResponse->taxItemVector())
    {
      //TODO use TaxApply::initializeTaxItem
      TaxItem* pTaxItem = nullptr;
      _trx->dataHandle().get(pTaxItem);
      if (pTaxItem == nullptr)
      {
        return nullptr;
      }

      *pTaxItem = *taxItem;
      if(pTaxItem->gstTax() && pTaxItem->taxAmountAdjusted())
      {
        pTaxItem->taxAmount() = pTaxItem->taxAmountAdjusted();
      }

      taxResponse->taxItemVector().push_back(pTaxItem);
    }

    farePath.itin()->mutableTaxResponses().push_back(taxResponse);
  }

  // Check to Build Tax Records in TaxResponse for Ticketing
  LOG4CXX_INFO(logger, "Entering TaxRecord::buildTicketLine");
  TaxRecord taxRecord;
  taxRecord.buildTicketLine(*_trx, *taxResponse, cspi);

  if (fallback::fixCoreDumpTaxItinerary(_trx))
    _trx->countrySettlementPlanInfo() = origCspi; // reset

  return taxResponse;
}

void
TaxItinerary::storeTaxResponses(FarePath& farePath,
                               std::vector<TaxResponse*>& taxResponses,
                               bool hasTCHForMultipleSp) const
{
  for (TaxResponse* taxResponse : taxResponses)
  {
    // Update VC and TaxResponse map in the FarePath
    if (_trx->isValidatingCxrGsaApplicable() &&
        (_trx->diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
         _trx->diagnostic().diagnosticType() == TaxRecSummaryDiagnostic ||
         _trx->diagnostic().diagnosticType() == LegacyTaxDiagnostic24 ||
         _trx->diagnostic().diagnosticType() == PFCRecSummaryDiagnostic ||
         _trx->diagnostic().diagnosticType() == Diagnostic24  ||
         _trx->diagnostic().diagnosticType() == Diagnostic817 ||
         _trx->diagnostic().diagnosticType() == Diagnostic827))
    {
      // itin()->valCxrTaxResponses is used in diagnostics
      farePath.itin()->valCxrTaxResponses().push_back(taxResponse);
    }

    if(_trx->isValidatingCxrGsaApplicable())
    {
      farePath.setValCxrTaxResponses(farePath.itin()->validatingCarrier(), taxResponse);

      // Do not REMOVE when: single carrier and (single sp OR multiple sp with no-TCH)
      if (!farePath.validatingCarriers().empty() || hasTCHForMultipleSp)
        removeTaxResponseFromItin(farePath, taxResponse);
    }
    else if (!farePath.valCxrTaxResponseMap().empty()) // Non GSA
    {
      farePath.setValCxrTaxResponse(farePath.itin()->validatingCarrier(), taxResponse);
      // @todo Need a test to see whether removeCurrentTaxRepsonse is needed here
      if(!taxResponse->taxRecordVector().empty())
        removeCurrentTaxResponse(farePath); // remove taxResponse from itin
    }
  }
}

void
TaxItinerary::removeCurrentTaxResponse(FarePath& farePath) const
{
  if(!farePath.itin()->getTaxResponses().empty())
    farePath.itin()->mutableTaxResponses().pop_back();
}

void
TaxItinerary::removeTaxResponseFromItin(FarePath& farePath, TaxResponse* taxResponse) const
{
  std::vector<TaxResponse*>& taxResponses = farePath.itin()->mutableTaxResponses();
  if(!taxResponses.empty())
  {
    auto it = std::find(taxResponses.begin(), taxResponses.end(), taxResponse);
    if (it != taxResponses.end())
      taxResponses.erase(it);
  }
}

CountrySettlementPlanInfo*
TaxItinerary::getNonTCHCountrySettlementPlanInfo(const std::vector<CountrySettlementPlanInfo*>& cspiCol) const
{
  for (CountrySettlementPlanInfo* cspi : cspiCol)
    if ("TCH" != cspi->getSettlementPlanTypeCode())
      return cspi;
  return nullptr;
}

}
