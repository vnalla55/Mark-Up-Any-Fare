//-------------------------------------------------------------------
//
//  File:        ICSection.h
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

#pragma once

#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Templates/Section.h"

namespace tse
{

class ICSection : public Section
{
public:
  ICSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override { return; }
  void buildDisplay(PaxTypeFare& paxTypeFare);

private:
  static const std::string ROUND_UP;
  static const std::string ROUND_DOWN;
  static const std::string ROUND_NEAREST;
  static const std::string ROUND_NONE;

  LocCode _interior;
  LocCode _gateway;
  FareClassCode _fareClass;
  CurrencyCode _currency;
  MoneyAmount _fare = 0;
  AddonZone _zone = 0;
  CurrencyCode _displayCurrency;
  MoneyAmount _origAddonFare = 0;
  MoneyAmount _destAddonFare = 0;
  TariffNumber _addonTariff = 0;
  TariffCode _addonTariffCode;

  ExchRate _exchangeRate = 0;
  CurrencyNoDec _exchangeRateNoDec = 0;
  RoundingFactor _roundingFactor = 0;
  RoundingRule _roundingRule = RoundingRule::EMPTY;

  void displayHeader();
  void displayHeadings();
  void displayBlankLine();
  void addOnOriginLine(PaxTypeFare& paxTypeFare);
  void addOnOriginZoneLine(PaxTypeFare& paxTypeFare);
  void specifiedFareLine(PaxTypeFare& paxTypeFare);
  void addOnDestinationLine(PaxTypeFare& paxTypeFare);
  void addOnDestinationZoneLine(PaxTypeFare& paxTypeFare);
  void displayNucConversionLine(PaxTypeFare& paxTypeFare);
  void setFields(PaxTypeFare& paxTypeFare, char component);
  void setFares(PaxTypeFare& paxTypeFare, char component);
  void setAddonTariff(PaxTypeFare& paxTypeFare, char component);
  void setRoundTripFares();
  void convertFare(PaxTypeFare& paxTypeFare);
  bool nucConversion(PaxTypeFare& paxTypeFare);
  void displayTotals(PaxTypeFare& paxTypeFare);
  void displayRoundTripTotals(PaxTypeFare& paxTypeFare);
  void displayOneWayHeader();
  void displayRoundTripHeader();
  void displayTrailerMessage();
  void displayTrailerMsgDiscount();
};
} // namespace tse
