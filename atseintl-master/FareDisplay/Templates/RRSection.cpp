//-------------------------------------------------------------------
//
//  File:        RRSection.cpp
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

#include "FareDisplay/Templates/RRSection.h"

#include "Common/CurrencyUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Xform/ResponseFormatter.h"

namespace tse
{
const char* RRSection::INDENTION = "   ";
const char* RRSection::SELLING_LEVEL_CALCULATION = "SELLING LEVEL CALCULATION";
const char* RRSection::ASL_ADJUSTED_SELLING_LEVEL_CALCULATION =
    "ASL-ADJUSTED SELLING LEVEL CALCULATION";

void
RRSection::buildDisplay(PaxTypeFare& ptf)
{
  NegPaxTypeFareRuleData* negRuleData = nullptr;

  if (ptf.hasCat35Filed())
  {
    negRuleData = ptf.getNegRuleData();
    if (negRuleData && !negRuleData->fareRetailerRuleId())
      negRuleData = nullptr;
  }

  if (!negRuleData && (!ptf.getAdjustedSellingCalcData() || _trx.getOptions()->isXRSForFRRule()))
  {
    _trx.response() << INDENTION << "NOT APPLICABLE" << std::endl;
    return;
  }

  if (negRuleData)
  {
    displayFareRetailerBussinesRuleID(*negRuleData, ptf.fcaDisplayCatType());

    const bool calculationInfoCondition =
        ptf.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD &&
        (ptf.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE ||
         ptf.fareDisplayCat35Type() == RuleConst::NET_FARE);

    if (calculationInfoCondition)
      netFareCreation(ptf.currency(), *negRuleData);
  }

  if (ptf.getAdjustedSellingCalcData())
  {
    if (negRuleData)
      addBlankLine();

    displaySellingFareBussinesID(ptf.getAdjustedSellingCalcData()->getFareRetailerRuleId(),
        ptf.getAdjustedSellingCalcData()->getSourcePcc());

    if (_trx.getOptions()->isPDOForFRRule())
      displayAdjustedSellingLvl(*ptf.getAdjustedSellingCalcData());
  }
}

namespace
{
std::string
wrapRuleId(const std::string& base, const std::string& ruleId)
{
  std::string ret(base);
  if (base.size() + ruleId.size() < DISPLAY_MAX_SIZE)
  {
    ret += SPACE;
  }
  else
  {
    ret += '\n';
    ret += RRSection::INDENTION;
  }

  ret += ruleId;
  return ret;
}
}

void
RRSection::displayFareRetailerBussinesRuleID(const NegPaxTypeFareRuleData& negRuleData,
                                             const Indicator fdType)
{
  const uint64_t fareRetailerRuleId = negRuleData.fareRetailerRuleId();
  const PseudoCityCode& pcc = negRuleData.sourcePseudoCity();
  std::string bussinesRuleType;
  switch (fdType)
  {
  case RuleConst::NET_SUBMIT_FARE_UPD:
    if (negRuleData.cat25Responsive())
      bussinesRuleType.assign("NET RESPONSIVE C25");
    else
      bussinesRuleType.assign("NET LEVEL");
    break;

  case RuleConst::NET_SUBMIT_FARE:
  case RuleConst::SELLING_FARE:
    bussinesRuleType.assign("REDISTRIBUTION");
    break;
  default:
    return;
  }

  std::ostringstream responseBase;
  responseBase << INDENTION << pcc << SPACE << "FARE RETAILER" << SPACE << bussinesRuleType << SPACE
               << "BUSINESS RULE";
  std::ostringstream responseRule;
  responseRule << "ID" << SPACE << fareRetailerRuleId;

  _trx.response() << wrapRuleId(responseBase.str(), responseRule.str()) << std::endl;
}

void
RRSection::displayNetBussinesID(const uint64_t fareRetailerRuleId, const PseudoCityCode& pcc,
    const Indicator fdCat35Type)
{
  std::ostringstream responseBase;
  responseBase << INDENTION << pcc << SPACE;
  if(fdCat35Type == RuleConst::REDISTRIBUTED_FARE)
    responseBase << "FARE RETAILER REDISTRIBUTION BUSINESS RULE";
  else
    responseBase << "FARE RETAILER NET LEVEL BUSINESS RULE";

  std::ostringstream responseRule;
  responseRule << "ID" << SPACE << fareRetailerRuleId;
  _trx.response() << wrapRuleId(responseBase.str(), responseRule.str()) << std::endl;
}

void
RRSection::displaySellingFareBussinesID(const uint64_t fareRetailerRuleId, const PseudoCityCode& pcc)
{
  std::ostringstream responseBase;
  responseBase << INDENTION << pcc << SPACE << "FARE RETAILER SELLING LEVEL BUSINESS RULE";
  std::ostringstream responseRule;
  responseRule << "ID" << SPACE << fareRetailerRuleId;
  _trx.response() << wrapRuleId(responseBase.str(), responseRule.str()) << std::endl;
}

void
RRSection::addWrappedTextToResponse(const std::string& msg)
{
  std::vector<std::string> lines;
  TseUtil::splitTextLine(msg, lines, LINE_SIZE, true);

  for (std::string& line : lines)
    _trx.response() << INDENTION << line << std::endl;
}

void
RRSection::displayNETFareCreation(const NegPaxTypeFareRuleData& negRuleData)
{
  std::string percent;
  const Percent negAbsPercent = std::fabs(negRuleData.percent() - 100.0);
  const bool isPositive = (negRuleData.percent() >= 100.0 - EPSILON);

  if (negAbsPercent > EPSILON || fabs(negRuleData.ruleAmt().value()) < EPSILON)
    formatAmountText(percent, negAbsPercent, negRuleData.noDecPercent());

  std::string fareAmt;
  if ((negRuleData.ruleAmt().value()) > EPSILON)
    formatAmountText(fareAmt, negRuleData.ruleAmt().value(), negRuleData.noDecAmt());

  FareCreation fareCreation(
      negRuleData.calcInd(), percent, fareAmt, negRuleData.ruleAmt().code(), isPositive);

  addWrappedTextToResponse(fareCreation.getFareCreation(negRuleData.cat25Responsive()));
}

void
RRSection::netFareCreation(const CurrencyCode& currCode, NegPaxTypeFareRuleData& negRuleData)
{
  std::string responsiveInd;
  if (negRuleData.cat25Responsive())
    responsiveInd.assign("RESPONSIVE CAT25/35 ");

  _trx.response() << INDENTION << responsiveInd << SELLING_LEVEL_CALCULATION << std::endl;

  if (negRuleData.ruleAmt().code().empty())
    negRuleData.ruleAmt() = currCode;

  displayNETFareCreation(negRuleData);
}

void
RRSection::formatAmountText(std::string& percentStr, const Percent percent,
    const int getNoDecPercent) const
{
  constexpr bool removePeriodWhenInt = true;

  if(fabs(percent) > EPSILON)
  {
    CurrencyUtil::formatDouble(
        percent + EPSILON, percentStr, getNoDecPercent, removePeriodWhenInt);
  }
  else
    CurrencyUtil::formatDouble(0.0, percentStr, getNoDecPercent, removePeriodWhenInt);
}

void
RRSection::displayAdjustedSellingLvl(const AdjustedSellingCalcData& adjSellCalcData)
{
  _trx.response() << INDENTION << ASL_ADJUSTED_SELLING_LEVEL_CALCULATION << std::endl;

  std::string percent;
  const Percent adjAbsPercent = std::fabs(adjSellCalcData.getPercent() - 100.0);
  const bool isPositive = (adjSellCalcData.getPercent() >= 100.0 - EPSILON);

  if (adjAbsPercent > EPSILON || fabs(adjSellCalcData.getRuleAmount()) < EPSILON)
    formatAmountText(percent, adjAbsPercent, adjSellCalcData.getNoDecPercent());

  std::string fareAmt;
  if (!adjSellCalcData.getCalculatedASLCurrency().empty())
    formatAmountText(fareAmt, adjSellCalcData.getRuleAmount(), adjSellCalcData.getNoDecAmt());

  FareCreation fareCreation(adjSellCalcData.getCalcInd(),
                            percent,
                            fareAmt,
                            adjSellCalcData.getCalculatedASLCurrency(),
                            isPositive);

  addWrappedTextToResponse(fareCreation.getFareCreation());
}

const char* RRSection::FareCreation::PLUS_TEXT = "PLUS";
const char* RRSection::FareCreation::MINUS_TEXT = "MINUS";

const char* RRSection::FareCreation::BASE_FARE = "BASE FARE";
const char* RRSection::FareCreation::BASE_FARE_CAT25 = "BASE FARE USED TO CREATE THE CAT 25";
const char* RRSection::FareCreation::PERCENT  = "PERCENT";
const char* RRSection::FareCreation::EQUALS  = "EQUALS";

RRSection::FareCreation::FareCreation(const Indicator fareInd,
                                      const std::string& percent,
                                      const std::string& fareAmt,
                                      const CurrencyCode& currencyCode,
                                      bool isPositive)
  : _fareInd(fareInd), _percent(percent), _fareAmt(fareAmt), _currencyCode(currencyCode)
{
  if (isPositive)
    _baseFare += FareCreation::PLUS_TEXT;
  else
    _baseFare += FareCreation::MINUS_TEXT;

  size_t pos = _percent.find(PERIOD);
  try
  {
    if(pos != std::string::npos
        && !boost::lexical_cast<int>(_percent.substr(pos + 1, _percent.length())))
      _percent = _percent.substr(0, pos);
  } catch(...)
  {}

  pos = _fareAmt.find(PERIOD);
  try
  {
    if(pos != std::string::npos
        && !boost::lexical_cast<int>(_fareAmt.substr(pos + 1, _fareAmt.length())))
      _fareAmt = _fareAmt.substr(0, pos);
  } catch(...)
  {}
}

std::string
RRSection::FareCreation::getFareCreation(bool isCat25) const
{
  if(_percent.empty() && _fareAmt.empty())
    return "";

  std::ostringstream fareCreation;

  const auto& base = isCat25 ? BASE_FARE_CAT25 : BASE_FARE;

  if(_fareInd == RuleConst::NF_SPECIFIED && !_fareAmt.empty() && !_currencyCode.empty())
  {
    fareCreation << base << SPACE << EQUALS << SPACE << _currencyCode << _fareAmt << PERIOD;
    return fareCreation.str();
  }

  if(!_percent.empty())
    fareCreation << base << SPACE << _baseFare << SPACE << _percent << SPACE << PERCENT;

  if (!_fareAmt.empty() && !_currencyCode.empty())
  {
    if(_percent.empty())
      fareCreation << base;

    if (_fareInd == RuleConst::NF_ADD)
      fareCreation << SPACE << PLUS_TEXT;
    else if (_fareInd == RuleConst::NF_MINUS)
      fareCreation << SPACE << MINUS_TEXT;
    else
      fareCreation << SPACE << PLUS_TEXT;

    fareCreation << SPACE << _currencyCode << _fareAmt;
  }

  fareCreation << PERIOD;
  return fareCreation.str();
}
} // tse namespace
