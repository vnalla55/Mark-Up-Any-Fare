//-------------------------------------------------------------------
//
//  File:        ICSection.cpp
//  Authors:     Doug Batchelor
//  Created:     Sep 20, 2005
//  Description: This class renders the International
//               Construction display.
//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/ICSection.h"

#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/DataHandle.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"

#include <sstream>
#include <string>

namespace tse
{
const std::string
tse::ICSection::ROUND_UP("ROUNDED UP TO NEXT");
const std::string
tse::ICSection::ROUND_DOWN("ROUNDED DOWN TO");
const std::string
tse::ICSection::ROUND_NEAREST("ROUNDED TO NEAREST");
const std::string
tse::ICSection::ROUND_NONE("NO ROUNDING");

const char ORIG_ADDON = 'O';
const char DEST_ADDON = 'D';
const char BASE = 'B';

void
ICSection::buildDisplay(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isDiscounted())
  {
    // Display Trailer msg for Discounted Fares
    displayTrailerMsgDiscount();
  }
  else
  {
    displayHeader();

    if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      displayRoundTripHeader();
      displayHeadings();
      displayTotals(paxTypeFare);
      displayBlankLine();
      displayNucConversionLine(paxTypeFare);
    }
    else if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    {
      displayOneWayHeader();
      displayHeadings();
      displayTotals(paxTypeFare);
      displayBlankLine();
      displayNucConversionLine(paxTypeFare);
    }
    // ONEWAY MAY BE DOUBLED
    else if (!_trx.isTwoColumnTemplate())
    {
      displayOneWayHeader();
      displayHeadings();
      displayTotals(paxTypeFare);
      displayTrailerMessage();
      displayNucConversionLine(paxTypeFare);
    }
    else
    {
      displayOneWayHeader();
      displayHeadings();
      displayTotals(paxTypeFare);
      displayBlankLine();
      displayRoundTripHeader();
      displayRoundTripTotals(paxTypeFare);
      displayBlankLine();
      displayNucConversionLine(paxTypeFare);
    }
  }
}

void
ICSection::displayTotals(PaxTypeFare& paxTypeFare)
{
  setFares(paxTypeFare, ORIG_ADDON);
  setAddonTariff(paxTypeFare, ORIG_ADDON);
  addOnOriginLine(paxTypeFare);
  addOnOriginZoneLine(paxTypeFare);
  setFares(paxTypeFare, BASE);
  specifiedFareLine(paxTypeFare);
  setFares(paxTypeFare, DEST_ADDON);
  setAddonTariff(paxTypeFare, DEST_ADDON);
  addOnDestinationLine(paxTypeFare);
  addOnDestinationZoneLine(paxTypeFare);
}

void
ICSection::displayRoundTripTotals(PaxTypeFare& paxTypeFare)
{
  setFares(paxTypeFare, ORIG_ADDON);
  setAddonTariff(paxTypeFare, ORIG_ADDON);
  setRoundTripFares();
  addOnOriginLine(paxTypeFare);
  addOnOriginZoneLine(paxTypeFare);
  setFares(paxTypeFare, BASE);
  setRoundTripFares();
  specifiedFareLine(paxTypeFare);
  setFares(paxTypeFare, DEST_ADDON);
  setAddonTariff(paxTypeFare, DEST_ADDON);
  setRoundTripFares();
  addOnDestinationLine(paxTypeFare);
  addOnDestinationZoneLine(paxTypeFare);
}

void
ICSection::displayHeader()
{
  _trx.response() << "** ADDONS FOR INFORMATION ONLY **" << std::endl;
}

void
ICSection::displayHeadings()
{
  _trx.response() << "                            PUBLISHED AMOUNT   CONVERTED AMOUNT" << std::endl;
  _trx.response() << "ADDON      CITIES  F/B      CUR                VIA NUC" << std::endl;
}

void
ICSection::displayBlankLine()
{
  _trx.response() << " " << std::endl;
}

void
ICSection::displayOneWayHeader()
{
  _trx.response() << "FARE--OW" << std::endl;
}

void
ICSection::displayRoundTripHeader()
{
  _trx.response() << "FARE--RT" << std::endl;
}

void
ICSection::displayTrailerMessage()
{
  _trx.response() << " " << std::endl;
  _trx.response() << "ONE WAY AMOUNTS MAY BE DOUBLED FOR ROUND TRIP" << std::endl;
}

void
ICSection::displayTrailerMsgDiscount()
{
  _trx.response() << "CONSTRUCTION NOT DISPLAYED FOR DISCOUNTED FARES" << std::endl;
}

void
ICSection::addOnOriginLine(PaxTypeFare& paxTypeFare)
{

  if (paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN)
  {
    return;
  }

  if (!paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION)
  {
    return;
  }

  setFields(paxTypeFare, ORIG_ADDON);

  const DateTime& ticketingDate = _trx.ticketingDate();

  // Format ORIGIN - GATEWAY, FARE CLASS, and CURRENCY
  // ADDON ORG PUS-SEL H******     KRW
  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);
  oss[0].seekp(0, std::ios_base::beg);
  oss[0] << std::setw(9) << "ADDON ORG";

  oss[0].setf(std::ios::left, std::ios::adjustfield);
  oss[0].seekp(11, std::ios_base::beg);
  oss[0] << _interior << "-" << _gateway;

  oss[0].setf(std::ios::left, std::ios::adjustfield);
  oss[0].seekp(19, std::ios_base::beg);
  oss[0] << std::setw(13) << _fareClass;

  oss[0].seekp(28, std::ios_base::beg);
  oss[0] << std::setw(3) << _currency;

  // Display FARE AMOUNT in filed currency
  // ADDON ORG PUS-SEL H******     KRW         77200
  oss[0].seekp(36, std::ios_base::beg);
  oss[0].setf(std::ios::fixed, std::ios::floatfield);
  oss[0].setf(std::ios::right, std::ios::adjustfield);
  Money moneyPayment(_currency);
  oss[0].precision(moneyPayment.noDec(ticketingDate));
  oss[0] << std::setw(8) << _fare;
  oss[0].setf(std::ios::left, std::ios::adjustfield);

  convertFare(paxTypeFare);
  _origAddonFare = _fare;

  // Display converted FARE AMOUNT in constructed fare currency
  // ADDON ORG PUS-SEL H******     KRW         77200  EUR      772000
  oss[0].seekp(47, std::ios_base::beg);
  oss[0] << std::setw(3) << paxTypeFare.currency();

  oss[0].seekp(54, std::ios_base::beg);
  oss[0].setf(std::ios::fixed, std::ios::floatfield);
  oss[0].setf(std::ios::right, std::ios::adjustfield);
  Money moneyPayment2(paxTypeFare.currency());
  oss[0].precision(moneyPayment2.noDec(ticketingDate));
  oss[0] << std::setw(8) << _fare;
  oss[0].setf(std::ios::left, std::ios::adjustfield);

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::addOnOriginZoneLine()
// -------------------------------------------------------------------
void
ICSection::addOnOriginZoneLine(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN)
  {
    return;
  }

  if (!paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION)
  {
    return;
  }

  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);

  if (paxTypeFare.vendor() == ATPCO_VENDOR_CODE)
    oss[0].seekp(11, std::ios_base::beg);
  else
    oss[0].seekp(10, std::ios_base::beg);

  oss[0].setf(std::ios::right, std::ios::adjustfield);
  oss[0] << paxTypeFare.vendor() << " ZONE " << _zone;
  oss[0].seekp(28, std::ios_base::beg);
  oss[0] << "ADD-ON TARIFF " << _addonTariffCode << "/" << _addonTariff;

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::specifiedFareLine()
// -------------------------------------------------------------------
void
ICSection::specifiedFareLine(PaxTypeFare& paxTypeFare)
{
  setFields(paxTypeFare, BASE);

  const DateTime& ticketingDate = _trx.ticketingDate();

  // Format SPECIFIED SEL-DEN MLPX3BM     KRW    3859800
  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);
  oss[0].seekp(0, std::ios_base::beg);
  oss[0] << std::setw(9) << "PUBLISHED";

  oss[0].seekp(11, std::ios_base::beg);
  oss[0] << _interior << "-" << _gateway;
  oss[0].setf(std::ios::left, std::ios::adjustfield);

  oss[0].seekp(19, std::ios_base::beg);
  std::string fareBasis = paxTypeFare.createFareBasisCodeFD(_trx);
  oss[0] << std::setw(13) << fareBasis;

  oss[0].seekp(28, std::ios_base::beg);
  oss[0] << std::setw(3) << paxTypeFare.currency();

  oss[0].seekp(36, std::ios_base::beg);
  oss[0].setf(std::ios::fixed, std::ios::floatfield);
  oss[0].setf(std::ios::right, std::ios::adjustfield);

  Money moneyPayment(paxTypeFare.currency());
  oss[0].precision(moneyPayment.noDec(ticketingDate));
  oss[0] << std::setw(8) << _fare;
  oss[0].setf(std::ios::left, std::ios::adjustfield);
  oss[0].seekp(47, std::ios_base::beg);
  oss[0] << std::setw(3) << paxTypeFare.currency();

  oss[0].seekp(54, std::ios_base::beg);
  oss[0].setf(std::ios::fixed, std::ios::floatfield);
  oss[0].setf(std::ios::right, std::ios::adjustfield);

  Money moneyPayment2(paxTypeFare.currency());
  oss[0].precision(moneyPayment2.noDec(ticketingDate));
  oss[0] << std::setw(8) << _fare;
  oss[0].setf(std::ios::left, std::ios::adjustfield);

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::addOnDestinationLine()
// -------------------------------------------------------------------
void
ICSection::addOnDestinationLine(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION)
  {
    return;
  }

  if (!paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN)
  {
    return;
  }

  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);
  oss[0].seekp(0, std::ios_base::beg);
  oss[0] << std::setw(9) << "ADDON DST";

  setFields(paxTypeFare, DEST_ADDON);

  const DateTime& ticketingDate = _trx.ticketingDate();

  // Format DESTINATION - GATEWAY, FARE CLASS, and CURRENCY
  // ADDON DST PUS-SEL H******     KRW
  oss[0].setf(std::ios::left, std::ios::adjustfield);
  oss[0].seekp(11, std::ios_base::beg);
  oss[0] << _gateway << "-" << _interior;

  oss[0].setf(std::ios::left, std::ios::adjustfield);
  oss[0].seekp(19, std::ios_base::beg);
  oss[0] << std::setw(13) << _fareClass;

  oss[0].seekp(28, std::ios_base::beg);
  oss[0] << std::setw(3) << _currency;

  // Display FARE AMOUNT in filed currency
  // ADDON DST PUS-SEL H******     KRW         77200
  oss[0].seekp(36, std::ios_base::beg);
  oss[0].setf(std::ios::fixed, std::ios::floatfield);
  oss[0].setf(std::ios::right, std::ios::adjustfield);
  Money moneyPayment(_currency);
  oss[0].precision(moneyPayment.noDec(ticketingDate));
  oss[0] << std::setw(8) << _fare;
  oss[0].setf(std::ios::left, std::ios::adjustfield);

  // Convert Fare for display
  convertFare(paxTypeFare);
  _destAddonFare = _fare;

  // Display converted FARE AMOUNT in display currency
  // ADDON DST PUS-SEL H******     KRW         77200  EUR      772000
  oss[0].seekp(47, std::ios_base::beg);
  oss[0] << std::setw(3) << paxTypeFare.currency();
  oss[0].seekp(54, std::ios_base::beg);
  oss[0].setf(std::ios::fixed, std::ios::floatfield);
  oss[0].setf(std::ios::right, std::ios::adjustfield);

  Money moneyPayment2(paxTypeFare.currency());
  oss[0].precision(moneyPayment2.noDec(ticketingDate));
  oss[0] << std::setw(8) << _fare;
  oss[0].setf(std::ios::left, std::ios::adjustfield);

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::addOnDestinationZoneLine()
// -------------------------------------------------------------------
void
ICSection::addOnDestinationZoneLine(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION)
  {
    return;
  }

  if (!paxTypeFare.isReversed() &&
      paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN)
  {
    return;
  }

  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);

  if (paxTypeFare.vendor() == ATPCO_VENDOR_CODE)
    oss[0].seekp(11, std::ios_base::beg);
  else
    oss[0].seekp(10, std::ios_base::beg);

  oss[0].setf(std::ios::right, std::ios::adjustfield);
  oss[0] << paxTypeFare.vendor() << " ZONE " << _zone;
  oss[0].seekp(28, std::ios_base::beg);
  oss[0] << "ADD-ON TARIFF " << _addonTariffCode << "/" << _addonTariff;

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::setFields()
//              Initialize fields for each addon component.
// -------------------------------------------------------------------
void
ICSection::setFields(PaxTypeFare& paxTypeFare, char component)
{
  if ((component == ORIG_ADDON && paxTypeFare.isReversed()) ||
      (component == DEST_ADDON && !paxTypeFare.isReversed()))
  {
    _interior = paxTypeFare.market2();
    _gateway = paxTypeFare.gateway2();
    _fareClass = paxTypeFare.destAddonFareClass();
    _currency = paxTypeFare.destAddonCurrency();
    _currency = paxTypeFare.destAddonCurrency();
    _zone = paxTypeFare.destAddonZone();
  }
  else if ((component == ORIG_ADDON && !paxTypeFare.isReversed()) ||
           (component == DEST_ADDON && paxTypeFare.isReversed()))
  {
    _interior = paxTypeFare.market1();
    _gateway = paxTypeFare.gateway1();
    _fareClass = paxTypeFare.origAddonFareClass();
    _currency = paxTypeFare.origAddonCurrency();
    _currency = paxTypeFare.origAddonCurrency();
    _zone = paxTypeFare.origAddonZone();
  }

  else if (component == BASE)
  {
    if (paxTypeFare.isReversed())
    {
      _interior = paxTypeFare.gateway2();
      _gateway = paxTypeFare.gateway1();
    }
    else
    {
      _interior = paxTypeFare.gateway1();
      _gateway = paxTypeFare.gateway2();
    }
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::setFares()
//              Initialize fields for each addon component.
// -------------------------------------------------------------------
void
ICSection::setFares(PaxTypeFare& paxTypeFare, char component)
{
  if (!paxTypeFare.isDiscounted())
  {
    if ((component == ORIG_ADDON && paxTypeFare.isReversed()) ||
        (component == DEST_ADDON && !paxTypeFare.isReversed()))
    {
      _fare = paxTypeFare.destAddonAmount();
    }
    else if ((component == ORIG_ADDON && !paxTypeFare.isReversed()) ||
             (component == DEST_ADDON && paxTypeFare.isReversed()))
    {
      _fare = paxTypeFare.origAddonAmount();
    }

    else if (component == BASE)
    {
      _fare = paxTypeFare.specifiedFareAmount();
    }
  }
  else
  {
    if (component == BASE)
      _fare = paxTypeFare.fareAmount();

    else
      _fare = 0;
  }

  if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    _fare = _fare * 2;
  }
}

// -------------------------------------------------------------------
// @MethodName  ICSection::setAddonTariff()
//              Initialize fields for each addon component.
// -------------------------------------------------------------------
void
ICSection::setAddonTariff(PaxTypeFare& paxTypeFare, char component)
{
  if ((component == ORIG_ADDON && paxTypeFare.isReversed()) ||
      (component == DEST_ADDON && !paxTypeFare.isReversed()))
  {
    _addonTariff = paxTypeFare.destAddonTariff();
  }
  else if ((component == ORIG_ADDON && !paxTypeFare.isReversed()) ||
           (component == DEST_ADDON && paxTypeFare.isReversed()))
  {
    _addonTariff = paxTypeFare.origAddonTariff();
  }

  _addonTariffCode = "";
  FareDisplayResponseUtil::getAddonTariffCode(
      _trx, paxTypeFare.vendor(), paxTypeFare.carrier(), _addonTariff, _addonTariffCode);
}

// -------------------------------------------------------------------
// @MethodName  ICSection::setRoundTripFares()
//              Initialize fields for each addon component.
// -------------------------------------------------------------------
void
ICSection::setRoundTripFares()
{
  _fare = _fare * 2;
}

// -----------------------------------------------------------------------
// @MethodName  ICSection::convertFare()
//              Convert Fare from published currency to NUC
//              Then convert Fare from NUC to currency of specified Fare.
// -----------------------------------------------------------------------
void
ICSection::convertFare(PaxTypeFare& paxTypeFare)
{
  // If the published currency of the addon component is equal to the
  // currency of the specified fare, just return the fare.
  if (_currency == paxTypeFare.currency())
    return;
  NUCCollectionResults nucResults;
  FareDisplayResponseUtil::convertFare(_trx, paxTypeFare, _fare, _currency, nucResults);
  _exchangeRate = nucResults.exchangeRate();
  _exchangeRateNoDec = nucResults.exchangeRateNoDec();
  _roundingFactor = nucResults.roundingFactor();
  _roundingRule = nucResults.roundingRule();
}

// -----------------------------------------------------------------------
// @MethodName  ICSection::nucConversion()
//              Determine whether a NUC conversion was done.
// -----------------------------------------------------------------------
bool
ICSection::nucConversion(PaxTypeFare& paxTypeFare)
{

  if ((paxTypeFare.origAddonCurrency().empty() ||
       paxTypeFare.origAddonCurrency() == paxTypeFare.currency()) &&
      (paxTypeFare.destAddonCurrency().empty() ||
       paxTypeFare.destAddonCurrency() == paxTypeFare.currency()))
  {
    return false;
  }

  return true;
}

// -------------------------------------------------------------------
// @MethodName  ICSection::addOnCurrencyConversionLine()
// -------------------------------------------------------------------
void
ICSection::displayNucConversionLine(PaxTypeFare& paxTypeFare)
{

  // Display only when all component currencies are not equal
  // to the specified currency
  if (!nucConversion(paxTypeFare))
    return;

  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);
  oss[0].seekp(1, std::ios_base::beg);

  // Format Rounding Message
  std::string roundingMsg;

  if (_roundingRule == UP)
    roundingMsg = ROUND_UP;
  else if (_roundingRule == DOWN)
    roundingMsg = ROUND_DOWN;
  else if (_roundingRule == NEAREST)
    roundingMsg = ROUND_NEAREST;
  else
    roundingMsg = ROUND_NONE;

  //  1 NUC - 9999999.99 JPY ROUNDED TO NEAREST
  oss[0].setf(std::ios::right, std::ios::adjustfield);
  oss[0] << "1 NUC - " << _exchangeRate << " " << _currency << " " << roundingMsg << " ";

  // Format Rounding Factor
  //  1 NUC - 9999999.99 JPY ROUNDED UP TO NEXT 9999999.99 JPY
  oss[0].setf(std::ios::left, std::ios::adjustfield);
  std::ostringstream roundingFactorStr;
  roundingFactorStr << _roundingFactor;

  if ((_roundingFactor > 0.0) && (_roundingFactor < 1.00))
  {
    oss[0].setf(std::ios::left, std::ios::adjustfield);
    oss[0] << roundingFactorStr.str().c_str() << " " << _currency;
  }
  else
  {
    oss[0].setf(std::ios::left, std::ios::adjustfield);
    oss[0].setf(std::ios::fixed, std::ios::floatfield);
    oss[0].precision(0);
    oss[0] << _roundingFactor << " " << _currency;
  }

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
}
} // tse namespace
