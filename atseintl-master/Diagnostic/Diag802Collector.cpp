//----------------------------------------------------------------------------
//  File:        Diag802Collector.C
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 802 formatter
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

#include "Diagnostic/Diag802Collector.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag802Collector::buildTaxRecordHeader
//
// Description:  This method will display diagnostic information to allow for a
//         quick debug of all tax processes. Diagnostic number must be set in
//         the Transaction Orchestrator to apply the following methods:
//
// @param  TaxOut - Tax Display and Fare Ladder/Configuration display Object
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag802Collector::buildTaxRecordHeader(const TaxResponse& taxResponse,
                                       TaxRecordHeaderType taxRecordDisplayHeaderType)
{

  if ((!_active) || (taxRecordDisplayHeaderType != DIAGNOSTIC_HEADER))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  if (!_skipCommonHeader)
    dc << "\n******************  TAX RECORD  ******************" << " \n";

  if(pricingTrx->isValidatingCxrGsaApplicable())
  {
    if(!_skipCommonHeader)
       _skipCommonHeader = true;
    dc << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n";

    if (!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
    {
      dc << "SETTLEMENT PLANS: " ;
      for (const SettlementPlanType& sp : taxResponse.settlementPlans())
      {
         dc << sp << " ";
      }
      dc << "\n";
    }
  }

  dc << "   CODE TAXAMT   CURR   TAX XT INTERLINE TAXNATION\n"
     << "--------------------------------------------------\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag802Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//         Tax Record Diagnostic Display.
//
// @param  PricingTrx - Transaction object
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag802Collector&
Diag802Collector::operator<<(const TaxResponse& taxResponse)
{

  if (!_active)
    return *this;

  if (!_taxRecordDisplayTotal || _skipCommonHeader)
  {
    buildTaxRecordHeader(taxResponse, DIAGNOSTIC_HEADER);
  }

  DiagCollector& dc = (DiagCollector&)*this;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  if (pricingTrx && (pricingTrx->getRequest()->owPricingRTTaxProcess() ||
                     pricingTrx->getTrxType() == PricingTrx::MIP_TRX))
  {
    dc << AirlineShoppingUtils::collectSegmentInfo(*pricingTrx, taxResponse.farePath()) << "\n";
  }
  if (typeid(*(taxResponse.farePath())) == typeid(NetFarePath) &&
      TrxUtil::isCat35TFSFEnabled(*pricingTrx))
    dc << "NET FARE PATH  \n";
  else if (taxResponse.farePath()->isAdjustedSellingFarePath())
    dc << "ADJUSTED FARE PATH  \n";

  uint32_t taxRecCount = 1;
  char taxRolledXT;
  char interlineInd;
  TaxCode taxCode;

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);

  MoneyAmount totalAmt = 0.0;
  std::vector<TaxRecord*>::const_iterator i;
  for (i = taxResponse.taxRecordVector().begin(); i != taxResponse.taxRecordVector().end();
       i++, taxRecCount++, _taxRecordDisplayTotal++)
  {
    Money moneyPayment((*i)->taxCurrencyCode());
    dc.precision(moneyPayment.noDec());

    taxRolledXT = 'Y';

    if (!(*i)->taxRolledXTInd())
      taxRolledXT = 'N';

    interlineInd = 'Y';

    if (!(*i)->interlineTaxInd())
      interlineInd = 'N';

    dc << taxRecCount << "  ";

    taxCode = (*i)->taxCode();

    if (strlen(taxCode.c_str()) < 3)
    {
      taxCode += ' ';
    }

    taxCode[2] = ' ';

    dc << taxCode;

    dc << std::setw(8) << (*i)->getTaxAmount() << "   " << (*i)->taxCurrencyCode() << "    "
       << (*i)->taxType() << "   " << taxRolledXT << "  " << interlineInd << "         "
       << (*i)->taxNation() << "\n";

    if (!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
      totalAmt += (*i)->getTaxAmount();
  } // End of TaxRecLoop

  if (!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
    dc << " " << "\nTOTAL AMOUNT: " << totalAmt;
  else
    dc << " ";

  dc << "\nPAX TYPE " << taxResponse.paxTypeCode()
     << "\n**************************************************\n";

  return *this;
}
}
