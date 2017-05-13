//----------------------------------------------------------------------------
//  File:        Diag803Collector.C
//  Authors:     Sommapan Lathitham/Dean Van Decker
//  Created:     Mar 2004
//
//  Description: PFCRecSummaryDiagnostic formatter - Diagnostic 803
//
//  Updates:
//          03/01/04 - Sommapan - Intitial Development
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

#include "Diagnostic/Diag803Collector.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/Pfc/PfcItem.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag803Collector::buildTaxDisplayHeader
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
Diag803Collector::buildPfcHeader(const TaxResponse& taxResponse, PfcHeaderType pfcDisplayHeaderType)
{

  if ((!_active) || (pfcDisplayHeaderType != DIAGNOSTIC_HEADER))
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  if (!_skipCommonHeader)
    dc << " \n***********  PASSENGER FACILITY CHARGE  ***********\n";

  if(pricingTrx->isValidatingCxrGsaApplicable())
  {
     if(!_skipCommonHeader)
        _skipCommonHeader = true;
     dc << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n"
        << "\n***************************************************\n";
  }

  dc << "    ABIND  PFC AMT  CURR  SEGNO  PFC AIRPORT       \n"
     << "---------------------------------------------------\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag803Collector::buildTaxDisplayHeader
//
// Description:  This method will be override base operator << to handle the
//               PFCRecSummary (TaxOutVector) Diagnostic Display.
//
// @param  PricingTrx - Transaction object
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag803Collector&
Diag803Collector::operator << (const TaxResponse& taxResponse)
{
  if (!_active)
    return *this;

  if (!_pfcDisplayTotal || _skipCommonHeader)
  {
    buildPfcHeader(taxResponse, DIAGNOSTIC_HEADER);
  }

  DiagCollector& dc = (DiagCollector&)*this;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  if (pricingTrx && (pricingTrx->getRequest()->owPricingRTTaxProcess() ||
                     pricingTrx->getTrxType() == PricingTrx::MIP_TRX))
  {
    dc << AirlineShoppingUtils::collectSegmentInfo(*pricingTrx, taxResponse.farePath()) << "\n";
  }
  if ((typeid(*(taxResponse.farePath())) == typeid(NetFarePath)) &&
      TrxUtil::isCat35TFSFEnabled(*pricingTrx))
    dc << "NET FARE PATH  \n";

  uint32_t pfcCount = 1;
  char pfcAbsorption = 'N';

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);

  std::vector<PfcItem*>::const_iterator i;

  for (i = taxResponse.pfcItemVector().begin(); i != taxResponse.pfcItemVector().end();
       i++, pfcCount++, _pfcDisplayTotal++)
  {
    Money moneyPayment((*i)->pfcCurrencyCode());
    dc.precision(moneyPayment.noDec());

    pfcAbsorption = 'N';

    if ((*i)->absorptionInd())
      pfcAbsorption = 'Y';

    dc << " " << pfcCount << "    " << pfcAbsorption << "   " << std::setw(8) << (*i)->pfcAmount()
       << "  " << (*i)->pfcCurrencyCode() << "     " << (*i)->couponNumber() << "       "
       << (*i)->pfcAirportCode() << "\n";
  } // End TaxOut For Loop

  dc << " "
     << "\nPAX TYPE " << taxResponse.paxTypeCode()
     << "\n***************************************************\n";

  return *this;
}
}
