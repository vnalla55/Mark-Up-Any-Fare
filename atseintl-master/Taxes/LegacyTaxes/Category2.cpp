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

#include "Taxes/LegacyTaxes/Category2.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Common/Money.h"
#include "Rules/RuleConst.h"
#include <sstream>
#include <vector>
#include <climits>

namespace tse
{

const uint32_t Category2::EARTH_CIRCUMFERENCE = 24900;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::TaxResponse
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category2::Category2() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::~TaxResponse
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category2::~Category2() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory2
//
// Description:  Complete all Category2 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category2::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::ostringstream tempStr;
  tempStr.setf(std::ios::fixed, std::ios::floatfield);
  tempStr.setf(std::ios::left, std::ios::adjustfield);

  Money moneyPublished(taxDisplayItem.taxCodeReg()->taxCur());
  tempStr.precision(moneyPublished.noDec(taxTrx.ticketingDate()));

  if (taxDisplayItem.taxCodeReg()->minTax() != 0.0)
  {
    tempStr << "* MINIMUM TAX ";
    tempStr << taxDisplayItem.taxCodeReg()->taxCur();
    tempStr << RuleConst::BLANK;
    tempStr << taxDisplayItem.taxCodeReg()->minTax();
    tempStr << std::endl;
    _subCat1 = tempStr.str();
    tempStr.str(EMPTY_STRING());
    ;
  }

  if (taxDisplayItem.taxCodeReg()->maxTax() != 0.0)
  {
    tempStr << "* MAXIMUM TAX ";
    tempStr << taxDisplayItem.taxCodeReg()->taxCur();
    tempStr << RuleConst::BLANK;
    tempStr << taxDisplayItem.taxCodeReg()->maxTax();
    tempStr << std::endl;
    _subCat2 = tempStr.str();
    tempStr.str(EMPTY_STRING());
  }

  if (taxDisplayItem.taxCodeReg()->plusupAmt() > 0.0)
  {
    tempStr << "* PLUS UP TAX ";
    tempStr << taxDisplayItem.taxCodeReg()->taxCur();
    tempStr << RuleConst::BLANK;
    tempStr << taxDisplayItem.taxCodeReg()->plusupAmt();
    tempStr << std::endl;
    _subCat3 = tempStr.str();
    tempStr.str(EMPTY_STRING());
  }

  if (!taxDisplayItem.taxCodeReg()->taxOnTaxCode().empty())
    tempStr << "* APPLIES TO TAX CODE/S ";

  std::string taxCodes = EMPTY_STRING();

  std::vector<std::string>::const_iterator taxOnTaxCodeIter =
      taxDisplayItem.taxCodeReg()->taxOnTaxCode().begin();
  std::vector<std::string>::const_iterator taxOnTaxCodeEndIter =
      taxDisplayItem.taxCodeReg()->taxOnTaxCode().end();

  for (; taxOnTaxCodeIter != taxOnTaxCodeEndIter; taxOnTaxCodeIter++)
  {
    if (!taxCodes.empty())
      taxCodes += ",";

    taxCodes += (*taxOnTaxCodeIter);
  }
  tempStr << taxCodes;

  if (!taxDisplayItem.taxCodeReg()->taxOnTaxCode().empty())
  {
    if (taxDisplayItem.taxCodeReg()->taxOnTaxExcl() == YES)
    {
      tempStr << " - NOT APPLIED TO BASE FARE";
    }
    else
    {
      tempStr << ", AND BASE FARE";
    }
    tempStr << std::endl;
  }
  _subCat4 = tempStr.str();
  tempStr.str(EMPTY_STRING());

  if (taxDisplayItem.taxCodeReg()->taxexcessbagInd() == YES)
    _subCat5 = "* TAX APPLIES ON EXCESS BAGGAGE CHARGES\n";

  if (taxDisplayItem.taxCodeReg()->taxfullFareInd() == YES)
    _subCat6 = "* TAX APPLIES ON FULL FARE AMOUNT BEFORE ANY DISCOUNT\n";

  if (taxDisplayItem.taxCodeReg()->occurrence() == 'O')
    _subCat7 = "* APPLY ONCE PER TAX SEQUENCE\n";

  if (taxDisplayItem.taxCodeReg()->occurrence() == 'M')
    _subCat7 = "* APPLY MULTIPLE TIMES\n";

  if (taxDisplayItem.taxCodeReg()->occurrence() == 'B')
    _subCat7 = "* APPLY PER TICKET BOOKLET\n";

  if (taxDisplayItem.taxCodeReg()->occurrence() == 'C')
    _subCat7 = "* APPLY ONCE PER TAX CODE\n";

  // low range without fractional part.

  std::string low_range = taxDisplayItem.taxCodeReg()->lowRangeInt().toString();
  // high range without fractional part
  std::string high_range = taxDisplayItem.taxCodeReg()->highRangeInt().toString();

  if (taxDisplayItem.taxCodeReg()->rangeType() == 'F')
  {
    tempStr.precision(taxDisplayItem.taxCodeReg()->fareRangeNodec());

    tempStr << "* APPLIES TO FARE RANGE - ";
    tempStr << taxDisplayItem.taxCodeReg()->taxCur();
    tempStr << RuleConst::BLANK;

    if (taxDisplayItem.taxCodeReg()->rangeincrement() == 0.0)
    {
      if (taxDisplayItem.taxCodeReg()->highRange() == 0.0)
      {
        tempStr << low_range;
        tempStr << " OR MORE";
      }
      else
      {
        tempStr << low_range;
        tempStr << " TO ";
        tempStr << high_range;
      }

      if (taxDisplayItem.taxCodeReg()->rangeInd() == 'T')
      {
        tempStr << " - BY TRIP";
      }
      else
      {
        tempStr << " - ON TOTAL";
      }
    }
    else
    {
      tempStr << taxDisplayItem.taxCodeReg()->rangeincrement();
      tempStr << " PER EACH FARE AMOUNT INCREMENT";
    }
    tempStr << std::endl;
    _subCat8 = tempStr.str();
  }

  if (taxDisplayItem.taxCodeReg()->rangeType() == 'M')
  {
    tempStr << "* APPLIES TO MILEAGE RANGE - ";
    // tempStr << taxDisplayItem.taxCodeReg()->taxCur();
    tempStr << RuleConst::BLANK;
    // HIGHRANGE inconsistency around TaxCODE minimal "infinity" is 25000 ?
    if (taxDisplayItem.taxCodeReg()->highRange() == 0.0 ||
        taxDisplayItem.taxCodeReg()->highRange() == INT_MAX ||
        taxDisplayItem.taxCodeReg()->highRange() > EARTH_CIRCUMFERENCE /* earth circumference */)
    {
      tempStr << low_range;
      tempStr << " OR MORE";
    }
    else
    {
      tempStr << low_range;
      tempStr << " TO ";
      tempStr << high_range;
    }

    tempStr << " MILES";

    if (taxDisplayItem.taxCodeReg()->rangeInd() == 'S')
      tempStr << " - MEASURED TO FIRST STOPOVER";
    else if ('D' == taxDisplayItem.taxCodeReg()->rangeInd())
      tempStr << " - MEASURED TO FINAL DESTINATION";

    tempStr << std::endl;
    _subCat8 = tempStr.str();
  }

  _subCat9 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::TAX_DETAIL);

  if (_subCat1 == EMPTY_STRING() && _subCat2 == EMPTY_STRING() && _subCat3 == EMPTY_STRING() &&
      _subCat4 == EMPTY_STRING() && _subCat5 == EMPTY_STRING() && _subCat6 == EMPTY_STRING() &&
      _subCat7 == EMPTY_STRING() && _subCat8 == EMPTY_STRING() && _subCat9 == EMPTY_STRING())
  {
    _subCat1 = "     NO TAX DETAIL RESTRICTIONS APPLY.\n";
  }
}
}
