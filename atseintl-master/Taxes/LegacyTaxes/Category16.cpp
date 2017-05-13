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

#include "Taxes/LegacyTaxes/Category16.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"

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

Category16::Category16() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category16::~Category16()
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category16::~Category16() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category16::build()
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category16::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  _subCat1 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::REISSUE);

  if (_subCat1 == EMPTY_STRING())
    _subCat1 = "     NO REISSUE RESTRICTIONS APPLY.\n";
}
