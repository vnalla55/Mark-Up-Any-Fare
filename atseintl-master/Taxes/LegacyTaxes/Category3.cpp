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

#include "Taxes/LegacyTaxes/Category3.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category3::Category3
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category3::Category3() : _subCat1(EMPTY_STRING()), _subCat2(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category3::~Category3
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category3::~Category3() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory3
//
// Description:  Complete all Category3 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category3::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  _subCat2 =
      TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(), TaxDisplayCommonText::SALE);

  if (taxDisplayItem.taxCodeReg()->posLoc() == EMPTY_STRING() &&
      taxDisplayItem.taxCodeReg()->poiLoc() == EMPTY_STRING() && _subCat2.empty())
  {
    _subCat1 = "     TICKETS SOLD AND ISSUED ANYWHERE.\n";
  }
  else
  {
    _subCat1 = formatPoiPosLine(taxTrx, taxDisplayItem);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function formatPoiPosLine
//
// Description:  POS and POI information for multiple sequences
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category3::formatPoiPosLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  PosPoi pos(taxTrx,
             taxDisplayItem.taxCodeReg()->posLocType(),
             taxDisplayItem.taxCodeReg()->posLoc(),
             taxDisplayItem.taxCodeReg()->posExclInd());
  PosPoi poi(taxTrx,
             taxDisplayItem.taxCodeReg()->poiLocType(),
             taxDisplayItem.taxCodeReg()->poiLoc(),
             taxDisplayItem.taxCodeReg()->poiExclInd());

  std::string poiPosDetail = "* TICKETS SOLD";

  if (pos != poi)
  {
    poiPosDetail += pos.addInOrSpace() + pos.locDecorator(taxTrx);
  }

  poiPosDetail += " AND ISSUED" + poi.addInOrSpace() + poi.locDecorator(taxTrx) + ".\n";

  return poiPosDetail;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @constructor PoiPos
//
// Description:
//
// </PRE>
// ----------------------------------------------------------------------------

Category3::PosPoi::PosPoi(TaxTrx& taxTrx, LocType& locType, LocCode& loc, Indicator& exlcInd)
  : _exlcInd(exlcInd), _locType(locType), ANYWHERE("ANYWHERE")
{
  _loc = LocationDescriptionUtil::description(taxTrx, _locType, loc);

  if (_loc.size() == 0)
  {
    _loc = ANYWHERE;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category3::PosPoi::operator!=
//
// Description: Not-equal-to operator
//
// </PRE>
// ----------------------------------------------------------------------------

bool
Category3::PosPoi::
operator!=(const PosPoi& rhs)
{
  if ((_loc == ANYWHERE) && (rhs._loc == ANYWHERE))
  {
    return false; //"SOLD ANYWHERE AND ISSUED ENYWHERE" prevention
  }
  else if (_locType != rhs._locType || _loc != rhs._loc || _exlcInd != rhs._exlcInd)
  {
    return true;
  }
  else
  {
    return false;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category3::PosPoi::locDecorator
//
// Description:  decorate POSLOC data
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category3::PosPoi::locDecorator(TaxTrx& taxTrx)
{

  if (_exlcInd != YES || (_exlcInd == YES && _loc.substr(0, 8) == string("ANYWHERE")))
  {
    return _loc; //"ANYWHERE EXCPPT ENYWHERE" prevention
  }

  return std::string("ANYWHERE EXCEPT " + _loc);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category3::PosPoi::inOrSpace
//
// Description: add "IN" or " " string
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category3::PosPoi::addInOrSpace()
{
  if (_exlcInd == YES || _loc.substr(0, 8) == std::string("ANYWHERE"))
    return std::string(" ");
  else
    return std::string(" IN ");
}
