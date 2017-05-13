// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Xform/XformUtil.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DBAccess/Currency.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/TaxNation.h"
#include "FareCalc/FareCalculation.h"
#include "Xform/PricingResponseSTLTags.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/STLMessage.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace tse
{

namespace
{
  class EmptyLinePredicate : public std::unary_function<const std::string&, bool>
  {
  public:
    EmptyLinePredicate(){}
    bool operator()(const std::string& element) const
    {
      return (element.empty() || element.find_first_not_of(' ') == std::string::npos);
    }
  };

  class ToLongLinePredicate : public std::unary_function<const std::string&, bool>
  {
  public:
    ToLongLinePredicate(uint32_t maxLineLen) : _maxLineLen(maxLineLen){}
    bool operator()(const std::string& element) const
    {
      return (element.length() > _maxLineLen);
    }
  private:
    uint32_t _maxLineLen;
  };

  class TrimToLeft : public std::unary_function<const std::string&, std::string>
  {
  public:
    TrimToLeft(){}
    std::string operator()(const std::string& element) const
    {
      return boost::trim_left_copy(element);
    }
  };

}

void
XformMsgFormatter::splitNewLine(const std::string& text, std::vector<std::string>& output, bool trimToLeft)
{
  typedef boost::tokenizer<boost::char_separator<char> > NewLineTokenizer;
  boost::char_separator<char> newLineSep("\n");
  NewLineTokenizer newLineTokens(text, newLineSep);

  for (const std::string& newLineToken : newLineTokens)
  {
    if (trimToLeft && newLineToken.find_first_not_of(' ') != std::string::npos)
      output.push_back(boost::trim_left_copy(newLineToken));
    else
      output.push_back(newLineToken);
  }
}

void
XformMsgFormatter::splitLineLen(const uint32_t maxLineLen, std::vector<std::string>& output)
{
  uint32_t offsets[1];
  offsets[0] = maxLineLen;
  boost::offset_separator lineLenSeparator(offsets, offsets + 1);

  std::vector<std::string> splitLines;
  for (const std::string& line : output)
  {
    if (line.length() > maxLineLen)
    {
      boost::tokenizer<boost::offset_separator> lineLenTokens(line, lineLenSeparator);
      std::transform(lineLenTokens.begin(), lineLenTokens.end(), back_inserter(splitLines), TrimToLeft());
    }
    else
      splitLines.push_back(line);
  }
  std::swap(output, splitLines);
}

void
XformMsgFormatter::splitBaggageText(const std::string& text, const uint32_t maxLineLen, std::vector<std::string>& output,
    bool notRemoveEmptyLine)
{
  splitNewLine(text, output, true);
  splitLineLen(maxLineLen, output);

  if(!notRemoveEmptyLine)
    output.erase(std::remove_if(output.begin(), output.end(), EmptyLinePredicate()), output.end());
}

void
XformMsgFormatter::splitGreenScreeText(const std::string& text, const uint32_t maxLineLen, std::vector<std::string>& output)
{
  splitNewLine(text, output, false);
  output.erase(std::remove_if(output.begin(), output.end(), ToLongLinePredicate(maxLineLen)), output.end());
}

void
XformMsgFormatter::formatFc(const std::string& tagHead, int tagSize, const std::string& replaceString, std::string& text)
{
  std::string::size_type position = 0;
  position = text.find(tagHead);

  if (position != std::string::npos && (position + tagSize <= text.size()))
  {
    text.erase(position, tagSize);
    if (!replaceString.empty())
      text.insert(position, replaceString);
  }

  position = 0;
  while ((position = text.find('\n', position)) != std::string::npos)
    text.replace(position, 1, "\\n");
}

BaggageResponseBuilder::BaggageResponseBuilder(PricingTrx& trx,
                                               const BaggageTagMap& baggageTagMap,
                                               const std::string& responseString)
  : _baggageTagMap(&baggageTagMap)
{
  initialize(trx);
  _lastBaggageId = findLastBaggageId(responseString);
}

BaggageResponseBuilder::BaggageResponseBuilder(PricingTrx& trx, const CalcTotals& calcTotals)
{
  initialize(trx);
  setCalcTotals(calcTotals);
}

BaggageResponseBuilder::BaggageResponseBuilder(PricingTrx& trx,
                                               const FarePath& fp,
                                               const PaxTypeCode ptc)
  : _farePath(&fp), _paxType(ptc)
{
  initialize(trx);
}

void
BaggageResponseBuilder::initialize(PricingTrx& trx)
{
  const FareCalcConfig& fcConfig = *FareCalcUtil::getFareCalcConfig(trx);
  _warnMessages = (fcConfig.warningMessages() == FareCalcConsts::FC_YES);
  _truePsgrTypeInd = (fcConfig.truePsgrTypeInd() == FareCalcConsts::FC_YES);
  _lineLength = _warnMessages ? (FareCalculation::WINDOW_WIDTH - 5) : FareCalculation::WINDOW_WIDTH;
  _isUsDot = !trx.itin().empty() && trx.itin().front()->getBaggageTripType().isUsDot();
  _useShortFooterMsg = trx.getRequest()->isSpecificAgencyText();
}

void
BaggageResponseBuilder::setCalcTotals(const CalcTotals& calcTotals)
{
  _farePath = calcTotals.farePath;
  _paxType = _truePsgrTypeInd ? calcTotals.truePaxType : calcTotals.requestedPaxType;
}

void
BaggageResponseBuilder::setFarePath(const FarePath& fp, const PaxTypeCode paxType)
{
  _farePath = &fp;
  _paxType = paxType;
}

bool
BaggageResponseBuilder::getPqResponse(std::vector<std::string>& output, const Trx* trx) const
{
  return getPqResponseImpl(output, trx);
}

bool
BaggageResponseBuilder::getWpaGsResponse(std::string& output) const
{
  if (!_farePath)
    return false;

  std::vector<std::string> lines;
  getGsResponseImpl(lines, true);

  for (const std::string& line : lines)
  {
    output += line;
    output += "\\n";
  }

  return true;
}

bool
BaggageResponseBuilder::getGsResponse(std::vector<std::string>& output) const
{
  if (!_farePath)
    return false;

  if (!_isUsDot)
    output.push_back(formatPassengerInfo());

  getGsResponseImpl(output, false);
  return true;
}

void
BaggageResponseBuilder::setBaggageTag(const std::string& baggageTag)
{
  if (baggageTag.size() != FreeBaggageUtil::BaggageTagTotalSize)
    return;
  _baggageId = baggageTag;
  _farePath = nullptr;
  _paxType.clear();

  const CalcTotals* tot = findCalcTotals(_baggageId);
  if (tot)
    setCalcTotals(*tot);
}

void
BaggageResponseBuilder::setIsUsDotForMultiTkt(PricingTrx& trx, const Itin* subItin)
{
  MultiTicketUtil::TicketSolution ts = MultiTicketUtil::getTicketSolution(trx);
  if (ts == MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT ||
      ts == MultiTicketUtil::SINGLETKT_NOT_APPLICABLE)
  {
    _isUsDot = subItin && subItin->getBaggageTripType().isUsDot();
  }
}

bool
BaggageResponseBuilder::getPqResponseImpl(std::vector<std::string>& output, const Trx* trx) const
{
  if (!_farePath)
    return false;

  XformMsgFormatter::splitBaggageText(_farePath->baggageResponse(), _lineLength, output);
  XformMsgFormatter::splitBaggageText(_farePath->baggageEmbargoesResponse(), _lineLength, output);


  for (std::vector<std::string>::iterator it = output.begin(); it != output.end(); ++it)
  {
    if (*it != "<ADD>")
      continue;

    if (_useShortFooterMsg)
      *it = "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY";
    else
    {
      *it = "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY DEPENDING ON";
      it = output.insert(++it, "FLYER-SPECIFIC FACTORS /E.G. FREQUENT FLYER STATUS/MILITARY/");
      it = output.insert(++it, "CREDIT CARD FORM OF PAYMENT/EARLY PURCHASE OVER INTERNET,ETC./");
    }
  }
  return true;
}

void
BaggageResponseBuilder::getGsResponseImpl(std::vector<std::string>& output,
                                          bool addEmbargoes) const
{
  XformMsgFormatter::splitBaggageText(_farePath->baggageResponse(), _lineLength, output, true);

  if (addEmbargoes || _lastBaggageId == _baggageId)
    XformMsgFormatter::splitBaggageText(
        _farePath->baggageEmbargoesResponse(), _lineLength, output, true);

  for (std::string& line : output)
  {
    if (line == "<ADD>")
      line = _warnMessages ? "ATTN*ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY"
                           : "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY";
    else
    {
      if (_warnMessages && line.find_first_not_of(' ') != std::string::npos)
        line = "ATTN*" + line;
    }
  }
}

const CalcTotals*
BaggageResponseBuilder::findCalcTotals(const BaggageFcTagType& baggageId) const
{
  if (!_baggageTagMap || _baggageTagMap->empty())
    return nullptr;

  const auto it = _baggageTagMap->find(baggageId);
  return (it != _baggageTagMap->end()) ? it->second : nullptr;
}

BaggageFcTagType
BaggageResponseBuilder::findLastBaggageId(const std::string& responseString) const
{
  const size_t pos = responseString.rfind(FreeBaggageUtil::BaggageTagHead);
  if (pos == std::string::npos)
    return BaggageFcTagType();
  if (pos + FreeBaggageUtil::BaggageTagTotalSize > responseString.size())
    return BaggageFcTagType();
  return responseString.substr(pos, FreeBaggageUtil::BaggageTagTotalSize);
}

std::string
BaggageResponseBuilder::formatPassengerInfo() const
{
  std::stringstream oss;
  oss << _paxType << '-';
  oss << std::setw(2) << std::setfill('0') << _farePath->paxType()->number();
  return oss.str();
}

// Use in formatting response
void
TicketingCxrResponseUtil::prepareResponseText(const std::string& responseString,
                                                      XMLConstruct& construct,
                                                      uint16_t& codeNum,
                                                      const bool noSizeLImit)
{
  std::string tmpResponse = responseString;

  size_t lastPos = 0;
  uint16_t recNum = codeNum;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  while (1)
  {
    lastPos = tmpResponse.rfind("\n");
    if (lastPos != std::string::npos && lastPos > 0 &&
        lastPos == (tmpResponse.length() - 1)) // lint !e530
      tmpResponse.replace(lastPos, 1, "\0");
    else
      break;
  }
  char* pHolder = nullptr;
  int tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
      token = strtok_r(nullptr, "\n", &pHolder))
  {
    tokenLen = strlen(token);

    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE && !noSizeLImit)
    {
      continue;
    }
    prepareMessage(construct, STL::Diagnostic, recNum++, token);
  }
  codeNum = recNum;
}

void
TicketingCxrResponseUtil::prepareMessage(XMLConstruct& construct,
                                                 const std::string& msgType,
                                                 const uint16_t msgCode,
                                                 const std::string& msgText)
{
  construct.openElement(STL::MessageInformation);
  construct.addAttribute(STL::MessageType, msgType);
  construct.addAttributeShort(STL::MessageCode, msgCode);
  construct.addElementData(msgText.c_str());
  construct.closeElement();
}

void
OBFeeUtil::getNumberOfOBFees(PricingTrx& trx,
                             const FarePath& farePath,
                             size_t& maxNumFType,
                             size_t& maxNumTType,
                             size_t& maxNumRType)
{
  size_t minNumFType = ((!trx.getRequest()->formOfPayment().empty()) &&
                        (trx.getRequest()->isCollectOBFee())) ?
                          farePath.collectedTktOBFees().size() : 2;

  maxNumTType = std::min(MAX_NUM_OB_FEE - minNumFType, farePath.collectedTTypeOBFee().size());
  maxNumRType = std::min(MAX_NUM_OB_FEE - minNumFType - maxNumTType, farePath.collectedRTypeOBFee().size());
  maxNumFType = MAX_NUM_OB_FEE - maxNumTType - maxNumRType;
}

bool
TaxUtil::getTaxRounding(PricingTrx& pricingTrx,
                        const CurrencyCode& currencyCode,
                        RoundingFactor& roundingFactor,
                        CurrencyNoDec& roundingNoDec,
                        RoundingRule& roundingRule)
{
  const DateTime& tickDate = pricingTrx.ticketingDate();
  const Currency* currency = nullptr;
  currency = pricingTrx.dataHandle().getCurrency( currencyCode );

  if (!currency)
  {
    //LOG4CXX_ERROR(logger, "DBAccess getCurrency returned null currency pointer");
    return false;
  }

  if (currency->taxOverrideRoundingUnit() > 0)
  {
    roundingFactor = currency->taxOverrideRoundingUnit();
    roundingNoDec = currency->taxOverrideRoundingUnitNoDec();
    roundingRule = currency->taxOverrideRoundingRule();
    return true;
  }

  const std::string controllingEntityDesc = currency->controllingEntityDesc();
  //LOG4CXX_INFO(logger, "Currency country description: " << currency->controllingEntityDesc());

  bool foundNationalCurrency = false;
  bool foundNation = false;
  NationCode nationWithMatchingNationalCurrency;
  NationCode nationCode;

  CurrencyUtil::getControllingNationCode(pricingTrx,
                                         controllingEntityDesc,
                                         nationCode,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         tickDate,
                                         currencyCode);

  if (foundNation)
  {
    const TaxNation* taxNation = pricingTrx.dataHandle().getTaxNation(nationCode, tickDate);
    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();
      return true;
    }
  }
  else if (foundNationalCurrency)
  {
    const TaxNation* taxNation =
        pricingTrx.dataHandle().getTaxNation(nationWithMatchingNationalCurrency, tickDate);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();
      return true;
    }
  }
  return false;
};

void
TaxUtil::convertTaxCurrency(PricingTrx& pricingTrx,
                            const Money& sourceMoney,
                            Money& targetMoney)
{
  if (sourceMoney.code() != targetMoney.code())
  {
    CurrencyConversionFacade converter;
    converter.convert(targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES);
    roundTaxCurrency(pricingTrx, targetMoney);
  }
  else
  {
    targetMoney.value() = sourceMoney.value();
  }
}

void
TaxUtil::roundTaxCurrency(PricingTrx& pricingTrx, Money& targetMoney)
{
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;

  if (getTaxRounding(pricingTrx, targetMoney.code(), roundingFactor, roundingNoDec, roundingRule))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoney, roundingFactor, roundingRule);
  }
}

std::string
SplitFormatterUtil::formatAmount(const MoneyAmount amount, const CurrencyNoDec decimalPlaces)
{
  std::ostringstream str;
  str.setf(std::ios::fixed, std::ios::floatfield);
  str.precision(decimalPlaces);
  str << amount;
  return str.str();
}

std::string
SplitFormatterUtil::formatAmount(const std::pair<MoneyAmount, CurrencyNoDec>& amountAndPrecision)
{
  return formatAmount(amountAndPrecision.first, amountAndPrecision.second);
}

void
MAFUtil::constructElementONV(XMLConstruct& xml)
{
  xml.openElement(xml2::DataView);
  xml.addAttribute(xml2::AdjustedSellingLevel, "T");
  xml.closeElement();
}

void
MAFUtil::checkElementONV(PricingTrx& pricingTrx,
                         FareCalcCollector& fareCalcCollector,
                         XMLConstruct& xml)
{
  if (pricingTrx.getOptions()->isPDOForFRRule())
    return;

  for (const CalcTotals* calc : fareCalcCollector.passengerCalcTotals())
  {
    if (calc->adjustedCalcTotal != nullptr)
    {
      constructElementONV(xml);
      return;
    }
  }
}

void
MAFUtil::checkElementONV(PricingTrx& pricingTrx, const CalcTotals* calc, XMLConstruct& xml)
{
  if (pricingTrx.getOptions()->isPDOForFRRule())
    return;

  if (calc->adjustedCalcTotal != nullptr)
    constructElementONV(xml);
}


} // namespace tse
