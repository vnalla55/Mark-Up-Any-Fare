//-------------------------------------------------------------------
//
//  File:        Section.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//
//  Updates:
//
//  Copyright Sabre 2005
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

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"

#include <sstream>
#include <string>

namespace tse
{

class Section
{
public:
  Section(FareDisplayTrx& trx) : _trx(trx) {}

  virtual ~Section() = default;

  Section(const Section&) = delete;
  Section& operator=(const Section&) = delete;

  virtual void buildDisplay() = 0;
  void addBlankLine();

  //--------------------------------------------------------------------------
  // @function Section::spaces
  //
  // Description: Method to add spaces to the current line in this section
  //
  // @param numSpaces - number of spaces to add
  //--------------------------------------------------------------------------
  void spaces(const int16_t& numSpaces);

  //--------------------------------------------------------------------------
  // @function Section::initializeLine
  //
  // Description: Blank and newline terminate the oss appropriate line
  //
  // @param oss - ostringstream to be altered
  // @param lineNumber - line number (1 based)
  //--------------------------------------------------------------------------
  void initializeLine(std::ostringstream* oss, const int16_t& lineNumber);

  // -------------------------------------------------------------------------
  // @functionSection::buildCurrencyLine()
  // Description: Method to build a currency line for Addon fare
  //
  // @param vendor - addon vendor
  // @param construct - bool, shows if the fare is constructed
  // -------------------------------------------------------------------------
  void buildCurrencyLine(const CurrencyCode& currency,
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
                         bool construct);

  // -------------------------------------------------------------------------
  // @functionSection::buildCurrencyLine()
  // Description: Method to build a currency line (RD changes for historical)
  //
  // @param construct - bool, shows if the fare is constructed
  // -------------------------------------------------------------------------
  void buildCurrencyLine(const CurrencyCode& currency,
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
                         bool construct);

  std::ostringstream& displayLine() { return _displayLine; }
  const std::ostringstream& displayLine() const { return _displayLine; }

protected:
  FareDisplayTrx& _trx;

private:
  std::ostringstream _displayLine;
};
} // namespace tse
