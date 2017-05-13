//----------------------------------------------------------------------------
//  File:        Diag801Collector.C
//  Authors:     Vladimir Reznikov
//  Created:     Feb 2004
//
//  Description: Diagnostic 02 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Diagnostic/Diag801Collector.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag801Collector::buildTaxDisplayHeader
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
Diag801Collector::buildDiag801Header(const TaxResponse& taxResponse,
                                     TaxDisplayHeaderType taxDisplayHeaderType)
{
  if ((!_active) || (taxDisplayHeaderType != DIAGNOSTIC_HEADER))
    return;

  uint32_t taxResponseSize = (uint32_t)taxResponse.taxItemVector().size();

  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  dc << "\n"
     << "***************************************************************\n";
  if ((typeid(*(taxResponse.farePath())) == typeid(NetFarePath)) &&
      TrxUtil::isCat35TFSFEnabled(*pricingTrx))
    dc << "NET FARE PATH  \n";
  dc << " ---- TAX OUT VECTOR  " << taxResponseSize << " ITEMS --- PSGR "
     // << taxResponse.paxIndex() << " "
     << taxResponse.paxTypeCode() << "----        \n"
     << "***************************************************************\n\n";

  if(pricingTrx->isValidatingCxrGsaApplicable())
  {
     dc << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n"
        << "***************************************************************\n";
  }

  if (pricingTrx && (pricingTrx->getRequest()->owPricingRTTaxProcess() ||
                     pricingTrx->getTrxType() == PricingTrx::MIP_TRX))
  {
    dc << AirlineShoppingUtils::collectSegmentInfo(*pricingTrx, taxResponse.farePath()) << "\n";
  }
  dc << "                                           THRU   THRU   RUR    \n"
     << "  CODE FC TYP  TXAMT      TXTTL   TXFARE   BOARD  OFF    CTY  A \n"
     << "                      \n";
  //  << "                           " << taxResponse.currencyCode() << "     "
  //  << taxResponse.currencyCode() << "\n";
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag801Collector::operator<<
//
// Description:  This method will be override base operator << to handle the
//         Customer WPQ/*281 Tax Diagnostic Display.
//
// @param  PricingTrx - Transaction object
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag801Collector&
Diag801Collector::operator << ( const TaxResponse& taxResponse )
{
  if (!_active)
    return *this;
  //  if ( _taxDisplayTotal == 0 )
  {
    buildDiag801Header(taxResponse, DIAGNOSTIC_HEADER);
  }

  DiagCollector& dc = (DiagCollector&)*this;
  uint32_t taxResponseNum = 1;

  TaxResponse::TaxItemVector::const_iterator i;

  if (taxResponse.taxItemVector().empty())
  {
    dc << " T A X E S   N O T   A P P L I C A B L E \n";
    return *this;
  }

  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.setf(std::ios::right, std::ios::adjustfield);

  for (i = taxResponse.taxItemVector().begin(); i != taxResponse.taxItemVector().end();
       i++, taxResponseNum++)
  {
    Money moneyPayment((*i)->paymentCurrency());
    dc.precision(moneyPayment.noDec());

    outputResponseNum(dc, taxResponseNum);
    dc << " ";
    outputTaxCode(dc, (*i));
    dc << "  ";
    outputFailCode(dc, (*i));
    dc << "  ";
    outputTaxType(dc, (*i));
    dc << " ";
    outputTaxAmounts(dc, (*i));
    outputLocalCities(dc, (*i));
    outputAbsorbtion(dc, (*i));
    dc << "\n";
    outputThruCities(dc, (*i));
    outputDescription(dc, (*i));
    outputRoundRule(dc, (*i));
    outputSequnceNumber(dc, (*i));
  } // End TaxResponse For Loop
  return *this;
}

void
Diag801Collector::outputResponseNum(DiagCollector& dc, uint32_t taxResponseNum)
{
  dc << taxResponseNum;
}

void
Diag801Collector::outputTaxCode(DiagCollector& dc, const TaxItem* taxItem)
{
  dc << taxItem->taxCode();

  if (taxItem->taxCode().size() < 3)
    dc << " ";
}

void
Diag801Collector::outputFailCode(DiagCollector& dc, const TaxItem* taxItem)
{
  dc << static_cast<int>(taxItem->failCode());
}

void
Diag801Collector::outputTaxType(DiagCollector& dc, const TaxItem* taxItem)
{
  dc << taxItem->taxType();
}

void
Diag801Collector::outputTaxAmounts(DiagCollector& dc, const TaxItem* taxItem)
{
  if (taxItem->taxType() != 'P')
  {
    dc << std::setw(8) << taxItem->taxAmount() << taxItem->paymentCurrency() << std::setw(8)
       << taxItem->taxAmount() << " " << std::setw(8) << taxItem->taxableFare() << "    ";
  }
  else
  {
    dc << std::setw(8) << taxItem->taxAmt() * 100 << "    " << std::setw(8) << taxItem->taxAmount()
       << " " << std::setw(8) << taxItem->taxableFare() << "   ";
  }
}

void
Diag801Collector::outputLocalCities(DiagCollector& dc, const TaxItem* taxItem)
{
  dc << taxItem->taxLocalBoard() << "   " << taxItem->taxLocalOff() << "     ";
}

void
Diag801Collector::outputAbsorbtion(DiagCollector& dc, const TaxItem* taxItem)
{
  dc << "\n";
}

void
Diag801Collector::outputThruCities(DiagCollector& dc, const TaxItem* taxItem)
{
  if (taxItem->specialProcessNo() != 0 &&
      (taxItem->taxThruBoard() != "" || taxItem->taxThruOff() != ""))
  {
    dc << "                                            " << taxItem->taxThruBoard() << "   "
       << taxItem->taxThruOff() << "\n";
  }
}

void
Diag801Collector::outputDescription(DiagCollector& dc, const TaxItem* taxItem)
{
  if (taxItem->taxDescription() != "")
  {
    dc << "    " << taxItem->taxDescription() << "\n";
  }
  else
  {
    dc << "INTERNATIONAL TOURISM ARRIVAL TAX\n";
  }
}

void
Diag801Collector::outputRoundRule(DiagCollector& dc, const TaxItem* taxItem)
{
  RoundingRule rr = taxItem->taxcdRoundRule();

  if (rr == UP)
  {
    dc << "           ROUND UP     " << taxItem->taxcdRoundUnit() << "\n";
  }
  else if (rr == DOWN)
  {
    dc << "           ROUND DOWN   " << taxItem->taxcdRoundUnit() << "\n";
  }
  else if (rr == NEAREST)
  {
    dc << "           ROUND NEAREST" << taxItem->taxcdRoundUnit() << "\n";
  }
  else
  {
    dc << "        "
       << "NO ROUNDING SPECIFIED\n";
  }
}

void
Diag801Collector::outputSequnceNumber(DiagCollector& dc, const TaxItem* taxItem)
{
  dc << "        SEQUENCE NUMBER: " << taxItem->seqNo()
     << " FOR CARRIER CODE: " << taxItem->carrierCode() << "\n";
}
}
