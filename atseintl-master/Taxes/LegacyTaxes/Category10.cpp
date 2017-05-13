// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
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

#include "Taxes/LegacyTaxes/Category10.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/TaxCodeReg.h"
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

Category10::Category10() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category10::~Category10()
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category10::~Category10() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory10
//
// Description:  Complete all Category10 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category10::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::FARE_TYPE);

  if (taxDisplayItem.taxCodeReg()->restrictionFareClass().empty() && _subCat2.empty())
  {
    _subCat1 = "     NO FARE TYPE/CLASS RESTRICTIONS APPLY.\n";
    return;
  }

  typedef std::vector<FareClassCode> rfcv_t;

  rfcv_t rfcv = taxDisplayItem.taxCodeReg()->restrictionFareClass();
  Indicator fclind = taxDisplayItem.taxCodeReg()->fareclassExclInd();

  _subCat1 = "* ";

  if (Indicator('Y') == fclind)
  {
    _subCat1 += "EXCEPT ";
  }
  _subCat1 += "FARE CLASS";

  if ("F-" == rfcv[0])
  {
    _subCat1 += "/FAMILY";
  }

  _subCat1 += " ";

  for (rfcv_t::iterator it = rfcv.begin(); it != rfcv.end(); it++)
  {
    if (it != rfcv.begin())
      _subCat1 += ", ";
    _subCat1 += it->c_str();
  }

  _subCat1 += ".\n";
}
