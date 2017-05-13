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

#include <sstream>
#include "DataModel/TaxTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/Billing.h"
#include "Taxes/LegacyTaxes/Category1.h"
#include "Taxes/LegacyTaxes/Category3.h"
#include "Taxes/LegacyTaxes/Category4.h"
#include "Taxes/LegacyTaxes/Category5.h"
#include "Taxes/LegacyTaxes/Category6.h"
#include "Taxes/LegacyTaxes/Category7.h"
#include "Taxes/LegacyTaxes/Category9.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/Nation.h"
#include "Common/Money.h"

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

Category1::Category1() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::~TaxResponse
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category1::~Category1() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory1
//
// Description:  Complete all Category1 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category1::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem, bool singleSequence)
{
  if (taxDisplayItem.taxCodeReg()->displayonlyInd() == YES)
    _displayOnly = "FOR DISPLAY ONLY - NOT AUTO-PRICEABLE\n";

  std::ostringstream tempStr;
  tempStr.setf(std::ios::fixed, std::ios::floatfield);
  tempStr.setf(std::ios::left, std::ios::adjustfield);

  tempStr << taxDisplayItem.taxCodeReg()->taxCur();

  Money moneyPublished(taxDisplayItem.taxCodeReg()->taxCur());
  tempStr.precision(moneyPublished.noDec(taxTrx.ticketingDate()));

  if (taxDisplayItem.taxCodeReg()->taxType() == 'P')
  {
    tempStr << "  " << taxDisplayItem.taxCodeReg()->taxAmtInt().percentageToString();
    tempStr << " PERCENT";
  }
  else
  {
    tempStr << "  " << taxDisplayItem.taxCodeReg()->taxAmt();
  }

  bool hdq = false;
  if (taxTrx.billing() != nullptr)
  {
    if (taxTrx.billing()->aaaCity() == "HDQ")
      hdq = true;
  }

  if (hdq)
  {
    tempStr << "  "
            << "SPN: ";
    tempStr << setw(6) << taxDisplayItem.taxCodeReg()->specialProcessNo();
  }

  tempStr << "  "
          << "SEQ: ";
  tempStr << setw(2) << taxDisplayItem.taxCodeReg()->seqNo() << endl;

  if (taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate())
  {
    tempStr << "EFFECTIVE FOR SALES ON OR AFTER ";
    tempStr << taxDisplayItem.taxCodeReg()->effDate().dateToString(DDMMMYY, "");
    tempStr << "." << endl;

    if (taxDisplayItem.taxCodeReg()->discDate().isValid())
    {
      tempStr << "DISCONTINUE FOR SALES ON AFTER  ";
      tempStr << taxDisplayItem.taxCodeReg()->discDate().dateToString(DDMMMYY, "");
      tempStr << "." << endl;
    }

    if (taxDisplayItem.taxCodeReg()->firstTvlDate().isValid())
    {
      tempStr << "EFFECTIVE FOR TRAVEL ON OR AFTER ";
      tempStr << taxDisplayItem.taxCodeReg()->firstTvlDate().dateToString(DDMMMYY, "");
      tempStr << "." << endl;
    }
    if (taxDisplayItem.taxCodeReg()->lastTvlDate().isValid())
    {
      tempStr << "LAST TRAVEL DATE BEFORE ";
      tempStr << taxDisplayItem.taxCodeReg()->lastTvlDate().dateToString(DDMMMYY, "");
      tempStr << "." << endl;
    }

    if (taxDisplayItem.taxCodeReg()->expireDate().isValid())
    {
      tempStr << "TAX VALUE EXPIRES ";
      tempStr << taxDisplayItem.taxCodeReg()->expireDate().dateToString(DDMMMYY, "");
      tempStr << "." << endl;
    }
  }

  _subCat1 = tempStr.str();
  tempStr.clear();

  if (singleSequence)
    return;

  if (taxTrx.getRequest()->categoryVec().empty())
  {
    buildCat3(taxTrx, taxDisplayItem);
    buildCat4(taxTrx, taxDisplayItem);
    buildCat5(taxTrx, taxDisplayItem);
    buildCat6(taxTrx, taxDisplayItem);
    buildCat9(taxTrx, taxDisplayItem);
    buildCat7(taxTrx, taxDisplayItem);
    return;
  }

  std::vector<uint32_t>::const_iterator categoryVecIter =
      taxTrx.getRequest()->categoryVec().begin();
  std::vector<uint32_t>::const_iterator categoryVecEndIter =
      taxTrx.getRequest()->categoryVec().end();

  for (; categoryVecIter != categoryVecEndIter; categoryVecIter++)
  {
    switch (*categoryVecIter)
    {
    case 3:
      buildCat3(taxTrx, taxDisplayItem);
      break;
    case 4:
      buildCat4(taxTrx, taxDisplayItem);
      break;
    case 5:
      buildCat5(taxTrx, taxDisplayItem);
      break;
    case 6:
      buildCat6(taxTrx, taxDisplayItem);
      break;
    case 7:
      buildCat7(taxTrx, taxDisplayItem);
      break;
    case 9:
      buildCat9(taxTrx, taxDisplayItem);
      break;
    default:
      break;
    }
  }
}

void
Category1::buildCat3(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category3 category3;
  category3.build(taxTrx, taxDisplayItem);
  _subCat2 = category3.subCat1();
}
void
Category1::buildCat4(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category4 category4;
  category4.build(taxTrx, taxDisplayItem);
  _subCat3 = category4.subCat1();
  _subCat4 = category4.subCat2();
  _subCat5 = category4.subCat3();
  _subCat6 = category4.subCat4();
}
void
Category1::buildCat5(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category5 category5;
  category5.build(taxTrx, taxDisplayItem);
  _subCat7 = category5.subCat1();
}
void
Category1::buildCat6(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category6 category6;
  category6.build(taxTrx, taxDisplayItem);
  _subCat8 = category6.subCat1();
}
void
Category1::buildCat7(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category7 category7;
  category7.build(taxTrx, taxDisplayItem);
  _subCat10 = category7.subCat1();
  _subCat11 = category7.subCat2();
}
void
Category1::buildCat9(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category9 category9;
  category9.build(taxTrx, taxDisplayItem);
  _subCat9 = category9.subCat1();
}
