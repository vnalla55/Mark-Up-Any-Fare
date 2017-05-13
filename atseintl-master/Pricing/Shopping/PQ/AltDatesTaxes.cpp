/*
 * AltDatesTaxes.cpp
 *
 *  Created on: May 3, 2012
 *      Author: Masud Khan
 *
 * The copyright to the computer program(s) herein
 * is the property of Sabre.
 * The program(s) may be used and/or copied only with
 * the written permission of Sabre or in accordance
 * with the terms and conditions stipulated in the
 * agreement/contract under which the program(s)
 * have been supplied.
 *
 */

#include "Pricing/Shopping/PQ/AltDatesTaxes.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/TaxResponse.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag941Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/Shopping/PQ/SoloADGroupFarePath.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SoloSurcharges.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

#include <cmath>
#include <exception>

namespace tse
{
Logger
AltDatesTaxes::_logger("atseintl.ShoppingPQ.AltDatesTaxes");
namespace
{
ConfigurableValue<uint64_t>
cSolutionPerDatePair("SHOPPING_DIVERSITY", "NUM_SOLUTIONS_PER_DATEPAIR");
ConfigurableValue<uint64_t>
cSolutionFirstCxr("SHOPPING_DIVERSITY", "NUM_SOLUTIONS_FIRST_CARRIER");
}
AltDatesTaxes::AltDatesTaxes(ShoppingTrx& trx, SoloSurcharges& soloSurcharges)
  : _cSolutionPerDatePair(0),
    _cSolutionFirstCxr(0),
    _trx(trx),
    _soloSurcharges(soloSurcharges),
    _isDiag941(trx.diagnostic().diagnosticType() == Diagnostic941),
    _taxDiag(nullptr)
{
  if (trx.getOptions()->isEnableCalendarForInterlines())
  {
    _cSolutionPerDatePair = trx.getOptions()->getRequestedNumberOfSolutions();
  }
  else
  {
    _cSolutionPerDatePair = cSolutionPerDatePair.getValue();
  }

  initTaxDiagnostics();
  _cSolutionFirstCxr = cSolutionFirstCxr.getValue();
  TSE_ASSERT(_cSolutionFirstCxr <= _cSolutionPerDatePair);
}

void
AltDatesTaxes::initTaxDiagnostics()
{
  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (diagType == AllPassTaxDiagnostic281 || diagType == LegacyTaxDiagnostic24)
  {
    DCFactory* factory = DCFactory::instance();
    _taxDiag = factory->create(_trx);
    _taxDiag->trx() = &_trx;
    _taxDiag->enable(diagType);
  }
}

void
AltDatesTaxes::flushTaxDiagnostics()
{
  if (_taxDiag)
  {
    _taxDiag->flushMsg();
  }
}

void
AltDatesTaxes::printTaxDiagnostics(const TaxResponse& taxResponse)
{
  if (_taxDiag)
  {
    *_taxDiag << taxResponse;
  }
}

void
AltDatesTaxes::getTaxedPrice(shpq::SoloGroupFarePath* gfp, Solution solution)
{
  _soloSurcharges.restoreTotalNUCAmount(gfp);

  FarePath* farePath = gfp->groupFPPQItem().front()->farePath();

  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SURCHARGE_RULE); // category 12
  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(FPRuleValidation,
                                                                        categories);

  // First cleanup existing tax amounts
  std::vector<PricingUnit*>::const_iterator i = farePath->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator j = farePath->pricingUnit().end();

  for (; i != j; ++i)
  {
    farePath->decreaseTotalNUCAmount((*i)->taxAmount());
    (*i)->setTotalPuNucAmount((*i)->getTotalPuNucAmount() - (*i)->taxAmount());
    ruleController.validate(_trx, *farePath, **i);
    (*i)->taxAmount() = 0;
  }

  MoneyAmount notaxPrice = farePath->getTotalNUCAmount();
  gfp->setTotalNUCBaseFareAmount(notaxPrice);

  // Price after tax
  MoneyAmount taxAmt = getTax(farePath, solution);

  RuleUtil::getSurcharges(_trx, *farePath);
  farePath->increaseTotalNUCAmount(taxAmt);

  gfp->setTotalNUCAmount(farePath->getTotalNUCAmount());
}

void
AltDatesTaxes::removeUnwantedSolutions(ShoppingTrx::FlightMatrix& flightMatrix)
{
  TSELatencyData metrics(_trx, "ALTDATES TAX");

  if (0 == _cSolutionPerDatePair)
  {
    // If number of solutions allowed per date pair is undefined, we will not remove any solution
    // from the flight matrix. _cSolutionPerDatePair == 0 indicates it's undefined.
    return;
  }

  DiagMonitor diag(DCFactory::instance(), _trx, Diagnostic941);
  bool diagEnabled = diag.diag().isActive();
  Diag941Collector* dc = nullptr;
  if (diagEnabled)
  {
    dc = &dynamic_cast<Diag941Collector&>(diag.diag());
    dc->printAltDatesTaxHeader(_cSolutionPerDatePair);
  }

  SortedFlightMatrix sortedFlightMatrix;
  applyTaxes(flightMatrix, sortedFlightMatrix);

  for (const SortedFlightMatrix::value_type& datePairScope : sortedFlightMatrix)
  {
    removeUnwantedSolutionsForDatePair(flightMatrix, datePairScope, dc);
  }
}

void
AltDatesTaxes::applyTaxes(ShoppingTrx::FlightMatrix& flightMatrix,
                          SortedFlightMatrix& sortedFlightMatrix)
{
  using shpq::SoloADGroupFarePath;

  for (Solution solution : flightMatrix)
  {
    SoloADGroupFarePath& gfp = dynamic_cast<SoloADGroupFarePath&>(*solution.second);
    getTaxedPrice(&gfp, solution);

    sortedFlightMatrix[gfp._datePair].insert(solution);
  }
}

void
AltDatesTaxes::removeUnwantedSolutionsForDatePair(
    ShoppingTrx::FlightMatrix& flightMatrix,
    const std::pair<DatePair, AltDatesTaxes::SolutionsInDatePair>& scope,
    Diag941Collector* dc)
{
  const SolutionsInDatePair& solInDP = scope.second;

  GroupFarePath* gfp = solInDP.begin()->second;
  MoneyAmount solTotalPrice = gfp->getTotalNUCAmount();
  MoneyAmount cheapestAmountInDP = solTotalPrice;

  size_t cSolutionKeptForDatepair = 0;
  _cxrCodeVec.clear(); // Clear the carrier code vector for a new date pair

  SolutionKeepVec solKeepVec; // To keep track of solutions to remove

  // Find the cutoff point for each datePair.
  // Remove the solution from flightMatrix if it's beyond the cutoff number
  for (const Solution& sol : solInDP)
  {
    std::string cxrCodes = getCarrierCode(sol.first);
    bool isNewCxr = checkAndAddCarrierCode(cxrCodes);

    gfp = sol.second;
    solTotalPrice = gfp->getTotalNUCAmount();

    const bool isDPCheapestSolution = (std::fabs(cheapestAmountInDP - solTotalPrice) < EPSILON);

    // Always keep the solution until reach the 1st cutoff limit
    // Between 1st and 2nd cutoff limit, only keep the solution if it's from a different carrier
    bool isKept = (cSolutionKeptForDatepair < _cSolutionFirstCxr) ||
                  ((cSolutionKeptForDatepair < _cSolutionPerDatePair) && (isNewCxr)) ||
                  isDPCheapestSolution;
    if (isKept)
    {
      cSolutionKeptForDatepair++;
      solKeepVec.push_back(SolutionKeepItem(sol, true));
    }
    else
    {
      solKeepVec.push_back(SolutionKeepItem(sol, false));
    }
  }

  // Keep more solutions if we don't have enough solutions, only for InterlineCalendar
  if ((_trx.getOptions()->isEnableCalendarForInterlines()) &&
      (cSolutionKeptForDatepair < _cSolutionPerDatePair))
  {
    for (uint16_t index = 0;
         (index < solKeepVec.size()) && (cSolutionKeptForDatepair < _cSolutionPerDatePair);
         index++)
    {
      if (solKeepVec[index].second == false)
      {
        solKeepVec[index].second = true;
        cSolutionKeptForDatepair++;
      }
    }
  }

  // TODO: if we still don't have enough solutions, add flight only solutions

  // Now remove unwanted solutions from flightMatrix
  for (auto& elem : solKeepVec)
  {
    if (dc)
    {
      solTotalPrice = ((elem.first).second)->getTotalNUCAmount();
      dc->printADSolution(elem.first, solTotalPrice, _optTaxData[(elem.first).first], elem.second);
    }

    if (elem.second == false)
    {
      // find is because erase(const key_type&) signature assumes converting
      // shpq::SopIdxVec to temporary object std::vector<int>
      flightMatrix.erase(flightMatrix.find((elem.first).first));
    }
  }
}

MoneyAmount
AltDatesTaxes::getTax(FarePath* farePath, Solution solution)
{
  Itin* curItin = farePath->itin();

  MoneyAmount taxAmt = 0;
  if ((curItin == nullptr) || (!curItin->farePath().empty()))
  {
    return taxAmt;
  }

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(_trx.dataHandle(), taxFactoryMap);

  // Update baseFareCurrency for FarePath
  if (curItin->originationCurrency().empty())
  {
    farePath->baseFareCurrency() = (_trx.journeyItin())->originationCurrency();
  }

  TaxItinerary taxItinerary;
  curItin->farePath().push_back(farePath);
  taxItinerary.initialize(_trx, *curItin, taxFactoryMap);
  taxItinerary.accumulator();
  curItin->farePath().clear();

  if (curItin->getTaxResponses().empty())
  {
    return taxAmt;
  }

  const TaxResponse* taxResponse = curItin->getTaxResponses().front();
  if (taxResponse == nullptr)
  {
    return taxAmt;
  }

  printTaxDiagnostics(*taxResponse);

  TaxResponse::TaxItemVector::const_iterator taxItemIter = taxResponse->taxItemVector().begin();
  TaxResponse::TaxItemVector::const_iterator taxItemEndIter = taxResponse->taxItemVector().end();
  CurrencyCode curCode;
  TaxComponentVec taxCompVec;

  CurrencyConversionFacade ccFacade;
  for (; taxItemIter != taxItemEndIter; taxItemIter++)
  {
    const TaxItem* taxItem = *taxItemIter;

    if (!taxItem->paymentCurrency().equalToConst("NUC") && !taxItem->paymentCurrency().empty() &&
        (taxItem->taxAmount() != 0))
    {
      try
      {
        Money nuc(NUC);
        Money taxCurrency(taxItem->taxAmount(), taxItem->paymentCurrency());

        if (LIKELY(ccFacade.convert(nuc, taxCurrency, _trx)))
          taxAmt += nuc.value();
      }
      catch (const std::exception& ex)
      {
        LOG4CXX_ERROR(_logger, "Currency conversion error: " << ex.what());
      }
    }
    else
      taxAmt += taxItem->taxAmount();

    if (_isDiag941)
    {
      TaxComponent taxComp(taxItem->taxCode(), taxItem);
      taxCompVec.push_back(taxComp);
    }
  }

  if (_isDiag941)
  {
    (_optTaxData[solution.first]) = taxCompVec;
  }

  return taxAmt;
}

std::string
AltDatesTaxes::getCarrierCode(shpq::SopIdxVec sopVec)
{
  std::string cxrCodes;
  CarrierCode cxrOB = _trx.legs()[0].sop()[sopVec[0]].governingCarrier();
  if (_trx.legs().size() == 2)
  {
    // Add inbound carrier into consideration too
    CarrierCode cxrIB = _trx.legs()[1].sop()[sopVec[1]].governingCarrier();
    cxrCodes = (cxrIB == cxrOB) ? cxrOB : "Interline";
  }
  else
  {
    cxrCodes = cxrOB;
  }
  return cxrCodes;
}

// Check if a carrier code already exists in vector. Return true if new
// and add to the vector. Otherwise return false
bool
AltDatesTaxes::checkAndAddCarrierCode(std::string cxrCode)
{
  bool found = false;
  if (!(_cxrCodeVec.empty()))
  {
    // Check for a possible match in the vector
    for (auto& elem : _cxrCodeVec)
    {
      if (elem == cxrCode)
      {
        found = true;
        break;
      }
    }
  }

  if (!found)
  {
    _cxrCodeVec.push_back(cxrCode);
  }

  return !found;
}

} /* namespace tse */
