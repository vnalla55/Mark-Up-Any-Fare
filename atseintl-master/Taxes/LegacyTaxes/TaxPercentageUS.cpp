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

#include "Common/LocUtil.h"
#include "Common/TaxRound.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxPercentageUS.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <sstream>

using namespace tse;
using namespace std;

const MoneyAmount TaxPercentageUS::MIN_AMOUNT = 0.00;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxPercentageUS::TaxPercentageUS
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

TaxPercentageUS::TaxPercentageUS(TaxRequest& r)
  : TAX_CODE_US1("US1"), TAX_CODE_US2("US2"), _request(r)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxPercentageUS::~TaxPercentageUS
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxPercentageUS::build()
{
  if (validHiAkConUSTrip())
    taxFactorsBuild();
  else if (validConUSTrip())
    taxContinentalBuild();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function taxContinentalBuild
//
// Description: Displays taxes for travel within the continental U.S.
//              Returns base/total fare, US1 tax.

//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxPercentageUS::taxContinentalBuild()
{

  TaxTrx trx;

  DateTime dt;
  Itin itin;
  TaxRequest taxRequest;
  TaxResponse taxResponse;

  trx.setRequest(&taxRequest);
  trx.getRequest()->ticketingDT() = _request.ticketingDT();
  trx.dataHandle().setTicketDate(trx.getRequest()->ticketingDT());
  trx.itin().push_back(&itin);

  // FIXME: whoa, that's bold. adding a pointer to local variable...
  trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

  const std::vector<TaxCodeReg*>& taxCodeReg =
      trx.dataHandle().getTaxCode(TAX_CODE_US1, trx.getRequest()->ticketingDT());

  if (taxCodeReg.empty())
    return;

  FareAndTax fareAndTax = calculateFareAndTax(taxCodeReg.front()->taxAmt());

  _output = amtFmt(fareAndTax.first) + "/" + amtFmt(fareAndTax.second);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function taxFactorsBuild
//
// Description: Displays taxes for travel between Alaska/Hawaii and
//              The Continental U.S., or between Alaska and Hawaii.
//              Returns base/total fare, US2 tax, part of US1 tax
//              applied, and the Alaska/Hawaii percentage factor.
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxPercentageUS::taxFactorsBuild()
{

  TaxTrx trx;

  DateTime dt;
  Itin itin;
  TaxRequest taxRequest;
  TaxResponse taxResponse;

  trx.setRequest(&taxRequest);
  trx.getRequest()->ticketingDT() = _request.ticketingDT();
  trx.dataHandle().setTicketDate(trx.getRequest()->ticketingDT());
  trx.itin().push_back(&itin);

  // FIXME: WTF? pointer to local variable
  trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

  const std::vector<TaxCodeReg*>& taxCodeReg =
      trx.dataHandle().getTaxCode(TAX_CODE_US2, trx.getRequest()->ticketingDT());

  if (taxCodeReg.empty())
    return;

  _output = "NO TAX FACTOR FOUND FOR CITY PAIR - MODIFY AND REENTER";

  const Loc* origin = trx.dataHandle().getLoc(_request.loc1(), time(nullptr));
  const Loc* destination = trx.dataHandle().getLoc(_request.loc2(), time(nullptr));

  if (origin == nullptr)
    return;

  if (destination == nullptr)
    return;

  if (!validHiAkConUSLoc(*origin, *destination))
    return;

  MoneyAmount factor = locateHiAkFactor(trx, origin, destination);

  MoneyAmount taxUS2 = taxRound(trx, *(taxCodeReg.front()), taxCodeReg.front()->taxAmt() / 2);

  if (_request.tripType() != ONE_WAY_TRIP)
    taxUS2 *= 2;

  FareAndTax fareAndTax = calculateFareAndTax(factor, taxUS2);

  _output = amtFmt(fareAndTax.first) + "/" + amtFmt(taxUS2) + "/" + amtFmt(fareAndTax.second) +
            " - TAX PCT. " + boost::lexical_cast<std::string>(amtFmt(factor * 100));
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function calculateFareAndTax
//
// Description: Calculate base/total fare and tax.
//
// </PRE>
// ----------------------------------------------------------------------------

TaxPercentageUS::FareAndTax
TaxPercentageUS::calculateFareAndTax(const MoneyAmount& factor, const double& taxUS2)
{

  FareAndTax fareAndTax(0.0, 0.0);

  if (_request.amtType() == BASE_FARE)
  {
    fareAndTax.second = factor * _request.fareAmt();
    fareAndTax.first = fareAndTax.second + _request.fareAmt() + taxUS2;
  }
  else if (_request.amtType() == TOTAL_FARE)
  {
    fareAndTax.first = (_request.fareAmt() - taxUS2) / (1 + factor);
    fareAndTax.second = fareAndTax.first * factor;
  }

  return fareAndTax;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function locateHiAkFactor
//
// Description: Calculate Hi/Ak tax factor.
//
// </PRE>
// ----------------------------------------------------------------------------
MoneyAmount
TaxPercentageUS::locateHiAkFactor(TaxTrx& trx, const Loc* origin, const Loc* destination) const
{
  MoneyAmount factor = 0;

  if (LocUtil::isHawaii(*origin))
    factor = taxUtil::locateHiFactor(trx, _request.loc2());
  else if (LocUtil::isHawaii(*destination))
    factor = taxUtil::locateHiFactor(trx, _request.loc1());
  else if (!LocUtil::isHawaii(*origin) || !LocUtil::isHawaii(*destination))
  {
    if (LocUtil::isAlaska(*origin))
      factor = taxUtil::locateAkFactor(trx, origin, _request.loc2());
    else if (LocUtil::isAlaska(*destination))
      factor = taxUtil::locateAkFactor(trx, destination, _request.loc1());
  }

  return factor;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function validAmount
//
// Description: Valid amount type and fare amount.
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxPercentageUS::validAmt(TaxRequest* req)
{
  if (req->fareAmt() >= MIN_AMOUNT && (req->amtType() == TOTAL_FARE || req->amtType() == BASE_FARE))
    return true;
  else
    return false;
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function validConUSTrip
//
// Description: Valid loc1 & loc2 and trip type.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxPercentageUS::validConUSTrip(TaxRequest* req)
{
  if (TaxPercentageUS::validAmt(req) && req->loc1() == EMPTY_STRING() &&
      req->loc2() == EMPTY_STRING() && req->tripType() == INVALID_INDICATOR)
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function validHiAkConUSTrip
//
// Description: Valid loc1 & loc2 and trip type.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxPercentageUS::validHiAkConUSTrip(TaxRequest* req)
{
  if (TaxPercentageUS::validAmt(req) && req->loc1() != EMPTY_STRING() &&
      req->loc2() != EMPTY_STRING() &&
      (req->tripType() == ONE_WAY_TRIP || req->tripType() == ROUND_TRIP))
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function validAmount
//
// Description: Valid amount type and fare amount.
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxPercentageUS::validAmount() const
{
  if (_request.fareAmt() >= MIN_AMOUNT &&
      (_request.amtType() == TOTAL_FARE || _request.amtType() == BASE_FARE))
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function validHiAkConUSTrip
//
// Description: Valid loc1 & loc2 and trip type.
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxPercentageUS::validHiAkConUSTrip() const
{
  if (validAmount() && _request.loc1() != EMPTY_STRING() && _request.loc2() != EMPTY_STRING() &&
      (_request.tripType() == ONE_WAY_TRIP || _request.tripType() == ROUND_TRIP))
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function validConUSTrip
//
// Description: Valid loc1 & loc2 and trip type.
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxPercentageUS::validConUSTrip() const
{
  if (validAmount() && _request.loc1() == EMPTY_STRING() && _request.loc2() == EMPTY_STRING() &&
      _request.tripType() == INVALID_INDICATOR)
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function validHiAkConUSLoc
//
// Description: Valid travel between Alaska/Hawaii and the Continental U.S.,
//              or between Alaska and Hawaii.
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxPercentageUS::validHiAkConUSLoc(const Loc& origin, const Loc& destination) const
{
  if ((LocUtil::isUS(origin) && LocUtil::isUS(destination)) &&
      ((LocUtil::isHawaii(origin) || LocUtil::isAlaska(origin)) ||
       (LocUtil::isHawaii(destination) || LocUtil::isAlaska(destination))))
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function taxRound
//
// Description: Round tax.
//
// </PRE>
// ----------------------------------------------------------------------------
MoneyAmount
TaxPercentageUS::taxRound(TaxTrx& trx, const TaxCodeReg& taxCodeReg, const MoneyAmount& taxAmt)
{
  RoundingFactor roundingUnit = 0.1;

  RoundingRule roundingRule = UP;

  if (!taxCodeReg.specConfigName().empty())
  {
    std::string rndConf = utc::getSpecConfigParameter(
        trx, taxCodeReg.specConfigName(), "HALFTAXROUND", _request.ticketingDT());
    if (!rndConf.empty())
    {
      if (rndConf[0] == 'D')
        roundingRule = DOWN;
      else if (rndConf[0] == 'U')
        roundingRule = UP;
      else if (rndConf[0] == 'N')
        roundingRule = NONE;
    }
  }

  TaxRound taxRound;

  MoneyAmount taxAmtRound =
      taxRound.applyTaxRound(taxAmt, taxCodeReg.taxCur(), roundingUnit, roundingRule);
  return taxAmtRound;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function outputFmt
//
// Description:Output amount format.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
TaxPercentageUS::amtFmt(const MoneyAmount& amt, unsigned int precision)
{

  std::ostringstream amtStream;

  amtStream.setf(std::ios::fixed, std::ios::floatfield);
  amtStream.setf(std::ios::left, std::ios::adjustfield);

  amtStream.precision(precision);

  amtStream << amt;

  return amtStream.str();
}
