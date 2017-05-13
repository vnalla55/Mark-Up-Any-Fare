// ----------------------------------------------------------------------------
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

#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder.h"
#include "Taxes/LegacyTaxes/ColumbiaPOS.h"
#include "Taxes/LegacyTaxes/GetTaxCodeReg.h"
#include "Taxes/LegacyTaxes/GetTaxCodeRegAdapter.h"
#include "Taxes/LegacyTaxes/GetTaxNation.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "Taxes/LegacyTaxes/TaxCodeRegSelector.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TaxUS2.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/LegacyTaxes/ZPAbsorption.h"
#include "Taxes/LegacyTaxes/TaxDriver.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFeeConfig.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFeeDriver.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TaxUS2.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/LegacyTaxes/ZPAbsorption.h"

#ifdef UT_DATA_DUMP
#include "Taxes/LegacyTaxes/UnitTestDataDumper.h"

#endif

#include "Common/Logger.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(taxRefactorDiags);
FALLBACK_DECL(automatedRefundCat33TaxDriverRefactoring);
FALLBACK_DECL(cat33FixTaxesOnChangeFeeAF);
FALLBACK_DECL(Cat33_Diag)
};
using namespace std;

log4cxx::LoggerPtr
tse::TaxDriver::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxDriver"));

namespace tse
{
const std::string
TaxDriver::TAX_DATA_ERROR("TAXES MAY BE INCOMPLETE");
const std::string
TaxDriver::TAX_US1("US1");
const std::string
TaxDriver::TAX_YZ1("YZ1");

const std::list<TaxCode>
taxOnChangeFeeList = {"XQ2", "XG2", "RC2"};

const NationCode
taxOnChangeFeeNation("CA");

TaxApplicator&
TaxApplyFactory::getTaxApplicator(PricingTrx& trx,
    const std::vector<TaxCodeReg*>& taxCodeReg,
    const DateTime& ticketDate)
{
  if (utc::isTaxOnOC(trx, *(taxCodeReg.front())))
  {
    return _taxApplyOnOC;
  }
  else if (utc::isTaxOnChangeFee(trx, *(taxCodeReg.front()), ticketDate))
  {
    return _taxApplyOnChangeFee;
  }
  else
  {
    AncillaryPricingTrx* ancillaryPricingTrx = dynamic_cast<AncillaryPricingTrx*>(&trx);

    if (UNLIKELY(ancillaryPricingTrx))
      return _taxApplicator;

    if (UNLIKELY(!trx.getRequest()->taxOverride().empty()))
      return _taxApplicator;

    return _taxApply;
  }
}

void
TaxDriver::processTaxesOnChangeFeeForCat33(PricingTrx& trx,
    TaxResponse& taxResponse, TaxMap& taxMap,
    const CountrySettlementPlanInfo* cspi)
{
  ItinSelector itinSelector(trx);
  if (itinSelector.isRefundTrx() && itinSelector.isNewItin())
  {
    GetTaxNation getTaxNation(trx, taxResponse, cspi);
    GetTaxCodeRegAdapter getTaxCodeRegAdapter(trx);
    UtcConfigAdapter utcConfigAdapter(trx);

    TaxOnChangeFeeDriver taxOnChangeFeeDriver(trx, getTaxNation, getTaxCodeRegAdapter, utcConfigAdapter);

    if (!taxOnChangeFeeDriver.getTaxCodeReg().empty())
    {
      validateAndApplyTaxSeq(trx, taxResponse, taxMap,
          taxOnChangeFeeDriver.getTaxCodeReg(), GetTicketingDate(trx).get(),
          cspi);
    }
  }
}

void
TaxDriver::processDefaultTaxAndFees(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxMap& taxMap,
                                    const CountrySettlementPlanInfo* cspi)
{
  buildTaxNationVector(trx, taxResponse, cspi);

  if (_taxNationVector.empty())
  {
    // Super Fare Hammer request for additional Data Error Information

    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

    if (taxTrx)
    {
      boost::lock_guard<boost::mutex> g(taxTrx->mutexWarningMsg());
      taxTrx->warningMsg()[taxResponse.farePath()->itin()->sequenceNumber()] = TAX_DATA_ERROR;
    }

    TaxDiagnostic::collectErrors(
        trx, taxResponse, TaxDiagnostic::NO_TAX_NATION_FOUND, Diagnostic808);
    return;
  }

  TaxTrx* taxTransaction = dynamic_cast<TaxTrx*>(&trx);

  if (LIKELY((taxTransaction == nullptr) || ((taxTransaction != nullptr) && (!taxTransaction->isShoppingPath()))))
  {
    if (!trx.getRequest()->taxOverride().empty())
    {
      LOG4CXX_INFO(_logger, "Tax Override Requested");

      TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::OVERRIDE, Diagnostic808);
    }
    else
    {
      if (!trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
      {
        validateAndApplyServicesFees(trx, taxResponse);
      }
    }
  }

  std::vector<const TaxNation*>::iterator taxNationI;
  std::vector<TaxCode>::const_iterator taxCodeI;

  std::list<TaxCode> changeFeeTaxes = taxOnChangeFeeList;

  for (taxNationI = _taxNationVector.begin(); taxNationI != _taxNationVector.end(); taxNationI++)
  {
    if ((*taxNationI)->taxCodeOrder().empty())
    {
      // Super Fare Hammer request for additional Data Error Information

      TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

      if (taxTrx)
      {
        boost::lock_guard<boost::mutex> g(taxTrx->mutexWarningMsg());
        taxTrx->warningMsg()[taxResponse.farePath()->itin()->sequenceNumber()] = TAX_DATA_ERROR;
      }
      continue;
    }


    std::list<TaxCode>* pChangeFeeTaxes = nullptr;

    if (!fallback::automatedRefundCat33TaxDriverRefactoring(&trx))
    {
      ItinSelector itinSelector(trx);
      pChangeFeeTaxes = itinSelector.isExchangeTrx()
              && taxOnChangeFeeNation == (*taxNationI)->nation() ?
              &changeFeeTaxes : nullptr;
    }
    else
    {
      pChangeFeeTaxes = trx.isExchangeTrx()
              && taxOnChangeFeeNation == (*taxNationI)->nation() ?
              &changeFeeTaxes : nullptr;
    }

    for (taxCodeI = (*taxNationI)->taxCodeOrder().begin();
         taxCodeI != (*taxNationI)->taxCodeOrder().end();
         taxCodeI++)
    {
      validateAndApplyTax(trx, taxResponse, taxMap, *taxCodeI, cspi, pChangeFeeTaxes);
    } //  End of for loop through taxCodeOrderVector

    while (pChangeFeeTaxes && !pChangeFeeTaxes->empty())
      validateAndApplyTax(
          trx, taxResponse, taxMap, pChangeFeeTaxes->front(), cspi, pChangeFeeTaxes);
  } //  End of for loop through taxNationVector
}

void
TaxDriver::ProcessTaxesAndFees(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxMap::TaxFactoryMap& taxFactoryMap,
                               const CountrySettlementPlanInfo* cspi)
{
  if (fallback::Cat33_Diag(&trx))
    return ProcessTaxesAndFees_OLD(trx, taxResponse, taxFactoryMap, cspi);


  if (!tax::FareDisplayUtil::checkTicketingAgentInformation(trx))
  {
    LOG4CXX_WARN(_logger, "No Ticketing Agent Information");

    TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::POINT_OF_SALE, Diagnostic808);
    return;
  }

#ifdef UT_DATA_DUMP
  UnitTestDataDumper::dump(trx, taxResponse);
#endif

  TaxDiagnostic::printHelpInfo(trx);

  TaxMap taxMap(trx.dataHandle());
  taxMap.initialize(taxFactoryMap);

  ItinSelector itinSelector(trx);
  if (!itinSelector.isNewItin() || !itinSelector.isCat33FullRefund())
    processDefaultTaxAndFees(trx, taxResponse, taxMap, cspi);

  if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
    processTaxesOnChangeFeeForCat33(trx, taxResponse, taxMap, cspi);

  if (!trx.getRequest()->taxOverride().empty())
  {
    LOG4CXX_INFO(_logger, "Tax Override Requested");

    TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::OVERRIDE, Diagnostic808);
    return;
  }

  ZPAbsorption zPAbsorption;
  zPAbsorption.applyZPAbsorption(trx, taxResponse);
}

void
TaxDriver::ProcessTaxesAndFees_OLD(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxMap::TaxFactoryMap& taxFactoryMap,
                               const CountrySettlementPlanInfo* cspi)
{
  if (!tax::FareDisplayUtil::checkTicketingAgentInformation(trx))
  {
    LOG4CXX_WARN(_logger, "No Ticketing Agent Information");

    TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::POINT_OF_SALE, Diagnostic808);
    return;
  }

#ifdef UT_DATA_DUMP
  UnitTestDataDumper::dump(trx, taxResponse);
#endif

  TaxDiagnostic::printHelpInfo(trx);

  buildTaxNationVector(trx, taxResponse, cspi);

  if (_taxNationVector.empty())
  {
    // Super Fare Hammer request for additional Data Error Information

    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

    if (taxTrx)
    {
      boost::lock_guard<boost::mutex> g(taxTrx->mutexWarningMsg());
      taxTrx->warningMsg()[taxResponse.farePath()->itin()->sequenceNumber()] = TAX_DATA_ERROR;
    }

    TaxDiagnostic::collectErrors(
        trx, taxResponse, TaxDiagnostic::NO_TAX_NATION_FOUND, Diagnostic808);
    return;
  }

  TaxTrx* taxTransaction = dynamic_cast<TaxTrx*>(&trx);

  if (LIKELY((taxTransaction == nullptr) || ((taxTransaction != nullptr) && (!taxTransaction->isShoppingPath()))))
  {
    if (!trx.getRequest()->taxOverride().empty())
    {
      LOG4CXX_INFO(_logger, "Tax Override Requested");

      TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::OVERRIDE, Diagnostic808);
    }
    else
    {
      if (!trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
      {
        validateAndApplyServicesFees(trx, taxResponse);
      }
    }
  }

  TaxMap taxMap(trx.dataHandle());
  taxMap.initialize(taxFactoryMap);

  std::vector<const TaxNation*>::iterator taxNationI;
  std::vector<TaxCode>::const_iterator taxCodeI;

  std::list<TaxCode> changeFeeTaxes = taxOnChangeFeeList;

  for (taxNationI = _taxNationVector.begin(); taxNationI != _taxNationVector.end(); taxNationI++)
  {
    if ((*taxNationI)->taxCodeOrder().empty())
    {
      // Super Fare Hammer request for additional Data Error Information

      TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

      if (taxTrx)
      {
        boost::lock_guard<boost::mutex> g(taxTrx->mutexWarningMsg());
        taxTrx->warningMsg()[taxResponse.farePath()->itin()->sequenceNumber()] = TAX_DATA_ERROR;
      }
      continue;
    }


    std::list<TaxCode>* pChangeFeeTaxes = nullptr;

    if (!fallback::automatedRefundCat33TaxDriverRefactoring(&trx))
    {
      ItinSelector itinSelector(trx);
      pChangeFeeTaxes = itinSelector.isExchangeTrx()
              && taxOnChangeFeeNation == (*taxNationI)->nation() ?
              &changeFeeTaxes : nullptr;
    }
    else
    {
      pChangeFeeTaxes = trx.isExchangeTrx()
              && taxOnChangeFeeNation == (*taxNationI)->nation() ?
              &changeFeeTaxes : nullptr;
    }

    for (taxCodeI = (*taxNationI)->taxCodeOrder().begin();
         taxCodeI != (*taxNationI)->taxCodeOrder().end();
         taxCodeI++)
    {
      validateAndApplyTax(trx, taxResponse, taxMap, *taxCodeI, cspi, pChangeFeeTaxes);
    } //  End of for loop through taxCodeOrderVector

    while (pChangeFeeTaxes && !pChangeFeeTaxes->empty())
      validateAndApplyTax(
          trx, taxResponse, taxMap, pChangeFeeTaxes->front(), cspi, pChangeFeeTaxes);
  } //  End of for loop through taxNationVector

  if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
    processTaxesOnChangeFeeForCat33(trx, taxResponse, taxMap, cspi);

  if (!trx.getRequest()->taxOverride().empty())
  {
    LOG4CXX_INFO(_logger, "Tax Override Requested");

    TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::OVERRIDE, Diagnostic808);
    return;
  }

  ZPAbsorption zPAbsorption;
  zPAbsorption.applyZPAbsorption(trx, taxResponse);
}

// ----------------------------------------------------------------------------
// Description:  TaxDriver
// ----------------------------------------------------------------------------

void
TaxDriver::ProcessTaxesAndFees(FareDisplayTrx& fareDisplayTrx, TaxResponse& taxResponse)
{
  try
  {
    PricingTrx& trx = dynamic_cast<PricingTrx&>(fareDisplayTrx);
    if (!tax::FareDisplayUtil::checkTicketingAgentInformation(trx))
    {
      LOG4CXX_ERROR(_logger, "No Ticketing Agent Information For Fare Quote");
      return;
    }

    TaxMap taxMap(trx.dataHandle());
    taxMap.initialize();

    if (fareDisplayTrx.getRequest()->requestType() != FARE_TAX_REQUEST)
    {
      TaxCode taxCode = TAX_YZ1;
      validateAndApplyTax(trx, taxResponse, taxMap, taxCode, trx.countrySettlementPlanInfo());
    }

    if (LocUtil::isUS(*trx.getRequest()->ticketingAgent()->agentLocation()) &&
        !LocUtil::isUSTerritoryOnly(*trx.getRequest()->ticketingAgent()->agentLocation()) &&
        fareDisplayTrx.getRequest()->requestType() != FARE_TAX_REQUEST)
    {
      if (taxResponse.farePath()->itin()->geoTravelType() == GeoTravelType::Domestic ||
          taxResponse.farePath()->itin()->geoTravelType() == GeoTravelType::Transborder)
      {
        TaxCode taxCode = TAX_US1;
        validateAndApplyTax(trx, taxResponse, taxMap, taxCode, trx.countrySettlementPlanInfo());
      }

      else if (fareDisplayTrx.getOptions()->displayBaseTaxTotalAmounts() != TRUE_INDICATOR)

        return;
    }

    TaxDiagnostic::printHelpInfo(trx);

    buildTaxNationVector(trx, taxResponse, trx.countrySettlementPlanInfo());

    if (_taxNationVector.empty())
    {
      LOG4CXX_WARN(_logger, "No Nations Information");
      return;
    }

    std::vector<const TaxNation*>::iterator taxNationI;
    std::vector<TaxCode>::const_iterator taxCodeI;
    std::list<TaxCode> changeFeeTaxes = taxOnChangeFeeList;

    for (taxNationI = _taxNationVector.begin(); taxNationI != _taxNationVector.end(); taxNationI++)
    {
      if ((*taxNationI)->taxCodeOrder().empty())
        continue;

      std::list<TaxCode>* pChangeFeeTaxes = nullptr;

      if (!fallback::automatedRefundCat33TaxDriverRefactoring(&trx))
      {
        ItinSelector itinSelector(trx);
        pChangeFeeTaxes = itinSelector.isExchangeTrx()
                && taxOnChangeFeeNation == (*taxNationI)->nation() ?
                &changeFeeTaxes : nullptr;
      }
      else
      {
        pChangeFeeTaxes = trx.isExchangeTrx()
                && taxOnChangeFeeNation == (*taxNationI)->nation() ?
                &changeFeeTaxes : nullptr;
      }

      for (taxCodeI = (*taxNationI)->taxCodeOrder().begin();
           taxCodeI != (*taxNationI)->taxCodeOrder().end();
           taxCodeI++)
      {
        validateAndApplyTax(
            trx, taxResponse, taxMap, *taxCodeI, trx.countrySettlementPlanInfo(), pChangeFeeTaxes);
      } //  End of for loop through taxCodeOrderVector

      while (pChangeFeeTaxes && !pChangeFeeTaxes->empty())
        validateAndApplyTax(trx,
                            taxResponse,
                            taxMap,
                            pChangeFeeTaxes->front(),
                            trx.countrySettlementPlanInfo(),
                            pChangeFeeTaxes);
    } //  End of for loop through taxNationVector
  }
  catch (bad_cast&)
  {
    LOG4CXX_ERROR(_logger, "Dynamic Cast From Fare Quote Trx To Pricing Trx");
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void* TaxDriver::buildTaxNationVector
//
// Description:  This function will build a vector of countries represented in the
// itinerary and the point of Sale country. It will determine which countries are
// required by using the NationCollect data, and collectionNation1 & collectionNation2.
// This vector will be used to access the appropriate taxes for the itinerary.
//
// @param  itin - itinerary object
//
// @param  inDate - sales date
//
// @return - pointer to a vector of nations whose taxes are to be collected
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxDriver::buildTaxNationVector(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                const CountrySettlementPlanInfo* cspi)
{

  addNation(trx, NATION_ALL);

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (trx.getRequest()->getSettlementMethod() == "TCH" ||
        (cspi && cspi->getSettlementPlanTypeCode() == "TCH"))
      addNation(trx, NATION_ALL);
  }
  else
    if (trx.getRequest()->getSettlementMethod() == "TCH")
      addNation(trx, NATION_ALL);

  const Loc* pointOfSaleLocation = TrxUtil::ticketingLoc(trx);

  if (trx.getRequest()->ticketPointOverride().empty())
  {
    pointOfSaleLocation = TrxUtil::saleLoc(trx);
  }

  const TaxNation* taxNation =
      trx.dataHandle().getTaxNation(pointOfSaleLocation->nation(), trx.getRequest()->ticketingDT());

  if (taxNation == nullptr)
    return;

  const AirSeg* airSeg;
  std::vector<TravelSeg*>::iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  if (LocUtil::isColumbia(*pointOfSaleLocation))
  {
    ColumbiaPOS columbiaPOS;

    if (columbiaPOS.chargeUSTaxes(trx, taxResponse))
      addNation(trx, UNITED_STATES);

    if (columbiaPOS.chargeMXTaxes(trx, taxResponse))
      addNation(trx, MEXICO);

    if (columbiaPOS.chargeAllTaxes(trx, taxResponse))
    {
      for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
      {
        airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (!airSeg)
          continue;

        addNation(trx, (*travelSegI)->origin()->nation());

        if ((*travelSegI)->origin()->nation() != (*travelSegI)->destination()->nation())
        {
          addNation(trx, (*travelSegI)->destination()->nation());
        }
      }
    }
  }

  std::vector<const Loc*>::iterator locI;
  bool rv = false;
  bool rc = false;

  switch (taxNation->taxCollectionInd())
  {
  case NONE:
    return;

  case SALE_COUNTRY:

    rv = addNation(trx, pointOfSaleLocation->nation());

    if (!taxNation->collectionNation1().empty())
    {
      rv = addNation(trx, taxNation->collectionNation1());
    }

    if (!taxNation->collectionNation2().empty())
    {
      rv = addNation(trx, taxNation->collectionNation2());
    }
    return;

  case ALL:

    addNation(trx, TrxUtil::saleLoc(trx)->nation());
    addNation(trx, TrxUtil::ticketingLoc(trx)->nation());

    for (travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
         travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
         travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      addNation(trx, (*travelSegI)->origin()->nation());

      if ((*travelSegI)->origin()->nation() != (*travelSegI)->destination()->nation())
      {
        addNation(trx, (*travelSegI)->destination()->nation());
      }

      for (locI = (*travelSegI)->hiddenStops().begin(); locI != (*travelSegI)->hiddenStops().end();
           locI++)
      {
        addNation(trx, (*locI)->nation());
      }
    }
    break;

  case SELECTED:
  case EXCLUDED:
    rv = addNation(trx, pointOfSaleLocation->nation());

    for (travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
         travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
         travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      rc = findNationCollect((*travelSegI)->origin()->nation(), *taxNation);

      if (rc)
      {
        rv = addNation(trx, (*travelSegI)->origin()->nation());
      }

      if ((*travelSegI)->origin()->nation() != (*travelSegI)->destination()->nation())
      {
        rc = findNationCollect((*travelSegI)->destination()->nation(), *taxNation);

        if ((rc && taxNation->taxCollectionInd() == SELECTED) ||
            (!rc && taxNation->taxCollectionInd() == EXCLUDED))
        {
          rv = addNation(trx, (*travelSegI)->destination()->nation());
        }
      }
    }
    break;

  default:
    break;
  }
  for (size_t index = 0; index < _taxNationVector.size(); ++index)
  {
    const TaxNation* taxNation(trx.dataHandle().getTaxNation(_taxNationVector[index]->nation(),
                                                             trx.getRequest()->ticketingDT()));
    if (UNLIKELY(!taxNation))
    {
      continue;
    }
    if (!taxNation->collectionNation1().empty())
    {
      rv = addNation(trx, taxNation->collectionNation1());
      if (rv)
      {
        index = _taxNationVector.size() - 1;
      }
    }
    if (UNLIKELY(!taxNation->collectionNation2().empty()))
    {
      rv = addNation(trx, taxNation->collectionNation2());
      if (rv)
      {
        index = _taxNationVector.size() - 1;
      }
    }
  }
} // End of BuildTaxNationVector

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool TaxDriver::addNation
//
// Description:  This routine will add the nation to the TaxNationVector if it
// does not already exist in the vector.
//
// @param  inCountry - country to be added
//
// @param  pTNData - pointer to taxNation Data
//
// @return - true if added, false if not added to the vector
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxDriver::addNation(PricingTrx& trx, const NationCode& inCountry)
{
  if (findNation(trx, inCountry))
    return false;

  if (!TaxDiagnostic::isValidNation(trx, inCountry))
    return false;

  const TaxNation* taxNation =
      trx.dataHandle().getTaxNation(inCountry, trx.getRequest()->ticketingDT());

  if (UNLIKELY(taxNation == nullptr))
  {
    LOG4CXX_WARN(_logger, "No TaxNation Table For Nation: " << inCountry);
    return false;
  }

  _taxNationVector.push_back(taxNation);

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool TaxDriver::findNation
//
// Description:  This routine will find the nation in the TaxNationVector
//
// @param  inCountry - country searching for in the TaxNationVector
//
// @return - true if found, false if not found
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxDriver::findNation(PricingTrx& trx, const NationCode& inCountry) const
{
  if (!fallback::taxRefactorDiags(&trx))
  {
    auto nationEquals = [&inCountry](const TaxNation* taxNation) { return taxNation->nation() == inCountry; };
    return std::any_of(_taxNationVector.begin(), _taxNationVector.end(), nationEquals);
  }
  else
  {
    std::vector<const TaxNation*>::const_iterator taxNationI;
    for (taxNationI = _taxNationVector.cbegin(); taxNationI != _taxNationVector.cend(); taxNationI++)
    {
      if ((*taxNationI)->nation() == inCountry)
        return true;
    }
    return false;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool TaxDriver::findNationCollect
//
// Description:  This routine will look for a country in the NationCollect.
// If not found, it will return false. If found, it will return true.
//
// @param  inCountry - country searching for in NationExceptions data
//
// @param  ptrTNData - pointer to TaxNationData which includes the tax Nation
//             Collection Vector
//
// @return - true if found, false if not found
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxDriver::findNationCollect(const NationCode& inCountry, const TaxNation& taxNation)
{
  std::vector<NationCode>::const_iterator taxNationCollectI;

  for (taxNationCollectI = taxNation.taxNationCollect().begin();
       taxNationCollectI != taxNation.taxNationCollect().end();
       taxNationCollectI++)
  {
    if (inCountry == (*taxNationCollectI))
    {
      return true;
    }
  }

  return false;
}

// ----------------------------------------------------------------------------
//
// @function void* TaxDriver::validateAndApplyTax
//
// Description:
// ----------------------------------------------------------------------------
void
TaxDriver::validateAndApplyTax(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxMap& taxMap,
                               const TaxCode& taxCode,
                               const CountrySettlementPlanInfo* cspi,
                               std::list<TaxCode>* pChangeFeeTaxes)
{
  if (!TaxDiagnostic::isValidTaxCode(trx, taxCode))
    return;

  const std::vector<TaxCodeReg*>* taxCodeReg = nullptr;
  DateTime ticketDate = trx.dataHandle().ticketDate();

  if (fallback::automatedRefundCat33TaxDriverRefactoring(&trx))
  {
    taxCodeReg = &trx.dataHandle().getTaxCode(taxCode, trx.getRequest()->ticketingDT());

    if (pChangeFeeTaxes && trx.isExchangeTrx())
    {
      auto it = std::find(pChangeFeeTaxes->begin(), pChangeFeeTaxes->end(), taxCode);
      if (it != pChangeFeeTaxes->end())
      {
        pChangeFeeTaxes->erase(it);
        const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);
        if (exchangeTrx && exchangeTrx->currentTicketingDT().isValid())
        {
          trx.dataHandle().setTicketDate(exchangeTrx->currentTicketingDT());

          const std::vector<TaxCodeReg*>* taxCodeRegNew =
            &trx.dataHandle().getTaxCode(taxCode, exchangeTrx->currentTicketingDT());

          trx.dataHandle().setTicketDate(ticketDate);

          if (taxCodeRegNew && !taxCodeRegNew->empty() &&
            utc::isMatchOrigTicket(trx, *taxCodeRegNew->front(), exchangeTrx->currentTicketingDT()))
          {
            taxCodeReg = taxCodeRegNew;
            ticketDate = exchangeTrx->currentTicketingDT();
          }
        }
      }
    }
  }
  else
  {
    std::unique_ptr<GetTaxCodeReg> getTaxCodeReg = GetTaxCodeReg::create(trx, taxCode, pChangeFeeTaxes);
    taxCodeReg = getTaxCodeReg->taxCodeReg();
    ticketDate = getTaxCodeReg->ticketingDate();
  }

  if (!taxCodeReg || taxCodeReg->empty())
  {
    // Super Fare Hammer request for additional Data Error Information
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

    if (UNLIKELY(taxTrx))
    {
      boost::lock_guard<boost::mutex> g(taxTrx->mutexWarningMsg());
      taxTrx->warningMsg()[taxResponse.farePath()->itin()->sequenceNumber()] = TAX_DATA_ERROR;
    }

    LOG4CXX_WARN(_logger, "No TaxCodeReg Table For TaxCode: " << taxCode);

    TaxDiagnostic::collectErrors(trx, taxResponse, TaxDiagnostic::NO_TAX_CODE, Diagnostic808,
        std::string("NO SEQUENCE FOR ") + taxCode);

    return;
  }

  TaxCodeRegSelector taxCodeRegSelector(trx, *taxCodeReg);

  if (taxCodeRegSelector.hasItin() && !trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
  {
    validateAndApplyTaxSeq(
        trx, taxResponse, taxMap, taxCodeRegSelector.getItin(), ticketDate, cspi);
  }

  if (taxCodeRegSelector.hasOC() && !trx.atpcoTaxesActivationStatus().isTaxOnOCBaggage())
  {
    validateAndApplyTaxSeq(
        trx, taxResponse, taxMap, taxCodeRegSelector.getOC(), ticketDate, cspi);
  }

  if (taxCodeRegSelector.hasChangeFee() && !trx.atpcoTaxesActivationStatus().isTaxOnChangeFee())
  {
    if (!fallback::cat33FixTaxesOnChangeFeeAF(&trx))
    {
      if (ItinSelector(trx).isExchangeTrx())
      {
        validateAndApplyTaxSeq(
            trx, taxResponse, taxMap, taxCodeRegSelector.getChangeFee(), ticketDate, cspi);
      }
    }
    else
    {
      validateAndApplyTaxSeq(
          trx, taxResponse, taxMap, taxCodeRegSelector.getChangeFee(), ticketDate, cspi);
    }
  }

  return;
}

// ----------------------------------------------------------------------------
//
// @function void* TaxDriver::validateAndApplyTaxSeq
//
// Description:
// ----------------------------------------------------------------------------
void
TaxDriver::validateAndApplyTaxSeq(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxMap& taxMap,
                                  const std::vector<TaxCodeReg*>& taxCodeReg,
                                  const DateTime& ticketDate,
                                  const CountrySettlementPlanInfo* cspi)
{
  TaxApplicator& taxApplicator = _taxApplyFactory.getTaxApplicator(trx, taxCodeReg, ticketDate);

  std::vector<TaxCodeReg*>::const_iterator taxCodeRegI = taxCodeReg.begin();

  for (; taxCodeRegI != taxCodeReg.end(); taxCodeRegI++)
  {
    if ((*taxCodeRegI)->specialProcessNo() == 13 &&
        taxMap.routeToOldUS2(*(taxResponse.farePath()->itin()), trx) &&
        trx.getRequest()->taxOverride().empty())
    {
      if (!TaxCodeValidator().validateTaxCode(trx, taxResponse, **taxCodeRegI))
        continue;

      TaxUS2 taxUS2;
      taxUS2.applyUS2(trx, taxResponse, **taxCodeRegI);
      break;
    }

    taxApplicator.applyTax(trx, taxResponse, taxMap, **taxCodeRegI, cspi);
  }
  return;
}

void
TaxDriver::validateAndApplyServicesFees(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (LIKELY(taxResponse.farePath()->itin()->anciliaryServiceCode().empty() && // Not Tax on OC OTA
      // request
      dynamic_cast<AncillaryPricingTrx*>(&trx) == nullptr)) // Not Ancillary Pricing
  {
    tse::YQYR::ServiceFee serviceFee;
    serviceFee.collectFees(trx, taxResponse);
  }
}
} // tse namespace
