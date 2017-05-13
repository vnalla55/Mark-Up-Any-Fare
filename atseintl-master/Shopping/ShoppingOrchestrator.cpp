/*----------------------------------------------------------------------------
 *  File:    ShoppingOrchestrator.C
 *  Created: July 9, 2003
 *  Authors: David White, Adrienne A. Stipe
 *
 *  Description:
 *
 *  Change History:
 *
 *  Copyright Sabre 2004
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#include "Shopping/ShoppingOrchestrator.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/ShoppingRexUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/ExcItin.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Response.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Rules/VoluntaryChanges.h"
#include "Server/TseServer.h"
#include "Shopping/RexConstrainsConsolidator.h"
#include "Shopping/Cat31RestrictionMerger.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <functional>
#include <sstream>
#include <vector>

namespace tse
{

//----------------------------------------------------------------------------
ShoppingOrchestrator::ShoppingOrchestrator(TseServer& srv) : _config(srv.config())
{
}

//----------------------------------------------------------------------------
ShoppingOrchestrator::~ShoppingOrchestrator() {}

/*-----------------------------------------------------------------------------
 * process function
 *
 * main Pricing Orchestrator process transaction method
 *
 * @param trx - reference to a valid transaction object
 * @return    - returns true on success, false on error
 *----------------------------------------------------------------------------*/
bool
ShoppingOrchestrator::process(MetricsTrx& trx)
{
  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "SO Metrics");
  MetricsUtil::lineItemHeader(oss);

  MetricsUtil::lineItem(oss, MetricsUtil::SO_PROCESS);

  return true;
}

//-----------------------------------------------------------------
bool
ShoppingOrchestrator::process(ShoppingTrx& trx)
{
  // after TAX is processed then add tax amount to group/fare path total amount.
  if (!trx.taxResponse().empty() && !trx.isAltDates())
  {
    ShoppingTrx::FlightMatrix& flightMatrix = trx.flightMatrix();
    ShoppingTrx::FlightMatrix::iterator fltMatIEnd = flightMatrix.end();
    MoneyAmount totalTaxGroupPath = 0;
    const TaxResponse* taxResponsePtr = nullptr;

    for (ShoppingTrx::FlightMatrix::iterator fltMatrixI = flightMatrix.begin();
         fltMatrixI != fltMatIEnd;
         ++fltMatrixI)
    {
      GroupFarePath* gfp = fltMatrixI->second;
      if ((gfp == nullptr) || (gfp->getTotalNUCAmount() == 1000000) ||
          (gfp->getTotalNUCAmount() == 0))
      {
        continue;
      }
      else
      {
        std::vector<FPPQItem*>::iterator it = gfp->groupFPPQItem().begin();
        std::vector<FPPQItem*>::iterator itEnd = gfp->groupFPPQItem().end();
        totalTaxGroupPath = 0;
        for (; it != itEnd; ++it)
        {
          FarePath* farePath = (*it)->farePath();
          taxResponsePtr = matchFarePath(farePath, trx);
          if ((farePath != nullptr) && (taxResponsePtr != nullptr))
          {
            totalTaxGroupPath += updateFarePath(gfp, farePath, taxResponsePtr, trx);
          }
        }
        gfp->increaseTotalNUCAmount(totalTaxGroupPath);
      }
    }
  }

  return (true);
}

bool
ShoppingOrchestrator::process(RexShoppingTrx& trx)
{
  // Merge all data across rec3/cat31
  RexShoppingTrx::OADResponseDataMap& oadMergedR3Data = trx.oadResponse();
  const std::vector<FareCompInfo*>& fareComponents = trx.exchangeItin().front()->fareComponent();
  CarrierCode carrier = fareComponents.front()->fareMarket()->governingCarrier();
  auto SameCarrier = [&](const FareCompInfo* fci)
                     {
                       return fci->fareMarket()->governingCarrier() == carrier;
                     };

  bool oneCarrierTicket = std::all_of(++fareComponents.begin(), fareComponents.end(), SameCarrier);

  std::for_each(trx.oadData().begin(), trx.oadData().end(),
                Cat31RestrictionMerger(oadMergedR3Data,
                                       oneCarrierTicket,
                                       *trx.exchangeItin().front(),
                                       trx));

  RexConstrainsConsolidator rCC(trx);
  rCC.process();

  return true;
}

const TaxResponse*
ShoppingOrchestrator::matchFarePath(FarePath* farePath, ShoppingTrx& trx)
{
  // FIXME: why this method doesn't simply take itinerary from given farePath?
  std::vector<TaxResponse*>::iterator taxResponseI;
  std::vector<Itin*>::iterator itineraryI = trx.itin().begin();
  for (; itineraryI != trx.itin().end(); itineraryI++)
  {
    if (const TaxResponse* response = TaxResponse::findFor(*itineraryI, farePath))
    {
      return response;
    }
  }
  return nullptr;
}

MoneyAmount
ShoppingOrchestrator::updateFarePath(GroupFarePath* gfp,
                                     FarePath* farePath,
                                     const TaxResponse* taxResponse,
                                     ShoppingTrx& trx)
{
  if (taxResponse->taxItemVector().empty())
  {
    return 0;
  }
  TaxResponse::TaxItemVector::const_iterator taxItemI = taxResponse->taxItemVector().begin();
  CurrencyCode curCode;
  MoneyAmount totalTax = 0;

  for (; taxItemI != taxResponse->taxItemVector().end(); taxItemI++)
  {
    if ((*taxItemI)->taxAmount() != 0)
    {
      curCode = (*taxItemI)->paymentCurrency();
      totalTax += (*taxItemI)->taxAmount();
    }
  }

  Money nuc(NUC);
  CurrencyConversionFacade ccFacade;

  if ((curCode != "NUC") && (!curCode.empty()) && (totalTax != 0))
  {
    try
    {
      Money taxCurrency(totalTax, curCode);
      if (ccFacade.convert(nuc, taxCurrency, trx))
      {
        totalTax = nuc.value();
      }
    }
    catch (...) { return 0; }
  }

  if (!gfp->isShortCutPricing())
    farePath->increaseTotalNUCAmount(totalTax);

  return totalTax;
}
}
