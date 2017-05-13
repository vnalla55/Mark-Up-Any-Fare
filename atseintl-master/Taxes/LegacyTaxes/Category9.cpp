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

#include "Taxes/LegacyTaxes/Category9.h"
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

Category9::Category9() : _subCat1(EMPTY_STRING()), _subCat2(EMPTY_STRING())
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

Category9::~Category9() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory9
//
// Description:  Complete all Category9 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category9::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::PASSEMGER_TYPE);

  if (taxDisplayItem.taxCodeReg()->restrictionPsg().empty() && _subCat2.empty())
  {
    _subCat1 = "     NO PASSENGER TYPE RESTRICTIONS APPLY.\n";
    return;
  }

  _subCat1 = "* PASSENGER TYPE ";

  if (taxDisplayItem.taxCodeReg()->psgrExclInd() == YES)
    _subCat1 = "* EXCEPT PASSENGER TYPE ";

  std::string noPsgAge = EMPTY_STRING();
  std::string psgAgeFareZero = EMPTY_STRING();

  std::vector<TaxRestrictionPsg>::iterator restrictionPsgIter =
      taxDisplayItem.taxCodeReg()->restrictionPsg().begin();
  std::vector<TaxRestrictionPsg>::iterator restrictionPsgEndIter =
      taxDisplayItem.taxCodeReg()->restrictionPsg().end();

  for (; restrictionPsgIter != restrictionPsgEndIter; restrictionPsgIter++)
  {
    if ((*restrictionPsgIter).showPsg() != YES)
      continue;

    if (((*restrictionPsgIter).minAge() == 0) && ((*restrictionPsgIter).maxAge() == 0) &&
        ((*restrictionPsgIter).fareZeroOnly() != YES))
    {
      if (!noPsgAge.empty())
        noPsgAge += ", ";

      noPsgAge += (*restrictionPsgIter).psgType();
      continue;
    }

    if (!psgAgeFareZero.empty())
      psgAgeFareZero += ", ";

    psgAgeFareZero += (*restrictionPsgIter).psgType();

    if (((*restrictionPsgIter).minAge() == 0) && ((*restrictionPsgIter).maxAge() != 0))
    {
      psgAgeFareZero += " AGES 0 TO ";
      formatNumber(psgAgeFareZero, (*restrictionPsgIter).maxAge());
    }
    else if (((*restrictionPsgIter).minAge() != 0) && ((*restrictionPsgIter).maxAge() == 0))
    {
      psgAgeFareZero += " AGES ";
      formatNumber(psgAgeFareZero, (*restrictionPsgIter).minAge());
      psgAgeFareZero += " OR OLDER";
    }
    else if (((*restrictionPsgIter).minAge() != 0) && ((*restrictionPsgIter).maxAge() != 0))
    {
      psgAgeFareZero += " AGES ";
      formatNumber(psgAgeFareZero, (*restrictionPsgIter).minAge());
      psgAgeFareZero += " TO ";
      formatNumber(psgAgeFareZero, (*restrictionPsgIter).maxAge());
    }

    if (((*restrictionPsgIter).fareZeroOnly() == YES))
      psgAgeFareZero += " WHEN FARE IS ZERO -0-";
  }
  _subCat1 += psgAgeFareZero;

  if (!psgAgeFareZero.empty() && !noPsgAge.empty())
    _subCat1 += ", ";

  _subCat1 += noPsgAge;
  _subCat1 += ".\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Format Number
//
// Description:  Append integer number to string
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category9::formatNumber(std::string& psgAge, uint16_t age)
{
  char temp[20];
  sprintf(temp, "%u", age);
  psgAge += temp;
}
