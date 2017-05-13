
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"

#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace tse;

const TaxCode TaxInfoBuilderZP::TAX_CODE = "ZP";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderZP::TaxInfoBuilderZP
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilderZP::TaxInfoBuilderZP() : _prevArpt("")
{
  response().initZP();
  taxCode() = TAX_CODE;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderZP::~TaxInfoBuilderZP
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilderZP::~TaxInfoBuilderZP() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderZP::build
//
// Description:  Build ZP Info.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilderZP::buildDetails(TaxTrx& trx)
{

  if (!taxCodeReg())
  {
    std::get<Response::TAX::ERROR>(response().taxAttrValues()) = NO_DATA_ON_FILE;
    return;
  }

  if (airports()->size() < 2)
  {
    std::get<Response::TAX::ERROR>(response().taxAttrValues()) =
        MISSING_AIRPORTS; // because ZP is segment tax
    return;
  }
  else
  {
    _prevArpt = airports()->front();

    std::for_each(++(airports()->begin()),
                  airports()->end(),
                  boost::bind(&TaxInfoBuilderZP::buildItem, this, boost::ref(trx), _1));
  }

  TaxInfoResponse::AptItem item;

  std::get<Response::ZP::AIRPORT>(item) = airports()->back();

  if (!isAirport(getLoc(trx, airports()->back())))
  {
    std::get<Response::ZP::ERROR>(item) = INVALID_AIRPORT;
  }
  else
  {
    const Loc* loc = getLoc(trx, airports()->back());

    if (loc)
    {
      isDomestic(*loc) ? std::get<Response::ZP::IS_DOMESTIC>(item) = TaxInfoBuilder::TRUE
                       : std::get<Response::ZP::IS_DOMESTIC>(item) = TaxInfoBuilder::FALSE;

      loc->ruralarpind() ? std::get<Response::ZP::IS_RURAL>(item) = TaxInfoBuilder::TRUE
                         : std::get<Response::ZP::IS_RURAL>(item) = TaxInfoBuilder::FALSE;
    }
  }

  response().aptAttrValues().push_back(item);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderZP::buildItem
//
// Description:  Build response item.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilderZP::buildItem(TaxTrx& trx, LocCode& airport)
{

  TaxInfoResponse::AptItem item;

  bool isPrevValidAirpt = isAirport(getLoc(trx, _prevArpt));

  std::get<Response::ZP::AIRPORT>(item) = _prevArpt;

  if (!isPrevValidAirpt)
    std::get<Response::ZP::ERROR>(item) = INVALID_AIRPORT;
  else
  {
    std::get<Response::ZP::CURRENCY>(item) = taxCodeReg()->taxCur();

    const Loc* prevLoc = getLoc(trx, _prevArpt);
    const Loc* loc = getLoc(trx, airport);

    bool isArptCurr = isAirport(loc);

    if (prevLoc && loc)
    {
      bool isDomesticNonRuralCurr = loc && isDomestic(*loc) && !loc->ruralarpind();
      bool isDomesticNonRuralPrev = prevLoc && isDomestic(*prevLoc) && !prevLoc->ruralarpind();

      if (isArptCurr && isDomesticNonRuralPrev && isDomesticNonRuralCurr)
        std::get<Response::ZP::AMOUNT>(item) = amtToString(
            taxCodeReg()->taxAmt(), taxCodeReg()->taxCur(), trx.getRequest()->ticketingDT());
      else
        std::get<Response::ZP::AMOUNT>(item) =
            amtToString(0, taxCodeReg()->taxCur(), trx.getRequest()->ticketingDT());

      isDomestic(*prevLoc) ? std::get<Response::ZP::IS_DOMESTIC>(item) = TaxInfoBuilder::TRUE
                           : std::get<Response::ZP::IS_DOMESTIC>(item) = TaxInfoBuilder::FALSE;

      prevLoc->ruralarpind() ? std::get<Response::ZP::IS_RURAL>(item) = TaxInfoBuilder::TRUE
                             : std::get<Response::ZP::IS_RURAL>(item) = TaxInfoBuilder::FALSE;
    }
  }

  response().aptAttrValues().push_back(item);
  _prevArpt = airport;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderZP::isDomestic
//
// Description:  If domestic loc returns true.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderZP::isDomestic(const Loc& loc)
{

  return LocUtil::isUS(loc) || (LocUtil::isCanada(loc) && loc.bufferZoneInd()) ||
         (LocUtil::isMexico(loc) && loc.bufferZoneInd());
}
