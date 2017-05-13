//-------------------------------------------------------------------
//
//  File:        RRSection.h
//  Authors:     Artur Krezel
//  Created:     Nov 05, 2015
//  Description: This class renders the Fare Mark-Up
//               display.
//
//
//  Copyright Sabre 2015
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

#include <sstream>

namespace tse
{
class AdjustedSellingCalcData;
class NegPaxTypeFareRuleData;
class PaxTypeFare;

class RRSection final : public Section
{
  friend class RRSectionTest;

public:
  RRSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override {}
  void buildDisplay(PaxTypeFare& ptf);

  static const char* INDENTION;

private:
  static constexpr uint8_t LINE_SIZE = 60;
  static const char* SELLING_LEVEL_CALCULATION;
  static const char* ASL_ADJUSTED_SELLING_LEVEL_CALCULATION;

  void displayNetBussinesID(const uint64_t fareRetailerRuleId, const PseudoCityCode& pcc,
      const Indicator fdCat35Type);

  void displayFareRetailerBussinesRuleID(const NegPaxTypeFareRuleData& negRuleData,
                                         const Indicator fdType);
  void displaySellingFareBussinesID(const uint64_t fareRetailerRuleId, const PseudoCityCode& pcc);
  void addWrappedTextToResponse(const std::string& msg);
  void displayNETFareCreation(const NegPaxTypeFareRuleData& negRuleData);
  void netFareCreation(const CurrencyCode& currCode, NegPaxTypeFareRuleData& negRuleData);
  void formatAmountText(std::string& percentStr, const Percent percent,
      const int getNoDecPercent) const;
  void displayAdjustedSellingLvl(const AdjustedSellingCalcData& adjSellCalcData);

  // Nested Classes
public:
  class FareCreation
  {
    friend class RRSectionTest;

  public:
    static const char* PLUS_TEXT;
    static const char* MINUS_TEXT;

    static const char* BASE_FARE;
    static const char* BASE_FARE_CAT25;
    static const char* PERCENT;
    static const char* EQUALS;

    FareCreation(const Indicator fareInd,
                 const std::string& percent,
                 const std::string& fareAmt,
                 const CurrencyCode& currencyCode,
                 bool isPositive);

    std::string getFareCreation(bool isCat25 = false) const;

  private:
    const Indicator _fareInd;
    std::string _percent;
    std::string _fareAmt;
    const CurrencyCode& _currencyCode;
    std::string _baseFare;
  };
};
} // namespace tse
