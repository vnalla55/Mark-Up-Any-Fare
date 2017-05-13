
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

#include "Taxes/LegacyTaxes/Category18.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category18::Category18
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category18::Category18() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category18::~Category18
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category18::~Category18() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category18::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category18::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{

  _subCat1 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::REFUND);

  if (_subCat1 == EMPTY_STRING())
    _subCat1 = "     NO REFUND RESTRICTIONS APPLY.\n";
}
