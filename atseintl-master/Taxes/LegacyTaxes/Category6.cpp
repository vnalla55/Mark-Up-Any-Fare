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

#include "Taxes/LegacyTaxes/Category6.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/Nation.h"
#include "Common/Money.h"
#include "Common/LocUtil.h"
#include "Taxes/Common/TaxUtility.h"
#include "Common/TaxRound.h"
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

Category6::Category6() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::~TaxResponse
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category6::~Category6() {}

// ----------------------------------------------------------------------------
std::string
Category6::getGroup3str(Indicator formOfPayment)
{
  std::string tmpstr = "* ON TICKETS PAID ";

  switch (formOfPayment)
  {
  case 'C':
    tmpstr += "IN CASH";
    break;
  case 'K':
    tmpstr += "BY CHECK";
    break;
  case 'R':
    tmpstr += "BY CREDIT CARD";
    break;
  default:
    return "";
  }
  tmpstr += ".\n";
  return tmpstr;
}

// ----------------------------------------------------------------------------

std::string
Category6::getGroup1str(Indicator taxequivAmtInd)
{
  if (YES == taxequivAmtInd)
  {
    return "* TAX APPLIES ON EQUIVALENT AMOUNT PAID.\n";
  }
  else
    return "";
}

// ----------------------------------------------------------------------------
std::string
Category6::getGroup4str(const RoundingRule rrule,
                        const CurrencyNoDec currnodec,
                        const std::string& roundfac)
{
  std::string tmpstr = "* TAX CODE ROUNDING OVERRIDE APPLIES AS ";

  switch (rrule)
  {
  case UP:
    tmpstr += "ROUND UP TO NEXT";
    break;
  case DOWN:
    tmpstr += "ROUND DOWN TO";
    break;
  case NEAREST:
    tmpstr += "ROUND UP/DOWN TO NEAREST";
    break;
  case NONE:
    tmpstr += "NO ROUNDING APPLIED.\n";
    return tmpstr;
  case EMPTY:
    return "";
  default:
    return "";
  }

  tmpstr += " ";
  tmpstr += roundfac;
  tmpstr += " .\n";

  return tmpstr;
}

//---------------------------------------------------------------------------------------------
std::string
Category6::getGroup6str(Indicator val)
{
  if ('Y' == val)
    return "* MULTI OCCURRENCE CONVERSION ROUNDING APPLIES.\n";
  return "";
}
//---------------------------------------------------------------------------------------------

std::string
Category6::getGroup5str(Indicator val)
{
  if ('Y' == val)
    return "* SPECIAL DOMESTIC TAX ROUNDING APPLIES.\n";
  return "";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory6
//
// Description:  Complete all Category6 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category6::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  tse::TaxCodeReg* tcreg = taxDisplayItem.taxCodeReg();

  if (!tcreg->sellCur().empty())
  {
    if (YES == tcreg->sellCurExclInd())
      _subCat2 += "* EXCEPT";
    else
      _subCat2 = "*";

    _subCat2 += " ON TICKETS SOLD IN CURRENCY ";
    _subCat2 += tcreg->sellCur();
    _subCat2 += ".\n";
  }

  _subCat1 = getGroup1str(tcreg->taxequivAmtInd());

  _subCat3 = getGroup3str(tcreg->formOfPayment());

  _subCat4 = getGroup4str(
      tcreg->taxcdRoundRule(), tcreg->taxcdRoundUnitNodec(), tcreg->taxcdRoundUnitInt().toString());

  _subCat5 = getGroup5str(tcreg->spclTaxRounding());

  _subCat6 = getGroup6str(tcreg->multioccconvrndInd());

  _subCat7 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::CURRENCY);

  if (_subCat1.empty() && _subCat2.empty() && _subCat3.empty() && _subCat4.empty() &&
      _subCat5.empty() && _subCat6.empty() && _subCat7.empty())
    _subCat1 = "     NO CURRENCY RESTRICTIONS APPLY.\n";
}
