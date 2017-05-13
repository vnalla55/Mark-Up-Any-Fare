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

#include "Taxes/LegacyTaxes/Category4.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "DBAccess/Nation.h"
#include "Common/Money.h"
#include "Rules/RuleConst.h"
#include <sstream>

namespace tse
{
//---------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::TaxResponse
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category4::Category4()
  : _subCat1(EMPTY_STRING()),
    _subCat2(EMPTY_STRING()),
    _subCat3(EMPTY_STRING()),
    _subCat4(EMPTY_STRING()),
    _subCat5(EMPTY_STRING())
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

Category4::~Category4() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory4
//
// Description:  Complete all Category4 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category4::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem, bool singleSequence)
{
  if (taxDisplayItem.taxCodeReg()->loc1Type() != LOCTYPE_NONE)
  {
    _location1 = LocationDescriptionUtil::description(
        taxTrx,
        static_cast<LocType>(taxDisplayItem.taxCodeReg()->loc1Type()),
        taxDisplayItem.taxCodeReg()->loc1());
  }
  if (taxDisplayItem.taxCodeReg()->loc2Type() != LOCTYPE_NONE)
  {
    _location2 = LocationDescriptionUtil::description(
        taxTrx,
        static_cast<LocType>(taxDisplayItem.taxCodeReg()->loc2Type()),
        taxDisplayItem.taxCodeReg()->loc2());
  }
  if (taxDisplayItem.taxCodeReg()->originLocType() != LOCTYPE_NONE)
  {
    _originLoc = LocationDescriptionUtil::description(taxTrx,
                                                      taxDisplayItem.taxCodeReg()->originLocType(),
                                                      taxDisplayItem.taxCodeReg()->originLoc());
  }

  _subCat1 = formatTravelLine(taxTrx, taxDisplayItem);

  if (taxDisplayItem.taxCodeReg()->nextstopoverrestr() == YES)
  {
    _subCat1 += RuleConst::BLANK;
    // _subCat1 += BLANK;
    _subCat1 += "WHERE THE SECOND LOCATION IS THE NEXT STOPOVER POINT OR\n";
    _subCat1 += RuleConst::BLANK;
    _subCat1 += RuleConst::BLANK;
    _subCat1 += "THE FIRST POINT OUTSIDE THE TAXATION COUNTRY";
  }

  if (!_subCat1.empty() && _subCat1.find(".") == std::string::npos)
    _subCat1 += ".\n";

  _subCat2 = formatLocationLine(taxTrx, taxDisplayItem);
  if (taxDisplayItem.taxCodeReg()->originLocType() != UNKNOWN_LOC)
  {
    // Itinerary originating
    _subCat2 += formatOriginRestriction(taxTrx, taxDisplayItem);
  }

  if (taxDisplayItem.taxCodeReg()->travelType() == 'D')
  {
    _subCat3 = "* ON DOMESTIC TRAVEL";
    _subCat3 += ".\n";
  }
  else if (taxDisplayItem.taxCodeReg()->travelType() == 'I')
  {
    _subCat3 = "* ON INTERNATIONAL TRAVEL";
    _subCat3 += ".\n";
  }

  if (taxDisplayItem.taxCodeReg()->itineraryType() == 'R')
  {
    _subCat4 = "* ON ROUND TRIP TRAVEL";
    _subCat4 += ".\n";
  }
  else if (taxDisplayItem.taxCodeReg()->itineraryType() == 'O')
  {
    _subCat4 = "* ON ONE WAY TRAVEL";
    _subCat4 += ".\n";
  }
  else if (taxDisplayItem.taxCodeReg()->itineraryType() == 'J')
  {
    _subCat4 = "* ON OPEN JAW TRAVEL";
    _subCat4 += ".\n";
  }

  _subCat5 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::TRAVEL);

  if (_subCat1.empty() && _subCat2.empty() && _subCat3.empty() && _subCat4.empty() &&
      _subCat5.empty())
  {
    _subCat1 = "     NO TRAVEL RESTRICTIONS APPLY.\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function formatTravelLine
//
// Description:  Travel information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category4::formatTravelLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::string travelDetail = EMPTY_STRING();

  if (taxDisplayItem.taxCodeReg()->tripType() == RuleConst::BLANK)
    return travelDetail;

  if ((taxDisplayItem.taxCodeReg()->loc1Type() == LOCTYPE_NONE) &&
      (taxDisplayItem.taxCodeReg()->loc2Type() == LOCTYPE_NONE))
    return travelDetail;

  travelDetail = "* TRAVEL ";
  travelDetail += formatTripType(taxTrx, taxDisplayItem);
  return travelDetail;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function formatTravelLine
//
// Description:  Travel information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category4::formatLocationLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::string locationDetail;

  if ((taxDisplayItem.taxCodeReg()->loc1Appl() != RuleConst::BLANK &&
       taxDisplayItem.taxCodeReg()->loc1Type() != LOCTYPE_NONE) ||
      (taxDisplayItem.taxCodeReg()->loc2Appl() != RuleConst::BLANK &&
       taxDisplayItem.taxCodeReg()->loc2Type() != LOCTYPE_NONE))
  {
    // Enplanement and Origination
    locationDetail = formatLocationRestriction1(taxTrx, taxDisplayItem);

    // Destination, Termintaion, and Deplanement
    locationDetail += formatLocationRestriction2(taxTrx, taxDisplayItem);

    /// new functionality - no fallback
    if (locationDetail.find(".") == std::string::npos && !locationDetail.empty())
      locationDetail += ".\n";
  }
  return locationDetail;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function formatTripType
//
// Description:  Travel information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category4::formatTripType(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::string travelDetail = EMPTY_STRING();

  switch (taxDisplayItem.taxCodeReg()->tripType())
  {
  case TripTypesValidator::TAX_FROM_TO:
  {
    travelDetail += "FROM ";

    if (taxDisplayItem.taxCodeReg()->loc1ExclInd() == YES)
      travelDetail += "ANYWHERE EXCEPT ";

    travelDetail += _location1;
    travelDetail += " TO ";

    if (taxDisplayItem.taxCodeReg()->loc2Type() == LOCTYPE_NONE)
    {
      travelDetail += "ANYWHERE";
    }
    else
    {
      if (taxDisplayItem.taxCodeReg()->loc2ExclInd() == YES)
        travelDetail += "ANYWHERE EXCEPT ";

      travelDetail += _location2;
    }
    break;
  }

  case TripTypesValidator::TAX_BETWEEN:
  {
    travelDetail += "BETWEEN ";
    travelDetail += _location1;
    travelDetail += " AND ";
    travelDetail += _location2;
    break;
  }

  case TripTypesValidator::TAX_WITHIN_SPEC:
  {
    travelDetail += "WITHIN SPECIFIED LOCATION ";
    travelDetail += _location1;
    break;
  }

  case TripTypesValidator::TAX_WITHIN_WHOLLY:
  {
    std::string whollyWithin = _location1;

    if ((taxDisplayItem.taxCodeReg()->loc1Appl() == LocRestrictionValidator::TAX_ORIGIN) ||
        (taxDisplayItem.taxCodeReg()->loc1Appl() == LocRestrictionValidator::TAX_ENPLANEMENT))
      whollyWithin = _location2;

    travelDetail += "WHOLLY WITHIN ";
    travelDetail += whollyWithin;
    break;
  }

  default:
    break;
  } // end of switch

  return travelDetail;
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function LocationRestriction1
//
// Description:  Enplanement and Origination information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category4::formatLocationRestriction1(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::string locationDetail = EMPTY_STRING();

  switch (taxDisplayItem.taxCodeReg()->loc1Appl())
  {
  case LocRestrictionValidator::TAX_ORIGIN:
  {
    locationDetail += "* ORIGIN FROM ";

    if (taxDisplayItem.taxCodeReg()->loc1ExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    locationDetail += _location1;
    break;
  }

  case LocRestrictionValidator::TAX_ENPLANEMENT:
  {
    locationDetail += "* ENPLANEMENT FROM ";

    if (taxDisplayItem.taxCodeReg()->loc1ExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    locationDetail += _location1;
    break;
  }
  default:
    break;
  } // end of switch

  return locationDetail;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function LocationRestriction2
//
// Description:  Destination, Termintaion, and Deplanement
//               information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category4::formatLocationRestriction2(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::string locationDetail = EMPTY_STRING();

  // if loc1 exist add \n on the line begining
  if (!formatLocationRestriction1(taxTrx, taxDisplayItem).empty())
    locationDetail = ".\n";

  switch (taxDisplayItem.taxCodeReg()->loc2Appl())
  {

  case LocRestrictionValidator::TAX_DEPLANEMENT:
  {
    locationDetail += "* DEPLANEMENT LOCATION ";

    if (taxDisplayItem.taxCodeReg()->loc2ExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    locationDetail += _location2;
    break;
  }

  case LocRestrictionValidator::TAX_DESTINATION:
  {
    locationDetail += "* DESTINATION LOCATION ";

    if (taxDisplayItem.taxCodeReg()->loc2ExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    locationDetail += _location2;
    break;
  }

  case LocRestrictionValidator::TAX_TERMINATION:
  {
    locationDetail += "* TERMINATION LOCATION ";

    if (taxDisplayItem.taxCodeReg()->loc2ExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    locationDetail += _location2;
    break;
  }
  default:
  {
    if (taxDisplayItem.taxCodeReg()->loc1ExclInd() == YES)
      break;

    locationDetail = " TO ";

    if (taxDisplayItem.taxCodeReg()->loc2ExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    if (_location2.empty())
    {
      locationDetail += "ANYWHERE";
      break;
    }

    locationDetail += _location2;
    break;
  }
  } // end of switch
  return locationDetail;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function formatOriginRestriction
//
// Description: Itinerary originating information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category4::formatOriginRestriction(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  std::string locationDetail;

  if (taxDisplayItem.taxCodeReg()->originLocType() != LOCTYPE_NONE)
  {
    locationDetail += "* ITINERARY ORIGINATING ";

    if (taxDisplayItem.taxCodeReg()->originLocExclInd() == YES)
      locationDetail += "ANYWHERE EXCEPT ";

    locationDetail += "IN ";
    locationDetail += _originLoc + ".\n";
  };
  return locationDetail;
}
}
