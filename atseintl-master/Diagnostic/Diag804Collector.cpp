//----------------------------------------------------------------------------
//  File:        Diag804Collector.C
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 02 formatter
//
//  Updates:
//          02/28/04 - DVD - Intitial Development
//
//  Copyright Sabre 2004
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
#include "Diagnostic/Diag804Collector.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <iomanip>

namespace tse
{
namespace
{
  void printExternalTaxes(DiagCollector& dc, const TaxResponse::TaxItemVector& taxItems)
  {
    dc << "\n EXTERNAL TAXES VECTOR\n";
    dc << " TAX  TAX AMT TVL PORTION\n";
    dc << " --- -------- -----------\n";
    std::string tvlPortion;
    for (const TaxItem* taxItem: taxItems)
    {
      tvlPortion = std::to_string(taxItem->travelSegStartIndex() + 1) + "-" + std::to_string(taxItem->travelSegEndIndex() + 1);
      dc << " " << taxItem->taxCode() << " " << std::right << std::setw(8) << taxItem->taxAmount()
         << " " << std::setw(11) << tvlPortion << "\n";
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag804Collector::buildTaxDisplayHeader
//
// Description:  This method will display diagnostic information to allow for a
//         quick debug of all tax processes. Diagnostic number must be set in
//         the Transaction Orchestrator to apply the following methods:
//
// @param  PricingTrx - Transaction object
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag804Collector::buildTaxDisplayHeader(const TaxResponse& taxResponse,
                                        TaxDisplayHeaderType taxDisplayHeaderType)
{
  if ((!_active) || (taxDisplayHeaderType != DIAGNOSTIC_HEADER))
    return;

  CurrencyCode currencyCode = CurrencyCode("   ");

  TaxResponse::TaxItemVector::const_iterator i = taxResponse.taxItemVector().begin();

  for (; i != taxResponse.taxItemVector().end(); i++)
  {
    if ((*i)->failCode())
      continue;

    currencyCode = (*i)->paymentCurrency();
    break;
  }

  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  if (!_skipCommonHeader)
    dc << "\n********************* TAX LOGIC ANALYSIS *********************\n";

  if(pricingTrx->isValidatingCxrGsaApplicable())
  {
     if(!_skipCommonHeader)
        _skipCommonHeader = true;
     dc << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n"
        << "\n**************************************************************\n";
  }

  dc << "                  FARE AMT               TAX AMT   PUBLISHED\n"
     << "TAX  MARKET         " << currencyCode << "      PERCENTAGE    "
     << currencyCode << "     AMOUNT"
     << "\n--------------------------------------------------------------\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag804Collector::operator <<
//
// Description:  This method will be override base operator << to handle the
//         Customer WPQ/*24 Tax Diagnostic Display.
//
// @param  PricingTrx - Transaction object
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag804Collector&
Diag804Collector::operator << (const  TaxResponse& taxResponse )
{
  if (!_active)
    return *this;

  if (!_taxDisplayTotal || _skipCommonHeader)
  {
    buildTaxDisplayHeader(taxResponse, DIAGNOSTIC_HEADER);
  }

  DiagCollector& dc = (DiagCollector&)*this;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  if (pricingTrx && (pricingTrx->getRequest()->owPricingRTTaxProcess() ||
                     pricingTrx->getTrxType() == PricingTrx::MIP_TRX))
  {
    dc << AirlineShoppingUtils::collectSegmentInfo(*pricingTrx, taxResponse.farePath()) << "\n";
  }
  uint32_t taxOutCount = 0;
  TaxCode taxCode;

  TaxResponse::TaxItemVector::const_iterator i;

  if (taxResponse.taxItemVector().empty())
  {
    dc << " T A X E S   N O T   A P P L I C A B L E \n";
    return *this;
  }
  if ((typeid(*(taxResponse.farePath())) == typeid(NetFarePath)) &&
      TrxUtil::isCat35TFSFEnabled(*pricingTrx))
    dc << "NET FARE PATH  \n";

  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.setf(std::ios::right, std::ios::adjustfield);

  for (i = taxResponse.taxItemVector().begin(); i != taxResponse.taxItemVector().end();
       i++, taxOutCount++, _taxDisplayTotal++)
  {
    if ((*i)->failCode())
      continue;

    taxCode = (*i)->taxCode();

    if (taxCode.size() < 3)
    {
      taxCode += ' ';
    }

    dc << taxCode;

    Money moneyPayment((*i)->paymentCurrency());
    dc.precision(moneyPayment.noDec());

    if ((*i)->partialTax())
    {
      if ((*i)->taxableFare() != (*i)->taxablePartialFare())
      {
        dc << "  " << (*i)->taxThruBoard() << (*i)->taxThruOff() << "     " << std::setw(8)
           << (*i)->taxableFare();
      }
    }

    dc << "\n";

    dc << "     " << (*i)->taxLocalBoard() << (*i)->taxLocalOff() << "     ";

    if ((*i)->partialTax())
    {
      dc << std::setw(8) << (*i)->taxablePartialFare();
    }
    else
    {
      dc << std::setw(8) << (*i)->taxableFare();
    }

    if ((*i)->taxType() == PERCENTAGE)
    {
      dc.precision(3);

      if ((*i)->specialPercentage())
      {
        dc << "  " << std::setw(8) << (*i)->specialPercentage() * 100;
      }
      else
      {
        dc << "  " << std::setw(8) << (*i)->taxAmt() * 100;
      }
      Money moneyPublished((*i)->taxCur());
      dc.precision(moneyPayment.noDec());

      dc << "     " << std::setw(8) << (*i)->taxAmount() << "\n";
    }
    else
    {
      dc << "               ";

      dc << std::setw(8) << (*i)->taxAmount() << " ";

      Money moneyPublished((*i)->taxCur());
      dc.precision(moneyPublished.noDec());

      dc << std::setw(8) << (*i)->taxAmt();
      dc << (*i)->taxCur() << "\n";
    }
  } // End TaxOut For Loop

  dc << " "
     << "\nPAX TYPE " << taxResponse.paxTypeCode();
  RexExchangeTrx* rexExcTrx = dynamic_cast<RexExchangeTrx*>(trx());
  if (rexExcTrx && taxResponse.farePath() &&
      rexExcTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    size_t itinIndex = rexExcTrx->getItinPos(taxResponse.farePath()->itin());
    dc << "\nNEW ITIN " << itinIndex;
  }

  if (taxResponse.farePath() && !taxResponse.farePath()->getExternalTaxes().empty())
    printExternalTaxes(dc, taxResponse.farePath()->getExternalTaxes());
  dc << "\n**************************************************************\n";

  return *this;
}
}
