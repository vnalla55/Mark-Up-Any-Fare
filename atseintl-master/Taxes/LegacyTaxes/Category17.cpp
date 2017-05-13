
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

#include "Taxes/LegacyTaxes/Category17.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category17::Category17
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category17::Category17() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category17::~Category17
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category17::~Category17() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category17::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category17::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{

  _subCat1 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::ROUTING);

  if (_subCat1 == EMPTY_STRING())
    _subCat1 = "     NO ROUTING RESTRICTIONS APPLY.\n";
}
