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

#include "Taxes/LegacyTaxes/Category11.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"
#include "Taxes/Common/TaxUtility.h"
#include "Common/Money.h"
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

Category11::Category11() : _subCat1(EMPTY_STRING()), _subCat2(EMPTY_STRING())
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

Category11::~Category11() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory11
//
// Description:  Complete all Category11 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category11::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::TRANSIT);

  if (taxDisplayItem.taxCodeReg()->restrictionTransit().empty())
  {

    if (_subCat2.empty())
    {
      _subCat1 = "     NO TRANSIT RESTRICTIONS APPLY.\n";
    }

    return;
  }

  TaxRestrictionTransit& restrictTransit =
      taxDisplayItem.taxCodeReg()->restrictionTransit().front();

  ostringstream tempStr;
  tempStr.setf(std::ios::left, std::ios::adjustfield);

  tempStr << "* ";

  if (restrictTransit.transitTaxonly() != YES)
  {
    tempStr << "EXCEPT ";
  }
  else
  {
    tempStr << "APPLIES ";
  }

  tempStr << "WHEN TRANSITING ";

  if (!restrictTransit.viaLoc().empty())
  {
    tempStr << LocationDescriptionUtil::description(
                   taxTrx, restrictTransit.viaLocType(), restrictTransit.viaLoc());
  }
  else
  {
    tempStr << taxDisplayItem.taxNation();
  }

  if (restrictTransit.flightArrivalHours() > 0)
  {
    tempStr << ", ON FLIGHT ARRIVALS ON/AFTER ";
    tempStr << restrictTransit.flightArrivalHours();
    tempStr << ':';
    if (restrictTransit.flightArrivalMinutes() != 0)
    {
      if (restrictTransit.flightArrivalMinutes() < 10)
        tempStr << '0';
      tempStr << restrictTransit.flightArrivalMinutes();
    }
    else
    {
      tempStr << "00";
    }

    tempStr << " ON FLIGHT DEPARTIRES ON/AFTER ";
    tempStr << restrictTransit.flightDepartHours();
    tempStr << ':';
    if (restrictTransit.flightDepartMinutes() != 0)
    {
      if (restrictTransit.flightDepartMinutes() < 10)
        tempStr << '0';
      tempStr << restrictTransit.flightDepartMinutes();
    }
    else
    {
      tempStr << "00";
    }
  }

  if (restrictTransit.transitHours() > 0)
  {
    tempStr << " WITHIN ";
    tempStr << restrictTransit.transitHours();
    tempStr << " HOURS";
  }

  if (restrictTransit.sameDayInd() == YES)
  {
    tempStr << ", THE SAME DAY";
  }

  if (restrictTransit.nextDayInd() == YES)
  {
    tempStr << ", THE NEXT DAY";
  }

  if (restrictTransit.transitDomDom() == YES)
  {
    tempStr << " UNLESS ON DOMESTIC TO DOMESTIC SECTOR TRANSIT";
  }

  if (restrictTransit.transitDomIntl() == YES)
  {
    tempStr << " UNLESS ON DOMESTIC TO INTERNATIONAL SECTOR TRANSIT";
  }

  if (restrictTransit.transitIntlDom() == YES)
  {
    tempStr << ", ON INTERNATIONAL SECTOR TO DOMESTIC SECTOR TRANSIT";
  }

  if (restrictTransit.transitIntlIntl() == YES)
  {
    tempStr << ", ON INTERNATIONAL SECTOR TO INTERNATIONAL SECTOR TRANSIT";
  }

  if (restrictTransit.transitSurfDom() == YES)
  {
    tempStr << ", ON DOMESTIC SURFACE SECTOR TRANSIT";
  }

  if (restrictTransit.transitSurfIntl() == YES)
  {
    tempStr << ", ON INTERNATIONAL SURFACE SECTOR TRANSIT";
  }

  if (restrictTransit.sameFlight() == YES)
  {
    tempStr << ", ON SAME CARRIER FLIGHT NUMBER TRANSIT";
  }

  if (restrictTransit.transitOfflineCxr() == YES)
  {
    tempStr << ", UNLESS TRANSIT CONNECTION BETWEEN OFF LINE CARRIERS";
  }

  tempStr << ".";
  tempStr << endl;

  _subCat1 = tempStr.str();
}
