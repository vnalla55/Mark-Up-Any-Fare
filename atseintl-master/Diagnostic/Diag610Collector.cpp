//----------------------------------------------------------------------------
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
#include "Diagnostic/Diag610Collector.h"

#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "Diagnostic/DiagnosticUtil.h"

#include <iomanip>

namespace tse
{
FALLBACK_DECL(fallbackDisplaySelectedItinNumInDiag)

void
Diag610Collector::printHeader()
{
  if (_active)
  {
    ((DiagCollector&)*this) << "*********** HALF ROUND TRIP COMBINATION ANALYSIS **************\n";
  }
}

DiagCollector&
Diag610Collector::operator<<(const FarePath& farePath)
{
  if (!_active)
    return *this;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  // XXX: Unify this when removing fallbacks
  if (!fallback::fallbackDisplaySelectedItinNumInDiag(_trx))
  {
    if (pricingTrx && DiagnosticUtil::showItinInMipDiag(*pricingTrx, farePath.itin()->itinNum()))
    {
      if (farePath.itin()->itinNum() != INVALID_INT_INDEX)
        *this << "ITIN-NO: " << farePath.itin()->itinNum() << " \n";
    }
    else
    {
      return *this;
    }
  }
  else
  {
    // sg952572 -  Including init number code to diagnosting report for MIPs
    if ((pricingTrx != nullptr) && (pricingTrx->getTrxType() == PricingTrx::MIP_TRX))
    {
      if((_trx->diagnostic().diagParamIsSet("ITIN_NUM", "")) ||
         (_trx->diagnostic().diagParamIsSet("ITIN_NUM", std::to_string(farePath.itin()->itinNum())))
         )
      {
        *this << "ITIN-NO: " << farePath.itin()->itinNum() << " \n";
      }
      else
      {
        return *this;
      }
    }
  }

  DiagCollector& dc = DiagCollector::operator<<(farePath);

  if (_trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "CABIN")
  {
    dc << "CABIN COMBINATION: ";
    if(pricingTrx)
      DiagnosticUtil::displayCabinCombination(*pricingTrx, farePath, dc);
    dc << std::endl;
  }

  return dc;
}
}
