
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DBAccess/TaxCodeReg.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <sstream>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayCommonText::TaxDisplayCommonText
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

TaxDisplayCommonText::TaxDisplayCommonText() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayCommonText::~TaxDisplayCommonText
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

TaxDisplayCommonText::~TaxDisplayCommonText() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayCommonText::getCommonText
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
TaxDisplayCommonText::getCommonText(TaxCodeReg& taxCodeReg, Indicator msgId)
{
  std::string txt;

  for (const auto taxCodeGenText : taxCodeReg.taxCodeGenTexts())
  {
    if (taxCodeGenText->messageDisplayCat() == msgId)
    {
      if (!taxCodeGenText->txtMsgs().empty())
      {
        std::ostringstream taxCodeGenTextStream;
        taxCodeGenTextStream << "** ";

        std::copy(taxCodeGenText->txtMsgs().begin(),
                  taxCodeGenText->txtMsgs().end(),
                  std::ostream_iterator<std::string>(taxCodeGenTextStream, "\n   "));

        txt += taxCodeGenTextStream.str();
        txt.erase(txt.rfind("   "));
      }
    }
  }

  if (txt == "** ")
    txt = EMPTY_STRING();

  return txt;
}
