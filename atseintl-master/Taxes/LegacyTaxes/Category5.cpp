// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/Category5.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/Nation.h"
#include "Common/Money.h"
#include "Common/LocUtil.h"
#include <sstream>
#include <vector>

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::TaxResponse
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category5::Category5() : _subCat1(EMPTY_STRING()), _subCat2(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::~TaxResponse
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category5::~Category5() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory5
//
// Description:  Complete all Category5 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category5::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::VALIDATING_CARRIER);

  if (taxDisplayItem.taxCodeReg()->restrictionValidationCxr().empty() && _subCat2.empty())
  {
    _subCat1 = "     NO VALIDATING CARRIER RESTRICTIONS APPLY.\n";
    return;
  }

  std::string carrierCodes = EMPTY_STRING();

  std::vector<CarrierCode>::iterator carrierCodeIter =
      taxDisplayItem.taxCodeReg()->restrictionValidationCxr().begin();
  std::vector<CarrierCode>::iterator carrierCodeEndIter =
      taxDisplayItem.taxCodeReg()->restrictionValidationCxr().end();

  for (; carrierCodeIter != carrierCodeEndIter; carrierCodeIter++)
  {
    if (!carrierCodes.empty())
      carrierCodes += ", ";

    carrierCodes += *carrierCodeIter;
  }

  if (taxDisplayItem.taxCodeReg()->valcxrExclInd() == YES)
  {
    _subCat1 = "* EXCEPT TICKETS VALIDATED ON CARRIER/S ";
  }
  else
  {
    _subCat1 = "* TICKETS VALIDATED ON CARRIER/S ";
  }

  _subCat1 += carrierCodes;
  _subCat1 += ".\n";
}
