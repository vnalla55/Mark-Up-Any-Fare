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

#include "Taxes/LegacyTaxes/Category8.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"

#include <iterator>
#include <sstream>
#include <algorithm>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category8::Category8
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category8::Category8() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category8::~Category8
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category8::~Category8() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category8::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category8::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{
  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::EQUIPMENT);

  if (taxDisplayItem.taxCodeReg()->equipmentCode().empty() && _subCat2.empty())
  {
    _subCat1 = "     NO EQUIPMENT RESTRICTIONS APPLY.\n";
    return;
  }

  if ('Y' == taxDisplayItem.taxCodeReg()->exempequipExclInd())
  {
    _subCat1 = "* EXCEPT ON CARRIER EQUIPMENT ";
  }
  else
  {
    _subCat1 = "* ON CARRIER EQUIPMENT ";
  }

  std::ostringstream equipmentsStream;
  std::copy(taxDisplayItem.taxCodeReg()->equipmentCode().begin(),
            taxDisplayItem.taxCodeReg()->equipmentCode().end(),
            std::ostream_iterator<std::string>(equipmentsStream, ", "));

  _subCat1 += equipmentsStream.str();

  std::string::size_type replacepos = _subCat1.rfind(", ");

  if (replacepos != std::string::npos)
    _subCat1.replace(replacepos, 2, ".\n");
}
