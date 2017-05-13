// ----------------------------------------------------------------------------
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
// ----------------------------------------------------------------------------

#include "DataModel/AutomaticPfcTaxExemptionData.h"

#include "Diagnostic/Diag807Collector.h"
#include "Diagnostic/DiagManager.h"

namespace tse
{

AutomaticPfcTaxExemptionData::AutomaticPfcTaxExemptionData()
  : _automaticPfcTaxExemptionEnabled(false),
    _taxExemptionOption(NO_EXEMPT),
    _pfcExemptionOption(NO_EXEMPT),
    _firstTaxExempted(true)
{
}

bool
AutomaticPfcTaxExemptionData::firstTaxExempted()
{
  if (_firstTaxExempted)
  {
    _firstTaxExempted = false;
    return true;
  }
  else
  {
    return false;
  }
}
}
