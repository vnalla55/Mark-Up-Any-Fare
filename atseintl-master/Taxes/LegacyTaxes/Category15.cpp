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

#include "Taxes/LegacyTaxes/Category15.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/TaxCodeReg.h"

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

Category15::Category15()
  : _subCat1(EMPTY_STRING()),
    _subCat2(EMPTY_STRING()),
    _subCat3(EMPTY_STRING()),
    _subCat4(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category15::~Category15()
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category15::~Category15() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category15::build()
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category15::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  _subCat4 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::MISCELLANEOUS);

  if (taxDisplayItem.taxCodeReg()->interlinableTaxInd() != 'Y' &&
      taxDisplayItem.taxCodeReg()->feeInd() != 'Y' &&
      taxDisplayItem.taxCodeReg()->showseparateInd() != 'Y' && _subCat4.empty())
  {
    _subCat1 = "     NO MISCELANEOUS INFORMATION RESTRICTIONS APPLY.\n";
  }

  if ('Y' == taxDisplayItem.taxCodeReg()->interlinableTaxInd())
    _subCat1 = "* TAX AMOUNT IS INTERLINEABLE.\n";

  if ('Y' == taxDisplayItem.taxCodeReg()->feeInd())
    _subCat2 = "* TAX DISPLAYS AS FEE IN FT MESSAGE.\n";

  if ('Y' == taxDisplayItem.taxCodeReg()->showseparateInd())
    _subCat3 = "* TAX NOT COMBINED WITH XT LOGIC.\n";
}
