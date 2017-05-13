//-------------------------------------------------------------------
//
//  File:        RBSection.cpp
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only :with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/RBSection.h"

#include "DataModel/PaxTypeFare.h"

#include <sstream>
#include <string>

namespace tse
{
const std::string RBSection::HARD_CODED_DOMESTIC_RB_RESTRICTION =
    "SECONDARY CARRIER NOT APPLICABLE FOR DOMESTIC";

void
RBSection::buildDisplay()
{
  const std::string& rtgText = _trx.fdResponse()->rtgText();
  if (rtgText.length() > 0)
  {
    _trx.response() << rtgText;
  }

  if (_trx.isDiagnosticRequest())
  {
    const std::string& rtgDiagInfo = _trx.fdResponse()->rtgDiagInfo();
    if (!rtgDiagInfo.empty())
    {
      _trx.response() << " " << std::endl << rtgDiagInfo << std::endl;
    }
  }
}
} // tse namespace
