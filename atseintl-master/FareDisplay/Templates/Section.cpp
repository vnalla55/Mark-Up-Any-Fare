//-------------------------------------------------------------------
//
//  File:        Section.cpp
//  Authors:     Mike Carroll
//  Created:     July 24, 2005
//  Description: Base class for a document section.
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

#include "FareDisplay/Templates/Section.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "DataModel/FareDisplayRequest.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleConst.h"

namespace tse
{
namespace
{
ConfigurableValue<bool>
enableNewRDHeader("FAREDISPLAY_SVC", "NEW_RDHEADER", false);
}
void
Section::addBlankLine()
{
  _trx.response() << SPACE << std::endl;
}

void
Section::spaces(const int16_t& numSpaces)
{
  _trx.response() << std::setfill(' ') << std::setw(numSpaces) << SPACE;
}

void
Section::initializeLine(std::ostringstream* oss, const int16_t& lineNumber)
{
  if (lineNumber > 1)
    oss->seekp(MAX_PSS_LINE_SIZE * (lineNumber - 1), std::ios_base::beg);
  oss->setf(std::ios::right, std::ios::adjustfield);
  *oss << std::setw(MAX_PSS_LINE_SIZE) << std::setfill(' ') << "\n";
}

void
Section::buildCurrencyLine(const CurrencyCode& currency,
                           const MoneyAmount& amount,
                           const CurrencyNoDec& dec,
                           const RoutingNumber& routing,
                           const DateTime& createDate,
                           const DateTime& fareDisc,
                           const DateTime& effDate,
                           const DateTime& disDate,
                           std::string footNote1,
                           std::string footNote2,
                           const VendorCode& vendor,
                           bool construct)
{
  std::ostringstream& oss = _trx.response();
  oss << currency;

  // display amount
  Money money(amount, currency);

  oss << std::right << std::fixed << std::setprecision(dec);
  oss << std::setfill(' ') << std::setw(9);
  oss << money.value();
  spaces(2);

  // display routing
  // bool isAddOnFare = ( _trx->getRequest()->inclusionCode() == ADDON_FARES ) ;

  if (construct)
    oss << "CONS";

  else if (routing == MILEAGE_ROUTING)
    oss << " MPM";

  else
  {
    oss << std::right << std::fixed;
    oss << std::setfill(' ') << std::setw(4) << routing;
  }

  spaces(2);

  // effective date / discontinue date
  oss << "E";
  if (effDate.isValid())
    oss << effDate.dateToString(DDMMMYY, "");
  else
    oss << " - N/A ";
  spaces(1);

  oss << "D";

  if (disDate.isValid())
    oss << disDate.dateToString(DDMMMYY, "");
  else
    oss << "-INFINITY";

  // display footnote
  oss << "  FN-";
  if (!footNote1.empty() || !footNote2.empty())
  {
    oss << footNote1;
    if (!footNote2.empty())
      oss << "/" << footNote2;
    else
      spaces(3);
  }
  else
    spaces(5);

  spaces(2);

  // display vendor
  oss << "VENDOR-" << vendor;

  oss << std::endl;

  // display System dates
  oss << "SYSTEM DATES - CREATED ";
  if (createDate.isValid())
  {
    oss << createDate.dateToString(DDMMMYY, "");
    oss << "/";
    // display time
    oss << createDate.timeToString(HHMM, "");
    spaces(2);
  }
  else
  {
    oss << " N/A   ";
    spaces(7);
  }
  oss << "EXPIRES ";
  if (fareDisc.isValid())
  {
    oss << fareDisc.dateToString(DDMMMYY, "");
    oss << "/";
    // display time
    oss << fareDisc.timeToString(HHMM, "");
  }
  else
    oss << "INFINITY";

  oss << std::endl;
}

void
Section::buildCurrencyLine(const CurrencyCode& currency,
                           const MoneyAmount& amount,
                           const CurrencyNoDec& dec,
                           const RoutingNumber& routing,
                           const DateTime& createDate,
                           const DateTime& fareDisc,
                           const DateTime& effDate,
                           const DateTime& disDate,
                           const FareClassCode& fareClass,
                           const TariffNumber& fareTariff,
                           const TariffCode& fareTariffCode,
                           const std::string& footNote1,
                           const std::string& footNote2,
                           bool construct)
{
  std::ostringstream& oss = _trx.response();
  oss << currency;

  // display amount
  Money money(amount, currency);

  oss << std::right << std::fixed << std::setprecision(dec);
  oss << std::setfill(' ') << std::setw(9);
  oss << money.value();
  spaces(2);

  if (construct)
    oss << "CONS";

  else if (routing == MILEAGE_ROUTING)
    oss << " MPM";

  else
  {
    oss << std::right << std::fixed;
    oss << std::setfill(' ') << std::setw(4) << routing;
  }
  spaces(2);

  // display create date / effective date / discontinue date
  oss << "E";
  if (effDate.isValid())
    oss << effDate.dateToString(DDMMMYY, "");
  else
    oss << " - N/A ";
  spaces(1);

  oss << "D";

  if (enableNewRDHeader.getValue())
  {
    oss << std::left << std::setfill(' ') << std::setw(9);

    if (disDate.isValid())
      oss << disDate.dateToString(DDMMMYY, "");
    else
      oss << "-INFINITY";

    spaces(1);

    // display fare class
    oss << "FC-" << std::left << std::setfill(' ') << std::setw(8) << fareClass;

    spaces(1);
    // display fare tariff
    if (fareTariffCode.empty())
      oss << "-";
    else
      oss << fareTariffCode;

    oss << "/" << fareTariff;
  }
  else
  {
    if (disDate.isValid())
      oss << disDate.dateToString(DDMMMYY, "");
    else
      oss << "-INFINITY";

    spaces(3);

    // display fare class
    oss << "FC-" << fareClass;

    // display footnote
    oss << "  FN-";
    if (!footNote1.empty() || !footNote2.empty())
    {
      oss << footNote1;
      if (!footNote2.empty())
        oss << "/" << footNote2;
      else
        spaces(3);
    }
    else
      spaces(5);
  }

  oss << std::endl;

  // display System dates
  oss << "SYSTEM DATES - CREATED ";
  if (createDate.isValid())
  {
    oss << createDate.dateToString(DDMMMYY, "");
    oss << "/";
    // display time
    oss << createDate.timeToString(HHMM, "");
    spaces(2);
  }
  else
  {
    oss << " N/A   ";
    spaces(7);
  }
  oss << "EXPIRES ";
  if (fareDisc.isValid())
  {
    oss << fareDisc.dateToString(DDMMMYY, "");
    oss << "/";
    // display time
    oss << fareDisc.timeToString(HHMM, "");
  }
  else
    oss << "INFINITY";

  oss << std::endl;
}
} // tse namespace
