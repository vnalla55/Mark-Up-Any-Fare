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

#include "Taxes/LegacyTaxes/Category14.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include <sstream>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category14::Category14
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category14::Category14() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category14::~Category14
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category14::~Category14() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category14::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category14::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{

  if (0 < taxDisplayItem.taxCodeReg()->discPercent())
  {
    _subCat1 = "* TICKETS DISCOUNTED ";
    _subCat1 += taxDisplayItem.taxCodeReg()->discPercentInt().percentageToString();
    _subCat1 += " PERCENT OR MORE ARE EXEMPT.\n";
  }

  if ('Y' == taxDisplayItem.taxCodeReg()->freeTktexempt())
  {
    _subCat1 += "* FREE TICKET IS EXEMPT.\n";
  }

  if ('Y' == taxDisplayItem.taxCodeReg()->idTvlexempt())
  {
    _subCat1 += "* INDUSDTRY TRAVEL TICKETS ARE EXEMPT.\n";
  }

  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::DISCOUNT);

  if (_subCat1.empty() && _subCat2.empty())
    _subCat1 = "     NO DISCOUNT RESTRICTIONS APPLY.\n";
}

std::string&
Category14::subCat1()
{
  return _subCat1;
}

std::string&
Category14::subCat2()
{
  return _subCat2;
}
